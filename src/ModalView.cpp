#include "main.hpp"
#include "ModalView.hpp"
#include "assets.hpp"

#include "GlobalNamespace/IPreviewBeatmapLevel.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"

#include "bsml-lite/shared/Creation/Layout.hpp"

#include "bsml/shared/BSML.hpp"
#include "bsml/shared/Helpers/utilities.hpp"

#include "fmt/format.h"

#include "UnityEngine/Resources.hpp"

DEFINE_TYPE(MultiplayerSongInfo, ModalView)

const std::vector<std::string> difficulties = {"Easy", "Normal", "Hard", "Expert", "Expert+"};
const std::vector<std::string> months = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

namespace MultiplayerSongInfo {
    
    void ModalView::PopulateWithSongData(GlobalNamespace::PreviewDifficultyBeatmap* beatmap)
    {
        if(!beatmap)
            return;

        currentDifficulties.clear();
        currentBeatmap = beatmap;

        std::string levelId = static_cast<std::string>(beatmap->beatmapLevel->get_levelID());
        getLogger().info("Trying to find song '%s'...", levelId.c_str());

        if(!songDetails->songs.FindByHash(levelId.substr(13), currentSong))
        {
            getLogger().error("Could not find song");
            // Show modal with error
            return;
        }

        songNameText->SetText(currentSong->songName());
        songAuthorText->SetText(currentSong->songAuthorName());
        songDurationText->SetText("Duration: " + TimeFormat(currentSong->songDurationSeconds));

        time_t time = currentSong->uploadTimeUnix;
        tm* local = localtime(&time);
        uploadDateText->SetText(fmt::format("Uploaded: {} {}{} {}", months[local->tm_mon], local->tm_wday, GetDateEnding(local->tm_wday),1900 + local->tm_year));

        int rating = currentSong->rating() * 100;
        songRatingText->SetText(fmt::format("Rating: {}% - <color=green>{}</color> / <color=red>{}</color>", rating, currentSong->upvotes, currentSong->downvotes));

        List<StringW>* difflist = List<StringW>::New_ctor();
        for(int i = 0; i < 5; i++)
        {
            const SongDetailsCache::SongDifficulty* diff;
            if(currentSong->GetDifficulty(diff, SongDetailsCache::MapDifficulty(i), static_cast<std::string>(beatmap->beatmapCharacteristic->serializedName)))
            {
                difflist->Add(difficulties[i]);
                currentDifficulties.push_back(diff);
                if(i == beatmap->beatmapDifficulty.value)
                    PopulateWithDifficultyData(diff);
            }
        }
        difficultyTextSegment->SetTexts(difflist->i_IReadOnlyList_1_T());
        difficultyTextSegment->SelectCellWithNumber(difflist->IndexOf(difficulties[beatmap->beatmapDifficulty.value]));

        for(auto text : difficultyTextSegment->GetComponentsInChildren<HMUI::CurvedTextMeshPro*>())
        {
            text->set_fontSize(4.0f);
        }

        getLogger().info("Downloading cover from URL '%s'", currentSong->coverURL().c_str());
        BSML::Utilities::DownloadData(currentSong->coverURL(), [this](ArrayW<uint8_t> data)
        {
            auto sprite = BSML::Utilities::LoadSpriteRaw(data, 2);
            songCoverImage->set_sprite(sprite);
        });
    }

    void ModalView::PopulateWithDifficultyData(const SongDetailsCache::SongDifficulty* difficulty)
    {
        currentDifficulty = difficulty;
        lastSelectedDifficulty = (int)difficulty->difficulty;
        getLogger().info("Showing info for difficulty %i", (int)difficulty->difficulty);

        float nps = (float)difficulty->notes / (float)currentSong->songDurationSeconds;
        notesPerSecondText->SetText(fmt::format("{:.2f} NPS", nps));

        if(difficulty->ranked())
        {
            rankedStatusText->SetText("Ranked");
            rankedStatusText->set_color(UnityEngine::Color::get_green());

            ppValueText->SetText(fmt::format("{:.2f} PP", difficulty->approximatePpValue()));
            ppValueText->get_gameObject()->SetActive(true);

            starValueText->SetText(fmt::format("{:.2f} Stars", difficulty->stars));
            starValueText->get_gameObject()->SetActive(true);
        }
        else
        {
            rankedStatusText->SetText("Not ranked");
            rankedStatusText->set_color(UnityEngine::Color::get_red());

            ppValueText->get_gameObject()->SetActive(false);
            starValueText->get_gameObject()->SetActive(false);
        }
    }

    void ModalView::SelectButtonClick()
    {
        if(!lobbyPlayersDataModel)
        {
            getLogger().error("LobbyPlayersDataModel was null!");
            return;
        }
        getLogger().info("Constructing new beatmap...");
        auto newBeatmap = GlobalNamespace::PreviewDifficultyBeatmap::New_ctor(currentBeatmap->beatmapLevel, currentBeatmap->beatmapCharacteristic, GlobalNamespace::BeatmapDifficulty(lastSelectedDifficulty));
        getLogger().info("Setting beatmap...");
        lobbyPlayersDataModel->SetLocalPlayerBeatmapLevel(newBeatmap);
        getLogger().info("Done!");
        modal->Hide();
    }

    void ModalView::DifficultySegmentClick(HMUI::SegmentedControl* segmentedControl, int idx)
    {
        PopulateWithDifficultyData(currentDifficulties[idx]);
    }

    std::string ModalView::TimeFormat(float duration)
    {
        int minutes = duration / 60;
        int seconds = (int)duration % 60;

        std::string dur = std::to_string(minutes) + ":";
        if(seconds < 10)
            dur += "0";
        dur += std::to_string(seconds);
        return dur;
    }

    std::string ModalView::GetDateEnding(int date)
    {
        if(date == 1 || date == 21 || date == 31)
            return "st";
        else if(date == 2 || date == 22)
            return "nd";
        else if(date == 3 || date == 23)
            return "rd";
        return "th";
    }

    void ModalView::ctor(UnityEngine::Transform* parent, GlobalNamespace::ILobbyPlayersDataModel* model)
    {
        getLogger().info("Creating modal");
        BSML::parse_and_construct(IncludedAssets::ModalView_bsml, parent, this);
        modal->get_transform()->set_name("MultiplayerSongInfoModal");
        modal->dismissOnBlockerClicked = true;
        lobbyPlayersDataModel = model;
        getLogger().info("Created modal");
    }
};