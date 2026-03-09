#include "functions.h"
#include "debug_output.h"
#include "settings.h"

void ProcessActors(){
    // Skipping player homes
    if (g_settings.skipPlayerHome && isPlayerHome()){
        debug_output("Redresser: Player home detected, skipping");
        return;
    }

    auto processLists = RE::ProcessLists::GetSingleton();
    if (!processLists)
        return;

    debug_output("Redresser: processing fully loaded actors (highActorHandles). Found {} actors.", processLists->highActorHandles.size());
    for (auto& handle : processLists->highActorHandles) {
        auto actor = handle.get().get();
        if(actor)
            AutoEquipActor(actor);
    }

    debug_output("Redresser: processing partially simulated actors (middleHighActorHandles). Found {} actors.", processLists->middleHighActorHandles.size());
    for (auto& handle : processLists->middleHighActorHandles) {
        auto actor = handle.get().get();
        if(actor)
            AutoEquipActor(actor);
    }
}

void AutoEquipActor(RE::Actor* actor){
    // Skipping player, active followers, animals and dead people
    if (!actor || !actor->Get3D() || actor->IsPlayerRef() || actor->IsPlayerTeammate() || actor->HasKeyword(g_keywordAnimal))
        return;

    // Checking for a dead actor
    if (!g_settings.dressDeadActors && actor->IsDead())
        return;

    if (g_settings.selectedActors == Settings::NPCPool::NakedOnly && !isNaked(actor)){
        debug_output("Redresser: {} is not naked, skipping", actor->GetName());
        return;
    }
    else if (g_settings.selectedActors == Settings::NPCPool::NoMainArmorOnly && !isMissingMainArmor(actor)){
        debug_output("Redresser: {} has the main armor slot occupied, skipping", actor->GetName());
        return;
    }

    debug_output("Redresser: scanning {}", actor->GetName());
    
    auto inv = actor->GetInventory();

    for (auto& [item, data] : inv) {
        if (!item || !item->IsArmor())
            continue;
        
        auto armor = item->As<RE::TESObjectARMO>();
        if (!armor)
            continue;

        // Prevents equipping shields
        if (armor->IsShield())
            continue;

        auto armorSlots = static_cast<uint32_t>(armor->GetSlotMask());

        constexpr uint32_t kBody = static_cast<uint32_t>(RE::BGSBipedObjectForm::BipedObjectSlot::kBody);
        constexpr uint32_t kHead = static_cast<uint32_t>(RE::BGSBipedObjectForm::BipedObjectSlot::kHead);
        constexpr uint32_t kHands = static_cast<uint32_t>(RE::BGSBipedObjectForm::BipedObjectSlot::kHands);
        constexpr uint32_t kFeet = static_cast<uint32_t>(RE::BGSBipedObjectForm::BipedObjectSlot::kFeet);

        // Prevents equipping misc items (jewelry etc.)
        constexpr uint32_t mainSlots = kBody | kHead | kHands | kFeet;
        if (g_settings.equipMainSlotsOnly && !(armorSlots & mainSlots))
            continue;

        bool slotOccupied = false;

        for (uint32_t i = 0; i < 32; i++) {
            uint32_t slot = 1u << i;

            if (armorSlots & slot) {
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

        debug_output("Redresser: {}: equipped {}", actor->GetName(), armor->GetName());
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

    // checking for player house keyword
    auto curLocation = player->GetCurrentLocation();
    if (!curLocation)
        return false;
    
    if (curLocation->HasKeyword(g_locTypePlayerHouse))
        return true;

    // checking for cell ownership
    auto cell = player->GetParentCell();
    
    if (!cell || !cell->IsInteriorCell())
        return false;

    auto actorOwner = cell->GetActorOwner();
    if (actorOwner && actorOwner == player->GetActorBase())
        return true;

    return false;
}

void InitializeKeywords() {
    g_keywordAnimal = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("ActorTypeAnimal");
    g_locTypePlayerHouse = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("LocTypePlayerHouse");
}
