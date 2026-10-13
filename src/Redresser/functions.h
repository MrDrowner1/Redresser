#pragma once
#include <RE/A/ActorEquipManager.h>

inline RE::BGSKeyword* g_keywordAnimal = nullptr;
inline RE::BGSKeyword* g_locTypePlayerHouse = nullptr;

void ProcessActors();

void AutoEquipActor(RE::Actor* actor);

// Use instead of Actor::IsDead, whose hardcoded vtable index is wrong on 1.7.99.
inline bool IsActorDead(RE::Actor* a_actor)
{
    if (!a_actor)
        return true;

    if (auto* state = a_actor->AsActorState()) {
        switch (state->GetLifeState()) {
        case RE::ACTOR_LIFE_STATE::kDead:
        case RE::ACTOR_LIFE_STATE::kDying:
        case RE::ACTOR_LIFE_STATE::kRecycle:
            return true;
        default:
            return false;
        }
    }

    return a_actor->IsDead();
}

bool isNaked(RE::Actor* actor);

bool isMissingMainArmor(RE::Actor* actor);

bool isPlayerHome();

void InitializeKeywords();

void GiveDefaultOutfitItems(RE::Actor* actor, uint32_t ignoredSlotMask = 0);

void ResolveOutfitItem(RE::TESForm* form, uint16_t actorLevel, std::vector<RE::TESBoundObject*>& out);

bool hasItemForSlot(RE::Actor* actor, RE::TESForm* item);

RE::BGSBipedObjectForm::BipedObjectSlot GetOutfitItemSlot(RE::TESForm* form);

bool isOutfitItem(RE::BGSOutfit* outfit, RE::TESForm* item);

bool isInFaction(RE::Actor* actor, std::string_view factionEditorID);

bool hasKeyword(RE::Actor* actor, std::string_view keywordEditorID);