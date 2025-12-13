#include "MVE_STD_WC_AudioPanel.h"
#include "API/Public/MVE_API_GMTest.h"
#include "MVE.h"
#include "MVE_API_Helper.h"
#include "MVE_STD_WC_AudioPlayer.h"
#include "MVE_STD_WC_AudioSearch.h"
#include "MVE_STD_WC_AudioSearchResult.h"
#include "Kismet/GameplayStatics.h"

void UMVE_STD_WC_AudioPanel::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (AudioSearch)
	{
		AudioSearch->OnAudioSearchResultSelected.AddDynamic(this, &UMVE_STD_WC_AudioPanel::HandleAudioSearchResultSelected);
	}
}

void UMVE_STD_WC_AudioPanel::HandleAudioSearchResultSelected(const FMVE_STD_AudioSearchResultData& AudioData)
{
	if (AudioPlayer)
	{
		AudioPlayer->SetAudioData(AudioData);
		
		AudioPlayer->OnPlayClicked.Clear();
		AudioPlayer->OnPlayClicked.AddDynamic(this, &UMVE_STD_WC_AudioPanel::HandlePlayClicked);
		
		AudioPlayer->OnStopClicked.Clear();
		AudioPlayer->OnStopClicked.AddDynamic(this, &UMVE_STD_WC_AudioPanel::HandleStopClicked);
		
		AudioPlayer->ResetPlaybackUI();
		AudioPlayer->UpdatePlayPauseButton(false); // Set to paused state
		
		LoadAudioFromPresignedUrlAndSetPlayer(AudioData);
	}
}

void UMVE_STD_WC_AudioPanel::HandlePlayClicked()
{
	PRINTNETLOG(this, TEXT("Play button clicked. Relaying to GameMode to send play command to all clients."));
	
	AMVE_API_GMTest* GameMode = GetWorld()->GetAuthGameMode<AMVE_API_GMTest>();
	if (GameMode)
	{
		GameMode->SendPlayCommandToAllClients();
	}
	else
	{
		PRINTNETLOG(this, TEXT("Failed to get AMVE_API_GMTest GameMode."));
	}
}

void UMVE_STD_WC_AudioPanel::HandleStopClicked()
{
	PRINTNETLOG(this, TEXT("Stop button clicked. Relaying to GameMode to send stop command to all clients."));
	
	AMVE_API_GMTest* GameMode = GetWorld()->GetAuthGameMode<AMVE_API_GMTest>();
	if (GameMode)
	{
		GameMode->SendStopCommandToAllClients();
	}
	else
	{
		PRINTNETLOG(this, TEXT("Failed to get AMVE_API_GMTest GameMode."));
	}
}

void UMVE_STD_WC_AudioPanel::LoadAudioFromPresignedUrlAndSetPlayer(const FMVE_STD_AudioSearchResultData& AudioData)
{
	PRINTNETLOG(this, TEXT("Requesting Presigned URL for Title: %s"), *AudioData.Title);
	
	FOnStreamAudioComplete OnResult;
	OnResult.BindLambda([this](const bool bSuccess, const FStreamAudioResponseData& ResponseData, const FString& ErrorCode)
	{
		if (bSuccess)
		{
			const FString PresignedUrl = ResponseData.StreamUrl;
			PRINTNETLOG(this, TEXT("Got PresignedUrl = {%s}, relaying to GameMode to send to all clients."), *PresignedUrl);

			AMVE_API_GMTest* GameMode = GetWorld()->GetAuthGameMode<AMVE_API_GMTest>();
			if (GameMode)
			{
				GameMode->SendPresignedUrlToAllClients(PresignedUrl);
			}
			else
			{
				PRINTNETLOG(this, TEXT("Failed to get AMVE_API_GMTest GameMode."));
			}
		}
		else
		{
			PRINTNETLOG(this, TEXT("Failed to get Presigned URL. Error: %s"), *ErrorCode);
		}
	});
	UMVE_API_Helper::StreamAudio(AudioData.Id, OnResult);
}
