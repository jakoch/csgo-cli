# csgo-cli

[![Appveyor Build status](https://ci.appveyor.com/api/projects/status/github/jakoch/csgo-cli?branch=master&svg=true)](https://ci.appveyor.com/project/jakoch/csgo-cli)
=======

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
