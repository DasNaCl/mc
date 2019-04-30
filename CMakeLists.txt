cmake_minimum_required(VERSION 3.8.2)

project(my VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose type of build. Options are \"Debug\" or \"Release\"." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

macro(find_target TARGET_NAME TARGET_PACKAGE)
	if(TARGET ${TARGET_NAME})
		get_target_property(TGT_VERSION ${TARGET_NAME} VERSION)
		message(STATUS "Found Target: ${TARGET_NAME} (found version \"${TGT_VERSION}\")")
		set(${TARGET_PACKAGE}_FOUND TRUE)
	else()
		find_package(${TARGET_PACKAGE} ${ARGN})
	endif()
endmacro()

add_definitions(-DFMT_HEADER_ONLY)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/hopscotch-map)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/my/include/)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/hopscotch-map/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/spdlog/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/fmt/include/)

add_executable(my my/src/main.cpp
                  my/src/SourceRange.cpp
                  my/src/tokenizer.cpp
                  my/src/parser.cpp
                  my/src/ast.cpp
                  my/src/util.cpp
                  my/src/log.cpp
                  )


