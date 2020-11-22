cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

cmake_policy(SET CMP0091 NEW)

project(csgo_cli)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
include(FindPackageHandleStandardArgs)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build")

#-------------------------------------------------------------------
# Setup Dependency Manager: VCPKG
#-------------------------------------------------------------------

set(VCPKG_TARGET_TRIPLET        x64-windows-static)

#if (WIN32)
    # x64-windows-static
    #set(VCPKG_TARGET_ARCHITECTURE   x64)
    #set(VCPKG_CRT_LINKAGE           static)
    #set(VCPKG_LIBRARY_LINKAGE       static)

    # x64-dynamic-static
    #set(VCPKG_TARGET_ARCHITECTURE   x64)
    #set(VCPKG_CRT_LINKAGE           dynamic)
    #set(VCPKG_LIBRARY_LINKAGE       static)
#endif (WIN32)

include(setup-vcpkg)

# ==================================================================================================
# Options
# ==================================================================================================

option(ENABLE_LTO   "Enable link-time optimizations if supported by the compiler"   OFF)

# ==================================================================================================
# Link time optimizations (LTO)
# ==================================================================================================
if (ENABLE_LTO)
    include(CheckIPOSupported)

    check_ipo_supported(RESULT IPO_SUPPORT)

    if (IPO_SUPPORT)
        message(STATUS "LTO support is enabled")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

# ==================================================================================================
# OS specific
# ==================================================================================================

