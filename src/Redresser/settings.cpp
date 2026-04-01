#include "settings.h"
#include <SimpleIni.h>


void Settings::ParseList(const std::string& raw, std::set<std::string>& out)
{
    out.clear();
    std::stringstream ss(raw);
    std::string token;
    while (std::getline(ss, token, ',')) {
        auto start = token.find_first_not_of(" \t");
        auto end = token.find_last_not_of(" \t");
        token = (start != std::string::npos) ? token.substr(start, end - start + 1) : "";
        if (!token.empty()){
            std::transform(token.begin(), token.end(), token.begin(), ::tolower);
            out.insert(token);
        }
    }
}


void Settings::LoadRedressFiles()
{
    const std::filesystem::path dataDir("Data");

    for (const auto& entry : std::filesystem::directory_iterator(dataDir)) {
        if (entry.path().extension() != ".ini")
            continue;
        if (!entry.path().stem().string().ends_with("_REDRESS"))
            continue;

        CSimpleIniA ini;
        ini.SetUnicode();
        if (ini.LoadFile(entry.path().string().c_str()) < 0) {
            logger::warn("Failed to load {}", entry.path().string());
            continue;
        }

        std::set<std::string> iniKeywords, iniFactions;
        ParseList(ini.GetValue("Redress", "ExcludedFactions", ""), iniFactions);
        ParseList(ini.GetValue("Redress", "ExcludedKeywords", ""), iniKeywords);

        loadedFactions.insert(iniFactions.begin(), iniFactions.end());
        loadedKeywords.insert(iniKeywords.begin(), iniKeywords.end());

        logger::info("Loaded {}", entry.path().filename().string());
    }
}


void Settings::LoadConfig()
{
    constexpr auto path = "Data/SKSE/Plugins/Redresser.ini";

    CSimpleIniA ini;
    ini.SetUnicode();

    if (ini.LoadFile(path) < 0) {
        return;
    }

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

    auto rawExcludedFactions = ini.GetValue("General", "ExcludedFactions", "");
    ParseList(rawExcludedFactions, excludedFactions);
    // Adding the factions from loaded ini files
    excludedFactions.insert(loadedFactions.begin(), loadedFactions.end());

    auto rawExcludedKeywords = ini.GetValue("General", "ExcludedKeywords", "");
    ParseList(rawExcludedKeywords, excludedKeywords);
    // Adding the keywords from loaded ini files
    excludedKeywords.insert(loadedKeywords.begin(), loadedKeywords.end());   
}