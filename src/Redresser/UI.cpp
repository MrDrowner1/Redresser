#pragma warning(push)
#pragma warning(disable: 4996 4099 5054)
#include "SKSEMenuFramework.h"
#pragma warning(pop)
#include "UI.h"
#include "Redresser/settings.h"

namespace {
    constexpr size_t kTextBufSize = 2048;
    constexpr size_t kSlotsBufSize = 4096;

    char factionsBuf[kTextBufSize]  = {};
    char keywordsBuf[kTextBufSize]  = {};
    char factionSlotsBuf[kSlotsBufSize] = {};

    bool savedMessage = false;
    float savedTimer  = 0.0f;

    bool textDirty = false;
    float textSaveTimer = 0.0f;

    void ApplyTextSettings()
    {
        g_settings.rawExcludedFactions = factionsBuf;
        g_settings.rawExcludedKeywords = keywordsBuf;

        g_settings.rawFactionSlotsIgnore.clear();
        std::istringstream ss(factionSlotsBuf);
        std::string line;
        while (std::getline(ss, line)) {
            auto start = line.find_first_not_of(" \t\r");
            if (start != std::string::npos)
                g_settings.rawFactionSlotsIgnore.push_back(line.substr(start));
        }

        ::Settings::ParseList(factionsBuf, g_settings.excludedFactions);
        g_settings.excludedFactions.insert(
            g_settings.loadedFactions.begin(), g_settings.loadedFactions.end());

        ::Settings::ParseList(keywordsBuf, g_settings.excludedKeywords);
        g_settings.excludedKeywords.insert(
            g_settings.loadedKeywords.begin(), g_settings.loadedKeywords.end());

        g_settings.factionSlotsIgnore.clear();
        for (const auto& entry : g_settings.rawFactionSlotsIgnore)
            ::Settings::ParseFactionSlotEntry(entry, g_settings.factionSlotsIgnore);
        for (const auto& [faction, mask] : g_settings.loadedFactionSlots)
            g_settings.factionSlotsIgnore[faction] |= mask;
    }

    void Save()
    {
        g_settings.SaveConfig();
        savedMessage = true;
        savedTimer   = 3.0f;
    }
}

void UI::Settings::InitBuffers()
{
    strncpy_s(factionsBuf, sizeof(factionsBuf),
              g_settings.rawExcludedFactions.c_str(), _TRUNCATE);

    strncpy_s(keywordsBuf, sizeof(keywordsBuf),
              g_settings.rawExcludedKeywords.c_str(), _TRUNCATE);

    std::string slotsStr;
    for (const auto& entry : g_settings.rawFactionSlotsIgnore) {
        if (!slotsStr.empty())
            slotsStr += '\n';
        slotsStr += entry;
    }
    strncpy_s(factionSlotsBuf, sizeof(factionSlotsBuf),
              slotsStr.c_str(), _TRUNCATE);

    g_settings.uiNeedsRefresh = false;
}

void UI::Register()
{
    if (!SKSEMenuFramework::IsInstalled())
        return;

    UI::Settings::InitBuffers();
    SKSEMenuFramework::SetSection("Redresser");
    SKSEMenuFramework::AddSectionItem("Settings", UI::Settings::Render);
    SKSEMenuFramework::AddSectionItem("Filters", UI::Filters::Render);
}

void __stdcall UI::Settings::Render()
{
    if (g_settings.uiNeedsRefresh)
        InitBuffers();

    const char* poolItems[] = {
        "Dress everyone",
        "Dress naked NPCs only (Body, Hands, Feet, Head all empty)",
        "Dress NPCs with missing main armor only (Body slot empty)"
    };
    int poolVal = static_cast<int>(g_settings.selectedActors);
    if (ImGui::Combo("Selected NPCs", &poolVal, poolItems, 3)) {
        g_settings.selectedActors = static_cast<::Settings::NPCPool>(poolVal);
        Save();
    }


    if (ImGui::Checkbox("Give default items if necessary", &g_settings.giveDefaultItems)) Save();
    if (g_settings.giveDefaultItems) {
        ImGui::Indent();
        if (ImGui::Checkbox("Give items to dead NPCs",              &g_settings.giveItemsIfDead))                  Save();
        if (ImGui::Checkbox("Only give items if the main armor is missing",       &g_settings.giveItemsOnlyIfMissingMainArmor))  Save();
        ImGui::Unindent();
    }
    if (ImGui::Checkbox("Dress dead NPCs",        &g_settings.dressDeadActors))    Save();
    if (ImGui::Checkbox("Dress alive NPCs",       &g_settings.dressAliveActors))   Save();
    if (ImGui::Checkbox("Skip player homes",         &g_settings.skipPlayerHome))     Save();
    if (ImGui::Checkbox("Skip active followers",    &g_settings.skipActiveFollowers)) Save();
    if (ImGui::Checkbox("Skip sleeping NPCs",     &g_settings.skipSleepingActors)) Save();
    if (ImGui::Checkbox("Debug logging in console", &g_settings.debug))              Save();

    if (ImGui::CollapsingHeader("Auto Equip Slots", ImGuiTreeNodeFlags_None)) {
        ImGui::TextDisabled("Slots Redresser is allowed to equip");
        struct SlotInfo { int num; const char* name; };
        constexpr SlotInfo kSlots[] = {
            {30,"Head"},    {31,"Hair"},    {32,"Body"},    {33,"Hands"},
            {34,"Forearms"},{35,"Amulet"}, {36,"Ring"},    {37,"Feet"},
            {38,"Calves"},  {39,"Shield"}, {40,"Tail"},    {41,"Long Hair"},
            {42,"Circlet"}, {43,"Ears"},          {44,nullptr},  {45,nullptr},
            {46,nullptr},   {47,nullptr},         {48,nullptr},  {49,nullptr},
            {50,"Decapitated Head"},{51,"Decapitate"},{52,nullptr},{53,nullptr},
            {54,nullptr},   {55,nullptr},         {56,nullptr},  {57,nullptr},
            {58,nullptr},   {59,nullptr},         {60,nullptr},  {61,"FX01"},
        };
        if (ImGui::BeginTable("##autoEquipSlots", 4, 0, ImVec2{0,0}, 0.0f)) {
            for (const auto& slot : kSlots) {
                ImGui::TableNextColumn();
                uint32_t bit = 1u << (slot.num - 30);
                bool enabled = (g_settings.autoEquipSlots & bit) != 0;
                char label[32];
                if (slot.name) snprintf(label, sizeof(label), "%d %s", slot.num, slot.name);
                else           snprintf(label, sizeof(label), "%d",    slot.num);
                if (ImGui::Checkbox(label, &enabled)) {
                    if (enabled) g_settings.autoEquipSlots |= bit;
                    else         g_settings.autoEquipSlots &= ~bit;
                    Save();
                }
            }
            ImGui::EndTable();
        }
    }

    if (savedMessage) {
        ImGui::TextColored(ImVec4{0.3f, 1.0f, 0.3f, 1.0f}, "Settings saved");
        savedTimer -= ImGui::GetIO()->DeltaTime;
        if (savedTimer <= 0.0f)
            savedMessage = false;
    }
}

