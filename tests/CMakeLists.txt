include(CTest)
enable_testing()

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

set(Catch2_DIR "${VCPKG_DIR}/share/catch2")
find_package(Catch2 CONFIG REQUIRED)

add_executable(TestSuite ${TEST_SOURCES})

target_link_libraries(TestSuite
    PRIVATE
        Catch2::Catch2
        csgo_cli_lib
)