// Modified by NZBGet Lite contributors, 2026-08-28; see MODIFICATIONS.md.
/*
 *  This file is part of nzbget. See <https://nzbget.com>.
 *
 *  Copyright (C) 2007-2016 Andrey Prygunkov <hugbug@users.sourceforge.net>
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "nzbget.h"
#include "Log.h"
#include "Options.h"
#include "StackTrace.h"

extern void ExitProc();


#ifdef DEBUG
typedef void(*sighandler)(int);
std::vector<sighandler> SignalProcList;
#endif

#ifdef HAVE_SYS_PRCTL_H
/**
* activates the creation of core-files
*/
void EnableCoreDump()
{
	rlimit rlim;
	rlim.rlim_cur= RLIM_INFINITY;
	rlim.rlim_max= RLIM_INFINITY;
	setrlimit(RLIMIT_CORE, &rlim);
	prctl(PR_SET_DUMPABLE, 1);
}
#endif

void PrintBacktrace()
{
#ifdef HAVE_BACKTRACE
	printf("Segmentation fault, tracing...\n");

	void *array[100];
	size_t size;
	char **strings;
	size_t i;

	size = backtrace(array, 100);
	strings = backtrace_symbols(array, size);

	// first trace to screen
	printf("Obtained %zu stack frames\n", size);
	for (i = 0; i < size; i++)
	{
		printf("%s\n", strings[i]);
	}

	// then trace to log
	error("Segmentation fault, tracing...");
	error("Obtained %zd stack frames", size);
	for (i = 0; i < size; i++)
	{
		error("Stacktrace: %s", strings[i]);
	}

	free(strings);
#else
	error("Segmentation fault");
#endif
}

/*
 * Signal handler
 */
void SignalProc(int signum)
{
	switch (signum)
	{
		case SIGINT:
			signal(SIGINT, SIG_DFL);   // Reset the signal handler
			ExitProc();
			break;

		case SIGTERM:
			signal(SIGTERM, SIG_DFL);   // Reset the signal handler
			ExitProc();
			break;

		case SIGCHLD:
			// ignoring
			break;

#ifdef DEBUG
		case SIGSEGV:
			signal(SIGSEGV, SIG_DFL);   // Reset the signal handler
			PrintBacktrace();
			break;
#endif
	}
}

void InstallErrorHandler()
{
#ifdef HAVE_SYS_PRCTL_H
	if (g_Options->GetCrashDump())
	{
		EnableCoreDump();
	}
#endif

	signal(SIGINT, SignalProc);
	signal(SIGTERM, SignalProc);
	signal(SIGPIPE, SIG_IGN);
#ifdef DEBUG
	if (g_Options->GetCrashTrace())
	{
		signal(SIGSEGV, SignalProc);
	}
#endif
#ifdef SIGCHLD_HANDLER
	// it could be necessary on some systems to activate a handler for SIGCHLD
	// however it make troubles on other systems and is deactivated by default
	signal(SIGCHLD, SignalProc);
#endif
}


#ifdef DEBUG
class SegFault
{
public:
	void DoSegFault()
	{
		char* N = nullptr;
		// cppcheck-suppress nullPointer
		*N = '\0';
	}
};

void TestSegFault()
{
	SegFault s;
	s.DoSegFault();
}
#endif
