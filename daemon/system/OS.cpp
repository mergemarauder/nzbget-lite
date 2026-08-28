// Modified by NZBGet Lite contributors, 2026-08-28; see MODIFICATIONS.md.
/*
 *  This file is part of nzbget. See <https://nzbget.com>.
 *
 *  Copyright (C) 2024 Denis <denis@nzbget.com>
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

#include "OS.h"
#include "Util.h"
#include "Log.h"
#include "FileSystem.h"

namespace System
{
	static const int BUFFER_SIZE = 128;

	OS::OS()
	{
		Init();
	}

	const std::string& OS::GetName() const
	{
		return m_name;
	}

	const std::string& OS::GetVersion() const
	{
		return m_version;
	}


#ifdef __linux__
#include <fstream>
	bool OS::IsRunningInDocker() const
	{
		return FileSystem::FileExists("/.dockerenv");
	}

	void OS::TrimQuotes(std::string& str) const
	{
		if (str.front() == '"')
		{
			str = str.substr(1);
		}

		if (str.back() == '"')
		{
			str = str.substr(0, str.size() - 1);
		}
	}

	void OS::Init()
	{
		InitOSInfoFromOSRelease();

		if (m_name.empty())
		{
			auto res = Util::Uname("-o");
			if (res.has_value())
			{
				m_name = std::move(res.value());
			}
			else
			{
				detail("Failed to get OS name. Couldn't read 'uname -o'");
			}
		}

		if (m_version.empty())
		{
			auto res = Util::Uname("-r");
			if (res.has_value())
			{
				m_version = std::move(res.value());
			}
			else
			{
				detail("Failed to get OS release. Couldn't read 'uname -r'");
			}
		}

		if (IsRunningInDocker())
		{
			m_version += " (Running in Docker)";
		}
	}

	void OS::InitOSInfoFromOSRelease()
	{
		std::ifstream osInfo("/etc/os-release");
		if (!osInfo.is_open())
		{
			return;
		}

		std::string line;
		while (std::getline(osInfo, line))
		{
			if (!m_name.empty() && !m_version.empty())
			{
				return;
			}

			// e.g NAME="Debian GNU/Linux"
			if (m_name.empty() && line.find("NAME=") == 0)
			{
				m_name = line.substr(line.find("=") + 1);

				Util::Trim(m_name);
				TrimQuotes(m_name);

				continue;
			}

			// e.g VERSION_ID="12"
			if (m_version.empty() && line.find("VERSION_ID=") == 0)
			{
				m_version = line.substr(line.find("=") + 1);

				Util::Trim(m_version);
				TrimQuotes(m_version);

				continue;
			}

			// e.g. BUILD_ID=rolling
			if (m_version.empty() && line.find("BUILD_ID=") == 0)
			{
				m_version = line.substr(line.find("=") + 1);
				Util::Trim(m_version);

				continue;
			}
		}
	}
#endif



}
