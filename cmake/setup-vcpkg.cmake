
# Please set VCPKG_ROOT: export VCPKG_ROOT=/opt/vcpkg/bin
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

if(NOT DEFINED ENV{VCPKG_FEATURE_FLAGS})
    set(ENV{VCPKG_FEATURE_FLAGS} "manifests,binarycaching")
endif()

# The VCPKG_DEFAULT_TRIPLET is automatically set by vcpkg.cmake.
# If you want to build for other platforms, e.g. build for Android on Windows-x64 (canadian-cross builds),
# please set VCPKG_TARGET_TRIPLET as env var: export VCPKG_TARGET_TRIPLET=x64-linux
#
if(DEFINED ENV{VCPKG_TARGET_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_TARGET_TRIPLET}" CACHE STRING "")
endif()

# VCPKG_DIR is the root folder for all compiled packages, e.g. /project/vcpkg_installed/x64-linux
# If may use this to set "Find" locations,
#   set(LibXY_INCLUDE_DIR "${VCPKG_DIR}/include")
#   set(LibXY_LIBRARY     "${VCPKG_DIR}/lib/libxy.lib")
#   set(CURL_DIR          "${VCPKG_DIR}/share/curl")
if(NOT DEFINED VCPKG_DIR)
    set(VCPKG_DIR "${CMAKE_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}")
endif()

message(STATUS "[VCPKG] VCPKG_ROOT              -> '$ENV{VCPKG_ROOT}'")
message(STATUS "[VCPKG] CMAKE_TOOLCHAIN_FILE    -> '${CMAKE_TOOLCHAIN_FILE}'")
message(STATUS "[VCPKG] VCPKG_TARGET_TRIPLET    -> '${VCPKG_TARGET_TRIPLET}'")
message(STATUS "[VCPKG] VCPKG_DIR               -> '${VCPKG_DIR}'")
message(STATUS "[VCPKG] VCPKG_MANIFEST_FILE     -> '${VCPKG_MANIFEST_FILE}'")
message(STATUS "[VCPKG] VCPKG_MANIFEST_INSTALL  -> '${VCPKG_MANIFEST_INSTALL}'")
