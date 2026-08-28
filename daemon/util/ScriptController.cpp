/*
 *  This file is part of nzbget. See <https://nzbget.com>.
 *
 *  Copyright (C) 2007-2017 Andrey Prygunkov <hugbug@users.sourceforge.net>
 *  Copyright (C) 2025 Denis <denis@nzbget.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "nzbget.h"
#include "ScriptController.h"
#include "Log.h"
#include "Util.h"
#include "FileSystem.h"
#include "Options.h"

// System global variable holding environments variables
extern char** environ;
extern char* (*g_EnvironmentVariables)[];

ScriptController::RunningScripts ScriptController::m_runningScripts;
Mutex ScriptController::m_runningMutex;

const int FORK_ERROR_EXIT_CODE = 254;

#ifdef CHILD_WATCHDOG
/**
 * Sometimes the forked child process doesn't start properly and hangs
 * just during the starting. I didn't find any explanation about what
 * could cause that problem except of a general advice, that
 * "a forking in a multithread application is not recommended".
 *
 * Workaround:
 * 1) child process prints a line into stdout directly after the start;
 * 2) parent process waits for a line for 60 seconds. If it didn't receive it
 *    the child process is assumed to be hanging and will be killed. Another attempt
 *    will be made.
 */

class ChildWatchDog : public Thread
{
public:
	void SetProcessId(pid_t processId) { m_processId = processId; }
	void SetInfoName(const char* infoName) { m_infoName = infoName; }
protected:
	virtual void Run();
private:
	pid_t m_processId;
	CString m_infoName;
};

void ChildWatchDog::Run()
{
	static const int WAIT_SECONDS = 60;
	time_t start = Util::CurrentTime();
	while (!IsStopped() && (Util::CurrentTime() - start) < WAIT_SECONDS)
	{
		Util::Sleep(10);
	}

	if (!IsStopped())
	{
		info("Restarting hanging child process for %s", *m_infoName);
		kill(m_processId, SIGKILL);
	}
}
#endif


void EnvironmentStrings::Clear()
{
	m_strings.clear();
}

void EnvironmentStrings::InitFromCurrentProcess()
{
	if (!g_EnvironmentVariables)
		return;

	for (int i = 0; (*g_EnvironmentVariables)[i]; i++)
	{
		char* var = (*g_EnvironmentVariables)[i];
		// Ignore all env vars set by NZBGet.
		// This is to avoid the passing of env vars after program update (when NZBGet is
		// started from a script which was started by a previous instance of NZBGet).
		// Format: NZBXX_YYYY (XX are any two characters, YYYY are any number of any characters).
		if (!(!strncmp(var, "NZB", 3) && strlen(var) > 5 && var[5] == '_'))
		{
			Append(var);
		}
	}
}

void EnvironmentStrings::Append(const char* envstr)
{
	m_strings.emplace_back(envstr);
}

void EnvironmentStrings::Append(CString&& envstr)
{
	m_strings.push_back(std::move(envstr));
}


/*
 * Returns environment block in format suitable for using with execve.
 */
std::vector<char*> EnvironmentStrings::GetStrings()
{
	std::vector<char*> strings;
	strings.reserve(m_strings.size() + 1);
	std::copy(m_strings.begin(), m_strings.end(), std::back_inserter(strings));
	strings.push_back(nullptr);
	return strings;
}


ScriptController::ScriptController()
{
	ResetEnv();

	Guard guard(m_runningMutex);
	m_runningScripts.push_back(this);
}

ScriptController::~ScriptController()
{
	UnregisterRunningScript();
}

void ScriptController::UnregisterRunningScript()
{
	Guard guard(m_runningMutex);
	m_runningScripts.erase(std::remove(m_runningScripts.begin(), m_runningScripts.end(), this), m_runningScripts.end());
}

void ScriptController::ResetEnv()
{
	m_environmentStrings.Clear();
	m_environmentStrings.InitFromCurrentProcess();
}

void ScriptController::SetEnvVar(const char* name, const char* value)
{
	m_environmentStrings.Append(CString::FormatStr("%s=%s", name, value));
}

void ScriptController::SetIntEnvVar(const char* name, int value)
{
	BString<1024> strValue("%i", value);
	SetEnvVar(name, strValue);
}

