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

    std::vector<std::string> excludedFactions;
    std::string rawExcludedFactions = "";

    std::vector<std::string> excludedKeywords;
    std::string rawExcludedKeywords = "";

    void Initialize();

    static void ParseList(const std::string& raw, std::vector<std::string>& out);
};

inline Settings g_settings;