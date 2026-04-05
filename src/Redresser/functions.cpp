#include "functions.h"
#include "debug_output.h"
#include "settings.h"
#include "RE/A/ActorState.h"

void ProcessActors(){
    // Skipping player homes
    if (g_settings.skipPlayerHome && isPlayerHome()){
        debug_output("Redresser: <Player home detected, skipping>");
        return;
    }

    auto player = RE::PlayerCharacter::GetSingleton();
    auto cell = player ? player->GetParentCell() : nullptr;
    if (!cell)
        return;

    // Scanning actors by references
    if (cell->IsInteriorCell()) {
        debug_output("Redresser: <scanning an interior cell>");
        for (auto& refPtr : cell->GetRuntimeData().references) {
            auto actor = refPtr.get() ? refPtr.get()->As<RE::Actor>() : nullptr;
            if (actor)
                AutoEquipActor(actor);
        }
    } else {
        debug_output("Redresser: <scanning exterior cells>");
        auto tes = RE::TES::GetSingleton();
        auto grid = tes ? tes->gridCells : nullptr;
        if (!grid)
            return;

        for (std::uint32_t x = 0; x < grid->length; ++x) {
            for (std::uint32_t y = 0; y < grid->length; ++y) {

                auto scanCell = grid->GetCell(x, y);
                if (!scanCell)
                    continue;

                for (auto& refPtr : scanCell->GetRuntimeData().references) {
                    auto ref = refPtr.get();
                    if (!ref)
                        continue;

                    auto actor = ref->As<RE::Actor>();
                    if (!actor)
                        continue;

                    AutoEquipActor(actor);
                }
            }
        }
    }

    // Scanning actors by process list to get actors from the previous cell
    auto processLists = RE::ProcessLists::GetSingleton();
    if (!processLists)
        return;

    debug_output("Redresser: <processing actors from the last visited cell>", processLists->middleHighActorHandles.size());
    for (auto& handle : processLists->middleHighActorHandles) {
        auto actor = handle.get().get();
        if(actor)
            AutoEquipActor(actor);
    }
}

