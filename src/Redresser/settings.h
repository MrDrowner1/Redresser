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

    bool equipMainSlotsOnly = false;
    
    bool skipPlayerHome = true;

    bool skipActiveFollowers = true;

    bool skipSleepingActors = true;

    bool giveDefaultItems = true;

    std::set<std::string> excludedFactions;

    std::set<std::string> excludedKeywords;

    std::map<std::string, uint32_t> factionSlotsIgnore;

    void LoadConfig();

    void LoadRedressFiles();

    void GenerateDefaultIni(const char* path);

    static void ParseList(const std::string& raw, std::set<std::string>& out);

    static void ParseFactionSlotEntry(const std::string& raw, std::map<std::string, uint32_t>& out);

    private:
        // From dynamically loaded ini files
        std::set<std::string> loadedFactions;
        std::set<std::string> loadedKeywords;
        std::map<std::string, uint32_t> loadedFactionSlots;
};

inline Settings g_settings;