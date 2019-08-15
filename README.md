# csgo-cli [![Appveyor Build status](https://ci.appveyor.com/api/projects/status/github/jakoch/csgo-cli?branch=master&svg=true)](https://ci.appveyor.com/project/jakoch/csgo-cli)

This console tool enables you to output informations of your CS:GO user profile and latest matches.
You can also use the tool to upload demo sharecodes to csgostats.gg.

## Requirements

- Windows
- Steam application running
- CS:GO application not running

## Usage

Display commands:

    csgo_cli 
    
Output:

    csgo_cli v1.0.3, https://github.com/jakoch/csgo-cli
    Copyright (c) 2018-2019 Jens A. Koch.

     CS:GO Console shows your user account, stats and latest matches.
     You can also use the tool to upload demo sharecodes to csgostats.gg.

    Usage:
      command [options] [arguments]

    Available commands:
      -user           Show your profile (SteamID, AccountID, MM-Rank, Likes, VAC-Status)
      -matches        Show your past matches in table form
      -upload         Upload your past matches to csgostats.gg
      -s, sharecode   Upload a demo sharecode to csgostats.gg

    Options:
      -h, help        Display this help message
      -v, verbose     Increase verbosity of messages
      -V, Version     Display application version
    
Fetch matches and upload to csgostats (or just use `update.bat`):
    
    csgo_cli -matches -upload
    
If you encounter any issues, please use the verbose mode for debugging purposes:

    csgo_cli -matches -upload -verbose
   
## How does this work internally?

The tool connects to your running Steam as CS:GO game client (SteamApp 730).
It communicates with the Steam API to request the serialized player and match infos.
The structure of the serialized data is described by the csgo-protobufs.
These infos are then deserialized using Protobuf and placed into 
iteratable objects for further processing and output.

#### Automatic Upload of Demo ShareCodes to http://csgostats.gg/

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

##### Development Notes: ShareCode Uploading to csgostats.gg

The file containing the logic for uploading the ShareCode is ShareCodeUpload.cpp.

uploadShareCode() uses cURL to POST the ShareCode.
Posting data to csgostats.gg is difficult, because the server is Cloudflare protected.
Even normal browsing behaviour can trigger a cloudflare redirect to a captcha page or a website ban.

Before we can POST one or multiple sharecodes, a GET request to csgostats.gg is needed to get a cURL connection handle, including all relevant cookies.
The cURL handle is then re-used for one or more POST requests (sending the cookies as header data and the sharecode(s) as post data).

The reponse is then parsed by processJsonResponse().
There are 4 response possibilites:
There is a HTML response by Cloudflare, the HTML captcha page.
There are 3 JSON response types by csgostats.gg: error, queued, complete. See testProcessJsonResponse()

For testing purposes: Posting a ShareCode to csgostats.gg using cURL on the CLI
 - `curl "https://csgostats.gg/match/upload/ajax" -H "accept-language: en" -H "content-type: application/x-www-form-urlencoded; charset=UTF-8" -H "accept: application/json, text/javascript, */*; q=0.01" -H "x-requested-with: XMLHttpRequest" --data "sharecode=CSGO-WSACM-qX5Gv-ikbi3-Z6uOW-TGwPB&index=0"`

## Dependencies

- [Steamworks SDK](https://partner.steamgames.com/) [API Docs](https://partner.steamgames.com/doc/sdk/api)
  - The official Steamworks SDK is used for making the Steam API connection and requesting data.
- [Google Protobuf](https://developers.google.com/protocol-buffers/)
  - A language-neutral, platform-neutral, extensible mechanism for serializing structured data.
  - Protobuf is used for deserializing the player and match infos. 
- [CS:GO Protobufs](https://github.com/SteamDatabase/Protobufs/tree/master/csgo)
  - The csgo-protobufs are provided by the [steamdb.info](https://steamdb.info/) project.
  - The Protobuf Compiler (protoc) is used during the build process to compile the csgo-protobufs to C++ objects for data access.
- [RapidJson](http://rapidjson.org/)
  - RapidJson is a fast JSON parser/generator for C++ with both SAX/DOM style API.
  - It is used to parse the JSON responses from the csgostats server (error, complete, queued).
- [ConsoleTable](https://github.com/766F6964/ConsoleTable) (inside /src)
  - ConsoleTable is used to print tables to the console.
- [Curl](https://github.com/curl/curl)
  - libcurl is used for transferring data.
- [CMake](https://cmake.org/)
  - CMake is used as build tool.
- [Appveyor](https://www.appveyor.com/)
  - Appveyor is used as build and deployment service.

##### Download Dependencies

You might download the pre-compiled dependencies package from Appveyor. 
The package excludes the Steamworks SDK, whose source is non-redistributable.

##### Release Checklist

Before releasing a new version:

- [ ] Update Version number in VersionAndConstants.h
- [ ] Update CHANGELOG.md

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
- [ ] request additional steam profile data via web-api
- [ ] colors on the CLI (LOSS red, WIN green)
