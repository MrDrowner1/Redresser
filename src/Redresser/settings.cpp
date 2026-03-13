#include "settings.h"
#include <SimpleIni.h>

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

    equipMainSlotsOnly = ini.GetBoolValue("General", "EquipMainSlotsOnly", true);
    
    skipPlayerHome = ini.GetBoolValue("General", "SkipPlayerHome", true);

    giveDefaultItems = ini.GetBoolValue("General", "GiveDefaultItems", true);
}