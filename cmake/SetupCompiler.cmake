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

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

  message(STATUS "\n-- Compiler and platform toolset:\n")

  # Report compiler and platform toolset
  message(STATUS "USING COMPILER: 				${CMAKE_CXX_COMPILER_ID}")
  message(STATUS "MSVC_VERSION: 				${MSVC_VERSION}")
  message(STATUS "MSVC_TOOLSET_VERSION: 		${MSVC_TOOLSET_VERSION}")
  message(STATUS "CMAKE_MSVC_RUNTIME_LIBRARY: 	${CMAKE_MSVC_RUNTIME_LIBRARY}")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")

  # MSVC warning suppressions
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  add_definitions(-D_SILENCE_CXX20_IS_POD_DEPRECATION_WARNING) # protobuf triggers this warning
  add_definitions(-DNOMINMAX)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /nologo")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /LARGEADDRESSAWARE") # Allow more than 2 GB in 32 bit application.
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
  # /MD:   Creates a multi-threaded DLL using MSVCRT.lib.
  # /MDd:  Creates a debug multi-threaded DLL using MSVCRTD.lib.
  # /O2:   Creates fast code.
  # /Od:   Disables optimization.
  # /Oi:   Generates intrinsic functions.
  # /RTC1: Enables run-time error checking.
  # /W4:   Sets which warning level to output.
  # /Zi:   Generates complete debugging information.
  # /JMC:  Just my code enables the VS debugger to step over internal (system, framework, library, and other non-user) calls.
  set(CMAKE_CXX_FLAGS           "${CMAKE_CXX_FLAGS} /EHsc /GS /Zi /Gm-") #/W4
  set(CMAKE_CXX_FLAGS_DEBUG     "${CMAKE_CXX_FLAGS_DEBUG} /Od /RTC1 /JMC")
  set(CMAKE_CXX_FLAGS_RELEASE   "${CMAKE_CXX_FLAGS_RELEASE} /GL /O2 /Oi")

  message(STATUS "USING THESE CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # MSVC warning suppressions
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  add_definitions(-D_SILENCE_CXX20_IS_POD_DEPRECATION_WARNING) # protobuf triggers this warning
  add_definitions(-DNOMINMAX)

  #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ ") # -lc++abi
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
