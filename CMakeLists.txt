cmake_minimum_required(VERSION 3.30 FATAL_ERROR)

#-------------------------------------------------------------------
# Setup CMake
#-------------------------------------------------------------------

set(CMAKE_FIND_LIBRARY_PREFIXES ";")
set(CMAKE_FIND_LIBRARY_SUFFIXES ";")

# Generate CompilationDatabase in `compile_commands.json`.
# The file is used by build tools and IntelliSense (VSCode, YouCompleteMe, etc).
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

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

# Policy CMP0069 is required for INTERPROCEDURAL_OPTIMIZATION
# https://cmake.org/cmake/help/git-stage/policy/CMP0069.html
if(POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
endif()

#-------------------------------------------------------------------
# Get Versioning Data
#-------------------------------------------------------------------

# vcpkg.json is the primary source for version data
file(READ ${CMAKE_SOURCE_DIR}/vcpkg.json VCPKG_JSON_STRING)
string(JSON APP_VERSION GET ${VCPKG_JSON_STRING} "version")

#-------------------------------------------------------------------
# Project
#-------------------------------------------------------------------

project(csgo_cli VERSION ${APP_VERSION} LANGUAGES CXX)

#-------------------------------------------------------------------
# Versioning
#-------------------------------------------------------------------

configure_file(
    "${PROJECT_SOURCE_DIR}/src/VersionAndConstants.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/src/VersionAndConstants.h"
)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src)

#-------------------------------------------------------------------
# Setup CMake Includes
#-------------------------------------------------------------------

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include(FindPackageHandleStandardArgs)
include(ShowBuildTargetProperties)
include(SetupVcpkg)
include(SetupCompiler)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# ==================================================================================================
# Options
# ==================================================================================================

option(BUILD_SHARED_LIBS  "Build shared or static libs"                                   OFF)
option(ENABLE_LTO         "Enable link-time optimizations if supported by the compiler"    ON)
option(ENABLE_ASAN        "Enable address sanitizer (ASAN)"                               OFF)
option(BUILD_TESTS        "Build tests"                                                    ON)

# ==================================================================================================
# Setup Dependencies
# ==================================================================================================

message(STATUS "\n-- Dependencies:\n")

#
# Steamworks SDK
#
set(STEAMWORKS_INCLUDE_DIR    "${CMAKE_SOURCE_DIR}/dependencies/sw_sdk/public")
set(STEAMWORKS_REDISTBIN_DIR  "${CMAKE_SOURCE_DIR}/dependencies/sw_sdk/redistributable_bin")

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

#
# Protobuf is non-trivial to include.
#
set(Protobuf_USE_STATIC_LIBS ON)
set(Protobuf_DIR "${VCPKG_DIR}/share/protobuf")
list(APPEND CMAKE_PREFIX_PATH "${VCPKG_DIR}/tools/protobuf")

#
# Please fix your protobuf_generate(), okay?
# https://stackoverflow.com/questions/52533396/cmake-cant-find-protobuf-protobuf-generate-cpp
#
set(protobuf_MODULE_COMPATIBLE ON CACHE BOOL "")

find_program(Protobuf_PROTOC_EXECUTABLE
    NAMES protoc
    DOC "Google Protocol Buffers Compiler"
    PATHS
      "${VCPKG_DIR}/tools/protobuf"
      "${VCPKG_ROOT}/packages/protobuf_${VCPKG_TARGET_TRIPLET}/tools/protobuf"
)

if(NOT Protobuf_PROTOC_EXECUTABLE)
    if(WIN32)
        set(PROTOC "${VCPKG_DIR}/tools/protobuf/protoc.exe")
        set(Protobuf_PROTOC_EXECUTABLE "${VCPKG_DIR}/tools/protobuf/protoc.exe")
    else()
        set(PROTOC "${VCPKG_DIR}/tools/protobuf/protoc")
        set(Protobuf_PROTOC_EXECUTABLE "${VCPKG_DIR}/tools/protobuf/protoc")
    endif()
endif()

message(STATUS "Found protoc: ${Protobuf_PROTOC_EXECUTABLE}")

# Find Protobuf package
find_package(Protobuf CONFIG REQUIRED)

#
# SteamDatabase Protobufs Data Files
#
include(FetchContent)

# Define the repository and the destination directory
FetchContent_Declare(steamdatabase_protobufs
    GIT_REPOSITORY "https://github.com/SteamDatabase/Protobufs.git"
    GIT_TAG "origin/master"
)
FetchContent_MakeAvailable(steamdatabase_protobufs)
message("--[CSGO Protobuf] Fetched SteamDatabase Protobufs Data Files to steamdatabase_protobufs_SOURCE_DIR: ${steamdatabase_protobufs_SOURCE_DIR}")

# Patch CSGO Protobufs
# After patch was applied ${Protobuf_INCLUDE_DIRS} should contain the file "patch_applied_marker".
include(PatchCsGoProtobufs)
if(NOT EXISTS "${steamdatabase_protobufs_SOURCE_DIR}/csgo/patch_applied_marker")
    patch_csgo_protobufs()
else()
    message("--[CSGO Protobuf] Already patched. Skipping.")
endif()

