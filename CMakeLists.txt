# Modified by NZBGet Lite contributors, 2026-08-28; see MODIFICATIONS.md.
cmake_minimum_required(VERSION 3.13)

if(POLICY CMP0167)
	cmake_policy(SET CMP0167 NEW)
endif()

if (PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
	message(FATAL_ERROR "In-source builds are not allowed. You should create separate directory for build files.")
endif()

set(VERSION "27.0")
set(PACKAGE "nzbget")
set(LIBS "")
set(INCLUDES ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

set(PACKAGE_BUGREPORT "https://github.com/mergemarauder/nzbget-lite/issues")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

string(REGEX MATCH "^([0-9]+)\\.([0-9]+)" VERSION_MATCH ${VERSION})
set(VERSION_MAJOR ${CMAKE_MATCH_1})
set(VERSION_MINOR ${CMAKE_MATCH_2})

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

option(BUILD_ONLY_TESTS "Build only tests (for CI)")
set(USE_SANITIZERS "" CACHE STRING
    "Comma-separated sanitizers: address,undefined,thread,memory,leak (MSVC supports only address)")

project(
	nzbget
	VERSION ${VERSION}
	DESCRIPTION "NZBGet is a binary downloader, which downloads files from Usenet"
	LANGUAGES C CXX
)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
	message(FATAL_ERROR "nzbget-lite supports Linux only")
endif()

set(EXTERNAL_DEPS "")
include(cmake/common.cmake)
include(daemon/sources.cmake)

include(cmake/posix.cmake)

configure_file(
	${CMAKE_SOURCE_DIR}/cmake/config.h.in
	${CMAKE_BINARY_DIR}/config.h
)

if(NOT BUILD_ONLY_TESTS)
	add_executable(${PACKAGE} ${CMAKE_SOURCE_DIR}/daemon/main/nzbget.cpp ${SRC})

	if(EXTERNAL_DEPS)
		add_dependencies(${PACKAGE} ${EXTERNAL_DEPS})
	endif()

	target_precompile_headers(${PACKAGE} PRIVATE ${CMAKE_SOURCE_DIR}/daemon/main/nzbget.h)
	target_link_libraries(${PACKAGE} PRIVATE ${LIBS})
	target_include_directories(${PACKAGE} PRIVATE ${INCLUDES})

	apply_sanitizers(${PACKAGE})

	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		add_custom_command(
			TARGET ${PACKAGE} POST_BUILD
				COMMAND ${CMAKE_STRIP}
				ARGS ${PACKAGE}
		)
	endif()
endif()

if(NOT BUILD_ONLY_TESTS)
	include(${CMAKE_SOURCE_DIR}/cmake/install.cmake)
endif()

if(ENABLE_TESTS OR BUILD_ONLY_TESTS)
	include(CTest)
	add_subdirectory(tests)
endif()
