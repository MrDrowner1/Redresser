#pragma once

struct Settings
{
    bool debug = false;

    enum class NPCPool : uint32_t {
        Everyone = 0,
        NakedOnly = 1,
        NoMainArmorOnly = 2
    };

    NPCPool selectedActors = NPCPool::NoMainArmorOnly;

    bool dressDeadActors = true;

    bool dressAliveActors = true;

    uint32_t autoEquipSlots = 0x7FCFFFFF;  // all slots except 50, 51, 61
    
    bool skipPlayerHome = true;

    bool skipActiveFollowers = true;

    bool skipSleepingActors = true;

    bool giveDefaultItems = true;

    bool giveItemsIfDead = false;

    bool giveItemsOnlyIfMissingMainArmor = true;

    std::set<std::string> excludedFactions;

    std::set<std::string> excludedKeywords;

    std::map<std::string, uint32_t> factionSlotsIgnore;

    // Raw user-defined strings (from Redresser.ini only)
    std::string rawExcludedFactions;
    std::string rawExcludedKeywords;
    std::vector<std::string> rawFactionSlotsIgnore;

    // From dynamically loaded _REDRESS ini files
    std::set<std::string> loadedFactions;
    std::set<std::string> loadedKeywords;
    std::map<std::string, uint32_t> loadedFactionSlots;

    // Set to true by LoadConfig() so the UI knows to refresh its buffers
    bool uiNeedsRefresh = true;

    void LoadConfig();

    void LoadRedressFiles();

    void SaveConfig();

    void GenerateDefaultIni(const char* path);

    static void ParseList(const std::string& raw, std::set<std::string>& out);

    static void ParseFactionSlotEntry(const std::string& raw, std::map<std::string, uint32_t>& out);
};

inline Settings g_settings;