#include "../Public/MVE_STU_WC_ConcertStudioPanel.h"
#include "MVE.h"
#include "../Public/MVE_STD_WC_AudioPlayer.h"
#include "../Public/MVE_STD_WC_AudioSearch.h"
#include "StageLevel/Widget/Public/MVE_STU_WidgetController_StudioConcert.h"
#include "MVE_GIS_SessionManager.h"

void UMVE_STU_WC_ConcertStudioPanel::NativeConstruct()
{
	Super::NativeConstruct();

	// Controller 생성 및 초기화
	AudioController = NewObject<UMVE_STU_WidgetController_StudioConcert>(this);
	if (AudioController)
	{
		AudioController->Initialize(AudioPlayer, AudioSearch);
		PRINTNETLOG(this, TEXT("AudioController initialized successfully"));
	}
	else
	{
		PRINTNETLOG(this, TEXT("Failed to create AudioController"));
		return;
	}
    
	// AudioSearch 이벤트 → Controller 연결
	if (AudioSearch)
	{
		AudioSearch->OnAudioSearchResultSelected.AddDynamic(
			AudioController, &UMVE_STU_WidgetController_StudioConcert::OnTrackSelected);
		PRINTNETLOG(this, TEXT("AudioSearch events connected to Controller"));

		// SessionManager에서 PlaylistBuilder로부터 받은 재생목록 가져오기
		if (const UGameInstance* GameInstance = GetGameInstance())
		{
			if (UMVE_GIS_SessionManager* SessionManager = GameInstance->GetSubsystem<UMVE_GIS_SessionManager>())
			{
				if (TArray<FAudioFile> PendingPlaylist = SessionManager->GetPendingPlaylist(); PendingPlaylist.Num() > 0)
				{
					PRINTNETLOG(this, TEXT("PlaylistBuilder에서 받은 재생목록 적용: %d곡"), PendingPlaylist.Num());
					AudioSearch->SetPlaylistFromBuilder(PendingPlaylist);

					// 사용 완료 후 초기화
					SessionManager->ClearPendingPlaylist();
				}
				else
				{
					PRINTNETLOG(this, TEXT("PlaylistBuilder에서 받은 재생목록 없음 - 기본 동작"));
				}
			}
		}
	}
	else
	{
		PRINTNETLOG(this, TEXT("AudioSearch widget is null"));
	}
}

void UMVE_STU_WC_ConcertStudioPanel::NativeDestruct()
{
	PRINTNETLOG(this, TEXT("AudioPanel destroyed"));
    
	Super::NativeDestruct();
}