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
# Setup Test Suite                                 (app: test_suite)
#-------------------------------------------------------------------

set(TEST_SOURCES
    main.tests.cpp
    ShareCodeUpload.tests.cpp
)

include_directories(
    ./src
    ./src/commands
    ./src/csgo
    ./src/csgostats
    ./src/platform/windows
)

add_executable(test_suite ${TEST_SOURCES})

if(${CMAKE_VERSION} VERSION_LESS "3.21.0")
    target_compile_features(test_suite PUBLIC cxx_std_20)
else()
    target_compile_features(test_suite PUBLIC cxx_std_23)
endif()

target_link_libraries(test_suite
    PRIVATE
        Catch2::Catch2
        csgo_cli_lib
)

#-------------------------------------------------------------------
# Setup Catch's Test Discovery
#-------------------------------------------------------------------

if(DEFINED ENV{GITHUB_ACTIONS} AND "$ENV{GITHUB_ACTIONS}" STREQUAL "true")
  # if tests are running on CI server, create an xml test report
  catch_discover_tests(test_suite
    REPORTER        junit
    OUTPUT_DIR      ${CMAKE_CURRENT_BINARY_DIR}
    OUTPUT_SUFFIX   _test_report.junit.xml
  )
else()
  # if tests are running on local machine, print test results to console
  catch_discover_tests(test_suite REPORTER console)
endif()
