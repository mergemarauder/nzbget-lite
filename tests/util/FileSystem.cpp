// Modified by NZBGet Lite contributors, 2026-08-28; see MODIFICATIONS.md.
/*
 *  This file is part of nzbget. See <https://nzbget.com>.
 *
 *  Copyright (C) 2016 Andrey Prygunkov <hugbug@users.sourceforge.net>
 *  Copyright (C) 2024-2026 Denis <denis@nzbget.com>
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

#include <boost/test/unit_test.hpp>

#include "FileSystem.h"

BOOST_AUTO_TEST_SUITE(UtilTest)


BOOST_AUTO_TEST_CASE(ExtractFilePathCmdTest)
{
	BOOST_CHECK(FileSystem::ExtractFilePathFromCmd("/usr/nzbget/unrar") == "/usr/nzbget/unrar");
	BOOST_CHECK(FileSystem::ExtractFilePathFromCmd("/usr/nzbget/unrar -ai") == "/usr/nzbget/unrar");
	BOOST_CHECK(FileSystem::ExtractFilePathFromCmd("") == "");
}

BOOST_AUTO_TEST_CASE(EscapePathForShellTest)
{
	BOOST_CHECK(FileSystem::EscapePathForShell("/usr/my dir/nzbget/unrar") == "\"/usr/my dir/nzbget/unrar\"");
	BOOST_CHECK(FileSystem::EscapePathForShell("") == "");
}


BOOST_AUTO_TEST_CASE(SplitPathAndFilenameTest)
{
	{
		std::string fullPath = "/path/to/filename.txt";
		std::pair<std::string, std::string> result = FileSystem::SplitPathAndFilename(fullPath);
		BOOST_TEST(result.first == "/path/to");
		BOOST_TEST(result.second == "filename.txt");
	}

	{
		std::string fullPath = "/path/to/";
		std::pair<std::string, std::string> result = FileSystem::SplitPathAndFilename(fullPath);
		BOOST_TEST(result.first == "/path/to");
		BOOST_TEST(result.second == "");
	}

	{
		std::string fullPath = "C:\\path\\to\\filename.txt";
		std::pair<std::string, std::string> result = FileSystem::SplitPathAndFilename(fullPath);
		BOOST_TEST(result.first == "C:\\path\\to");
		BOOST_TEST(result.second == "filename.txt");
	}

	{
		std::string fullPath = "C:\\path\\to\\";
		std::pair<std::string, std::string> result = FileSystem::SplitPathAndFilename(fullPath);
		BOOST_TEST(result.first == "C:\\path\\to");
		BOOST_TEST(result.second == "");
	}

	{
		std::string fullPath = "/path\\to/filename.txt";
		std::pair<std::string, std::string> result = FileSystem::SplitPathAndFilename(fullPath);
		BOOST_TEST(result.first == "/path\\to");
		BOOST_TEST(result.second == "filename.txt");
	}

	{
		std::string fullPath = "";
		std::pair<std::string, std::string> result = FileSystem::SplitPathAndFilename(fullPath);
		BOOST_TEST(result.first == "");
		BOOST_TEST(result.second == "");
	}

	{
		std::string fullPath = "/filename.txt";
		std::pair<std::string, std::string> result = FileSystem::SplitPathAndFilename(fullPath);
		BOOST_TEST(result.first == "");
		BOOST_TEST(result.second == "filename.txt");
	}

	{
		std::string fullPath = "\\filename.txt";
		std::pair<std::string, std::string> result = FileSystem::SplitPathAndFilename(fullPath);
		BOOST_TEST(result.first == "");
		BOOST_TEST(result.second == "filename.txt");
	}

	{
		std::string fullPath = "/path/to\\a/b\\c/filename.txt";
		std::pair<std::string, std::string> result = FileSystem::SplitPathAndFilename(fullPath);
		BOOST_TEST(result.first == "/path/to\\a/b\\c");
		BOOST_TEST(result.second == "filename.txt");
	}

	{
		std::string fullPath = "/";
		std::pair<std::string, std::string> result = FileSystem::SplitPathAndFilename(fullPath);
		BOOST_TEST(result.first == "");
		BOOST_TEST(result.second == "");
	}

	{
		std::string fullPath = "\\";
		std::pair<std::string, std::string> result = FileSystem::SplitPathAndFilename(fullPath);
		BOOST_TEST(result.first == "");
		BOOST_TEST(result.second == "");
	}
}

BOOST_AUTO_TEST_SUITE_END()
