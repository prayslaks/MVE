// Fill out your copyright notice in the Description page of Project Settings.

#include "MVE_STD_WC_PlaylistBuilder.h"
#include "MVE_API_Helper.h"
#include "MVE_STD_WC_AudioSearchResult.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "MVE.h"

void UMVE_STD_WC_PlaylistBuilder::NativeConstruct()
{
	Super::NativeConstruct();

	// 검색창 이벤트 바인딩
	if (SearchBox)
	{
		SearchBox->OnTextChanged.AddDynamic(this, &UMVE_STD_WC_PlaylistBuilder::OnSearchTextChanged);
	}

	// 드롭다운 숨기기
	if (DropdownPanel)
	{
		DropdownPanel->SetVisibility(ESlateVisibility::Collapsed);
	}

	// 서버에서 음악 리스트 로드
	LoadMusicListFromServer();
}

void UMVE_STD_WC_PlaylistBuilder::LoadMusicListFromServer()
{
	PRINTLOG(TEXT("음악 리스트 로딩 시작..."));

	FOnGetAudioListComplete OnResult;
	OnResult.BindUObject(this, &UMVE_STD_WC_PlaylistBuilder::OnMusicListLoaded);
	UMVE_API_Helper::GetAudioList(OnResult);
}

void UMVE_STD_WC_PlaylistBuilder::OnMusicListLoaded(bool bSuccess, const FAudioListResponseData& ResponseData, const FString& ErrorCode)
{
	if (bSuccess)
	{
		CachedMusicList = ResponseData.AudioFiles;
		PRINTLOG(TEXT("음악 리스트 로딩 성공: %d개"), CachedMusicList.Num());
	}
	else
	{
		PRINTLOG(TEXT("음악 리스트 로딩 실패: %s"), *ErrorCode);
	}
}

