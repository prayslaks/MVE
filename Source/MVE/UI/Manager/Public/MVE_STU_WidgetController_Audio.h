
#pragma once

#include "CoreMinimal.h"
#include "MVE_STD_WC_AudioSearchResult.h"
#include "UObject/Object.h"
#include "MVE_STU_WidgetController_Audio.generated.h"

class UMVE_STD_WC_AudioPlayer;

UCLASS()
class MVE_API UMVE_STU_WidgetController_Audio : public UObject
{
	GENERATED_BODY()

	/*
public:
	// Search에서 호출
	void OnTrackSelected(const FMVE_STD_AudioSearchResultData& AudioData);
    
	// Player에서 호출
	void OnPlayRequested();
	void OnStopRequested();
    
private:
	// 내부 로직
	void LoadPresignedUrl(const FMVE_STD_AudioSearchResultData& AudioData);
	void SendPlayCommand();
	void SendStopCommand();
    
	// 델리게이트 (Player 업데이트용)
	FOnAudioDataLoaded OnAudioDataLoaded;
    
	// 참조
	TObjectPtr<UMVE_STD_WC_AudioPlayer> PlayerWidget;
	FMVE_STD_AudioSearchResultData CurrentTrack;
	*/
};
