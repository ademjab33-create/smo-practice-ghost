cmake_minimum_required(VERSION 3.21)
project(subsdk)

if (NOT SWITCH)
  message(FATAL_ERROR "Not targeting switch, make sure to specify -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain.cmake")
endif()

if (NOT GAME_VERSION)
  set(GAME_VERSION 100)
endif()

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_language(ASM C CXX)

set(CONFIG_TITLE_ID 0x0100000000010000)
add_compile_definitions(EXL_PROGRAM_ID=${CONFIG_TITLE_ID}
                        EXL_LOAD_KIND=Module
                        EXL_LOAD_KIND_ENUM=2
                        NNSDK
                        IMGUI_USER_CONFIG="imgui_backend/nvn_imgui_config.h"
                        private=public
                        protected=public
                        GAME_VERSION=${GAME_VERSION}
                        )

set(NN_SDK_MAJOR 4)
set(NN_SDK_MINOR 0)
set(NN_SDK_PATCH 0)
set(NN_SDK_TYPE "Release")
set(NN_WARE_MAJOR 4)
set(NN_WARE_MINOR 0)
set(NN_WARE_PATCH 0)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/module/subsdk9.json.template ${CMAKE_CURRENT_BINARY_DIR}/subsdk9.json)

set (INCLUDE_DIRS 
  ${PROJECT_SOURCE_DIR}/src/
  ${PROJECT_SOURCE_DIR}/src/lib 
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/lib/sead/include
  ${PROJECT_SOURCE_DIR}/lib/NintendoSDK/include
  ${PROJECT_SOURCE_DIR}/lib/NintendoSDK/include/nvn
  ${PROJECT_SOURCE_DIR}/lib/imgui
  ${PROJECT_SOURCE_DIR}/lib/odyssey/lib
  ${PROJECT_SOURCE_DIR}/lib/odyssey/src
  ${PROJECT_SOURCE_DIR}/lib/odyssey/lib/agl/include
)
include_directories(${INCLUDE_DIRS})

file(GLOB_RECURSE SOURCES_ASM ${PROJECT_SOURCE_DIR}/src/*.s)
file(GLOB_RECURSE SOURCES_C ${PROJECT_SOURCE_DIR}/src/*.c)

file(GLOB_RECURSE SOURCES_LIB ${PROJECT_SOURCE_DIR}/src/lib/*.cpp)
file(GLOB SOURCES_HELPERS ${PROJECT_SOURCE_DIR}/src/helpers/*.cpp)
file(GLOB SOURCES_IMGUI_BACKEND ${PROJECT_SOURCE_DIR}/src/imgui_backend/*.cpp)
file(GLOB SOURCES_GHOST ${PROJECT_SOURCE_DIR}/src/program/pe/Ghost/*.cpp)

set(SOURCES_CXX
  ${PROJECT_SOURCE_DIR}/src/program/main.cpp
  ${PROJECT_SOURCE_DIR}/src/program/imgui_nvn.cpp
  ${SOURCES_LIB}
  ${SOURCES_HELPERS}
  ${SOURCES_IMGUI_BACKEND}
  ${SOURCES_GHOST}
)

file(GLOB IMGUI_SOURCES ${PROJECT_SOURCE_DIR}/lib/imgui/*.cpp)

include(${CMAKE_SOURCE_DIR}/cmake/SwitchTools.cmake)

set(CMAKE_EXECUTABLE_SUFFIX ".nss")
add_executable(subsdk9 ${SOURCES_ASM} ${SOURCES_C} ${SOURCES_CXX} ${IMGUI_SOURCES})

set(SUBSDK_LD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/module/subsdk9.ld)
execute_process(COMMAND uname OUTPUT_VARIABLE uname)
if (uname MATCHES "^MINGW")
  string(REGEX REPLACE "^/([a-zA-Z])/" "\\1:/" SUBSDK_LD_PATH "${SUBSDK_LD_PATH}")
endif ()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/module/subsdk9.specs.template ${CMAKE_CURRENT_BINARY_DIR}/subsdk9.specs)

add_nso_target_subsdk(subsdk9)
set_target_properties(subsdk9 PROPERTIES CONFIG_JSON "${CMAKE_CURRENT_BINARY_DIR}/subsdk9.json")
__generate_npdm(subsdk9)
set_target_properties(subsdk9 PROPERTIES LINK_FLAGS "-specs ${CMAKE_BINARY_DIR}/subsdk9.specs -g ${ARCH} -nostartfiles -nostdlib")