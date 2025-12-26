#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StageLevel/Data/MVE_EffectSequenceData.h"
#include "API/Public/MVE_API_ResponseData.h"
#include "MVE_STU_WC_EffectSequencePreview.generated.h"

class AMVE_StageLevel_EffectSequenceManager;
class UButton;
class USlider;
class UTextBlock;
class UImage;
class UCanvasPanel;
class UTexture2D;
class UAudioComponent;
class USoundWave;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEffectPreviewPlayClicked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEffectPreviewStopClicked);

/**
 * Effect Sequence Preview Widget
 * AI 분석 결과를 시각화하고 음악 프리뷰를 제공하는 위젯
 * - 음악 재생 타임라인 슬라이더
 * - TimeStamp 위치에 이펙트 아이콘 표시
 * - EffectSequenceManager와 연동하여 실시간 이펙트 재생
 */
UCLASS()
class MVE_API UMVE_STU_WC_EffectSequencePreview : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 재생 버튼 클릭 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Effect Preview|Event")
	FOnEffectPreviewPlayClicked OnPlayClicked;

	/** 정지 버튼 클릭 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Effect Preview|Event")
	FOnEffectPreviewStopClicked OnStopClicked;

	/**
	 * AI 분석 결과를 설정하고 타임라인에 아이콘 생성
	 * @param SequenceData AI가 분석한 TimeStamp와 AssetID 배열
	 * @param TotalDuration 음악 총 길이 (1/10초 단위)
	 */
	UFUNCTION(BlueprintCallable, Category = "Effect Preview")
	void SetSequenceData(const TArray<FEffectSequenceData>& SequenceData, int32 TotalDuration);

	/**
	 * 재생 진행 상황 업데이트
	 * @param CurrentTimeStamp 현재 재생 시간 (1/10초 단위)
	 */
	UFUNCTION(BlueprintCallable, Category = "Effect Preview")
	void UpdatePlaybackProgress(int32 CurrentTimeStamp);

	/**
	 * 재생/일시정지 버튼 상태 업데이트
	 * @param bIsPlaying 재생 중 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Effect Preview")
	void UpdatePlayPauseButton(bool bIsPlaying);

	/**
	 * 프리뷰 초기화
	 */
	UFUNCTION(BlueprintCallable, Category = "Effect Preview")
	void ResetPreview();

	/**
	 * EffectSequenceManager 설정
	 */
	UFUNCTION(BlueprintCallable, Category = "Effect Preview")
	void SetEffectSequenceManager(AMVE_StageLevel_EffectSequenceManager* Manager);

	/**
	 * 스테이지 프리뷰 RenderTarget 설정
	 * @param RenderTarget StagePreviewCaptureActor가 캡처한 RenderTarget
	 */
	UFUNCTION(BlueprintCallable, Category = "Effect Preview")
	void SetRenderTarget(UTextureRenderTarget2D* RenderTarget);

	/**
	 * 테스트용 더미 데이터 로드
	 * AI 연동 전 테스트를 위한 임시 함수
	 */
	UFUNCTION(BlueprintCallable, Category = "Effect Preview|Test")
	void LoadTestData();

	/**
	 * 재생할 음악 설정
	 * @param AudioFile PlaylistBuilder에서 선택한 음악 정보
	 */
	UFUNCTION(BlueprintCallable, Category = "Effect Preview")
	void SetAudioFile(const FAudioFile& AudioFile);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> StagePreviewImage;

	/** 재생/일시정지 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PlayButton;

	/** 재생/일시정지 아이콘 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PlayStateImage;

	/** 타임라인 슬라이더 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> PlaybackSlider;

	/** 현재 재생 시간 텍스트 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrentTimeTextBlock;

	/** 총 재생 시간 텍스트 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TotalTimeTextBlock;

	/** 이펙트 아이콘을 배치할 Canvas Panel */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> IconCanvas;

	/** 재생 아이콘 텍스처 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect Preview|Icon")
	TObjectPtr<UTexture2D> PlayIconTexture;

	/** 일시정지 아이콘 텍스처 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect Preview|Icon")
	TObjectPtr<UTexture2D> PauseIconTexture;

	/** Spotlight 아이콘 텍스처 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect Preview|Icon")
	TObjectPtr<UTexture2D> SpotlightIconTexture;

	/** Flame 아이콘 텍스처 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect Preview|Icon")
	TObjectPtr<UTexture2D> FlameIconTexture;

	/** Fanfare 아이콘 텍스처 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect Preview|Icon")
	TObjectPtr<UTexture2D> FanfareIconTexture;

private:
	
	/** 재생 버튼 클릭 핸들러 */
	UFUNCTION()
	void OnPlayButtonClicked();

	/** 슬라이더 드래그 시작 핸들러 */
	UFUNCTION()
	void OnPlaybackSliderMouseCaptureBegin();

	/** 슬라이더 드래그 종료 핸들러 */
	UFUNCTION()
	void OnPlaybackSliderMouseCaptureEnd();

	/** 슬라이더 값 변경 핸들러 */
	UFUNCTION()
	void OnPlaybackSliderValueChanged(float Value);

	/**
	 * TimeStamp 배열을 기반으로 타임라인에 아이콘 생성
	 */
	void CreateEffectIcons();

	/**
	 * 모든 아이콘 제거
	 */
	void ClearEffectIcons();

	/**
	 * GameplayTag에 따라 적절한 아이콘 텍스처 반환
	 */
	UTexture2D* GetIconTextureForTag(const FGameplayTag& Tag) const;

	/**
	 * 시간 포맷 (1/10초 → MM:SS 형식)
	 * @param TimeStamp 시간 (1/10초 단위)
	 * @return 포맷된 문자열 (예: "2:31")
	 */
	static FString FormatTime(int32 TimeStamp);

	/**
	 * AudioComponent의 재생 퍼센트 업데이트 콜백
	 */
	UFUNCTION()
	void OnAudioPlaybackPercentChanged(const USoundWave* PlayingSoundWave, const float PlaybackPercent);

private:
	/** AI 분석 결과 */
	TArray<FEffectSequenceData> SequenceDataArray;

	/** 음악 총 길이 (1/10초 단위) */
	int32 TotalDurationTimeStamp;

	/** EffectSequenceManager 레퍼런스 */
	UPROPERTY()
	AMVE_StageLevel_EffectSequenceManager* EffectSequenceManager;

	/** 생성된 아이콘 위젯들 (정리용) */
	UPROPERTY()
	TArray<UImage*> EffectIconWidgets;

	/** 슬라이더 드래그 중 여부 */
	bool bIsSliderDragging;

	/** 음악 재생용 AudioComponent */
	UPROPERTY()
	UAudioComponent* AudioComponent;

	/** 현재 선택된 음악 정보 */
	UPROPERTY()
	FAudioFile CurrentAudioFile;

	/** 현재 로드된 사운드 */
	UPROPERTY()
	USoundWave* CurrentSound;
};
