#-------------------------------------------------------------------
# Setup Catch Test Library
#-------------------------------------------------------------------

set(Catch2_DIR "${VCPKG_DIR}/share/Catch2")
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
# Build Target: test_suite                              (executable)
#-------------------------------------------------------------------

add_executable(test_suite)

target_sources(test_suite
  PRIVATE
    #main.tests.cpp
    ShareCodeUpload.tests.cpp
)

target_include_directories(test_suite
  PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
)

target_compile_features(test_suite
  PUBLIC
    cxx_std_20
)

if(MSVC)
    target_compile_options(test_suite PRIVATE /utf-8)
endif()

target_link_libraries(test_suite
  PRIVATE
    Catch2::Catch2WithMain
    csgo_cli_lib
    # Catch2::Catch2   # Optional alternative
)

#-------------------------------------------------------------------
# Display Target Properties
#-------------------------------------------------------------------

show_build_target_properties(test_suite)

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
  catch_discover_tests(test_suite
    REPORTER        junit
    OUTPUT_DIR      ${CMAKE_CURRENT_BINARY_DIR}
    OUTPUT_SUFFIX   test_results_${VCPKG_TARGET_TRIPLET}_${CMAKE_BUILD_TYPE}.junit.xml
  )
else()
  # if tests are running on local machine, print test results to console
  catch_discover_tests(test_suite REPORTER console)
endif()