void ScriptController::PrepareArgs()
{
	*m_cmdLine = '\0';
	m_cmdArgs.clear();

	if (m_args.size() == 1)
	{
		const auto found = Util::FindExecutorProgram(m_args[0].Str(), g_Options->GetShellOverride());
		if (!found)
		{
			strncpy(m_cmdLine, m_args[0], sizeof(m_cmdLine) - 1);
		}
		else
		{
			strncpy(m_cmdLine, found.value().c_str(), sizeof(m_cmdLine) - 1);
			m_cmdArgs.emplace_back(found.value().c_str());
		}

		debug("CmdLine: %s", m_cmdLine);
	}
	else
	{
		strncpy(m_cmdLine, m_args[0], sizeof(m_cmdLine) - 1);
	}
}

int ScriptController::Execute()
{
	PrepareArgs();

	m_completed = false;
	int exitCode = 0;

#ifdef CHILD_WATCHDOG
	bool childConfirmed = false;
	while (!childConfirmed && !m_terminated)
	{
#endif

	int pipein = -1, pipeout = -1;
	StartProcess(&pipein, &pipeout);
	if (pipein == -1)
	{
		m_completed = true;
		return -1;
	}

	// open the read end
	m_readpipe = fdopen(pipein, "r");
	if (!m_readpipe)
	{
		PrintMessage(Message::mkError, "Could not open read pipe to %s", *m_infoName);
		close(pipein);
		close(pipeout);
		m_completed = true;
		return -1;
	}

	m_writepipe = 0;
	if (m_needWrite)
	{
		// open the write end
		m_writepipe = fdopen(pipeout, "w");
		if (!m_writepipe)
		{
			PrintMessage(Message::mkError, "Could not open write pipe to %s", *m_infoName);
			fclose(m_readpipe);
			m_readpipe = nullptr;
			close(pipeout);
			m_completed = true;
			return -1;
		}
	}

#ifdef CHILD_WATCHDOG
	debug("Creating child watchdog");
	ChildWatchDog watchDog;
	watchDog.SetAutoDestroy(false);
	watchDog.SetProcessId(m_processId);
	watchDog.SetInfoName(m_infoName);
	watchDog.Start();
#endif

	CharBuffer buf(1024 * 10);

	debug("Entering pipe-loop");
	bool firstLine = true;
	bool startError = false;
	while (!m_terminated && !m_detached && !feof(m_readpipe))
	{
		if (ReadLine(buf, buf.Size(), m_readpipe) && m_readpipe)
		{
#ifdef CHILD_WATCHDOG
			if (!childConfirmed)
			{
				childConfirmed = true;
				watchDog.Stop();
				debug("Child confirmed");
				continue;
			}
#endif
			if (firstLine && !strncmp(buf, "[ERROR] Could not start ", 24))
			{
				startError = true;
			}
			ProcessOutput(buf);
			firstLine = false;
		}
	}
	debug("Exited pipe-loop");

#ifdef CHILD_WATCHDOG
	debug("Destroying WatchDog");
	if (!childConfirmed)
	{
		watchDog.Stop();
	}
	while (watchDog.IsRunning())
	{
		Util::Sleep(5);
	}
#endif

	if (m_readpipe)
	{
		fclose(m_readpipe);
	}

	if (m_writepipe)
	{
		fclose(m_writepipe);
	}

	if (m_terminated && m_infoName)
	{
		warn("Interrupted %s", *m_infoName);
	}

	exitCode = 0;

	if (!m_detached)
	{
		exitCode = WaitProcess();
		if (exitCode == FORK_ERROR_EXIT_CODE && startError)
		{
			exitCode = -1;
		}
	}

#ifdef CHILD_WATCHDOG
	}	// while (!bChildConfirmed && !m_bTerminated)
#endif

	debug("Exit code %i", exitCode);
	m_completed = true;
	return exitCode;
}


