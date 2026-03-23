#include "settings.h"
#include <SimpleIni.h>


void Settings::ParseList(const std::string& raw, std::vector<std::string>& out)
{
    out.clear();
    std::stringstream ss(raw);
    std::string token;
    while (std::getline(ss, token, ',')) {
        auto start = token.find_first_not_of(" \t");
        auto end = token.find_last_not_of(" \t");
        token = (start != std::string::npos) ? token.substr(start, end - start + 1) : "";
        if (!token.empty())
            out.push_back(token);
    }
}


void Settings::Initialize()
{
    constexpr auto path = "Data/SKSE/Plugins/Redresser.ini";

    CSimpleIniA ini;
    ini.SetUnicode();

    if (ini.LoadFile(path) < 0) {
        return;
    }

    // Initializing the settings
    debug = ini.GetBoolValue("General", "Debug", false);

    auto selectMode = ini.GetLongValue("General", "SelectedActors", 0);
    selectedActors = static_cast<Settings::NPCPool>(selectMode);

    dressDeadActors = ini.GetBoolValue("General", "DressDeadActors", true);

    dressAliveActors = ini.GetBoolValue("General", "DressAliveActors", true);

    equipMainSlotsOnly = ini.GetBoolValue("General", "EquipMainSlotsOnly", true);
    
    skipPlayerHome = ini.GetBoolValue("General", "SkipPlayerHome", true);

    skipActiveFollowers = ini.GetBoolValue("General", "SkipActiveFollowers", true);

    skipSleepingActors = ini.GetBoolValue("General", "SkipSleepingActors", true);
    
    giveDefaultItems = ini.GetBoolValue("General", "GiveDefaultItems", true);

    rawExcludedFactions = ini.GetValue("General", "ExcludedFactions", "");
    Settings::ParseList(rawExcludedFactions, excludedFactions);

    rawExcludedKeywords = ini.GetValue("General", "ExcludedKeywords", "");
    Settings::ParseList(rawExcludedKeywords, excludedKeywords);
}