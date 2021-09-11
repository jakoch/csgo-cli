cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

#-------------------------------------------------------------------
# Setup CMake Policies
#-------------------------------------------------------------------

# Policy CMP0074 is required to let find_package() use <PackageName>_ROOT variables.
# https://cmake.org/cmake/help/git-stage/policy/CMP0074.html
if(POLICY CMP0074)
   cmake_policy(SET CMP0074 NEW)
endif()

# Policy CMP0091 is required to enable MSVC_RUNTIME_LIBRARY property.
# This needs to be set before the first project for the policy to have an effect.
# https://cmake.org/cmake/help/git-stage/policy/CMP0091.html
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

#-------------------------------------------------------------------
# CMake Project
#-------------------------------------------------------------------

# vcpkg.json is the primary source for version data
file(READ ${CMAKE_SOURCE_DIR}/vcpkg.json VCPKG_JSON_STRING)
string(JSON APP_VERSION GET ${VCPKG_JSON_STRING} "version")

project(csgo_cli VERSION ${APP_VERSION} LANGUAGES CXX)

# inject version data into version header
configure_file(
    "${PROJECT_SOURCE_DIR}/src/VersionAndConstants.h.in"
    "${PROJECT_SOURCE_DIR}/src/VersionAndConstants.h"
)

#-------------------------------------------------------------------
# Setup CMake Includes
#-------------------------------------------------------------------

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include(FindPackageHandleStandardArgs)
include(ShowBuildTargetProperties)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

#-------------------------------------------------------------------
# Setup Dependency Manager: VCPKG
#-------------------------------------------------------------------

include(setup-vcpkg)

# ==================================================================================================
# Options
# ==================================================================================================

option(BUILD_SHARED_LIBS  "Build shared or static libs"                                   OFF)
option(ENABLE_LTO         "Enable link-time optimizations if supported by the compiler"    ON)
option(ENABLE_ASAN        "Enable address sanitizer (ASAN)"                               OFF)
option(BUILD_TESTS        "Build tests"                                                    ON)

# ==================================================================================================
# Link time optimizations (LTO)
# ==================================================================================================

if (ENABLE_LTO)
    # https://cmake.org/cmake/help/git-stage/policy/CMP0069.html
    if(POLICY CMP0069)
        cmake_policy(SET CMP0069 NEW)
        set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
    endif()

    include(CheckIPOSupported)
    check_ipo_supported(RESULT IPO_SUPPORT OUTPUT IPO_OUTPUT)

    if (IPO_SUPPORT)
        message(STATUS "[INFO] Link time optimizations (LTO) is ON.")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "[INFO] Link time optimizations (LTO) is not supported: ${IPO_OUTPUT}")
    endif()

    if(MSVC)
        # CMake IPO doesn't set the LTCG flag, which causes the linker to restart
        add_link_options($<$<BOOL:${CMAKE_INTERPROCEDURAL_OPTIMIZATION}>:/LTCG>)
    endif()
endif()

# ==================================================================================================
# OS specific
# ==================================================================================================

if (WIN32)
    # Build for a Windows 10 host system.
    set(CMAKE_SYSTEM_VERSION 10.0)

    message(STATUS "[INFO] BUILD_SHARED_LIBS -> '${BUILD_SHARED_LIBS}'.")

    # When we build statically (MT):
    if(NOT BUILD_SHARED_LIBS)
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
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251") # Warning C4251: 'identifier': class 'type' needs to have dll-interface to be used by clients of class 'type2'

  # /EHsc: Specifies the model of exception handling.
  # /GL:   Enables whole program optimization.
  # /Gm:   Enable minimal rebuilds; conflicts with /MP
  # /GS:   Buffers security check.
  # /MD:   Creates a multithreaded DLL using MSVCRT.lib.
  # /MDd:  Creates a debug multithreaded DLL using MSVCRTD.lib.
  # /O2:   Creates fast code.
  # /Od:   Disables optimization.
  # /Oi:   Generates intrinsic functions.
  # /RTC1: Enables run-time error checking.
  # /W4:   Sets which warning level to output.
  # /Zi:   Generates complete debugging information.
  # /JMC:  Just my code enables the VS debugger to step over internal (system, framework, library, and other non-user) calls.
  set(CMAKE_CXX_FLAGS           "/EHsc /GS /Zi /Gm-") #/W4
  set(CMAKE_CXX_FLAGS_DEBUG     "/Od /RTC1 /JMC")
  set(CMAKE_CXX_FLAGS_RELEASE   "/GL /O2 /Oi")

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