if (WIN32)

    set(BUILD_SHARED_LIBS OFF)

    cmake_policy(SET CMP0091 NEW) # https://cmake.org/cmake/help/v3.15/variable/CMAKE_MSVC_RUNTIME_LIBRARY.html
    if (NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
      set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()

endif()

# ==================================================================================================
# Compiler flags
# ==================================================================================================

# Define C++ Standard to use
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

  message(STATUS "\n-- Compiler and platform toolset:\n")

  # Report compiler and platform toolset
  message(STATUS "USING COMPILER: ${CMAKE_CXX_COMPILER_ID}")
  message(STATUS "MSVC_VERSION: ${MSVC_VERSION}")
  message(STATUS "MSVC_TOOLSET_VERSION: ${MSVC_TOOLSET_VERSION}")
  message(STATUS "CMAKE_MSVC_RUNTIME_LIBRARY: ${CMAKE_MSVC_RUNTIME_LIBRARY}")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")

  # Be quiet
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /nologo")

  # MSVC warning suppressions
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  add_definitions(-D_SILENCE_CXX20_IS_POD_DEPRECATION_WARNING) # protobuf triggers this warning
  add_definitions(-DNOMINMAX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4146") # Warning C4146: unary minus operator applied to unsigned type, result still unsigned
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244") # Warning C4244: 'initializing': conversion from 'double' to 'int', possible loss of data
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267") # Warning C4267: 'var' : conversion from 'size_t' to 'type', possible loss of data
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4305") # Warning C4305: 'initializing': truncation from 'double' to 'float'
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4800") # Warning C4800: 'uint32_t' : forcing value to bool 'true' or 'false' (performance warning)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4838") # Warning C4838: conversion from 'double' to 'float' requires a narrowing conversion
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996") # Warning C4996: Call to 'function' with parameters that may be unsafe
  # You don't want to know why.
  # https://github.com/protocolbuffers/protobuf/blob/master/cmake/README.md#notes-on-compiler-warnings
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251") # Warning C4251:
  message(STATUS "USING THESE CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # MSVC warning suppressions
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  add_definitions(-D_SILENCE_CXX20_IS_POD_DEPRECATION_WARNING) # protobuf triggers this warning
  add_definitions(-DNOMINMAX)

  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ ") # -lc++abi
endif()

# ==================================================================================================
# Linker flags
# ==================================================================================================

# Link static runtime libraries
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}") # /MD

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_link_options("/nologo")
    add_link_options("/VERBOSE:LIB")
endif()

# ==================================================================================================
# Setup Dependencies
# ==================================================================================================

message(STATUS "\n-- Dependencies:\n")

#
# Steamworks SDK
#
set(STEAMWORKS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/dependencies/sw_sdk/public")
set(STEAMWORKS_REDISTBIN_DIR "${CMAKE_SOURCE_DIR}/dependencies/sw_sdk/redistributable_bin")

configure_file("${CMAKE_SOURCE_DIR}/res/steam_appid.txt" "build/steam_appid.txt" COPYONLY)

if(WIN32)
    set(STEAMWORKS_LIBRARY "${STEAMWORKS_REDISTBIN_DIR}/win64/steam_api64.lib")
    configure_file("${STEAMWORKS_REDISTBIN_DIR}/win64/steam_api64.dll" "build/steam_api64.dll" COPYONLY)
else()
    set(STEAMWORKS_LIBRARY "${STEAMWORKS_REDISTBIN_DIR}/linux64/libsteam_api.so")
    configure_file("${STEAMWORKS_LIBRARY}" "build/libsteam_api.so" COPYONLY)
endif()

find_package(Steamworks REQUIRED)

#
# Google Protobuf
#
set(protobuf_MSVC_STATIC_RUNTIME on)

if(MSVC AND protobuf_MSVC_STATIC_RUNTIME)
    set(CompilerFlags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL
            CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_MINSIZEREL
            CMAKE_C_FLAGS_RELWITHDEBINFO
            )
    foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
        set(${CompilerFlag} "${${CompilerFlag}}" CACHE STRING "msvc compiler flags" FORCE)
        message("MSVC flags: ${CompilerFlag}:${${CompilerFlag}}")
    endforeach()
endif()

set(Protobuf_INCLUDE_DIR         "${VCPKG_DIR}/include")
set(Protobuf_IMPORT_DIRS         "${VCPKG_DIR}/include") # additional proto files from include folder
set(Protobuf_LIBRARY             "${VCPKG_DIR}/lib/libprotobuf.lib")

set(Protobuf_USE_STATIC_LIBS ON)
set(Protobuf_PROTOC_EXECUTABLE   "${VCPKG_DIR}/tools/protobuf/protoc") # protoc.exe

set(Protobuf_DIR "${VCPKG_DIR}/share/protobuf")
set(protobuf_MODULE_COMPATIBLE ON CACHE BOOL "")
find_package(Protobuf CONFIG REQUIRED)

#
# Process CS:GO Protobufs
#
# dependencies/csgo-protobufs/csgo/engine_gcmessages.proto
# dependencies/csgo-protobufs/csgo/cstrike15_gcmessages.proto
# dependencies/csgo-protobufs/csgo/steammessages.proto
# dependencies/csgo-protobufs/csgo/gcsdk_gcmessages.proto
# dependencies/csgo-protobufs/csgo/gcsystemmsgs.proto

protobuf_generate_cpp(PROTO_ENGINE_GCMESSAGES_SRC PROTO_ENGINE_GCMESSAGES_HDR
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/csgo-protobufs/csgo/engine_gcmessages.proto
)
protobuf_generate_cpp(PROTO_CSTRIKE15_GCMESSAGES_SRC PROTO_CSTRIKE15_GCMESSAGES_HDR
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/csgo-protobufs/csgo/cstrike15_gcmessages.proto
)
protobuf_generate_cpp(PROTO_STEAMMESSAGES_SRC PROTO_STEAMMESSAGES_HDR
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/csgo-protobufs/csgo/steammessages.proto
)
protobuf_generate_cpp(PROTO_GCSDK_GCMESSAGES_SRC PROTO_GCSDK_GCMESSAGES_HDR
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/csgo-protobufs/csgo/gcsdk_gcmessages.proto
)
protobuf_generate_cpp(PROTO_GCSYSTEMMSGS_SRC PROTO_GCSYSTEMMSGS_HDR
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/csgo-protobufs/csgo/gcsystemmsgs.proto
)

#
# zlib
#
if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW) #3.12.0 `find_package()`` uses ``<PackageName>_ROOT`` variables.
endif()
set(ZLIB_ROOT "${VCPKG_DIR}")
find_package(ZLIB)#REQUIRED)

#
# Curl
#
set(CURL_DIR "${VCPKG_DIR}/share/curl")
find_package(CURL CONFIG REQUIRED)

#
# FMT
#
set(FMT_DIR "${VCPKG_DIR}/share/fmt")
find_package(FMT CONFIG REQUIRED)

#
# nlohman-json
#
set(nlohmann_json_DIR "${VCPKG_DIR}/share/nlohmann_json")
find_package(nlohmann_json CONFIG REQUIRED)

#
# Copy Additional Files
#
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/README.md"                "build/readme.txt"    COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"                  "build/license.txt"   COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/res/update.bat"           "build/update.bat"    COPYONLY)
configure_file("${CMAKE_SOURCE_DIR}/dependencies/curl/cacert.pem"     "build/cacert.pem"    COPYONLY)

#
# Add Exe Resources
#
if(WIN32)
    set(ADDITIONAL_RESOURCES "res/CSGO_CLI.rc")
else()
    set(ADDITIONAL_RESOURCES "")
endif()

#
# Application Library
#

add_library(csgo_cli_lib
        # base
        src/DataObject.h
        src/DataObject.cpp
        src/DateTimeUtils.h
        src/DateTimeUtils.cpp
        src/ErrorHandler.h
        src/ExceptionHandler.h
        src/ExceptionHandler.cpp
        src/ShareCode.h
        src/ShareCode.cpp
        src/SteamId.h
        src/SteamId.cpp
        src/VersionAndConstants.h
        # commands
        src/commands/cmd.help.h
        src/commands/cmd.help.cpp
        src/commands/cmd.user.h
        src/commands/cmd.user.cpp
        src/commands/cmd.upload.h
        src/commands/cmd.upload.cpp
        src/commands/cmd.matches.h
        src/commands/cmd.matches.cpp
        src/commands/cmd.scoreboard.h
        src/commands/cmd.scoreboard.cpp
        src/commands/cmd.globalstats.h
        src/commands/cmd.globalstats.cpp
        # csgo api
        src/csgo/CSGOClient.h
        src/csgo/CSGOClient.cpp
        src/csgo/GCMsgHandler.h
        src/csgo/CSGOMMHello.h
        src/csgo/CSGOMMHello.cpp
        src/csgo/CSGOMatchList.h
        src/csgo/CSGOMatchList.cpp
        src/csgo/CSGOMatchData.h
        src/csgo/CSGOMatchPlayerScore.h
        src/csgo/CSGOMatchPlayerScore.cpp
        src/csgo/CSGORankUpdate.cpp
        src/csgo/CSGORankUpdate.h
        # csgostats
        src/csgostats/ShareCodeUpload.h
        src/csgostats/ShareCodeUpload.cpp
        src/csgostats/ShareCodeCache.h
        src/csgostats/ShareCodeCache.cpp
        # windows platform
        src/platform/windows/WinCliColors.h
        src/platform/windows/WinCliColors.cpp

    # Generated Protobuf Includes
        ${PROTO_ENGINE_GCMESSAGES_SRC}
        ${PROTO_ENGINE_GCMESSAGES_HDR}
        ${PROTO_CSTRIKE15_GCMESSAGES_SRC}
        ${PROTO_CSTRIKE15_GCMESSAGES_HDR}
        ${PROTO_STEAMMESSAGES_SRC}
        ${PROTO_STEAMMESSAGES_HDR}
        ${PROTO_GCSDK_GCMESSAGES_SRC}
        ${PROTO_GCSDK_GCMESSAGES_HDR}
        ${PROTO_GCSYSTEMMSGS_SRC}
        ${PROTO_GCSYSTEMMSGS_HDR}
)

include_directories(
    ${STEAMWORKS_INCLUDE_DIR}
    ${PROTOBUF_INCLUDE_DIR}
    ${CURL_INCLUDE_DIR}
    ${fmt_INCLUDE_DIRS}
)

target_link_libraries(csgo_cli_lib
    PRIVATE
        ${STEAMWORKS_LIBRARY}
        CURL::libcurl
        ZLIB::ZLIB
        protobuf::libprotobuf
        fmt::fmt
        nlohmann_json::nlohmann_json
)

set_property(TARGET csgo_cli_lib PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_compile_features(csgo_cli_lib PUBLIC cxx_std_20)

#
# Executable
#

add_executable(csgo_cli
    # icon
    ${ADDITIONAL_RESOURCES}
    # application files
    src/main/main.cpp
)

target_link_libraries(csgo_cli
    PRIVATE
        csgo_cli_lib
)

set_property(TARGET csgo_cli PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_compile_features(csgo_cli PUBLIC cxx_std_20)

#
# Create "source-group" folders for these files in Visual Studio
#
source_group("Protobuf Protocol Files" FILES
    proto/engine_gcmessages.proto
    proto/cstrike15_gcmessages.proto
    proto/steammessages.proto
    proto/gcsdk_gcmessages.proto
    proto/gcsystemmsgs.proto
)
source_group("Protobuf Generated Files" FILES
    ${PROTO_ENGINE_GCMESSAGES_SRC}
    ${PROTO_ENGINE_GCMESSAGES_HDR}
    ${PROTO_CSTRIKE15_GCMESSAGES_SRC}
    ${PROTO_CSTRIKE15_GCMESSAGES_HDR}
    ${PROTO_STEAMMESSAGES_SRC}
    ${PROTO_STEAMMESSAGES_HDR}
    ${PROTO_GCSDK_GCMESSAGES_SRC}
    ${PROTO_GCSDK_GCMESSAGES_HDR}
    ${PROTO_GCSYSTEMMSGS_SRC}
    ${PROTO_GCSYSTEMMSGS_HDR}
)

option(CSGO_CLI_BUILD_TESTS "Build tests" ON)

if(CSGO_CLI_BUILD_TESTS)
    add_subdirectory(tests)
endif()