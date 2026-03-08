#include "functions.h"
#include "debug_output.h"
#include "settings.h"

void AutoEquipHighActors()
{
    auto processLists = RE::ProcessLists::GetSingleton();
    if (!processLists)
        return;

    for (auto& handle : processLists->highActorHandles) {
        auto actor = handle.get().get();
        if (!actor || actor->IsPlayerRef() || actor->IsDead() || actor->IsPlayerTeammate())
            continue;

        if (r_settings.SelectedActors == Settings::NPCPool::NakedOnly && !isNaked(actor)){
            debug_output("Redresser: {} is not naked, skipping", actor->GetName());
            continue;
        }
        else if (r_settings.SelectedActors == Settings::NPCPool::NoMainArmorOnly && !isNoMainArmor(actor)){
            debug_output("Redresser: {} has main armor slot occupied, skipping", actor->GetName());
            continue;
        }
        else if (r_settings.SelectedActors == Settings::NPCPool::Everyone){
            debug_output("Redresser: processing {}", actor->GetName());
            // No skipping, the actor is eligible
        }
        else{
            debug_output("Incorrect settings");
            return;
        }
        
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

bool isNoMainArmor(RE::Actor* actor){
    using Slot = RE::BGSBipedObjectForm::BipedObjectSlot;
    bool noMainArmor = !actor->GetWornArmor(Slot::kBody);

    return noMainArmor;
}