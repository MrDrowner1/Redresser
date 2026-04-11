#include "settings.h"
#include <SimpleIni.h>
#include <fstream>


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
    loadedFactions.clear();
    loadedKeywords.clear();
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
        GenerateDefaultIni(path);
        if (ini.LoadFile(path) < 0)
            return;
    }

    debug = ini.GetBoolValue("General", "Debug", false);

    auto selectMode = ini.GetLongValue("General", "SelectedActors", 0);
    selectedActors = static_cast<Settings::NPCPool>(selectMode);

    dressDeadActors = ini.GetBoolValue("General", "DressDeadActors", true);

    dressAliveActors = ini.GetBoolValue("General", "DressAliveActors", true);

    equipMainSlotsOnly = ini.GetBoolValue("General", "EquipMainSlotsOnly", false);
    
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


void Settings::GenerateDefaultIni(const char* path)
{
    std::filesystem::path filePath(path);
    std::error_code ec;
    std::filesystem::create_directories(filePath.parent_path(), ec);

    std::ofstream f(filePath);
    if (!f) {
        logger::warn("Failed to generate default ini at {}", path);
        return;
    }

    f << "[General]\n"
      << "; Verbose messages in the console about what this mod does.\n"
      << "Debug = false\n"
      << "\n"
      << "; Gives NPCs default clothes if they don't have anything to cover themselves with.\n"
      << "GiveDefaultItems = true\n"
      << "\n"
      << "; Who will be selected for redressing:\n"
      << "; 0 = Everyone\n"
      << "; 1 = Only fully naked NPCs (must have all 4 slots empty: Body, Hands, Feet, Head)\n"
      << "; 2 = Only NPCs with no main armor equipped (at least the Body slot is empty)\n"
      << "SelectedActors = 2\n"
      << "\n"
      << "; Restricts the mod to equipping only Body, Hands, Feet or Head items.\n"
      << "; This prevents NPCs from equipping jewelry and other misc items that were previously stashed in their pockets.\n"
      << "EquipMainSlotsOnly = false\n"
      << "\n"
      << "; Enables dressing up dead NPCs.\n"
      << "DressDeadActors = true\n"
      << "\n"
      << "; Enables dressing up alive NPCs.\n"
      << "DressAliveActors = true\n"
      << "\n"
      << "; Nudity is allowed in player homes.\n"
      << "SkipPlayerHome = true\n"
      << "\n"
      << "; Active followers won't be dressed.\n"
      << "SkipActiveFollowers = true\n"
      << "\n"
      << "; Sleeping NPCs won't be dressed.\n"
      << "SkipSleepingActors = true\n"
      << "\n"
      << "; Comma separated EditorIDs of the factions you want to exclude. Whitespaces are ignored.\n"
      << "ExcludedFactions = \n"
      << "\n"
      << "; Comma separated EditorIDs of the keywords you want to exclude. Whitespaces are ignored.\n"
      << "ExcludedKeywords = \n"
      << "\n"
      << "; Ignore specific slots per faction (one per line).\n"
      << "; Example:\n"
      << "; FactionSlotsIgnore = ExampleFaction1|33,32,37\n"
      << "; FactionSlotsIgnore = ExampleFaction2|38,39,60\n"
      << "FactionSlotsIgnore = \n";

    logger::info("Generated default ini at {}", path);
}