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

    bool skipPlayerHome = false;

    bool equipMainSlotsOnly = true;

    void Initialize();
};

inline Settings g_settings;