#pragma once

#include "custom-types/shared/macros.hpp"
#include "song-details/shared/Data/Song.hpp"

#include "GlobalNamespace/PreviewDifficultyBeatmap.hpp"
#include "GlobalNamespace/ILobbyPlayersDataModel.hpp"

#include "HMUI/TextSegmentedControl.hpp"
#include "HMUI/SegmentedControl.hpp"
#include "HMUI/CurvedTextMeshPro.hpp"
#include "HMUI/ImageView.hpp"

#include "bsml/shared/BSML/Components/ModalView.hpp"
#include "bsml/shared/BSML/Components/ClickableText.hpp"

DECLARE_CLASS_CODEGEN(MultiplayerSongInfo, ModalView, Il2CppObject,

    DECLARE_CTOR(ctor, UnityEngine::Transform* parent, GlobalNamespace::ILobbyPlayersDataModel* model);

    DECLARE_INSTANCE_FIELD(BSML::ModalView*, modal);
    DECLARE_INSTANCE_FIELD(GlobalNamespace::PreviewDifficultyBeatmap*, currentBeatmap);

    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, songCoverImage);
    DECLARE_INSTANCE_FIELD(HMUI::TextSegmentedControl*, difficultyTextSegment);
    DECLARE_INSTANCE_METHOD(void, DifficultySegmentClick, HMUI::SegmentedControl* segmentedControl, int idx);

    DECLARE_INSTANCE_FIELD(GlobalNamespace::ILobbyPlayersDataModel*, lobbyPlayersDataModel);
    DECLARE_INSTANCE_FIELD(int, lastSelectedDifficulty);
    DECLARE_INSTANCE_METHOD(void, SelectButtonClick);

    DECLARE_INSTANCE_FIELD(HMUI::CurvedTextMeshPro*, songNameText);
    DECLARE_INSTANCE_FIELD(HMUI::CurvedTextMeshPro*, songAuthorText);
    DECLARE_INSTANCE_FIELD(HMUI::CurvedTextMeshPro*, songRatingText);
    DECLARE_INSTANCE_FIELD(HMUI::CurvedTextMeshPro*, uploadDateText);

    DECLARE_INSTANCE_FIELD(HMUI::CurvedTextMeshPro*, songDurationText);
    DECLARE_INSTANCE_FIELD(HMUI::CurvedTextMeshPro*, notesPerSecondText);
    DECLARE_INSTANCE_FIELD(HMUI::CurvedTextMeshPro*, rankedStatusText);
    DECLARE_INSTANCE_FIELD(HMUI::CurvedTextMeshPro*, ppValueText);
    DECLARE_INSTANCE_FIELD(HMUI::CurvedTextMeshPro*, starValueText);

    const SongDetailsCache::Song *currentSong;
    std::vector<const SongDetailsCache::SongDifficulty*> currentDifficulties;
    const SongDetailsCache::SongDifficulty* currentDifficulty;

    std::string TimeFormat(float duration);
    std::string GetDateEnding(int date);
    void PopulateWithSongData(GlobalNamespace::PreviewDifficultyBeatmap* beatmap);
    void PopulateWithDifficultyData(const SongDetailsCache::SongDifficulty* diff);
)