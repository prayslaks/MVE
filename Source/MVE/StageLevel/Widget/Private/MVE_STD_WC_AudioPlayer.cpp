
#include "../Public/MVE_STD_WC_AudioPlayer.h"
#include "MVE_STD_WC_AudioSearchResult.h"
#include "MVE.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Sound/SoundWave.h"
#include "Internationalization/Text.h"

void UMVE_STD_WC_AudioPlayer::SetAudioData(const FMVE_STD_AudioSearchResultData& AudioData) const
{
	if (TitleTextBlock)
	{
		TitleTextBlock->SetText(FText::FromString(AudioData.Title));
	}
	
	if (ArtistTextBlock)
	{
		ArtistTextBlock->SetText(FText::FromString(AudioData.Artist));
	}
	
	if (TotalTimeTextBlock)
	{
		TotalTimeTextBlock->SetText(FText::FromString(FormatTime(AudioData.Duration)));
	}
	
	ResetPlaybackUI();
}
void UMVE_STD_WC_AudioPlayer::UpdatePlaybackProgress(float CurrentTime, float TotalDuration)
{
	if (CurrentTimeTextBlock)
	{
		CurrentTimeTextBlock->SetText(FText::FromString(FormatTime(static_cast<int32>(CurrentTime))));
	}
	if (PlaybackSlider && TotalDuration > 0)
	{
		PlaybackSlider->SetValue(CurrentTime / TotalDuration);
	}
}
void UMVE_STD_WC_AudioPlayer::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (PlayButton)
	{
		PlayButton->OnClicked.AddDynamic(this, &UMVE_STD_WC_AudioPlayer::OnPlayButtonClicked);
	}
	if (NextButton)
	{
		NextButton->OnClicked.AddDynamic(this, &UMVE_STD_WC_AudioPlayer::OnNextButtonClicked);
	}
	if (PrevButton)
	{
		PrevButton->OnClicked.AddDynamic(this, &UMVE_STD_WC_AudioPlayer::OnPrevButtonClicked);
	}
	if (PlaybackSlider)
	{
		PlaybackSlider->OnMouseCaptureBegin.AddDynamic(this, &UMVE_STD_WC_AudioPlayer::OnPlaybackSliderMouseCaptureBegin);
		PlaybackSlider->OnMouseCaptureEnd.AddDynamic(this, &UMVE_STD_WC_AudioPlayer::OnPlaybackSliderMouseCaptureEnd);
		PlaybackSlider->OnValueChanged.AddDynamic(this, &UMVE_STD_WC_AudioPlayer::OnPlaybackSliderValueChanged);
	}
    
	UpdatePlayPauseButton(false);
}

void UMVE_STD_WC_AudioPlayer::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UMVE_STD_WC_AudioPlayer::NativeDestruct()
{
	Super::NativeDestruct();
}

void UMVE_STD_WC_AudioPlayer::OnPlayButtonClicked()
{
	// If it's stopped/paused, we want to play.
	PRINTNETLOG(this, TEXT("AudioPlayer: PlayButton clicked. Broadcasting OnPlayClicked."));
	OnPlayClicked.Broadcast();
}

void UMVE_STD_WC_AudioPlayer::OnNextButtonClicked()
{
	PRINTLOG(TEXT("Next button clicked - Playlist functionality not yet implemented."));
}
void UMVE_STD_WC_AudioPlayer::OnPrevButtonClicked()
{
	PRINTLOG(TEXT("Previous button clicked - Playlist functionality not yet implemented."));
}
void UMVE_STD_WC_AudioPlayer::OnPlaybackSliderMouseCaptureBegin()
{
	// This logic needs to be connected to the new network system
	// e.g., send an RPC to notify the server of a seek operation.
	PRINTLOG(TEXT("Slider mouse capture begin - Seeking not yet implemented with new system."));
}
void UMVE_STD_WC_AudioPlayer::OnPlaybackSliderMouseCaptureEnd()
{
	PRINTLOG(TEXT("Slider mouse capture end - Seeking not yet implemented with new system."));
}
void UMVE_STD_WC_AudioPlayer::OnPlaybackSliderValueChanged(const float Value)
{
	// This might be used for visual feedback while dragging, but the final seek
	// should be handled in OnPlaybackSliderMouseCaptureEnd.
}
FString UMVE_STD_WC_AudioPlayer::FormatTime(const int32 TotalSeconds)
{
	const int32 Minutes = TotalSeconds / 60;
	const int32 Seconds = TotalSeconds % 60;
	return FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
}
void UMVE_STD_WC_AudioPlayer::UpdatePlayPauseButton(const bool bIsPlaying) const
{
	if(PlayStateImage && PlayImage && PauseImage)
	{
		PlayStateImage->SetBrushFromTexture(bIsPlaying ? PauseImage : PlayImage);
	}
}
void UMVE_STD_WC_AudioPlayer::ResetPlaybackUI() const
{
	if (CurrentTimeTextBlock)
	{
		CurrentTimeTextBlock->SetText(FText::FromString(TEXT("00:00")));
	}
	if (PlaybackSlider)
	{
		PlaybackSlider->SetValue(0.f);
	}
}