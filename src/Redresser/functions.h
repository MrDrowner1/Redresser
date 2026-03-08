#pragma once
#include <RE/A/ActorEquipManager.h>

inline RE::BGSKeyword* g_keywordAnimal = nullptr;
inline RE::BGSKeyword* g_locTypePlayerHouse = nullptr;

void AutoEquipHighActors();

bool isNaked(RE::Actor* actor);

bool isNoMainArmor(RE::Actor* actor);

bool isPlayerHome();

void InitializeKeywords();