/*
* Returns file descriptor of the read-pipe or -1 on error.
*/
void ScriptController::StartProcess(int* pipein, int* pipeout)
{
	CString workingDir = m_workingDir;
	if (workingDir.Empty())
	{
		workingDir = FileSystem::GetCurrentDirectory();
	}

	const char* script = m_args[0];


	int pin[] = {0, 0};
	int pout[] = {0, 0};

	// create the pipes
	if (pipe(pin))
	{
		PrintMessage(Message::mkError, "Could not open read pipe: errno %i", errno);
		return;
	}
	if (m_needWrite && pipe(pout))
	{
		PrintMessage(Message::mkError, "Could not open write pipe: errno %i", errno);
		close(pin[0]);
		close(pin[1]);
		return;
	}

	*pipein = pin[0];
	*pipeout = pout[1];

	std::vector<char*> environmentStrings = m_environmentStrings.GetStrings();
	char** envdata = environmentStrings.data();

	CheckEnvSize(environmentStrings);

	std::copy(std::begin(m_args), std::end(m_args), std::back_inserter(m_cmdArgs));
	m_cmdArgs.emplace_back(nullptr);
	char* const* argdata = (char* const*)m_cmdArgs.data();

#ifdef DEBUG
	debug("Starting  process: %s", script);
	for (const char* arg : m_args)
	{
		debug("arg: %s", arg);
	}
#endif

	debug("forking");
	pid_t pid = fork();

	if (pid == -1)
	{
		PrintMessage(Message::mkError, "Could not start %s: errno %i", *m_infoName, errno);
		close(pin[0]);
		close(pin[1]);
		if (m_needWrite)
		{
			close(pout[0]);
			close(pout[1]);
		}
		return;
	}
	else if (pid == 0)
	{
		// here goes the second instance

		// only certain functions may be used here or the program may hang.
		// for a list of functions see chapter "async-signal-safe functions" in
		// http://man7.org/linux/man-pages/man7/signal.7.html

		// create new process group (see Terminate() where it is used)
		setsid();

		// make the pipeout to be the same as stdout and stderr
		dup2(pin[1], 1);
		dup2(pin[1], 2);
		close(pin[0]);
		close(pin[1]);

		if (m_needWrite)
		{
			// make the pipein to be the same as stdin
			dup2(pout[0], 0);
			close(pout[0]);
			close(pout[1]);
		}

#ifdef CHILD_WATCHDOG
		write(1, "\n", 1);
		fsync(1);
#endif

		chdir(workingDir);
		environ = envdata;

		execvp(m_cmdLine, argdata);

		if (errno == EACCES)
		{
			write(1, "[WARNING] Fixing permissions for", 32);
			write(1, script, strlen(script));
			write(1, "\n", 1);
			fsync(1);
			FileSystem::FixExecPermission(script);
			execvp(m_cmdLine, argdata);
		}

		// NOTE: the text "[ERROR] Could not start " is checked later,
		// by changing adjust the dependent code below.
		write(1, "[ERROR] Could not start ", 24);
		write(1, script, strlen(script));
		write(1, ": ", 2);
		char* errtext = strerror(errno);
		write(1, errtext, strlen(errtext));
		write(1, "\n", 1);
		fsync(1);
		_exit(FORK_ERROR_EXIT_CODE);
	}

	// continue the first instance
	debug("forked");
	debug("Child Process-ID: %i", (int)pid);

	m_processId = pid;

	// close unused pipe ends
	close(pin[1]);
	if (m_needWrite)
	{
		close(pout[0]);
	}
}

int ScriptController::WaitProcess()
{
	int status = 0;
	waitpid(m_processId, &status, 0);
	if (WIFEXITED(status))
	{
		int exitCode = WEXITSTATUS(status);
		return exitCode;
	}
	return 0;
}

void ScriptController::Terminate()
{
	debug("Stopping %s", *m_infoName);
	m_terminated = true;

	pid_t killId = m_processId;
	if (getpgid(killId) == killId)
	{
		// if the child process has its own group (setsid() was successful), kill the whole group
		killId = -killId;
	}
	bool ok = (killId && kill(killId, SIGKILL) == 0) || m_completed;

	if (ok)
	{
		debug("Terminated %s", *m_infoName);
	}
	else
	{
		error("Could not terminate %s", *m_infoName);
	}

	debug("Stopped %s", *m_infoName);
}

