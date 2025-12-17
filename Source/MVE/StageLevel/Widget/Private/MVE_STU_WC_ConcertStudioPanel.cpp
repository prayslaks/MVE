#include "../Public/MVE_STU_WC_ConcertStudioPanel.h"
#include "MVE.h"
#include "../Public/MVE_STD_WC_AudioPlayer.h"
#include "../Public/MVE_STD_WC_AudioSearch.h"
#include "StageLevel/Widget/Public/MVE_STU_WidgetController_StudioConcert.h"

void UMVE_STU_WC_ConcertStudioPanel::NativeConstruct()
{
	Super::NativeConstruct();

	// Controller 생성 및 초기화
	AudioController = NewObject<UMVE_STU_WidgetController_StudioConcert>(this);
	if (AudioController)
	{
		AudioController->Initialize(AudioPlayer);
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
