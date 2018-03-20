@echo off

REM 1. copy the Protobuf Descriptor file
mkdir "%~dp0..\csgo-protobufs\csgo\google\protobuf"
copy "C:\Program Files\protobuf\include\google\protobuf\descriptor.proto" "%~dp0..\csgo-protobufs\csgo\google\protobuf"

REM 2. proto-syntax-patch.txt contains 1 header line
copy "%~dp0proto-syntax-patch.txt" "%~dp0..\csgo-protobufs\csgo"

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
