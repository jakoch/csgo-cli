# Changelog
All notable changes to this project will be documented in this file.

## [Unreleased]

- added rank information for wingman and dangerzone to "-user"
- added GlobalStats command
- replaced RapidJson with nlohmann::json
- added GameTypes enum for mapnames, not sure if this is correct
- added tabulate to dependencies for table rendering
  - dropped ConsoleTable and TableFormat
- large code refactoring
  - new src folder structure, including /commands folder
- added display of "xp_percentage progress" for "current xp level"
- added display of "clan_name" and "clan_tag" to "-user"
- fixed the calculation and display of "current_player_xp"
- fixed crash after printing "-matches"
- renamed all occurances of "demo" to "replay"
- added fmt to dependencies
  - added cli color support
  - switch output printing from std::cout to fmt::print/fmt::format

## [1.2.0] - 2020-11-05

- added dns cache prepopulation to curl request handling
- refactored uploadShareCode function to do a single POST request
- changed compilation triplet from "x64-windows-dynamic" to "x64-windows-static"
  - all 3th-party dependencies are now baked-in
  - except redistributable steam64.dll, cacert.pem
- switched to automatic c++ dependency management using vcpkg
  - see vcpkg.json: rapidjson, zlib, curl, protobuf
- updated sw_sdk to v150
  - switched from secure-file to password protected zip for sw_sdk
- updated csgo protobufs

## [1.1.0] - 2019-08-23

- added ShareCodeCache to avoid re-posting sharecodes, [#2]
- fixes: tabs 2 spaces, removed invalid includes

## [1.0.5] - 2019-07-15

- Sharecode construction without MPIR dependency, [PR#5]

## [1.0.4] - 2019-07-13

- added missing comma in rank list
- added padding with zeros on match results output, [PR#4]

## [1.0.3] - 2019-07-01

### Bugfix
- Posting demo sharecodes to csgostats.gg (implemented two step request ), [#1]
- fixed curl multi-request and SSL issues

### Added
- ConsoleTable for table printing on the console

### Changed
- updated dependency: sw_sdk to v144
- updated dependency: protobuf to v3.7.0
- updated dependency: csgo-protobuf to #ab60446
- refactoring

## [1.0.2]

- skipped release

## [1.0.1] - 2018-07-14
### Added
- added deployment of `csgo-cli-dependencies.zip`

## [1.0.0] - 2018-04-22
### Added
- Initial Release

[Unreleased]: https://github.com/jakoch/csgo-cli/compare/v1.2.0...HEAD
[1.2.0]: https://github.com/jakoch/csgo-cli/compare/v1.1.0...v1.2.0
[1.1.0]: https://github.com/jakoch/csgo-cli/compare/v1.0.5...v1.1.0
[1.0.5]: https://github.com/jakoch/csgo-cli/compare/v1.0.4...v1.0.5
[1.0.4]: https://github.com/jakoch/csgo-cli/compare/v1.0.3...v1.0.4
[1.0.3]: https://github.com/jakoch/csgo-cli/compare/v1.0.1...v1.0.3
[1.0.1]: https://github.com/jakoch/csgo-cli/compare/v1.0.0...v1.0.1
[1.0.0]: https://github.com/jakoch/csgo-cli/releases/tag/v1.0.0

[#1]: https://github.com/jakoch/csgo-cli/issues/1
[#2]: https://github.com/jakoch/csgo-cli/issues/2
[PR#4]: https://github.com/jakoch/csgo-cli/pull/4
[PR#5]: https://github.com/jakoch/csgo-cli/pull/5