# /MP: build with multiple processors
# https://docs.microsoft.com/en-us/cpp/build/reference/mp-build-with-multiple-processes?view=msvc-160
if(CMAKE_GENERATOR MATCHES "Visual Studio" AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  if(CMake_MSVC_PARALLEL)
    if(CMake_MSVC_PARALLEL GREATER 0)
      string(APPEND CMAKE_C_FLAGS " /MP${CMake_MSVC_PARALLEL}")
      string(APPEND CMAKE_CXX_FLAGS " /MP${CMake_MSVC_PARALLEL}")
    else()
      string(APPEND CMAKE_C_FLAGS " /MP")
      string(APPEND CMAKE_CXX_FLAGS " /MP")
    endif()
  endif()
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

configure_file("${CMAKE_SOURCE_DIR}/resources/steam_appid.txt" "steam_appid.txt" COPYONLY)

if(WIN32)
    set(STEAMWORKS_LIBRARY "${STEAMWORKS_REDISTBIN_DIR}/win64/steam_api64.lib")
    configure_file("${STEAMWORKS_REDISTBIN_DIR}/win64/steam_api64.dll" "steam_api64.dll" COPYONLY)
else()
    set(STEAMWORKS_LIBRARY "${STEAMWORKS_REDISTBIN_DIR}/linux64/libsteam_api.so")
    configure_file("${STEAMWORKS_LIBRARY}" "libsteam_api.so" COPYONLY)
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
        message("MSVC flags: ${CompilerFlag}: ${${CompilerFlag}}")
    endforeach()
endif()

set(Protobuf_USE_STATIC_LIBS ON)
set(Protobuf_DIR "${VCPKG_DIR}/share/protobuf")
list(APPEND CMAKE_PREFIX_PATH "${VCPKG_DIR}/tools/protobuf")
#
# Please fix your protobuf_generate(), okay?
# https://stackoverflow.com/questions/52533396/cmake-cant-find-protobuf-protobuf-generate-cpp
#
set(protobuf_MODULE_COMPATIBLE ON CACHE BOOL "")

find_package(Protobuf CONFIG REQUIRED)

set(Protobuf_PROTOC_EXECUTABLE protobuf::libprotoc)

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
set(ZLIB_ROOT "${VCPKG_DIR}")
find_package(ZLIB REQUIRED)

#
# Curl
#
set(CURL_DIR "${VCPKG_DIR}/share/curl")
find_package(CURL CONFIG REQUIRED)

#
# FMT
#
set(fmt_DIR "${VCPKG_DIR}/share/fmt")
find_package(fmt CONFIG REQUIRED)

#
# nlohman-json
#
set(nlohmann_json_DIR "${VCPKG_DIR}/share/nlohmann_json")
find_package(nlohmann_json CONFIG REQUIRED)

#
# spdlog
#
set(spdlog_DIR "${VCPKG_DIR}/share/spdlog")
find_package(spdlog CONFIG REQUIRED)

#
# Copy Additional Files
#
configure_file("${CMAKE_SOURCE_DIR}/README.md"                      "readme.txt"  COPYONLY)
configure_file("${CMAKE_SOURCE_DIR}/LICENSE"                        "license.txt" COPYONLY)
configure_file("${CMAKE_SOURCE_DIR}/resources/update.bat"           "update.bat"  COPYONLY)
configure_file("${CMAKE_SOURCE_DIR}/dependencies/curl/cacert.pem"   "cacert.pem"  COPYONLY)

#
# Add Resources to Executable
#
if(WIN32)
    set(ADDITIONAL_RESOURCES "resources/CSGO_CLI.rc")
else()
    set(ADDITIONAL_RESOURCES "")
endif()

#-------------------------------------------------------------------
# Build Target: csgo_cli_lib                               (library)
#-------------------------------------------------------------------

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

#
# Process CS:GO Protobufs
# See comment above @ "protobuf_MODULE_COMPATIBLE"
#
#protobuf_generate(
#    LANGUAGE cpp
#    TARGET csgo_cli_lib
#    PROTOS
#    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/csgo-protobufs/csgo/steammessages.proto
#    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/csgo-protobufs/csgo/gcsdk_gcmessages.proto
#    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/csgo-protobufs/csgo/gcsystemmsgs.proto
#    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/csgo-protobufs/csgo/engine_gcmessages.proto
#    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/csgo-protobufs/csgo/cstrike15_gcmessages.proto
#)

include_directories(
    ${STEAMWORKS_INCLUDE_DIR}
    ${PROTOBUF_INCLUDE_DIR}
    ${CURL_INCLUDE_DIR}
    ${fmt_INCLUDE_DIRS}
)

target_link_libraries(csgo_cli_lib
    PRIVATE
        fmt::fmt
        #spdlog::spdlog
        nlohmann_json::nlohmann_json
        CURL::libcurl
        ZLIB::ZLIB
        ${STEAMWORKS_LIBRARY}
        protobuf::libprotobuf
)

set_property(TARGET csgo_cli_lib PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(${CMAKE_VERSION} VERSION_LESS "3.21.0")
    target_compile_features(csgo_cli_lib PUBLIC cxx_std_20)
else()
    target_compile_features(csgo_cli_lib PUBLIC cxx_std_23)
endif()

if(MSVC)
    if(ENABLE_ASAN)
        target_compile_options(csgo_cli_lib PUBLIC -fsanitize=address)
    endif()
endif()

#-------------------------------------------------------------------
# Build Target: csgo_cli                                       (app)
#-------------------------------------------------------------------

add_executable(csgo_cli
    # icon
    ${ADDITIONAL_RESOURCES}
    # application files
    src/main/main.cpp
)

target_link_libraries(csgo_cli
    PRIVATE
        csgo_cli_lib
        fmt::fmt-header-only
)

set_property(TARGET csgo_cli PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(${CMAKE_VERSION} VERSION_LESS "3.21.0")
    target_compile_features(csgo_cli PUBLIC cxx_std_20)
else()
    target_compile_features(csgo_cli PUBLIC cxx_std_23)
endif()

if(MSVC)
    if(ENABLE_ASAN)
        target_compile_options(csgo_cli PUBLIC -fsanitize=address)
    endif()
endif()

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

#-------------------------------------------------------------------
# Tests
#-------------------------------------------------------------------

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

#-------------------------------------------------------------------
# Installation Rules: csgo_cli                                 (app)
#-------------------------------------------------------------------

include(GNUInstallDirs)

install(TARGETS csgo_cli RESOURCE)

install(FILES "${CMAKE_BINARY_DIR}/readme.txt"        DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES "${CMAKE_BINARY_DIR}/license.txt"       DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES "${CMAKE_BINARY_DIR}/update.bat"        DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES "${CMAKE_BINARY_DIR}/cacert.pem"        DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES "${CMAKE_BINARY_DIR}/steam_appid.txt"   DESTINATION ${CMAKE_INSTALL_BINDIR})

if(WIN32)
    install(FILES "${CMAKE_BINARY_DIR}/steam_api64.dll" DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
    install(FILES "${CMAKE_BINARY_DIR}/libsteam_api.so" DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

#-------------------------------------------------------------------
# Display Compiler and Linker properties of Build Targets
#-------------------------------------------------------------------

show_build_target_properties(csgo_cli_lib)
show_build_target_properties(csgo_cli)