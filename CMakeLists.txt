cmake_minimum_required(VERSION 3.20)
project(subsdk CXX C ASM)

## Error if not using switch toolchain file
if (NOT SWITCH)
  message(FATAL_ERROR "Not targeting switch, make sure to specify -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain.cmake")
endif ()

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

## Target game version
set(GAME_VERSION 130 CACHE STRING "Target game version")
set(GAME_VERSION_VAL 130)

if (GAME_VERSION STREQUAL "100")
  set(GAME_VERSION_VAL 100)
endif ()

## Title Id
set(TITLE_ID 0100000000010000)
set(CONFIG_TITLE_ID 0x${TITLE_ID})

execute_process(
  COMMAND git describe --tags --abbrev=0
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  OUTPUT_VARIABLE GIT_VER
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

add_compile_definitions(EXL_PROGRAM_ID=${CONFIG_TITLE_ID} SMO_VER=${GAME_VERSION_VAL} NNSDK=1 IMGUI_USER_CONFIG="imgui_backend/nvn_imgui_config.h" GIT_VER="${GIT_VER}")
set(NN_WARE_PATCH 0) # idk

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/module/subsdk9.json.template ${CMAKE_CURRENT_BINARY_DIR}/subsdk9.json)

## Project sources
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
file(GLOB_RECURSE SOURCES_CXX ${PROJECT_SOURCE_DIR}/src/*.cpp)

file(GLOB IMGUI_SOURCES ${PROJECT_SOURCE_DIR}/lib/imgui/*.cpp)

## Include nx tools
include(${CMAKE_SOURCE_DIR}/cmake/SwitchTools.cmake)

## subsdk9
set(CMAKE_EXECUTABLE_SUFFIX ".nss")
add_executable(subsdk9 ${SOURCES_ASM} ${SOURCES_C} ${SOURCES_CXX} ${IMGUI_SOURCES})

## Prepare subsdk ld
set(SUBSDK_LD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/module/subsdk9.ld)
set(SYMS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/syms/${GAME_VERSION}")

execute_process(
    COMMAND bash -c "find ${SYMS_PATH} -type f -name '*.sym' -exec cat {} + > ${CMAKE_BINARY_DIR}/syms.ld"
)

execute_process(COMMAND uname OUTPUT_VARIABLE uname)
if (uname MATCHES "^MINGW")
  string(REGEX REPLACE "^/([a-zA-Z])/" "\\1:/" SUBSDK_LD_PATH "${SUBSDK_LD_PATH}")
endif ()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/module/subsdk9.specs.template ${CMAKE_CURRENT_BINARY_DIR}/subsdk9.specs)

## Build nso & generate npdm
add_nso_target_subsdk(subsdk9)
set_target_properties(subsdk9 PROPERTIES CONFIG_JSON "${CMAKE_CURRENT_BINARY_DIR}/subsdk9.json")
__generate_npdm(subsdk9)
set_target_properties(subsdk9 PROPERTIES LINK_FLAGS "-specs ${CMAKE_BINARY_DIR}/subsdk9.specs -g ${ARCH} -nostartfiles -nostdlib")