cmake_minimum_required(VERSION 3.20)
project(engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(engine
	S_Main.cpp
	S_Memory.cpp
	Com_Console.cpp
	Cmd_System.cpp
	Shared.cpp
	Common.cpp
	CVar_System.cpp
	S_System.cpp
	Math.cpp
)

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
