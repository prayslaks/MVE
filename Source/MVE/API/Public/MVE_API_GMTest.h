// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Template/MVEGameMode.h"
#include "MVE_API_GMTest.generated.h"

/**
 * 
 */
UCLASS()
class MVE_API AMVE_API_GMTest : public AMVEGameMode
{
	GENERATED_BODY()

public:
	/**
	 * @brief Sends a presigned URL to all clients to prepare audio playback.
	 * @param PresignedUrl The URL of the audio to be downloaded by clients.
	 */
	void SendPresignedUrlToAllClients(const FString& PresignedUrl);

	/**
	 * @brief Sends a command to all clients to play the previously prepared audio.
	 */
	void SendPlayCommandToAllClients();

	/**
	 * @brief Sends a command to all clients to stop the currently playing audio.
	 */
	void SendStopCommandToAllClients();
};
