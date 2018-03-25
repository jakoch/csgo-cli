# csgo-cli [![Appveyor Build status](https://ci.appveyor.com/api/projects/status/github/jakoch/csgo-cli?branch=master&svg=true)](https://ci.appveyor.com/project/jakoch/csgo-cli)

This console tool enables you to output informations of your CS:GO user profile and latest matches.
You can also use the tool to upload demo sharecodes to csgostats.gg.

## Usage



## Example Output



## Automatic Upload of Demo ShareCodes to http://csgostats.gg/

The ShareCode is an URL, which you might pass around to your friends.

It looks like so: steam://rungame/730/76561202255233023/+csgo_download_match%20CSGO-xxxxx-xxxxx-xxxxx-xxxxx-xxxxx

If you click on this URL or open it in a browser tab it will tell Steam to launch CS:GO,
automatically download the demo and open it for replay.
The code at the end of the URL is the id of the match.

The tool requests your latest matches and builds the ShareCodes for the demos.
The ShareCodes are then automatically uploaded to https://csgostats.gg/

This avoids the manual posting of the sharecode via the csgostats webinterface
or the posting to the csgostats steam group, where the bots pick it up.
https://csgostats.gg/getting-the-sharecode

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
- [RapidJson](http://rapidjson.org/)
  - RapidJson is a fast JSON parser/generator for C++ with both SAX/DOM style API.
  - It is used to parse the JSON responses from the csgostats server (error, complete, queued).
- [CMake](https://cmake.org/)
  - CMake is used as build tool.
- [Appveyor](https://www.appveyor.com/)
  - Appveyor is used as build and deployment service.

##### Download Dependencies

You might download the pre-compiled dependencies package from Appveyor. 
The package excludes the Steamworks SDK, whose source is non-redistributable.

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
  - [x] curl
  - [x] rapidjson
- [x] package and deploy the application
- [x] package and deploy the dependencies
  - [x] exclude sw_sdk (not redistributable), ship encrypted version
- [x] connect as CSGO GameClient to Steam
- [x] fetch player info
  - [x] calculate steamids
- [x] fetch latest matches
  - [x] calculate demo share-codes
- [x] upload demo share-codes to csgostats.gg
  - [x] curl based share-code uploader
  - [x] rapid-json based response parser
- [ ] store matches locally (json, cvs, sqlite) to avoid re-posting sharecodes
