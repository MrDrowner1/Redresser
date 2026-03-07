#include <SKSE/SKSE.h>
#include <RE/A/ActorEquipManager.h>

void AutoEquipHighActors()
{
    auto processLists = RE::ProcessLists::GetSingleton();
    if (!processLists)
        return;

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
                    nullptr, // pick equip slot automatically
                    false,
                    false, // no force equip 
                    false, // no playing sounds 
                    false
                );
        }
    }
}

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

        if (MenuEvent->menuName == RE::LoadingMenu::MENU_NAME && !MenuEvent->opening)
            AutoEquipHighActors();

        return RE::BSEventNotifyControl::kContinue;
    }
};

static void SKSEMessageHandler(SKSE::MessagingInterface::Message* message)
{
    static LoadingMenuSink g_loadingMenuSink;

	if (message->type == SKSE::MessagingInterface::kDataLoaded){
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