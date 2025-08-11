#
# SetupVcpkg
#
# This configures vcpkg using environment variables instead of a command-line options.
#
# https://github.com/microsoft/vcpkg/blob/master/docs/users/integration.md#using-an-environment-variable-instead-of-a-command-line-option
#
# Environment Variables: https://vcpkg.readthedocs.io/en/latest/users/config-environment/
#

if(DEFINED ENV{VCPKG_VERBOSE} AND NOT DEFINED VCPKG_VERBOSE)
    set(VCPKG_VERBOSE "$ENV{VCPKG_VERBOSE}" CACHE BOOL "")
endif()

#
# -- Automatic install of vcpkg dependencies.
#
# This is experimental.
# See https://github.com/Microsoft/vcpkg/issues/1653
#

set(VCPKG_APPLOCAL_DEPS_INSTALL ON)

# Copy dependencies into the output directory for executables.
if(DEFINED ENV{VCPKG_APPLOCAL_DEPS} AND NOT DEFINED VCPKG_APPLOCAL_DEPS)
    set(VCPKG_APPLOCAL_DEPS "$ENV{VCPKG_APPLOCAL_DEPS}" CACHE BOOL "")
endif()

# Copy dependencies into the install target directory for executables.
if(DEFINED ENV{X_VCPKG_APPLOCAL_DEPS_INSTALL} AND NOT DEFINED VCPKG_APPLOCAL_DEPS)

    # X_VCPKG_APPLOCAL_DEPS_INSTALL depends on CMake policy CMP0087
    if(POLICY CMP0087)
        cmake_policy(SET CMP0087 NEW)
    endif()

    set(X_VCPKG_APPLOCAL_DEPS_INSTALL "$ENV{X_VCPKG_APPLOCAL_DEPS_INSTALL}" CACHE BOOL "")
endif()

#
# -- Set "vcpkg.cmake" as CMAKE_TOOLCHAIN_FILE
#
# Please set VCPKG_ROOT on your env: export VCPKG_ROOT=/opt/vcpkg/bin
# This avoids passing it on the configure line: -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
#
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)

    # VCPKG_ROOT
    if(DEFINED VCPKG_ROOT AND EXISTS ${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
        message(STATUS "[VCPKG] Using system vcpkg at ${VCPKG_ROOT}: ${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        set(vcpkg_toolchain_file "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

    # ENV.VCPKG_ROOT
    elseif(DEFINED ENV{VCPKG_ROOT} AND EXISTS $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
        message(STATUS "[VCPKG] Using system vcpkg at ENV{VCPKG_ROOT}: $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        set(vcpkg_toolchain_file "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

    # VCPKG_INSTALLATION_ROOT is defined on Github Actions CI
    elseif (DEFINED ENV{VCPKG_INSTALLATION_ROOT} AND EXISTS $ENV{VCPKG_INSTALLATION_ROOT}/scripts/buildsystems/vcpkg.cmake)
    message(STATUS "[VCPKG] Using system vcpkg at Github Action VCPKG_INSTALLATION_ROOT: $ENV{VCPKG_INSTALLATION_ROOT}")
        set(vcpkg_toolchain_file $ENV{VCPKG_INSTALLATION_ROOT}/scripts/buildsystems/vcpkg.cmake)

    # otherwise VCPKG is missing and we install VCPKG from Github
    #else()
    #    message(STATUS "[vcpkg] Fetching latest VCPKG from Github...")
    #    include(FetchContent)
    #    FetchContent_Declare(vcpkg GIT_REPOSITORY https://github.com/microsoft/vcpkg.git)
    #    FetchContent_MakeAvailable(vcpkg)
    #    set(vcpkg_toolchain_file ${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake)
    endif()

    set(CMAKE_TOOLCHAIN_FILE ${vcpkg_toolchain_file})
endif()

iF(NOT DEFINED VCPKG_MANIFEST_FILE)
    set(VCPKG_MANIFEST_FILE "${CMAKE_SOURCE_DIR}/vcpkg.json")
endif()

# Must be before any find_package() calls
if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    # Choose triplet here:
    set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "Vcpkg target triplet")
endif()

# Add this file and the VCPKG_MANIFEST_FILE as a "vcpkg" source_group to the IDE.
# They are not automatically picked up and listed as "important project" files by IDEs, yet.
source_group("vcpkg" FILES
    "${CMAKE_SOURCE_DIR}/cmake/SetupVcpkg.cmake"
    "${CMAKE_SOURCE_DIR}/vcpkg.json"
)

#
# Check to make sure the VCPKG_TARGET_TRIPLET matches BUILD_SHARED_LIBS
#
if (DEFINED VCPKG_TARGET_TRIPLET)
    if ("${VCPKG_TARGET_TRIPLET}" MATCHES ".*-static")
        if (BUILD_SHARED_LIBS)
            message(FATAL_ERROR "When the VCPKG_TARGET_TRIPLET ends with '-static' the BUILD_SHARED_LIBS must be 'OFF'.")
        endif()
    else()
        if (NOT BUILD_SHARED_LIBS)
            message(FATAL_ERROR "When the VCPKG_TARGET_TRIPLET does not end with '-static' the BUILD_SHARED_LIBS must be 'ON'.")
        endif()
    endif()
endif()

#
# Print VCPKG configuration overview
#
message(STATUS "\n-- [VCPKG] Configuration Overview:\n")
message(STATUS "[VCPKG]  - VCPKG_VERBOSE             -> '${VCPKG_VERBOSE}'")
message(STATUS "[VCPKG]  - VCPKG_APPLOCAL_DEPS       -> '${VCPKG_APPLOCAL_DEPS}'")
message(STATUS "[VCPKG]  - E:VCPKG_FEATURE_FLAGS     -> '$ENV{VCPKG_FEATURE_FLAGS}'")
message(STATUS "[VCPKG]  - E:VCPKG_ROOT              -> '$ENV{VCPKG_ROOT}'")
message(STATUS "[VCPKG]  - CMAKE_TOOLCHAIN_FILE      -> '${CMAKE_TOOLCHAIN_FILE}'")
message(STATUS "[VCPKG]  - VCPKG_MANIFEST_FILE       -> '${VCPKG_MANIFEST_FILE}'")
message(STATUS "[VCPKG]  - VCPKG_TARGET_TRIPLET      -> '${VCPKG_TARGET_TRIPLET}'")
