#include "functions.h"
#include "debug_output.h"
#include "settings.h"

void AutoEquipHighActors()
{
    // Skipping player homes
    if (g_settings.skipPlayerHome && isPlayerHome()){
        debug_output("Redresser: Player home detected, skipping");
        return;
    }

    auto processLists = RE::ProcessLists::GetSingleton();
    if (!processLists)
        return;

    for (auto& handle : processLists->highActorHandles) {
        auto actor = handle.get().get();

        // Skipping player, active followers and dead people
        if (!actor || actor->IsPlayerRef() || actor->IsDead() || actor->IsPlayerTeammate())
            continue;

        // Skipping animals
        if (actor->HasKeyword(g_keywordAnimal))
            continue;

        if (g_settings.selectedActors == Settings::NPCPool::NakedOnly && !isNaked(actor)){
            debug_output("Redresser: {} is not naked, skipping", actor->GetName());
            continue;
        }
        else if (g_settings.selectedActors == Settings::NPCPool::NoMainArmorOnly && !isMissingMainArmor(actor)){
            debug_output("Redresser: {} has main armor slot occupied, skipping", actor->GetName());
            continue;
        }

        debug_output("Redresser: processing {}", actor->GetName());
        
        auto inv = actor->GetInventory();

        for (auto& [item, data] : inv) {
            if (!item || !item->IsArmor())
                continue;

            auto armor = item->As<RE::TESObjectARMO>();
            if (!armor)
                continue;

            auto actorSlots = static_cast<uint32_t>(armor->GetSlotMask());
            bool slotOccupied = false;

            for (uint32_t i = 0; i < 32; i++) {
                uint32_t slot = 1u << i;

                if (actorSlots & slot) {
                    if (actor->GetWornArmor(static_cast<RE::BGSBipedObjectForm::BipedObjectSlot>(slot))) {
                        slotOccupied = true;
                        // debug_output("{}: trying to equip {}, slot is already occupied", actor->GetName(), item->GetName());
                        break;
                    }
                }
            }

            if(slotOccupied)
                continue;

            auto equipManager = RE::ActorEquipManager::GetSingleton();
            if (equipManager)
                equipManager->EquipObject(
                    actor,
                    armor,
                    nullptr,
                    1,
                    nullptr,
                    false,
                    false,
                    false,
                    false
                );

            debug_output("Redresser. {}: equipped {}", actor->GetName(), armor->GetName());
        }
    }
}

bool isNaked(RE::Actor* actor){
    using Slot = RE::BGSBipedObjectForm::BipedObjectSlot;

    bool isNaked = 
        !actor->GetWornArmor(Slot::kBody) &&
        !actor->GetWornArmor(Slot::kHands) &&
        !actor->GetWornArmor(Slot::kFeet) &&
        !actor->GetWornArmor(Slot::kHead);

    return isNaked;
}

bool isMissingMainArmor(RE::Actor* actor){
    using Slot = RE::BGSBipedObjectForm::BipedObjectSlot;
    bool noMainArmor = !actor->GetWornArmor(Slot::kBody);

    return noMainArmor;
}

bool isPlayerHome(){
    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player)
        return false;

    auto curLocation = player->GetCurrentLocation();
    if (!curLocation)
        return false;
    
    if (curLocation->HasKeyword(g_locTypePlayerHouse))
        return true;
}

void InitializeKeywords() {
    g_keywordAnimal = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("ActorTypeAnimal");
    g_locTypePlayerHouse = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("LocTypePlayerHouse");
}
