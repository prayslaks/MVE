
#pragma once

#include "CoreMinimal.h"
#include "MVE_STD_WC_AudioSearchResult.h"
#include "STT/Public/STTSubsystem.h"
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
	 * @param InAudioSearch AudioSearch 위젯 참조
	 */
	void Initialize(UMVE_STD_WC_AudioPlayer* InPlayerWidget, UMVE_STD_WC_AudioSearch* InAudioSearch);

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

	/**
	 * 다음 곡으로 이동
	 * @param bAutoPlay 자동 재생 여부 (기본값: true)
	 */
	UFUNCTION()
	void OnNextTrackRequested(bool bAutoPlay = true);

	/**
	 * 이전 곡으로 이동
	 * @param bAutoPlay 자동 재생 여부 (기본값: true)
	 */
	UFUNCTION()
	void OnPreviousTrackRequested(bool bAutoPlay = true);

	// Setter
	void SetCurrentTrackData(const FMVE_STD_AudioSearchResultData& Data);

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

	/*
	 * STT 연동 콜백함수
	 * @param
	 */
	UFUNCTION()
	void HandleVoiceCommand(ESTTCommandType CommandType, const FString& OriginalText);

	/**
	 * SearchResultWidgets에서 특정 트랙의 인덱스 찾기
	 * @param TrackData 찾을 트랙 데이터
	 * @return 인덱스 (찾지 못하면 -1)
	 */
	int32 FindTrackIndex(const FMVE_STD_AudioSearchResultData& TrackData) const;

	UPROPERTY(VisibleAnywhere)
	bool bIsPlaying;
	
	/** AudioPlayer 위젯 참조 */
	UPROPERTY()
	TObjectPtr<UMVE_STD_WC_AudioPlayer> PlayerWidget;

	/** AudioSearch 위젯 참조 */
	UPROPERTY()
	TObjectPtr<UMVE_STD_WC_AudioSearch> AudioSearch;

	/** 현재 선택된 트랙 정보 */
	FMVE_STD_AudioSearchResultData CurrentTrackData;

	/** 트랙 선택 후 자동 재생 여부 */
	bool bAutoPlayAfterSelection = false;
};
