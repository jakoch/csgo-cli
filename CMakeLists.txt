cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(CSGO_CLI)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
include(FindPackageHandleStandardArgs)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(CMAKE_AUTOMOC ON)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++11")
    set(CMAKE_EXE_LINKER_FLAGS "-l protobuf -pthread")
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")
endif()

#
# Setup Dependencies
#

#
# Steamworks SDK
#
set(STEAMWORKS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/dependencies/sw_sdk/public/steam")

configure_file("${CMAKE_SOURCE_DIR}/bin/steam_appid.txt" "bin/steam_appid.txt" COPYONLY)

set(STEAMWORKS_REDISTBIN_DIR "${CMAKE_SOURCE_DIR}/dependencies/sw_sdk/redistributable_bin")

if(WIN32)
    set(STEAMWORKS_LIBRARY "${STEAMWORKS_REDISTBIN_DIR}/win64/steam_api64.lib")
    configure_file("${STEAMWORKS_REDISTBIN_DIR}/win64/steam_api64.dll" "bin/steam_api.dll" COPYONLY)
else()
    set(STEAMWORKS_LIBRARY "${STEAMWORKS_REDISTBIN_DIR}/linux64/libsteam_api.so")
    configure_file("${STEAMWORKS_LIBRARY}" "bin/libsteam_api.so" COPYONLY)
endif()

find_package(Steamworks REQUIRED)

#
# Google Protobuf
#
set(PROTOBUF_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/dependencies/protobuf/src")

if(WIN32)
    set(PROTOBUF_LIBRARY "${CMAKE_SOURCE_DIR}/dependencies/protobuf/cmake/Release/libprotobuf.lib")
    set(PROTOBUF_LIBRARY_DEBUG "${CMAKE_SOURCE_DIR}/dependencies/protobuf/cmake/Debug/libprotobuf.lib")
endif()

if(WIN32)
    set(PROTOBUF_PROTOC_EXECUTABLE "${CMAKE_SOURCE_DIR}/dependencies/protobuf/cmake/Release/protoc.exe")
else()
    set(PROTOBUF_PROTOC_EXECUTABLE "${CMAKE_SOURCE_DIR}/dependencies/protobuf/cmake/Release/protoc")
endif()

find_package(Protobuf REQUIRED)

set(libprotobuf optimized "${PROTOBUF_LIBRARY}" debug "${PROTOBUF_LIBRARY_DEBUG}")

#
# Process CS:GO Protobufs
#
PROTOBUF_GENERATE_CPP(PROTO_CSTRIKE15_GCMESSAGES_SRC  PROTO_CSTRIKE15_GCMESSAGES_HDR  ${CMAKE_CURRENT_SOURCE_DIR}/proto/cstrike15_gcmessages.proto)
PROTOBUF_GENERATE_CPP(PROTO_STEAMMESSAGES_SRC         PROTO_STEAMMESSAGES_HDR         ${CMAKE_CURRENT_SOURCE_DIR}/proto/steammessages.proto)
PROTOBUF_GENERATE_CPP(PROTO_GCSDK_GCMESSAGES_SRC      PROTO_GCSDK_GCMESSAGES_HDR      ${CMAKE_CURRENT_SOURCE_DIR}/proto/gcsdk_gcmessages.proto)
PROTOBUF_GENERATE_CPP(PROTO_GCSYSTEMMSGS_SRC          PROTO_GCSYSTEMMSGS_HDR          ${CMAKE_CURRENT_SOURCE_DIR}/proto/gcsystemmsgs.proto)

#
# Copy Additional Files
#
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/README.md"  "bin/readme-${PROJECT_NAME}.txt"  COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md" "bin/license-${PROJECT_NAME}.txt" COPYONLY)

#
# Add Exe Resources
#
if(WIN32)
    set(ADDITIONAL_RESOURCES "res/CSGO_CLI.rc")
else()
    set(ADDITIONAL_RESOURCES "")
endif()

add_executable(CSGO_CLI
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

    # CS:GO Protobufs
        proto/cstrike15_gcmessages.proto
        proto/steammessages.proto
        proto/gcsdk_gcmessages.proto
        proto/gcsystemmsgs.proto

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
)

target_link_libraries(CSGO_CLI 
    "${STEAMWORKS_LIBRARY}" 
    "${libprotobuf}"
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
