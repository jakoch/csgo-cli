message(STATUS "---------------------------------------------------------------")
message(STATUS "[ 🛠️ ] Building Unit Tests")
message(STATUS "---------------------------------------------------------------")

#-------------------------------------------------------------------
# Setup Catch Test Library
#-------------------------------------------------------------------

set(Catch2_DIR "${VCPKG_DIR}/share/catch2")

find_package(Catch2 CONFIG REQUIRED)

include(Catch)

#-------------------------------------------------------------------
# Setup CMake's CTest
#-------------------------------------------------------------------

# remove superflous additionaly added build targets (continous, nightly, etc.)
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)

set(CTEST_OUTPUT_ON_FAILURE TRUE)

include(CTest)
enable_testing()

#-------------------------------------------------------------------
# Build Target: all_tests                              (executable)
#-------------------------------------------------------------------

set(UNIT_TEST_SOURCES
  main.tests.cpp
  ShareCodeUpload.tests.cpp
  CrosshairShareCode.test.cpp
  CSGOMatchData.test.cpp
)

add_executable(all_tests ${UNIT_TEST_SOURCES})

target_include_directories(all_tests
  PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include"
)

target_compile_features(all_tests
  PUBLIC
    cxx_std_20
)

if(MSVC)
    target_compile_options(all_tests PRIVATE /utf-8)
endif()

set_property(TARGET all_tests PROPERTY
      MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_link_libraries(all_tests
  PRIVATE
    csgo_cli_lib
    Catch2::Catch2
)

add_test(NAME all_tests COMMAND all_tests)

#-------------------------------------------------------------------
# Display Target Properties
#-------------------------------------------------------------------

show_build_target_properties(all_tests)

#-------------------------------------------------------------------
# Install
#-------------------------------------------------------------------

# Optional: Uncomment if you want to install the test binary
# install(
#   TARGETS test_suite
#   DESTINATION test_suite-${APP_VERSION}
# )

#-------------------------------------------------------------------
# Setup Catch's Test Discovery
#-------------------------------------------------------------------

if(DEFINED ENV{GITHUB_ACTIONS} AND "$ENV{GITHUB_ACTIONS}" STREQUAL "true")
  # if tests are running on CI server, create an xml test report
  catch_discover_tests(all_tests
    REPORTER        junit
    OUTPUT_DIR      ${CMAKE_CURRENT_BINARY_DIR}
    OUTPUT_SUFFIX   test_results_${VCPKG_TARGET_TRIPLET}_${CMAKE_BUILD_TYPE}.junit.xml
  )
else()
  # if tests are running on local machine, print test results to console
  catch_discover_tests(all_tests REPORTER console)
endif()