void __stdcall UI::Filters::Render()
{
    if (g_settings.uiNeedsRefresh)
        UI::Settings::InitBuffers();

    ImGui::Text("Excluded Factions");
    ImGui::TextDisabled("Comma-separated EditorIDs of factions whose members will never be dressed");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::InputText("##factions", factionsBuf, kTextBufSize);
    if (ImGui::IsItemEdited()) { textDirty = true; textSaveTimer = 1.5f; }
    if (ImGui::IsItemDeactivatedAfterEdit()) { ApplyTextSettings(); Save(); textDirty = false; textSaveTimer = 0.0f; }

    ImGui::Text("Excluded Keywords");
    ImGui::TextDisabled("Comma-separated EditorIDs of keywords that will prevent an NPC from being dressed");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::InputText("##keywords", keywordsBuf, kTextBufSize);
    if (ImGui::IsItemEdited()) { textDirty = true; textSaveTimer = 1.5f; }
    if (ImGui::IsItemDeactivatedAfterEdit()) { ApplyTextSettings(); Save(); textDirty = false; textSaveTimer = 0.0f; }

    ImGui::Text("Ignored Slots By Faction");
    ImGui::TextDisabled("Ignore specific armor slots for members of a given faction. The specified slots will NOT be used for auto-equip.");
    ImGui::TextDisabled("One entry per line:  FactionEditorID | slot1, slot2, ...   (slots 30-61)");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::InputTextMultiline("##factionslots", factionSlotsBuf, kSlotsBufSize,
                              ImVec2{-1.0f, 120.0f}, 0, nullptr, nullptr);
    if (ImGui::IsItemEdited()) { textDirty = true; textSaveTimer = 1.5f; }
    if (ImGui::IsItemDeactivatedAfterEdit()) { ApplyTextSettings(); Save(); textDirty = false; textSaveTimer = 0.0f; }

    if ((!g_settings.loadedFactions.empty() || !g_settings.loadedKeywords.empty() ||
        !g_settings.loadedFactionSlots.empty()) &&
        ImGui::CollapsingHeader("Loaded from _REDRESS.ini files", ImGuiTreeNodeFlags_DefaultOpen)) {

        if (!g_settings.loadedFactions.empty()) {
            std::string s;
            for (const auto& f : g_settings.loadedFactions) {
                if (!s.empty()) s += ", ";
                s += f;
            }
            ImGui::Text("Factions:");
            ImGui::TextDisabled("%s", s.c_str());
        }
        if (!g_settings.loadedKeywords.empty()) {
            std::string s;
            for (const auto& k : g_settings.loadedKeywords) {
                if (!s.empty()) s += ", ";
                s += k;
            }
            ImGui::Text("Keywords:");
            ImGui::TextDisabled("%s", s.c_str());
        }
        if (!g_settings.loadedFactionSlots.empty()) {
            ImGui::Text("Ignored Slots By Faction:");
            for (const auto& [faction, mask] : g_settings.loadedFactionSlots) {
                std::string slots;
                for (int i = 0; i < 32; ++i) {
                    if (mask & (1u << i)) {
                        if (!slots.empty()) slots += ", ";
                        slots += std::to_string(i + 30);
                    }
                }
                ImGui::TextDisabled("%s | %s", faction.c_str(), slots.c_str());
            }
        }
    }

    if (textDirty) {
        textSaveTimer -= ImGui::GetIO()->DeltaTime;
        if (textSaveTimer <= 0.0f) {
            ApplyTextSettings(); Save(); textDirty = false;
        }
    }

    if (savedMessage) {
        ImGui::TextColored(ImVec4{0.3f, 1.0f, 0.3f, 1.0f}, "Settings saved!");
        savedTimer -= ImGui::GetIO()->DeltaTime;
        if (savedTimer <= 0.0f)
            savedMessage = false;
    }
}
