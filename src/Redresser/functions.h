#pragma once
#include <RE/A/ActorEquipManager.h>

inline RE::BGSKeyword* g_keywordAnimal = nullptr;
inline RE::BGSKeyword* g_locTypePlayerHouse = nullptr;

void ProcessActors();

void AutoEquipActor(RE::Actor* actor);

bool isNaked(RE::Actor* actor);

bool isMissingMainArmor(RE::Actor* actor);

bool isPlayerHome();

void InitializeKeywords();

void GiveDefaultOutfitItems(RE::Actor* actor);

RE::TESBoundObject* ResolveOutfitItem(RE::TESForm* form, uint16_t actorLevel);

bool hasItemForSlot(RE::Actor* actor, RE::TESForm* item);

RE::BGSBipedObjectForm::BipedObjectSlot GetOutfitItemSlot(RE::TESForm* form);

bool isOutfitItem(RE::BGSOutfit* outfit, RE::TESForm* item);