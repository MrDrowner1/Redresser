#include <SKSE/SKSE.h>
#include <RE/A/ActorEquipManager.h>

void AutoEquipHighActors()
{
    auto processLists = RE::ProcessLists::GetSingleton();
    if (!processLists)
        return;

    //RE::ConsoleLog::GetSingleton()->Print("Redresser started.");

    for (auto& handle : processLists->highActorHandles) {
        auto actor = handle.get().get();
        if (!actor || actor->IsPlayerRef() || actor->IsDead() || actor->IsPlayerTeammate())
            continue;

        auto inv = actor->GetInventory();

        for (auto& [item, data] : inv) {
            if (!item || !item->IsArmor())
                continue;

            auto armor = item->As<RE::TESObjectARMO>();
            if (!armor)
                continue;

            auto equipManager = RE::ActorEquipManager::GetSingleton();
            if (equipManager)
                equipManager->EquipObject(
                    actor,
                    armor,
                    nullptr,
                    1,
                    nullptr, // equip slot (auto) 
                    false,
                    false, // no force equip 
                    false, // no playing sounds 
                    false
                );
        }
    }
}

class LoadingMenuSink :
    public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
public:
    RE::BSEventNotifyControl ProcessEvent(
        const RE::MenuOpenCloseEvent* event,
        RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
    {
        if (!event)
            return RE::BSEventNotifyControl::kContinue;

        if (event->menuName == RE::LoadingMenu::MENU_NAME && !event->opening) {
            //RE::ConsoleLog::GetSingleton()->Print("Redresser: Loading menu closed.");
            AutoEquipHighActors();
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

static void SKSEMessageHandler(SKSE::MessagingInterface::Message* message)
{
    static LoadingMenuSink g_loadingMenuSink;

	switch (message->type) {
    case SKSE::MessagingInterface::kPostLoadGame:
        //RE::ConsoleLog::GetSingleton()->Print("Redresser: Save load.");
        AutoEquipHighActors();
        break;
    case SKSE::MessagingInterface::kDataLoaded:
        {
        auto ui = RE::UI::GetSingleton();
        if (ui)
            ui->AddEventSink<RE::MenuOpenCloseEvent>(&g_loadingMenuSink);
        }
        break;
    default:
        break;
    }
}
                
extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);

    const auto messaging = SKSE::GetMessagingInterface();
    if (!messaging->RegisterListener("SKSE", SKSEMessageHandler)) {
        logger::critical("Failed to register Listener");
        return false;
    }

    return true;
}