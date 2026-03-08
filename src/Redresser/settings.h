#pragma once

struct Settings
{
    bool debug = false;

    enum class NPCPool : uint32_t {
        Everyone = 0,
        NakedOnly = 1,
        NoMainArmorOnly = 2
    };

    NPCPool SelectedActors = NPCPool::Everyone;

    void Initialize();
};

inline Settings r_settings;