void AutoEquipActor(RE::Actor* actor){
    // Skipping player, animals, basic checks
    if (!actor || actor->IsPlayerRef() || !actor->Get3D() || actor->HasKeyword(g_keywordAnimal)){
        return;
    }

    //Checking if actor is sleeping
    if (g_settings.skipSleepingActors && actor->AsActorState()->GetSitSleepState() == RE::SIT_SLEEP_STATE::kIsSleeping){
        debug_output("Redresser: {} is sleeping, skipping", actor->GetName());
        return;
    }
    
    // Checking if actor is an active follower
    if (g_settings.skipActiveFollowers && actor->IsPlayerTeammate()){
        debug_output("Redresser: {} is player's teammate, skipping", actor->GetName());
        return;
    }

    // Checking for a dead actor
    if (!g_settings.dressDeadActors && actor->IsDead())
        return;

    // Checking for an alive actor
    if (!g_settings.dressAliveActors && !actor->IsDead())
        return;

    // Checking the selected actors option
    if (g_settings.selectedActors == Settings::NPCPool::NakedOnly && !isNaked(actor)){
        debug_output("Redresser: {} is not naked, skipping", actor->GetName());
        return;
    }
    else if (g_settings.selectedActors == Settings::NPCPool::NoMainArmorOnly && !isMissingMainArmor(actor)){
        debug_output("Redresser: {} has the main armor slot occupied, skipping", actor->GetName());
        return;
    }

    // Checking if actor is in a SexLab scene
    if (isInFaction(actor, "SexLabAnimatingFaction")) {
        debug_output("Redresser: {} is in a SexLab scene, skipping", actor->GetName());
        return;
    }

    // Checking for excluded factions
    for (const auto& factionEditorID : g_settings.excludedFactions) {
        if (isInFaction(actor, factionEditorID)) {
            debug_output("Redresser: {} is in {} faction, skipping", actor->GetName(), factionEditorID);
            return;
        }
    }

    // Building ignored slot mask from FactionSlotsIgnore entries
    uint32_t ignoredSlotMask = 0;
    for (const auto& [factionEditorID, mask] : g_settings.factionSlotsIgnore) {
        if (isInFaction(actor, factionEditorID)) {
            debug_output("Redresser: {} is in {} faction, ignoring slots {:08X}", actor->GetName(), factionEditorID, mask);
            ignoredSlotMask |= mask;
        }
    }

    //Checking for excluded keywords
    for (const auto& keywordEditorID : g_settings.excludedKeywords) {
        if (hasKeyword(actor, keywordEditorID)) {
            debug_output("Redresser: {} has {} keyword, skipping", actor->GetName(), keywordEditorID);
            return;
        }
    }


    debug_output("Redresser: scanning {}", actor->GetName());

    // Gives default items to alive NPC if they don't have anything else to cover up with
    if(isMissingMainArmor(actor) && g_settings.giveDefaultItems)
        GiveDefaultOutfitItems(actor);
    

    auto inv = actor->GetInventory();

    // Collecting actor's armor
    std::vector<RE::TESObjectARMO*> armorList;

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

        armorList.push_back(armor);
    }

    // Sorting armor by rating
    std::sort(
        armorList.begin(),
        armorList.end(),
        [](RE::TESObjectARMO* a, RE::TESObjectARMO* b)
        {
            return a->GetArmorRating() > b->GetArmorRating();
        }
    );

    // Equipping armor
    for (auto* armor : armorList) {

        auto armorSlots = static_cast<uint32_t>(armor->GetSlotMask());

        // Skip armor if all its slots are in the ignored mask
        if (ignoredSlotMask && (armorSlots & ignoredSlotMask) == armorSlots) {
            debug_output("Redresser: -- {}: skipping {} (ignored slot found) --", actor->GetName(), armor->GetName());
            continue;
        }

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

        debug_output("Redresser: -- {}: equipped {} --", actor->GetName(), armor->GetName());
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


void ResolveOutfitItem(RE::TESForm* form, uint16_t actorLevel, std::vector<RE::TESBoundObject*>& out) {
    if (!form) 
        return;

    auto* armor = form->As<RE::TESObjectARMO>();
    if (armor) {
        out.push_back(armor);
        debug_output("Redresser: >>>Got regular item from default outfit: {}", armor->GetName());
        return;
    }

    auto* levItem = form->As<RE::TESLevItem>();
    if (levItem) {
        RE::BSScrapArray<RE::CALCED_OBJECT> calcedObjects;
        levItem->CalculateCurrentFormList(actorLevel, 1, calcedObjects, 0, false);

        for (auto& calcedObj : calcedObjects) {
            auto* boundObj = calcedObj.form->As<RE::TESBoundObject>();
            if (boundObj) {
                out.push_back(boundObj);
                debug_output("Redresser: >>>Got a leveled list item from default outfit");
            }
        }
    }
}


void GiveDefaultOutfitItems(RE::Actor* actor) {
    if (!actor) 
        return;

    auto* actorBase = actor->GetActorBase();
    if (!actorBase || !actorBase->defaultOutfit) 
        return;

    const uint16_t level = actor->GetLevel();

    for (auto* item : actorBase->defaultOutfit->outfitItems) {
        std::vector<RE::TESBoundObject*> resolved;
        ResolveOutfitItem(item, level, resolved);

        for (auto* obj : resolved) {
            debug_output("Redresser: Considering to give {} to {}", obj->GetName(), actor->GetName());
            if (hasItemForSlot(actor, obj))
                continue;
            
            // Adding default clothes piece if actor has no suitable item
            actor->AddObjectToContainer(obj, nullptr, 1, nullptr);
            debug_output("Redresser: !!-- Given {} to {} --!!", obj->GetName(), actor->GetName());
        }
    }
}


bool isOutfitItem(RE::BGSOutfit* outfit, RE::TESForm* item) {
    if (!outfit) 
        return false;

    for (auto* outfitItem : outfit->outfitItems) {
        if (outfitItem == item) 
            return true;
    }
    
    return false;
}


bool hasItemForSlot(RE::Actor* actor, RE::TESForm* outfitItem) {
    if (!outfitItem) 
        return false;

    auto slot = GetOutfitItemSlot(outfitItem);  
    if (slot == RE::BGSBipedObjectForm::BipedObjectSlot::kNone) {
        debug_output("Redresser: slot is kNone for {:08X}", outfitItem->GetFormID());
        return false;
    }
    debug_output("Redresser: checking slot {:08X} for outfitItem {:08X}", static_cast<uint32_t>(slot), outfitItem->GetFormID());

    auto inv = actor->GetInventory();
    for (auto& [item, data] : inv) {
        auto* armor = item->As<RE::TESObjectARMO>();
        if (armor) {
            // True if duplicate
            if (armor == outfitItem->As<RE::TESObjectARMO>())
                return true;

            // Sleep items don't count
            auto* actorBase = actor->GetActorBase();
            if (!actorBase)
                continue;
            if (isOutfitItem(actorBase->sleepOutfit, armor))
                continue;

            if (static_cast<uint32_t>(armor->GetSlotMask()) &
                static_cast<uint32_t>(slot)) {
                return true;
            }
        }
    }
    debug_output("Redresser: no match found for slot {:08X}", static_cast<uint32_t>(slot));
    return false;
}


RE::BGSBipedObjectForm::BipedObjectSlot GetOutfitItemSlot(RE::TESForm* form) {
    if (!form) return RE::BGSBipedObjectForm::BipedObjectSlot::kNone;

    if (auto* armor = form->As<RE::TESObjectARMO>()) {
        return armor->GetSlotMask();
    }

    if (auto* levItem = form->As<RE::TESLevItem>()) {
        for (auto& entry : levItem->entries) {
            auto slot = GetOutfitItemSlot(entry.form);
            if (slot != RE::BGSBipedObjectForm::BipedObjectSlot::kNone) {
                return slot;
            }
        }
    }

    return RE::BGSBipedObjectForm::BipedObjectSlot::kNone;
}


bool isInFaction(RE::Actor* actor, std::string_view factionEditorID)
{
    if (!actor) return false;

    auto* form = RE::TESForm::LookupByEditorID(factionEditorID);
    if (!form)
        return false;

    auto* targetFaction = form->As<RE::TESFaction>();
    if (!targetFaction)
        return false;

    auto* actorBaseData = static_cast<RE::TESActorBaseData*>(actor->GetActorBase());
    if (!actorBaseData)
        return false;

    for (const auto& entry : actorBaseData->factions) {
        if (entry.faction == targetFaction)
            return true;
    }

    return false;
}


bool hasKeyword(RE::Actor* actor, std::string_view keywordEditorID)
{
    if (!actor) 
        return false;

    auto* base = actor->GetActorBase();
    if (!base) 
        return false;

    for (std::uint32_t i = 0; i < base->numKeywords; i++) {
        auto* kw = base->keywords[i];
        if (!kw)
            continue;
        
        std::string editorID = kw->GetFormEditorID();
        std::transform(editorID.begin(), editorID.end(), editorID.begin(), ::tolower);
        
        if (editorID == keywordEditorID)
            return true;
    }

    return false;
}