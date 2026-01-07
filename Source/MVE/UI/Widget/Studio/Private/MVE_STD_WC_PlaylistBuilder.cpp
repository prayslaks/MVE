// Fill out your copyright notice in the Description page of Project Settings.

#include "MVE_STD_WC_PlaylistBuilder.h"
#include "MVE_API_Helper.h"
#include "MVE_STD_WC_AudioSearchResult.h"
#include "MVE_GIS_SessionManager.h"
#include "MVE_StageLevel_EffectSequenceManager.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "commu/Public/SenderReceiver.h"
#include "MVE.h"
#include "Components/ScrollBoxSlot.h"

void UMVE_STD_WC_PlaylistBuilder::NativeConstruct()
{
	Super::NativeConstruct();

	// 검색창 이벤트 바인딩
	if (SearchBox)
	{
		SearchBox->OnTextChanged.AddDynamic(this, &UMVE_STD_WC_PlaylistBuilder::OnSearchTextChanged);
	}

	// AI 분석 버튼 이벤트 바인딩
	if (AnalyzePlaylistButton)
	{
		AnalyzePlaylistButton->OnClicked.AddDynamic(this, &UMVE_STD_WC_PlaylistBuilder::OnAnalyzePlaylistClicked);
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

void UMVE_STD_WC_PlaylistBuilder::OnAnalyzePlaylistClicked()
{
	PRINTLOG(TEXT("AI 분석 시작 버튼 클릭됨!"));
	
	// 재생목록이 비어있는지 확인
	if (Playlist.Num() == 0)
	{
		PRINTLOG(TEXT("❌ 재생목록이 비어있습니다. 먼저 곡을 추가하세요."));
		return;
	}

	PRINTLOG(TEXT("📤 재생목록 전체 분석 요청 - %d곡"), Playlist.Num());
	
	OnBatchAnalyzeRequested.Broadcast();
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

			// 드래그 가능하도록 설정
			PlaylistItemWidget->SetDraggable(true);
			PlaylistItemWidget->SetPlaylistIndex(Playlist.Num() - 1);
			PlaylistItemWidget->SetPlaylistBuilder(this);

			// 삭제 이벤트 바인딩
			PlaylistItemWidget->OnDeleteRequested.AddUObject(this, &UMVE_STD_WC_PlaylistBuilder::OnPlaylistItemDeleteRequested);

			// 클릭 이벤트 바인딩 (음악 선택)
			PlaylistItemWidget->OnAudioSearchResultClicked.AddUObject(this, &UMVE_STD_WC_PlaylistBuilder::OnPlaylistItemClicked);

			// ScrollBox에 추가
			UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(PlaylistScrollBox->AddChild(PlaylistItemWidget));
			if (ScrollSlot)
			{
				ScrollSlot->SetPadding(FMargin(0, 0, 0, 10));
			}
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

void UMVE_STD_WC_PlaylistBuilder::SavePlaylistToSessionManager()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UMVE_GIS_SessionManager* SessionManager = GameInstance->GetSubsystem<UMVE_GIS_SessionManager>())
		{
			SessionManager->SetPendingPlaylist(Playlist);
			PRINTLOG(TEXT("SessionManager에 재생목록 저장 완료: %d곡"), Playlist.Num());
		}
		else
		{
			PRINTLOG(TEXT("SessionManager를 찾을 수 없습니다!"));
		}
	}
	else
	{
		PRINTLOG(TEXT("GameInstance를 찾을 수 없습니다!"));
	}
}

void UMVE_STD_WC_PlaylistBuilder::SetEffectSequenceManager(AMVE_StageLevel_EffectSequenceManager* Manager)
{
	EffectSequenceManager = Manager;
	if (EffectSequenceManager)
	{
		PRINTLOG(TEXT("PlaylistBuilder에 EffectSequenceManager 설정 완료"));
	}
}

