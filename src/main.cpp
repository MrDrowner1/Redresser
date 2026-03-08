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
            // skipping 2 frames, just in case
            SKSE::GetTaskInterface()->AddTask([]() {
                SKSE::GetTaskInterface()->AddTask([]() {
                    debug_output("Redresser: starting");
                    AutoEquipHighActors();
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
        r_settings.Initialize();
        auto ui = RE::UI::GetSingleton();
        if (ui)
            ui->AddEventSink<RE::MenuOpenCloseEvent>(&g_loadingMenuSink);
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