# CMakeList.txt : CMake project for JoyFS, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.15)

project(JoyFS)

# Add source to this project's executable.
add_executable (${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}  
	PRIVATE 
		FSUIPC
		CONAN_PKG::boost 
		CONAN_PKG::spdlog 
		CONAN_PKG::sdl)

target_sources(${PROJECT_NAME} PRIVATE 
	"src/Sim.cpp"
	"src/Sim.h"
	"src/Main.cpp"
	"src/ReadSettings.cpp"
	"src/ReadSettings.h")


set (settingsfile_source "${CMAKE_CURRENT_SOURCE_DIR}/Resources/")


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${settingsfile_source} $<TARGET_FILE_DIR:${PROJECT_NAME}>)

