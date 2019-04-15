cmake_minimum_required(VERSION 3.8.2)

project(mf VERSION 0.0.1)
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

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/hopscotch-map)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/mf/include/)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/hopscotch-map/include/)

add_executable(mf mf/src/main.cpp
                  mf/src/SourceRange.cpp
                  mf/src/tokenizer.cpp
                  mf/src/parser.cpp
                  mf/src/ast.cpp
                  mf/src/util.cpp)


