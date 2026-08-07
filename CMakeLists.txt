cmake_minimum_required(VERSION 3.20)
project(SEGO LANGUAGES CXX)
find_package(OpenAL CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(engine
	src/core/Com_Console.cpp
	src/core/Cmd_System.cpp
	src/core/CVar_System.cpp
	src/core/Common.cpp
	src/core/Shared.cpp
	src/core/Math.cpp

	src/platform/S_Main.cpp
	src/platform/S_System.cpp
	src/platform/S_Memory.cpp

	src/filesystem/FS_Main.cpp

	src/audio/Snd_Main.cpp
)

target_include_directories(engine PRIVATE
	${CMAKE_SOURCE_DIR}/src/core
	${CMAKE_SOURCE_DIR}/api
)

target_link_libraries(engine PRIVATE OpenAL::OpenAL)

if(WIN32)
	target_compile_definitions(engine PRIVATE
		WIN32_LEAN_AND_MEAN
		NOMINMAX
	)
endif()

if(MSVC)
	target_compile_options(engine PRIVATE /W4)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	target_compile_options(engine PRIVATE -Wall -Wextra)
endif()