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


void Settings::ParseFactionSlotEntry(const std::string& raw, std::map<std::string, uint32_t>& out)
{
    auto sep = raw.find('|');
    if (sep == std::string::npos)
        return;

    std::string faction = raw.substr(0, sep);
    auto start = faction.find_first_not_of(" \t");
    auto end = faction.find_last_not_of(" \t");
    if (start == std::string::npos)
        return;
    faction = faction.substr(start, end - start + 1);
    std::transform(faction.begin(), faction.end(), faction.begin(), ::tolower);

    uint32_t mask = 0;
    std::stringstream ss(raw.substr(sep + 1));
    std::string token;
    while (std::getline(ss, token, ',')) {
        auto ts = token.find_first_not_of(" \t");
        auto te = token.find_last_not_of(" \t");
        if (ts == std::string::npos)
            continue;
        token = token.substr(ts, te - ts + 1);
        try {
            int slotNum = std::stoi(token);
            if (slotNum >= 30 && slotNum <= 61)
                mask |= (1u << (slotNum - 30));
            else
                logger::warn("FactionSlotsIgnore: slot {} out of range (30-61), skipping", slotNum);
        } catch (...) {
            logger::warn("FactionSlotsIgnore: invalid slot '{}', skipping", token);
        }
    }

    if (!faction.empty() && mask != 0)
        out[faction] |= mask;
}


void Settings::LoadRedressFiles()
{
    loadedFactionSlots.clear();

    const std::filesystem::path dataDir("Data");

    for (const auto& entry : std::filesystem::directory_iterator(dataDir)) {
        if (entry.path().extension() != ".ini")
            continue;
        if (!entry.path().stem().string().ends_with("_REDRESS"))
            continue;

        CSimpleIniA ini;
        ini.SetUnicode();
        ini.SetMultiKey(true);
        if (ini.LoadFile(entry.path().string().c_str()) < 0) {
            logger::warn("Failed to load {}", entry.path().string());
            continue;
        }

        std::set<std::string> iniKeywords, iniFactions;
        ParseList(ini.GetValue("Redress", "ExcludedFactions", ""), iniFactions);
        ParseList(ini.GetValue("Redress", "ExcludedKeywords", ""), iniKeywords);

        loadedFactions.insert(iniFactions.begin(), iniFactions.end());
        loadedKeywords.insert(iniKeywords.begin(), iniKeywords.end());

        CSimpleIniA::TNamesDepend slotEntries;
        if (ini.GetAllValues("Redress", "FactionSlotsIgnore", slotEntries)) {
            for (const auto& entry : slotEntries)
                ParseFactionSlotEntry(entry.pItem, loadedFactionSlots);
        }

        logger::info("Loaded {}", entry.path().filename().string());
    }
}


void Settings::LoadConfig()
{
    constexpr auto path = "Data/SKSE/Plugins/Redresser.ini";

    CSimpleIniA ini;
    ini.SetUnicode();
    ini.SetMultiKey(true);

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

    factionSlotsIgnore.clear();
    CSimpleIniA::TNamesDepend slotEntries;
    if (ini.GetAllValues("General", "FactionSlotsIgnore", slotEntries)) {
        for (const auto& entry : slotEntries)
            ParseFactionSlotEntry(entry.pItem, factionSlotsIgnore);
    }
    // Merging entries from loaded ini files
    for (const auto& [faction, mask] : loadedFactionSlots)
        factionSlotsIgnore[faction] |= mask;
}