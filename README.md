# csgo-cli

[![Appveyor Build status](https://ci.appveyor.com/api/projects/status/github/jakoch/csgo-cli?branch=master&svg=true)](https://ci.appveyor.com/project/jakoch/csgo-cli)
=======

# Todo

- [ ] CI setup: appveyor.yaml
  - [ ] cmake
  - [ ] VS15 x64
- [ ] vendor dependencies
  - [ ] steam sdk
    - [ ] encrypt
    - [ ] decrypt on appveyor via env secret
  - [ ] protobuf
  - [ ] cs:go protobufs
    - [ ] patch cs:go protobuf syntax
  - [ ] mpir
  - [ ] curl?
- [ ] package and deploy the application
- [ ] package and deploy the dependencies
  - [ ] exclude sw_sdk (not redistributable), ship encrypted version
- [ ] connect as CSGO GameClient to Steam
- [ ] fetch player info
  - [ ] calculate steamids
- [ ] fetch latest matches
  - [ ] calculate demo share-codes
- [ ] store matches locally (json, cvs, sqlite)?
- [ ] upload demo share-codes to csgostats.gg