void UMVE_STD_WC_PlaylistBuilder::ReorderPlaylistItem(int32 FromIndex, int32 ToIndex)
{
	if (!PlaylistScrollBox || FromIndex < 0 || ToIndex < 0 ||
		FromIndex >= Playlist.Num() || ToIndex >= Playlist.Num())
	{
		PRINTLOG(TEXT("ReorderPlaylistItem: 잘못된 인덱스 (From: %d, To: %d, Size: %d)"),
			FromIndex, ToIndex, Playlist.Num());
		return;
	}

	if (FromIndex == ToIndex)
	{
		return;
	}

	PRINTLOG(TEXT("재생목록 순서 변경: %d → %d"), FromIndex, ToIndex);

	// 1. Playlist 배열 재배열
	FAudioFile MovedItem = Playlist[FromIndex];
	Playlist.RemoveAt(FromIndex);
	Playlist.Insert(MovedItem, ToIndex);

	// 2. ScrollBox 위젯들을 임시 배열에 저장
	TArray<UWidget*> WidgetArray;
	for (int32 i = 0; i < PlaylistScrollBox->GetChildrenCount(); ++i)
	{
		WidgetArray.Add(PlaylistScrollBox->GetChildAt(i));
	}

	// 3. 위젯 배열 재배열
	if (WidgetArray.IsValidIndex(FromIndex) && WidgetArray.IsValidIndex(ToIndex))
	{
		UWidget* MovedWidget = WidgetArray[FromIndex];
		WidgetArray.RemoveAt(FromIndex);
		WidgetArray.Insert(MovedWidget, ToIndex);
	}

	// 4. ScrollBox 초기화 후 재배열된 순서대로 다시 추가
	PlaylistScrollBox->ClearChildren();
	for (UWidget* Widget : WidgetArray)
	{
		// ScrollBox에 추가
		UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(PlaylistScrollBox->AddChild(Widget));
		if (ScrollSlot)
		{
			ScrollSlot->SetPadding(FMargin(0, 0, 0, 10));
		}
	}

	// 5. 모든 항목의 인덱스 재설정
	RefreshPlaylistIndices();

	PRINTLOG(TEXT("재생목록 순서 변경 완료!"));
}

void UMVE_STD_WC_PlaylistBuilder::OnPlaylistItemDropped(int32 FromIndex, int32 ToIndex)
{
	ReorderPlaylistItem(FromIndex, ToIndex);
}

void UMVE_STD_WC_PlaylistBuilder::RefreshPlaylistIndices()
{
	if (!PlaylistScrollBox)
	{
		return;
	}

	for (int32 i = 0; i < PlaylistScrollBox->GetChildrenCount(); ++i)
	{
		if (UMVE_STD_WC_AudioSearchResult* ItemWidget = Cast<UMVE_STD_WC_AudioSearchResult>(PlaylistScrollBox->GetChildAt(i)))
		{
			ItemWidget->SetPlaylistIndex(i);
		}
	}
}

void UMVE_STD_WC_PlaylistBuilder::RemovePlaylistItem(int32 Index)
{
	if (!PlaylistScrollBox || Index < 0 || Index >= Playlist.Num())
	{
		PRINTLOG(TEXT("RemovePlaylistItem: 잘못된 인덱스 (Index: %d, Size: %d)"), Index, Playlist.Num());
		return;
	}

	PRINTLOG(TEXT("재생목록에서 항목 제거: %s (Index: %d)"), *Playlist[Index].Title, Index);

	// 1. Playlist 배열에서 제거
	Playlist.RemoveAt(Index);

	// 2. ScrollBox 위젯 제거
	PlaylistScrollBox->RemoveChildAt(Index);

	// 3. 모든 항목의 인덱스 재설정
	RefreshPlaylistIndices();

	PRINTLOG(TEXT("재생목록 항목 제거 완료! 남은 항목: %d개"), Playlist.Num());
}

void UMVE_STD_WC_PlaylistBuilder::OnPlaylistItemDeleteRequested(UMVE_STD_WC_AudioSearchResult* Widget)
{
	if (!Widget)
	{
		return;
	}

	int32 Index = Widget->GetPlaylistIndex();
	RemovePlaylistItem(Index);
}

void UMVE_STD_WC_PlaylistBuilder::OnPlaylistItemClicked(UMVE_STD_WC_AudioSearchResult* ClickedWidget)
{
	if (!ClickedWidget)
	{
		return;
	}

	int32 Index = ClickedWidget->GetPlaylistIndex();
	if (Index < 0 || Index >= Playlist.Num())
	{
		PRINTLOG(TEXT("잘못된 재생목록 인덱스: %d"), Index);
		return;
	}

	// 선택된 음악 저장
	SelectedAudioFile = Playlist[Index];
	PRINTLOG(TEXT("음악 선택됨: %s - %s"), *SelectedAudioFile.Title, *SelectedAudioFile.Artist);

	// 🔹 먼저 EffectSequenceManager->StopSequence() 호출
	if (EffectSequenceManager)
	{
		EffectSequenceManager->StopSequence();
		PRINTLOG(TEXT("EffectSequenceManager->StopSequence() 호출됨"));
	}

	// 🔹 그 다음 델리게이트 브로드캐스트
	OnAudioFileSelected.Broadcast(SelectedAudioFile);
}
