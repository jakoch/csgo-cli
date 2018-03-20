# csgo-cli [![Appveyor Build status](https://ci.appveyor.com/api/projects/status/github/jakoch/csgo-cli?branch=master&svg=true)](https://ci.appveyor.com/project/jakoch/csgo-cli)

This console tool enables you to output informations of your CS:GO user profile and latest matches.

## How does this work internally?

The tool connects to your running Steam as CS:GO game client (SteamApp 730).
It communicates with the Steam API to request the serialized player and match infos.
The structure of the serialized data is described by the csgo-protobufs.
These infos are then deserialized using Protobuf and placed into 
iteratable objects for further processing and output.

# Todo

- [x] CI setup: appveyor.yaml
  - [x] cmake
  - [x] VS15 x64
- [x] vendor dependencies
  - [x] steam sdk
    - [x] encrypt
    - [x] decrypt on appveyor via env secret
  - [x] protobuf
  - [x] cs:go protobufs
    - [x] patch cs:go protobuf syntax
  - [x] mpir
  - [ ] curl?
- [x] package and deploy the application
- [x] package and deploy the dependencies
  - [x] exclude sw_sdk (not redistributable), ship encrypted version
- [x] connect as CSGO GameClient to Steam
- [ ] fetch player info
  - [ ] calculate steamids
- [x] fetch latest matches
  - [x] calculate demo share-codes
- [ ] store matches locally (json, cvs, sqlite)?
- [ ] upload demo share-codes to csgostats.gg
