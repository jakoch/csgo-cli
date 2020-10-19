@echo off

set VCPKG_ROOT = "e:\tools\vcpkg"
::set "VCPKG_DEFAULT_TRIPLET=x64-windows"
set "VCPKG_DEFAULT_TRIPLET=x64-windows-static"

call %VCPKG_ROOT%\vcpkg install --feature-flags=manifests

call %VCPKG_ROOT%\vcpkg update --feature-flags=manifests

pause