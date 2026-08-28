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

#ifndef CPU_H
#define CPU_H

#include <string>
#include <optional>

namespace System
{
	class CPU final
	{
	public:
		CPU();
		const std::string& GetModel() const;
		const std::string& GetArch() const;

	private:
		void Init();
		std::optional<std::string> GetCPUArch() const;
		std::optional<std::string> GetCPUModel() const;
		std::string GetCanonicalCPUArch(const std::string& arch) const;

		std::optional<std::string> GetCPUModelFromCPUInfo() const;
		std::optional<std::string> GetCPUModelFromLSCPU() const;

		std::string m_model;
		std::string m_arch;
	};
}

#endif
