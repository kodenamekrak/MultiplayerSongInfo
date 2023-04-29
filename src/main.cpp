#include "main.hpp"
#include "ModalView.hpp"

#include "GlobalNamespace/IConnectedPlayer.hpp"
#include "GlobalNamespace/GameServerPlayerTableCell.hpp"
#include "GlobalNamespace/LobbyPlayersDataModel.hpp"
#include "GlobalNamespace/PreviewDifficultyBeatmap.hpp"
#include "GlobalNamespace/ServerPlayerListViewController.hpp"

#include "UnityEngine/Resources.hpp"

#include "bsml-lite/shared/Creation/Buttons.hpp"
#include "bsml/shared/BSML.hpp"

#include "song-details/shared/SongDetails.hpp"

#include <map>

using namespace GlobalNamespace;

SongDetailsCache::SongDetails* songDetails;

std::map<StringW, PreviewDifficultyBeatmap*> connectedPlayerLevels;
std::map<StringW, UnityEngine::UI::Button*> connectedPlayerButtons;

MultiplayerSongInfo::ModalView* modalView;
StringW localPlayerId;

static ModInfo modInfo;

void ShowModal(StringW userId)
{
    getLogger().info("Populating modal...");
    modalView->PopulateWithSongData(connectedPlayerLevels[userId]);
    modalView->modal->Show();
}

MAKE_HOOK_MATCH(LobbyPlayersDataModel_SetPlayerBeatmapLevel, &LobbyPlayersDataModel::SetPlayerBeatmapLevel, void, LobbyPlayersDataModel* self, StringW userId, PreviewDifficultyBeatmap* beatmapLevel)
{
    LobbyPlayersDataModel_SetPlayerBeatmapLevel(self, userId, beatmapLevel);
    if(beatmapLevel)
    {
        connectedPlayerLevels[userId] = beatmapLevel;
        connectedPlayerButtons[userId]->set_interactable(true);
    }
    else
    {
        connectedPlayerLevels.erase(userId);
        connectedPlayerButtons[userId]->set_interactable(false);
    }
}

MAKE_HOOK_MATCH(ServerPlayerListViewController_DidActivate, &ServerPlayerListViewController::DidActivate, void, ServerPlayerListViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
{
    getLogger().info("GameServerLobbyFlowCoordinator_DidActivate");
    ServerPlayerListViewController_DidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);
    if(firstActivation)
    {
        modalView = MultiplayerSongInfo::ModalView::New_ctor(self->get_transform(), self->lobbyPlayersDataModel);
        localPlayerId = self->lobbyPlayersDataModel->get_localUserId();
    }
}

MAKE_HOOK_MATCH(LobbyPlayersDataModel_HandleMultiplayerSessionManagerPlayerDisconnected, &LobbyPlayersDataModel::HandleMultiplayerSessionManagerPlayerDisconnected, void, LobbyPlayersDataModel* self, IConnectedPlayer* connectedPlayer)
{
    getLogger().info("LobbyPlayersDataModel_HandleMultiplayerSessionManagerPlayerDisconnected");
    LobbyPlayersDataModel_HandleMultiplayerSessionManagerPlayerDisconnected(self, connectedPlayer);
    connectedPlayerLevels.erase(connectedPlayer->get_userId());
    connectedPlayerButtons.erase(connectedPlayer->get_userId());
}

MAKE_HOOK_MATCH(GameServerPlayerTableCell_SetData, &GameServerPlayerTableCell::SetData, void, GameServerPlayerTableCell* self, IConnectedPlayer* connectedPlayer, ::ILobbyPlayerData* playerData, bool hasKickPermissions, bool allowSelection, ::System::Threading::Tasks::Task_1<::AdditionalContentModel::EntitlementStatus>* getLevelEntitlementTask)
{
    getLogger().info("GameServerPlayerTableCell_SetData");
    GameServerPlayerTableCell_SetData(self, connectedPlayer, playerData, hasKickPermissions, allowSelection, getLevelEntitlementTask);
    
    auto userId = connectedPlayer->get_userId();
    // if(userId == localPlayerId)
    //     return;

    if(!connectedPlayerButtons.contains(userId))
    {
        auto button = BSML::Lite::CreateUIButton(self->get_transform(), "i", [userId]()
        {
            ShowModal(userId);
        });
        button->set_interactable(false);
        
        connectedPlayerButtons[connectedPlayer->get_userId()] = button;
    }
}

Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = MOD_ID;
    info.version = VERSION;
    modInfo = info;
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();

    BSML::Init();
    songDetails = SongDetailsCache::SongDetails::Init().get();

    getLogger().info("Installing hooks...");
    INSTALL_HOOK(getLogger(), LobbyPlayersDataModel_SetPlayerBeatmapLevel);
    INSTALL_HOOK(getLogger(), LobbyPlayersDataModel_HandleMultiplayerSessionManagerPlayerDisconnected);
    INSTALL_HOOK(getLogger(), GameServerPlayerTableCell_SetData);
    INSTALL_HOOK(getLogger(), ServerPlayerListViewController_DidActivate);
    getLogger().info("Installed all hooks!");
}