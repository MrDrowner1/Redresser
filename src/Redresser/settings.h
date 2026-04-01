#pragma once

struct Settings
{
    bool debug = false;

    enum class NPCPool : uint32_t {
        Everyone = 0,
        NakedOnly = 1,
        NoMainArmorOnly = 2
    };

    NPCPool selectedActors = NPCPool::Everyone;

    bool dressDeadActors = true;

    bool dressAliveActors = true;

    bool equipMainSlotsOnly = true;
    
    bool skipPlayerHome = true;

    bool skipActiveFollowers = true;

    bool skipSleepingActors = true;

    bool giveDefaultItems = true;

    std::set<std::string> excludedFactions;

    std::set<std::string> excludedKeywords;
    
    void LoadConfig();

    void LoadRedressFiles();

    static void ParseList(const std::string& raw, std::set<std::string>& out);

    private:
        // From dynamically loaded ini files
        std::set<std::string> loadedFactions;
        std::set<std::string> loadedKeywords;
};

inline Settings g_settings;