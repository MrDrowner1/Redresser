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

        if (MenuEvent->menuName == RE::LoadingMenu::MENU_NAME && !MenuEvent->opening){
            // skipping 2 frames
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
        g_settings.Initialize();
        InitializeKeywords();
        auto ui = RE::UI::GetSingleton();
        if (ui)
            ui->AddEventSink<RE::MenuOpenCloseEvent>(&g_loadingMenuSink);
    }
    else if (message->type == SKSE::MessagingInterface::kPostLoadGame){
        // skipping 2 frames
            SKSE::GetTaskInterface()->AddTask([]() {
            SKSE::GetTaskInterface()->AddTask([]() {
                debug_output("Redresser: <starting after save load>");
                ProcessActors();
            });
            });
    }
}
                
extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);

    const auto messaging = SKSE::GetMessagingInterface();
    if (!messaging->RegisterListener("SKSE", SKSEMessageHandler)) {
        return false;
    }

    return true;
}