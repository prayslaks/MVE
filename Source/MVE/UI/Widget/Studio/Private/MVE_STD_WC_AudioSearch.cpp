// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Studio/Public/MVE_STD_WC_AudioSearch.h"

#include "MVE.h"
#include "MVE_API_Helper.h"
#include "MVE_GIS_API.h"
#include "MVE_STD_WC_AudioPanel.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "UI/Widget/Studio/Public/MVE_STD_WC_AudioSearchResult.h"

void UMVE_STD_WC_AudioSearch::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (GetAudioListButton)
	{
		GetAudioListButton.Get()->OnClicked.AddDynamic(this, &UMVE_STD_WC_AudioSearch::OnGetAudioListButtonClicked);
	}
}

void UMVE_STD_WC_AudioSearch::OnGetAudioListButtonClicked()
{
	// 서버에 음원 리스트 요청
	FOnGetAudioListComplete OnResult;
	OnResult.BindLambda([this](const bool bSuccess, const FAudioListResponseData& ResponseData, const FString& ErrorCode)
	{
		if (bSuccess)
		{
			if (AudioListScrollBox)
			{
				AudioListScrollBox->ClearChildren();
			}
			SearchResultWidgets.Empty();
			SelectedSearchResult = nullptr;
			
			for (const FAudioFile& AudioFile : ResponseData.audio_files)
			{
				FString Title, Artist, FilePath;
				int32 Id, Duration;

				Title = AudioFile.title;
				Artist = AudioFile.artist;
				Id = AudioFile.id;
				Duration = AudioFile.duration;

				PRINTLOG(TEXT("Title: %s, Artist: %s, Duration: %s"), *Title, *Artist, *FilePath);

				// 위젯 생성 후 추가
				if(auto* NewWidget = CreateWidget<UMVE_STD_WC_AudioSearchResult>(this, AudioSearchResultWidgetClass))
				{
					FMVE_STD_AudioSearchResultData ResultData(Title, Artist, Id, Duration);
					NewWidget->UpdateUI(ResultData);
					NewWidget->OnAudioSearchResultDoubleClicked.AddUObject(this, &UMVE_STD_WC_AudioSearch::OnSearchResultDoubleClicked);
					SearchResultWidgets.Add(NewWidget);
					AudioListScrollBox->AddChild(NewWidget);
				}
			}
		}
		else
		{
			if (const UMVE_GIS_API* Subsystem = UMVE_GIS_API::Get(this))
			{
				FText TranslatedErrorMessage = Subsystem->GetTranslatedErrorMessage(ErrorCode);
			}
		}
	});
	UMVE_API_Helper::GetAudioList(OnResult);
}

void UMVE_STD_WC_AudioSearch::OnSearchResultDoubleClicked(UMVE_STD_WC_AudioSearchResult* ClickedWidget)
{
	if (!ClickedWidget || SelectedSearchResult == ClickedWidget)
	{
		return;
	}

	if (SelectedSearchResult)
	{
		SelectedSearchResult->SetSelected(false);
	}

	SelectedSearchResult = ClickedWidget;
	SelectedSearchResult->SetSelected(true);
	
	OnAudioSearchResultSelected.Broadcast(SelectedSearchResult->GetAudioData());
}