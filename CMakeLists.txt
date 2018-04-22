cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(csgo_cli)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
include(FindPackageHandleStandardArgs)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build")

set(CMAKE_AUTOMOC ON)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++11")
    set(CMAKE_EXE_LINKER_FLAGS "-l protobuf -pthread")
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")
endif()

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  add_definitions(-DNOMINMAX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244") # Warning C4244: 'initializing': conversion from 'double' to 'int', possible loss of data
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4305") # Warning C4305: 'initializing': truncation from 'double' to 'float'
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4800") # Warning C4800: 'uint32_t' : forcing value to bool 'true' or 'false' (performance warning)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4838") # Warning C4838: conversion from 'double' to 'float' requires a narrowing conversion
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4146") # Warning C4146: unary minus operator applied to unsigned type, result still unsigned
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267")
endif()

set(VCPKG_LIBRARY_LINKAGE static)

#
# Setup Dependencies
#

#
# Steamworks SDK
#
set(STEAMWORKS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/dependencies/sw_sdk/public/steam")
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
set(PROTOBUF_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/dependencies/protobuf/src")

if(WIN32)
    set(PROTOBUF_LIBRARY       "${CMAKE_SOURCE_DIR}/dependencies/protobuf/cmake/release/libprotobuf.lib")
    set(PROTOBUF_LIBRARY_DEBUG "${CMAKE_SOURCE_DIR}/dependencies/protobuf/cmake/release/libprotobuf.lib")
endif()

if(WIN32)
    set(PROTOBUF_PROTOC_EXECUTABLE "${CMAKE_SOURCE_DIR}/dependencies/protobuf/cmake/release/protoc.exe")
else()
    set(PROTOBUF_PROTOC_EXECUTABLE "${CMAKE_SOURCE_DIR}/dependencies/protobuf/cmake/release/protoc")
endif()

find_package(Protobuf REQUIRED)

#
# mpir extended precision integer library
#
find_package(mpir REQUIRED)

#
# Process CS:GO Protobufs
#
# dependencies/csgo-protobufs/csgo/cstrike15_gcmessages.proto
# dependencies/csgo-protobufs/csgo/steammessages.proto
# dependencies/csgo-protobufs/csgo/gcsdk_gcmessages.proto
# dependencies/csgo-protobufs/csgo/gcsystemmsgs.proto

PROTOBUF_GENERATE_CPP(
    PROTO_CSTRIKE15_GCMESSAGES_SRC
    PROTO_CSTRIKE15_GCMESSAGES_HDR
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/csgo-protobufs/csgo/cstrike15_gcmessages.proto
)
PROTOBUF_GENERATE_CPP(
    PROTO_STEAMMESSAGES_SRC
    PROTO_STEAMMESSAGES_HDR
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/csgo-protobufs/csgo/steammessages.proto
)
PROTOBUF_GENERATE_CPP(
    PROTO_GCSDK_GCMESSAGES_SRC
    PROTO_GCSDK_GCMESSAGES_HDR
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/csgo-protobufs/csgo/gcsdk_gcmessages.proto
)
PROTOBUF_GENERATE_CPP(
    PROTO_GCSYSTEMMSGS_SRC
    PROTO_GCSYSTEMMSGS_HDR
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/csgo-protobufs/csgo/gcsystemmsgs.proto
)

# 
# Curl
#
set(CURL_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/dependencies/curl/include")
set(CURL_LIBRARY "${CMAKE_SOURCE_DIR}/dependencies/curl/lib/libcurl.lib")
find_package(CURL REQUIRED)

#
# RapidJson
#
set(RAPIDJSON_INCLUDEDIR "${CMAKE_SOURCE_DIR}/dependencies/rapidjson/include")
find_package(RAPIDJSON REQUIRED)

#
# Copy Additional Files
#
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/README.md"                "build/readme.txt"    COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"                  "build/license.txt"   COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/update.bat"               "build/update.bat"    COPYONLY)
configure_file("${CMAKE_SOURCE_DIR}/dependencies/curl/bin/cacert.pem" "build/cacert.pem"    COPYONLY)

#
# Add Exe Resources
#
if(WIN32)
    set(ADDITIONAL_RESOURCES "res/CSGO_CLI.rc")
else()
    set(ADDITIONAL_RESOURCES "")
endif()

add_executable(csgo_cli
    # icon
        ${ADDITIONAL_RESOURCES}

    # application files
        src/main.cpp
        src/VersionAndConstants.h
        src/CSGOMatchData.h
        src/CSGOClient.h
        src/CSGOClient.cpp
        src/GCMsgHandler.h
        src/CSGOMMHello.h
        src/CSGOMMHello.cpp
        src/CSGOMatchPlayerScore.h
        src/CSGOMatchPlayerScore.cpp
        src/CSGOMMLinkObject.h
        src/CSGOMMLinkObject.cpp
        src/CSGOMatchList.h
        src/CSGOMatchList.cpp   
        src/ExceptionHandler.h
        src/ExceptionHandler.cpp
        src/SteamId.h
        src/ShareCode.h
        src/ShareCodeUpload.h

    # Generated Protobuf Includes
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
    "${STEAMWORKS_INCLUDE_DIR}" 
    "${PROTOBUF_INCLUDE_DIR}"
    "${MPIR_INCLUDE_DIR}"
    "${CURL_INCLUDE_DIRS}"
	"${RAPIDJSON_INCLUDE_DIRS}"
)

target_link_libraries(csgo_cli
    "${STEAMWORKS_LIBRARY}" 
    "${PROTOBUF_LIBRARY}"
    "${MPIR_LIBRARIES}"
    "${CURL_LIBRARIES}"
)

#
# Create "source-group" folders for these files in Visual Studio
#
source_group("Protobuf Protocol Files" FILES
    proto/cstrike15_gcmessages.proto
    proto/steammessages.proto
    proto/gcsdk_gcmessages.proto
    proto/gcsystemmsgs.proto
)
source_group("Protobuf Generated Files" FILES
    ${PROTO_CSTRIKE15_GCMESSAGES_SRC}
    ${PROTO_CSTRIKE15_GCMESSAGES_HDR}
    ${PROTO_STEAMMESSAGES_SRC}
    ${PROTO_STEAMMESSAGES_HDR}
    ${PROTO_GCSDK_GCMESSAGES_SRC}
    ${PROTO_GCSDK_GCMESSAGES_HDR}
    ${PROTO_GCSYSTEMMSGS_SRC}
    ${PROTO_GCSYSTEMMSGS_HDR}
)

set(FILES_TO_DEPLOY
    # curl
    "${CMAKE_SOURCE_DIR}/dependencies/curl/bin/libcurl.dll"
    # mpir
    "${CMAKE_SOURCE_DIR}/dependencies/mpir/bin/mpir.dll"
    # openssl
    "${CMAKE_SOURCE_DIR}/dependencies/openssl/bin/libeay32.dll"
    "${CMAKE_SOURCE_DIR}/dependencies/openssl/bin/ssleay32.dll"
    # zlib
    "${CMAKE_SOURCE_DIR}/dependencies/zlib/bin/zlib1.dll"
)
INSTALL(FILES ${FILES_TO_DEPLOY} DESTINATION "${CMAKE_SOURCE_DIR}/build")