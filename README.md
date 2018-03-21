# csgo-cli [![Appveyor Build status](https://ci.appveyor.com/api/projects/status/github/jakoch/csgo-cli?branch=master&svg=true)](https://ci.appveyor.com/project/jakoch/csgo-cli)

This console tool enables you to output informations of your CS:GO user profile and latest matches.

## How does this work internally?

The tool connects to your running Steam as CS:GO game client (SteamApp 730).
It communicates with the Steam API to request the serialized player and match infos.
The structure of the serialized data is described by the csgo-protobufs.
These infos are then deserialized using Protobuf and placed into 
iteratable objects for further processing and output.

## Dependencies

- [Steamworks SDK](https://partner.steamgames.com/) [API Docs](https://partner.steamgames.com/doc/sdk/api)
  - The official Steamworks SDK is used for making the Steam API connection and requesting data.
- [Google Protobuf](https://developers.google.com/protocol-buffers/)
  - A language-neutral, platform-neutral, extensible mechanism for serializing structured data.
  - Protobuf is used for deserializing the player and match infos. 
- [CS:GO Protobufs](https://github.com/SteamDatabase/Protobufs/tree/master/csgo)
  - The csgo-protobufs are provided by the [steamdb.info](https://steamdb.info/) project.
  - The Protobuf Compiler (protoc) is used during the build process to compile the csgo-protobufs to C++ objects for data access.
- [MPIR](http://mpir.org/)
  - MPIR is a highly optimised library for bignum arithmetic forked from the GMP bignum library.
  - MPIR is used to calculate the demo share-code. Because, it's a BigInt, before the Base-57 decoding is applied.
  - Handling the BigInt with bitshifting might be possible. But i'm too lazy to figure it out.

##### Download Dependencies

You might download the pre-compiled dependencies package from Appveyor. 
The package excludes the Steamworks SDK, which source is non-redistributable.

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
