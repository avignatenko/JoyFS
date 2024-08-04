# CMakeList.txt : CMake project for AlfBFF, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.15)

project(AltBFF)

# Add source to this project's executable.
add_executable (${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}  PRIVATE BFFCLAPI Utils FSUIPC CSV CONAN_PKG::spdlog CONAN_PKG::catch2)

target_sources(${PROJECT_NAME} PRIVATE 
	"src/Main.cpp"
	"src/Model.cpp"
	"src/Model.h"
	"src/Sim.cpp"
	"src/Sim.h"
	"src/Tests.cpp" 
	"src/A2ASTec30AP.h"
	"src/A2ASTec30AP.cpp"
	"src/ReadSettings.cpp"
	"src/ReadSettings.h"
	"src/SimModelLoop.cpp"
	"src/SimModelLoop.h")


set (settingsfile_source "${CMAKE_CURRENT_SOURCE_DIR}/resources/")


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${settingsfile_source} $<TARGET_FILE_DIR:${PROJECT_NAME}>)

