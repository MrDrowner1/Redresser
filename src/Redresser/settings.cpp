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

    debug = ini.GetBoolValue("General", "Debug", false);

    auto selectMode = ini.GetLongValue("General", "SelectedActors", 0);
    SelectedActors = static_cast<Settings::NPCPool>(selectMode);
}