void UMVE_STD_WC_PlaylistBuilder::OnSearchTextChanged(const FText& Text)
{
	FString SearchQuery = Text.ToString().TrimStartAndEnd();

	// 검색어가 비어있으면 드롭다운 숨기기
	if (SearchQuery.IsEmpty())
	{
		if (DropdownPanel)
		{
			DropdownPanel->SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	// 로컬 필터링
	TArray<FAudioFile> SearchResults;
	for (const FAudioFile& AudioFile : CachedMusicList)
	{
		// 제목 또는 아티스트에 검색어가 포함되면 추가
		if (AudioFile.Title.Contains(SearchQuery) || AudioFile.Artist.Contains(SearchQuery))
		{
			SearchResults.Add(AudioFile);
		}
	}

	// 드롭다운 업데이트
	UpdateDropdown(SearchResults);
}

void UMVE_STD_WC_PlaylistBuilder::UpdateDropdown(const TArray<FAudioFile>& Results)
{
	if (!DropdownPanel)
	{
		return;
	}

	// 기존 항목 제거
	DropdownPanel->ClearChildren();

	// 검색 결과가 없으면 드롭다운 숨기기
	if (Results.Num() == 0)
	{
		DropdownPanel->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// 드롭다운 표시
	DropdownPanel->SetVisibility(ESlateVisibility::Visible);

	// 검색 결과 추가 (최대 10개)
	int32 MaxResults = FMath::Min(Results.Num(), 10);
	for (int32 i = 0; i < MaxResults; ++i)
	{
		const FAudioFile& AudioFile = Results[i];

		if (SearchResultWidgetClass)
		{
			UMVE_STD_WC_AudioSearchResult* ResultWidget = CreateWidget<UMVE_STD_WC_AudioSearchResult>(this, SearchResultWidgetClass);
			if (ResultWidget)
			{
				// 데이터 설정
				FMVE_STD_AudioSearchResultData SearchResultData;
				SearchResultData.Id = AudioFile.Id;
				SearchResultData.Title = AudioFile.Title;
				SearchResultData.Artist = AudioFile.Artist;
				SearchResultData.Duration = AudioFile.Duration;
				ResultWidget->UpdateUI(SearchResultData);

				// 클릭 이벤트 바인딩 (싱글클릭)
				ResultWidget->OnAudioSearchResultClicked.AddUObject(this, &UMVE_STD_WC_PlaylistBuilder::OnSearchResultClicked);

				// 드롭다운에 추가
				DropdownPanel->AddChild(ResultWidget);
			}
		}
	}
}

void UMVE_STD_WC_PlaylistBuilder::OnSearchResultClicked(UMVE_STD_WC_AudioSearchResult* ClickedWidget)
{
	PRINTLOG(TEXT("검색 결과 클릭됨!"));

	if (!ClickedWidget)
	{
		PRINTLOG(TEXT("ClickedWidget이 null입니다!"));
		return;
	}

	// 클릭된 항목의 데이터 가져오기
	FMVE_STD_AudioSearchResultData ClickedData = ClickedWidget->GetAudioData();
	PRINTLOG(TEXT("클릭된 음악: %s (ID: %d)"), *ClickedData.Title, ClickedData.Id);

	// 캐시에서 전체 데이터 찾기
	for (const FAudioFile& AudioFile : CachedMusicList)
	{
		if (AudioFile.Id == ClickedData.Id)
		{
			AddToPlaylist(AudioFile);
			break;
		}
	}

	// 드롭다운 숨기기
	if (DropdownPanel)
	{
		DropdownPanel->SetVisibility(ESlateVisibility::Collapsed);
	}

	// 검색창 초기화
	if (SearchBox)
	{
		SearchBox->SetText(FText::GetEmpty());
	}
}

void UMVE_STD_WC_PlaylistBuilder::AddToPlaylist(const FAudioFile& AudioFile)
{
	PRINTLOG(TEXT("AddToPlaylist 호출: %s"), *AudioFile.Title);

	// 중복 체크
	for (const FAudioFile& ExistingFile : Playlist)
	{
		if (ExistingFile.Id == AudioFile.Id)
		{
			PRINTLOG(TEXT("이미 재생목록에 있는 음악: %s"), *AudioFile.Title);
			return;
		}
	}

	// 재생목록에 추가
	Playlist.Add(AudioFile);
	PRINTLOG(TEXT("재생목록에 추가: %s - %s"), *AudioFile.Title, *AudioFile.Artist);

	PRINTLOG(TEXT("PlaylistScrollBox: %s"), PlaylistScrollBox ? TEXT("존재") : TEXT("null"));
	PRINTLOG(TEXT("PlaylistItemWidgetClass: %s"), PlaylistItemWidgetClass ? TEXT("설정됨") : TEXT("null"));

	// ScrollBox에 위젯 추가
	if (PlaylistScrollBox && PlaylistItemWidgetClass)
	{
		UMVE_STD_WC_AudioSearchResult* PlaylistItemWidget = CreateWidget<UMVE_STD_WC_AudioSearchResult>(this, PlaylistItemWidgetClass);
		if (PlaylistItemWidget)
		{
			// 데이터 설정
			FMVE_STD_AudioSearchResultData ItemData;
			ItemData.Id = AudioFile.Id;
			ItemData.Title = AudioFile.Title;
			ItemData.Artist = AudioFile.Artist;
			ItemData.Duration = AudioFile.Duration;
			PlaylistItemWidget->UpdateUI(ItemData);

			// ScrollBox에 추가
			PlaylistScrollBox->AddChild(PlaylistItemWidget);
			PRINTLOG(TEXT("ScrollBox에 위젯 추가 성공!"));
		}
		else
		{
			PRINTLOG(TEXT("PlaylistItemWidget 생성 실패!"));
		}
	}
	else
	{
		if (!PlaylistScrollBox)
		{
			PRINTLOG(TEXT("PlaylistScrollBox가 null입니다!"));
		}
		if (!PlaylistItemWidgetClass)
		{
			PRINTLOG(TEXT("PlaylistItemWidgetClass가 설정되지 않았습니다!"));
		}
	}
}

void UMVE_STD_WC_PlaylistBuilder::ClearPlaylist()
{
	Playlist.Empty();

	if (PlaylistScrollBox)
	{
		PlaylistScrollBox->ClearChildren();
	}

	PRINTLOG(TEXT("재생목록 초기화됨"));
}
