@echo off

echo "Patching CSGO Protobufs"

@echo ~dp0= %~dp0

echo "1. Copying the Protobuf Descriptor file"
mkdir "%~dp0..\csgo-protobufs\csgo\google\protobuf"

if("%GITHUB_WORKFLOW%"=="") (
    copy "%~dp0..\..\vcpkg_installed\x64-windows-static\include\google\protobuf\descriptor.proto" "%~dp0..\csgo-protobufs\csgo\google\protobuf"
) else (
    copy "C:\vcpkg\packages\x64-windows-static\include\google\protobuf\descriptor.proto" "%~dp0..\csgo-protobufs\csgo\google\protobuf"
)

echo "2. Copying the proto-syntax-patch.txt, which contains a new header line"
copy "%~dp0proto-syntax-patch.txt" "%~dp0..\csgo-protobufs\csgo"

echo "3. Prepend syntax patch to every file"
cd "%~dp0..\csgo-protobufs\csgo"
dir

for %%f in (*.proto) do (
    echo "Processing %%f"
    copy proto-syntax-patch.txt %%f.new
    REM echo "Appending original file content to file with first Line %%f.new"
    type %%f >>%%f.new
    REM echo "Rename/overwriting old file"
    move /y %%f.new %%f
)

del proto-syntax-patch.txt