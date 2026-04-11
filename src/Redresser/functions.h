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

void ResolveOutfitItem(RE::TESForm* form, uint16_t actorLevel, std::vector<RE::TESBoundObject*>& out);

bool hasItemForSlot(RE::Actor* actor, RE::TESForm* item);

RE::BGSBipedObjectForm::BipedObjectSlot GetOutfitItemSlot(RE::TESForm* form);

bool isOutfitItem(RE::BGSOutfit* outfit, RE::TESForm* item);

bool isInFaction(RE::Actor* actor, std::string_view factionEditorID);

bool hasKeyword(RE::Actor* actor, std::string_view keywordEditorID);