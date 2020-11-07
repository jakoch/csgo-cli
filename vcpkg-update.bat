@echo off
set VCPKG_ROOT = "e:\tools\vcpkg"

:: update VCPKG by pulling in latest "PortFile" changes from the GIT repository
pushd "%VCPKG_ROOT%"
git pull
popd

set "VCPKG_DEFAULT_TRIPLET=x64-windows-static"
call %VCPKG_ROOT%\vcpkg install --feature-flags=manifests
call %VCPKG_ROOT%\vcpkg update --feature-flags=manifests

pause