
#pragma once

#include "CoreMinimal.h"
#include "MVE_STD_WC_AudioSearchResult.h"
#include "UObject/Object.h"
#include "MVE_STU_WidgetController_StudioConcert.generated.h"

class UMVE_STD_WC_AudioPlayer;
class UMVE_STD_WC_AudioSearch;
struct FMVE_STD_AudioSearchResultData;

UCLASS()
class MVE_API UMVE_STU_WidgetController_StudioConcert : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Controller 초기화
	 * @param InPlayerWidget AudioPlayer 위젯 참조
	 */
	void Initialize(UMVE_STD_WC_AudioPlayer* InPlayerWidget);

	/**
	 * AudioSearch에서 트랙이 선택되었을 때 호출
	 * @param AudioData 선택된 오디오 데이터
	 */
	UFUNCTION()
	void OnTrackSelected(const FMVE_STD_AudioSearchResultData& AudioData);

	/**
	 * AudioPlayer에서 Play 버튼 클릭 시 호출
	 */
	UFUNCTION()
	void OnPlayRequested();

	/**
	 * AudioPlayer에서 Stop 버튼 클릭 시 호출
	 */
	UFUNCTION()
	void OnStopRequested();

private:
	/**
	 * Presigned URL을 로드하고 모든 클라이언트에 전송
	 * @param AudioData 오디오 데이터
	 */
	void LoadPresignedUrlAndBroadcast(const FMVE_STD_AudioSearchResultData& AudioData);

	/**
	 * Play 명령을 모든 클라이언트에 전송
	 */
	void SendPlayCommandToClients();

	/**
	 * Stop 명령을 모든 클라이언트에 전송
	 */
	void SendStopCommandToClients();

	/**
	 * AudioPlayer UI 초기화
	 * @param AudioData 설정할 오디오 데이터
	 */
	void SetupPlayerUI(const FMVE_STD_AudioSearchResultData& AudioData);

private:
	/** AudioPlayer 위젯 참조 */
	UPROPERTY()
	TObjectPtr<UMVE_STD_WC_AudioPlayer> PlayerWidget;

	/** 현재 선택된 트랙 정보 */
	FMVE_STD_AudioSearchResultData CurrentTrackData;
};
