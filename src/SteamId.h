#ifndef SteamID_H
#define SteamID_H

// 76561197960493477 -> 113874
uint32 calcSteamID32(uint64 steamID64)
{
    const uint64 steamID64Base = 76561197960265728;
    uint32 iSteamID32 = (steamID64 - steamID64Base);
    return iSteamID32;
}

// steam3ID: 76561197960493477 -> [U:1:227749]
std::string toSteamID32(uint64 steamID64)
{
    std::stringstream ss;
    ss << "[U:1:";
    ss << calcSteamID32(steamID64);
    ss << "]";
    return ss.str();
}

// steamID32: STEAM_0:1:113874
std::string toSteamIDClassic(uint64 steamID64)
{
    // modulus operator = modulo(a,2)
    int a = (steamID64 % 2 == 0) ? 0 : 1;

    int b = std::floor(calcSteamID32(steamID64) / 2);

    std::stringstream ss;
    ss << "STEAM_0:" << a << ":" << b;
    return ss.str();
}

#endif