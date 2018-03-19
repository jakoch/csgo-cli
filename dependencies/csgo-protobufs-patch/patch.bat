@echo off

REM 1. copy the Protobuf Descriptor file
xcopy "C:\Program Files (x86)\protobuf\include\google\protobuf\descriptor.proto" ..\csgo-protobufs\csgo\google\protobuf\descriptor.proto

REM 2. proto-syntax-patch.txt contains 1 header line
copy proto-syntax-patch.txt ..\csgo-protobufs\csgo

REM add an extension to every file
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
