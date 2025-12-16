// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_STD_WC_AudioPlayer.generated.h"

class UMVE_STU_WC_ConcertStudioPanel;
struct FMVE_STD_AudioSearchResultData;
class UImage;
class UButton;
class USlider;
class UTextBlock;
class UAudioComponent;
class USoundWave;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAudioPlayerPlayClicked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAudioPlayerStopClicked);

// 오디오 플레이어
UCLASS()
class MVE_API UMVE_STD_WC_AudioPlayer : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "AudioPlayer|Event")
	FOnAudioPlayerPlayClicked OnPlayClicked;

	UPROPERTY(BlueprintAssignable, Category = "AudioPlayer|Event")
	FOnAudioPlayerStopClicked OnStopClicked;
	
	void SetAudioData(const FMVE_STD_AudioSearchResultData& AudioData) const;

	// This function can be called by the owning panel to update the UI state
	void UpdatePlayPauseButton(bool bIsPlaying) const;
	
	// This function can be called to reset the playback UI (slider, time)
	void ResetPlaybackUI() const;

	// This function can be called to update the time and slider
	void UpdatePlaybackProgress(float CurrentTime, float TotalDuration);

protected:
	virtual void NativeConstruct() override;
	
	virtual void NativeOnInitialized() override;

	virtual void NativeDestruct() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> PlayButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> PlayStateImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> NextButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> PrevButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<USlider> PlaybackSlider;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> CurrentTimeTextBlock;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TotalTimeTextBlock;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TitleTextBlock;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ArtistTextBlock;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AudioPlayer|Icon")
	TObjectPtr<UTexture2D> PlayImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AudioPlayer|Icon")
	TObjectPtr<UTexture2D> PauseImage;

private:
	UFUNCTION()
	void OnPlayButtonClicked();

	UFUNCTION()
	void OnNextButtonClicked();

	UFUNCTION()
	void OnPrevButtonClicked();
	
	// Note: Slider interaction logic might need to be re-evaluated
	// as the playback is now driven externally. For now, we leave them.
	UFUNCTION()
    void OnPlaybackSliderMouseCaptureBegin();

    UFUNCTION()
    void OnPlaybackSliderMouseCaptureEnd();
	
    UFUNCTION()
    void OnPlaybackSliderValueChanged(float Value);
	
	static FString FormatTime(int32 TotalSeconds);
};
