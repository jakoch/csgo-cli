# csgo-cli

<<<<<<< Updated upstream
[![Appveyor Build status](https://ci.appveyor.com/api/projects/status/github/jakoch/csgo-cli?branch=master&svg=true)](https://ci.appveyor.com/project/jakoch/csgo-cli)
=======
>>>>>>> Stashed changes

# Todo

- [ ] CI setup: appveyor.yaml
  - [ ] cmake
  - [ ] VS15 x64
- [ ] vendor dependencies
  - [ ] steam sdk
  - [ ] protobuf
  - [ ] cs:go protobufs
  - [ ] curl
  - [ ] mpir
- [ ] connect as CSGO GameClient to Steam
- [ ] fetch player info
- [ ] fetch latest matches
  - [ ] calculate demo share-codes
- [ ] store matches locally (json, cvs, sqlite)?
- [ ] upload demo share-codes to csgostats.gg !

CSGO Protobufs

- https://github.com/SteamDatabase/Protobufs/tree/master/csgo

How to make a sparse checkout of the CSGO Protobuf files from the SteamDatabase/Protobufs repo?

git init game-tracking
cd game-tracking
git remote add origin https://github.com/SteamDatabase/Protobufs.git
git config core.sparseCheckout true
echo Protobufs/csgo/ >> .git/info/sparse-checkout
git pull --depth=1 origin master

https://github.com/SteamDatabase/Protobufs.git