# let the custom target depend on the patching run
add_custom_target(patch_steamdatabase_csgo_protobufs ALL DEPENDS "${steamdatabase_protobufs_SOURCE_DIR}/csgo/patch_applied_marker")

#
# Compile CS:GO Protobufs (proto -> cpp)
#
protobuf_generate_cpp(PROTO_ENGINE_GCMESSAGES_SRC PROTO_ENGINE_GCMESSAGES_HDR
    ${steamdatabase_protobufs_SOURCE_DIR}/csgo/engine_gcmessages.proto
)
protobuf_generate_cpp(PROTO_CSTRIKE15_GCMESSAGES_SRC PROTO_CSTRIKE15_GCMESSAGES_HDR
    ${steamdatabase_protobufs_SOURCE_DIR}/csgo/cstrike15_gcmessages.proto
)
protobuf_generate_cpp(PROTO_STEAMMESSAGES_SRC PROTO_STEAMMESSAGES_HDR
    ${steamdatabase_protobufs_SOURCE_DIR}/csgo/steammessages.proto
)
protobuf_generate_cpp(PROTO_GCSDK_GCMESSAGES_SRC PROTO_GCSDK_GCMESSAGES_HDR
    ${steamdatabase_protobufs_SOURCE_DIR}/csgo/gcsdk_gcmessages.proto
)
protobuf_generate_cpp(PROTO_GCSYSTEMMSGS_SRC PROTO_GCSYSTEMMSGS_HDR
    ${steamdatabase_protobufs_SOURCE_DIR}/csgo/gcsystemmsgs.proto
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
# Download "cacert.pem" for Curl -> "curl-ca-bundle.crt"
#
if(NOT EXISTS "${CMAKE_BINARY_DIR}/curl-ca-bundle.crt")
    message(STATUS "Downloading: https://curl.haxx.se/ca/cacert.pem -> curl-ca-bundle.crt")
    file(DOWNLOAD "https://curl.haxx.se/ca/cacert.pem" "${CMAKE_BINARY_DIR}/curl-ca-bundle.crt" TLS_VERIFY ON)
endif()

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
        src/VersionAndConstants.h # = ${CMAKE_CURRENT_BINARY_DIR}/src/VersionAndConstants.h
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

# Ensure that any targets that depend on the patched protobufs are built after the patching is done
add_dependencies(csgo_cli_lib patch_steamdatabase_csgo_protobufs)

#
# Process CS:GO Protobufs
# See comment above @ "protobuf_MODULE_COMPATIBLE"
#
#protobuf_generate(
#    LANGUAGE cpp
#    TARGET csgo_cli_lib
#    PROTOS
#    ${SteamDatabase_Protobufs_SOURCE_DIR}/csgo/steammessages.proto
#    ${SteamDatabase_Protobufs_SOURCE_DIR}/csgo/gcsdk_gcmessages.proto
#    ${SteamDatabase_Protobufs_SOURCE_DIR}/csgo/gcsystemmsgs.proto
#    ${SteamDatabase_Protobufs_SOURCE_DIR}/csgo/engine_gcmessages.proto
#    ${SteamDatabase_Protobufs_SOURCE_DIR}/csgo/cstrike15_gcmessages.proto
#)

target_include_directories(csgo_cli_lib
    PUBLIC
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

target_compile_features(csgo_cli_lib PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(csgo_cli_lib PUBLIC /utf-8)
    if(ENABLE_ASAN)
        target_compile_options(csgo_cli_lib PUBLIC -fsanitize=address)
    endif()
endif()

target_link_options(csgo_cli_lib
    PRIVATE
        $<$<CONFIG:Debug>: /NODEFAULTLIB:LIBCMTD;>
        #$<$<CONFIG:Release>: /NODEFAULTLIB:LIBCMT;>
)


#-------------------------------------------------------------------
# Build Target: csgo_cli                                       (app)
#-------------------------------------------------------------------

add_executable(csgo_cli
    # icon
    ${ADDITIONAL_RESOURCES}
    # application files
    src/main/main.cpp
)

target_include_directories(csgo_cli
    PUBLIC
        ${STEAMWORKS_INCLUDE_DIR}
)

target_link_libraries(csgo_cli
    PRIVATE
        csgo_cli_lib
        fmt::fmt-header-only
)

set_property(TARGET csgo_cli PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_compile_features(csgo_cli PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(csgo_cli PUBLIC /utf-8)
    if(ENABLE_ASAN)
        target_compile_options(csgo_cli PUBLIC -fsanitize=address)
    endif()
endif()

#
# Create "source-group" folders for these files in Visual Studio
#
# source_group("CSGO Protobuf Protocol Files" FILES
#     proto/engine_gcmessages.proto
#     proto/cstrike15_gcmessages.proto
#     proto/steammessages.proto
#     proto/gcsdk_gcmessages.proto
#     proto/gcsystemmsgs.proto
# )

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

install(FILES "${CMAKE_BINARY_DIR}/readme.txt"         DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES "${CMAKE_BINARY_DIR}/license.txt"        DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES "${CMAKE_BINARY_DIR}/update.bat"         DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES "${CMAKE_BINARY_DIR}/curl-ca-bundle.crt" DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES "${CMAKE_BINARY_DIR}/steam_appid.txt"    DESTINATION ${CMAKE_INSTALL_BINDIR})

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