void ScriptController::TerminateAll()
{
	Guard guard(m_runningMutex);
	for (ScriptController* script : m_runningScripts)
	{
		if (script->m_processId && !script->m_detached)
		{
			// send break signal and wait up to 5 seconds for graceful termination
			if (script->Break())
			{
				time_t curtime = Util::CurrentTime();
				while (!script->m_completed && std::abs(curtime - Util::CurrentTime()) <= 10)
				{
					Util::Sleep(100);
				}
			}
			script->Terminate();
		}
	}
}

bool ScriptController::Break()
{
	debug("Sending break signal to %s", *m_infoName);

	bool ok = kill(m_processId, SIGINT) == 0;

	if (ok)
	{
		debug("Sent break signal to %s", *m_infoName);
	}
	else
	{
		warn("Could not send break signal to %s", *m_infoName);
	}

	return ok;
}

void ScriptController::Detach()
{
	debug("Detaching %s", *m_infoName);
	m_detached = true;
	FILE* readpipe = m_readpipe;
	m_readpipe = nullptr;
	fclose(readpipe);
}

void ScriptController::Resume()
{
	m_terminated = false;
	m_detached = false;
	m_processId = 0;
}

bool ScriptController::ReadLine(char* buf, int bufSize, FILE* stream)
{
	return fgets(buf, bufSize, stream);
}

void ScriptController::ProcessOutput(char* text)
{
	debug("Processing output received from script");

	for (char* pend = text + strlen(text) - 1; pend >= text && (*pend == '\n' || *pend == '\r' || *pend == ' '); pend--) *pend = '\0';

	if (text[0] == '\0')
	{
		// skip empty lines
		return;
	}

	if (!strncmp(text, "[INFO] ", 7))
	{
		PrintMessage(Message::mkInfo, "%s", text + 7);
	}
	else if (!strncmp(text, "[WARNING] ", 10))
	{
		PrintMessage(Message::mkWarning, "%s", text + 10);
	}
	else if (!strncmp(text, "[ERROR] ", 8))
	{
		PrintMessage(Message::mkError, "%s", text + 8);
	}
	else if (!strncmp(text, "[DETAIL] ", 9))
	{
		PrintMessage(Message::mkDetail, "%s", text + 9);
	}
	else if (!strncmp(text, "[DEBUG] ", 8))
	{
		PrintMessage(Message::mkDebug, "%s", text + 8);
	}
	else
	{
		PrintMessage(Message::mkInfo, "%s", text);
	}

	debug("Processing output received from script - completed");
}

void ScriptController::AddMessage(Message::EKind kind, const char* text)
{
	switch (kind)
	{
		case Message::mkDetail:
			detail("%s", text);
			break;

		case Message::mkInfo:
			info("%s", text);
			break;

		case Message::mkWarning:
			warn("%s", text);
			break;

		case Message::mkError:
			error("%s", text);
			break;

		case Message::mkDebug:
			debug("%s", text);
			break;
	}
}

void ScriptController::PrintMessage(Message::EKind kind, const char* format, ...)
{
	BString<1024> tmp2;

	va_list ap;
	va_start(ap, format);
	tmp2.FormatV(format, ap);
	va_end(ap);

	if (m_logPrefix)
	{
		AddMessage(kind, BString<1024>("%s: %s", m_logPrefix, *tmp2));
	}
	else
	{
		AddMessage(kind, tmp2);
	}
}

void ScriptController::Write(const char* str)
{
	fwrite(str, 1, strlen(str), m_writepipe);
	fflush(m_writepipe);
}

void ScriptController::CheckEnvSize(const std::vector<char*>& envs)
{
	if (ARGS_LIMIT_SIZE <= 0) return;

	size_t limit = static_cast<size_t>(ARGS_LIMIT_SIZE);
	size_t total = 0;
	for (const char* envVar : envs)
	{
		if (!envVar) continue;
		total += strlen(envVar) + 1 + sizeof(void*);
		if (total > limit)
		{
			PrintMessage(Message::EKind::mkWarning,
						 "Total environment size (%zu bytes) exceeds OS limit (%zu bytes): %s",
						 total, limit, "This may cause process creation to fail");
			break;
		}
	}
}
