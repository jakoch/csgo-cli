@echo off

@REM UPDATE GIT SUBMODULES

git submodule update --init --recursive

call dependencies\csgo-protobufs-patch\patch.bat

@REM UPDATE VCPKG

PUSHD %cd%

cd f:\tools\vcpkg
git pull
call bootstrap-vcpkg.bat

POPD

@REM vcpkg doesnt support update in manifest mode WTF
@REM f:\tools\vcpkg\vcpkg update --triplet=x64-windows-static

f:\tools\vcpkg\vcpkg install --triplet=x64-windows-static