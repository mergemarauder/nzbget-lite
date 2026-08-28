# Modified by NZBGet Lite contributors, 2026-08-28; see MODIFICATIONS.md.
if (CMAKE_SYSTEM_PROCESSOR MATCHES "i386|i686|x86|x86_64|x64|amd64|AMD64")
	set(IS_X86 TRUE)
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|x64|amd64|AMD64")
		set(IS_X64 TRUE)
	endif()
endif()
if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm|ARM|aarch64|arm64|ARM64|armeb|aarch64be|aarch64_be")
	set(IS_ARM TRUE)
endif()
if (CMAKE_SYSTEM_PROCESSOR MATCHES "riscv64|rv64")
	set(IS_RISCV64 TRUE)
endif()
if (CMAKE_SYSTEM_PROCESSOR MATCHES "riscv32|rv32")
	set(IS_RISCV32 TRUE)
endif()

include(ExternalProject)
include(CheckCXXCompilerFlag)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckTypeSize)
include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)
include(CheckLibraryExists)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(DEBUG 1)

	if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		add_compile_options(-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic)
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		add_compile_options(-Wall -Wextra)
	endif()

elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
	add_compile_options(-fno-rtti -ffunction-sections -fdata-sections -Wno-unused-function)
	add_link_options(-Wl,--gc-sections)

	check_cxx_compiler_flag("-fstack-protector-strong" HAVE_STACK_PROTECT)
	if(HAVE_STACK_PROTECT)
		add_compile_options(-fstack-protector-strong)
	endif()
endif()

function(apply_sanitizers target)
	if(NOT USE_SANITIZERS)
		return()
	endif()

	target_compile_options(${target} PRIVATE
		-fsanitize=${USE_SANITIZERS}
		-fno-omit-frame-pointer
		-fno-sanitize-recover=all
	)
	target_link_options(${target} PRIVATE
		-fsanitize=${USE_SANITIZERS}
	)
endfunction()
