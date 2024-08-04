# CMakeList.txt : CMake project for AlfBFF, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.15)

project(FSUIPC)

# Add source to this project's library.
add_library (${PROJECT_NAME} STATIC)

target_include_directories(${PROJECT_NAME} PUBLIC "." )

target_sources(${PROJECT_NAME} 
	PRIVATE 
	IPCuser64.c
	IPCuser64.h
	FSUIPC_User64.h)
