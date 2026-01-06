#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StageLevel/Data/MVE_EffectSequenceData.h"
#include "API/Public/MVE_API_ResponseData.h"
#include "MVE_STU_WC_EffectSequencePreview.generated.h"

class UOverlay;
class AMVE_StageLevel_EffectSequenceManager;
class UButton;
class USlider;
class UTextBlock;
class UImage;
class UCanvasPanel;
class UTexture2D;
class UAudioComponent;
class USoundWave;
class UglTFRuntimeAsset;

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
	 * 음악 총 길이 가져오기 (1/10초 단위)
	 */
	UFUNCTION(BlueprintPure, Category = "Effect Preview")
	int32 GetTotalDurationTimeStamp() const { return TotalDurationTimeStamp; }

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
	 * @param AudioFile 더미 데이터를 생성할 음악 정보 (곡 길이 기반으로 동적 생성)
	 */
	UFUNCTION(BlueprintCallable, Category = "Effect Preview|Test")
	void LoadTestData(const FAudioFile& AudioFile);

	/**
	 * 재생할 음악 설정
	 * @param AudioFile PlaylistBuilder에서 선택한 음악 정보
	 */
	UFUNCTION(BlueprintCallable, Category = "Effect Preview")
	void SetAudioFile(const FAudioFile& AudioFile);

	/**
	 * 곡 길이 기반으로 테스트 데이터 생성 (내부 헬퍼 함수)
	 * @param TotalDuration 곡 총 길이 (1/10초 단위)
	 * @param SongTitle 곡 제목 (로그용)
	 */
	void GenerateTestDataFromDuration(int32 TotalDuration, const FString& SongTitle);

	/**
	 * 로딩 애니메이션 시작 (AI 서버 응답 대기 중 표시)
	 */
	UFUNCTION(BlueprintCallable, Category = "Effect Preview")
	void StartLoadingAnimation();

	/**
	 * 로딩 애니메이션 중지 (StagePreviewImage 복원)
	 */
	UFUNCTION(BlueprintCallable, Category = "Effect Preview")
	void StopLoadingAnimation();

	/**
	 * 지정된 시간 동안 로딩 애니메이션 재생 후 자동 중지 (TestMode용)
	 * @param Duration 로딩 애니메이션 재생 시간 (초)
	 */
	UFUNCTION(BlueprintCallable, Category = "Effect Preview")
	void StartLoadingAnimationWithDuration(float Duration);

	/** 테스트 모드 - true이면 더미 데이터 사용, false이면 AI 서버 응답 대기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Preview|Test")
	bool bTestMode = true;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> StagePreviewImage;

	/** 로딩 애니메이션 오버레이 이미지 (StagePreviewImage 위에 표시) */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> LoadingOverlayImage;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> LoadingBackgroundImage;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> LoadingOverlay;

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

	/** 로딩 애니메이션 프레임 (GIF를 여러 PNG로 나눠서 배열로 저장) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect Preview|Loading")
	TArray<TObjectPtr<UTexture2D>> LoadingFrames;

	/** 로딩 애니메이션 프레임 전환 속도 (초 단위) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect Preview|Loading")
	float LoadingFrameRate = 0.1f;

	/** 로딩 시작 시 재생할 사운드 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect Preview|Loading")
	TObjectPtr<USoundBase> LoadingStartSound;

	/** 로딩 중 반복 재생할 배경 음악 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect Preview|Loading")
	TObjectPtr<USoundBase> LoadingLoopSound;



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
	static FString FormatTime(const int32 TimeStamp);

	/**
	 * AudioComponent의 재생 퍼센트 업데이트 콜백
	 */
	UFUNCTION()
	void OnAudioPlaybackPercentChanged(const USoundWave* PlayingSoundWave, const float PlaybackPercent);

	/**
	 * glTFRuntime에서 URL로부터 오디오 로드 완료 시 호출되는 콜백
	 */
	UFUNCTION()
	void OnAudioLoadedFromUrl(UglTFRuntimeAsset* Asset);

	

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

	/** 오디오 일시정지 상태 */
	bool bIsAudioPaused;

	/** 음악 재생용 AudioComponent */
	UPROPERTY()
	UAudioComponent* AudioComponent;

	/** 현재 선택된 음악 정보 */
	UPROPERTY()
	FAudioFile CurrentAudioFile;

	/** 현재 로드된 사운드 */
	UPROPERTY()
	USoundWave* CurrentSound;

	/** Asset 캐시 (AudioFileId → glTFRuntimeAsset) */
	UPROPERTY()
	TMap<int32, UglTFRuntimeAsset*> CachedAssets;

	/** 로딩 애니메이션 타이머 핸들 (프레임 전환용) */
	FTimerHandle LoadingAnimationTimerHandle;

	/** 로딩 애니메이션 자동 중지 타이머 핸들 */
	FTimerHandle LoadingAnimationAutoStopTimerHandle;

	/** 현재 로딩 프레임 인덱스 */
	int32 CurrentLoadingFrameIndex = 0;

	/** 로딩 애니메이션 활성 여부 */
	bool bIsLoadingAnimationActive = false;

	/** 로딩 애니메이션 프레임 업데이트 */
	void UpdateLoadingFrame();

	/** 로딩 배경음악 오디오 컴포넌트 */
	UPROPERTY()
	TObjectPtr<UAudioComponent> LoadingAudioComponent;
};
