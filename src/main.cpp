#include <SKSE/SKSE.h>
#include <RE/A/ActorEquipManager.h>
#include <format>
#include <string>
#include "Redresser/functions.h"
#include "Redresser/debug_output.h"

class LoadingMenuSink:
    public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
public:
    RE::BSEventNotifyControl ProcessEvent(
        const RE::MenuOpenCloseEvent* MenuEvent,
        RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
    {
        if (!MenuEvent)
            return RE::BSEventNotifyControl::kContinue;

        if (MenuEvent->menuName == RE::LoadingMenu::MENU_NAME 
            && !MenuEvent->opening
            && !RE::UI::GetSingleton()->IsMenuOpen(RE::MainMenu::MENU_NAME)){
                g_settings.LoadConfig();

                auto excludedFactionsStr = g_settings.excludedFactions | std::views::join_with(std::string(", ")) | std::ranges::to<std::string>();
                logger::info("Excluded factions: {}", excludedFactionsStr);

                auto excludedKeywordsStr = g_settings.excludedKeywords | std::views::join_with(std::string(", ")) | std::ranges::to<std::string>();
                logger::info("Excluded keywords: {}", excludedKeywordsStr);

                // skipping 2 frames, just in case
                SKSE::GetTaskInterface()->AddTask([]() {
                SKSE::GetTaskInterface()->AddTask([]() {
                    debug_output("Redresser: <starting>");
                    ProcessActors();
                });
                });
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

static void SKSEMessageHandler(SKSE::MessagingInterface::Message* message)
{
    static LoadingMenuSink g_loadingMenuSink;

	if (message->type == SKSE::MessagingInterface::kDataLoaded){
        // Initializing
        InitializeKeywords(); 
        g_settings.LoadRedressFiles();

        auto ui = RE::UI::GetSingleton();
        if (ui)
            ui->AddEventSink<RE::MenuOpenCloseEvent>(&g_loadingMenuSink);
    }
    // Needed for VR
    /*
    else if (message->type == SKSE::MessagingInterface::kPostLoadGame){
        g_settings.LoadConfig();

        logger::info("Game loaded");

        // skipping 2 frames
        SKSE::GetTaskInterface()->AddTask([]() {
        SKSE::GetTaskInterface()->AddTask([]() {
            debug_output("Redresser: <starting after save load>");
            ProcessActors();
        });
        });
    }
    */
}
                
extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);

    const auto messaging = SKSE::GetMessagingInterface();
    if (!messaging->RegisterListener("SKSE", SKSEMessageHandler)) {
        return false;
    }

    logger::info("Game version : {}", a_skse->RuntimeVersion());

    return true;
}