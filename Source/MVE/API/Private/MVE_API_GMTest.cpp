// Fill out your copyright notice in the Description page of Project Settings.


#include "MVE_API_GMTest.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "API/Public/MVE_API_PCTest_StdComponent.h"
#include "MVE.h" // For PRINTNETLOG

void AMVE_API_GMTest::SendPresignedUrlToAllClients(const FString& PresignedUrl)
{
	PRINTNETLOG(this, TEXT("GameMode: Sending Presigned URL to all clients: %s"), *PresignedUrl);

	if (!GetWorld() || !GameState)
	{
		PRINTNETLOG(this, TEXT("GameMode: World or GameState is null. Cannot send URL."));
		return;
	}

	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (PlayerState)
		{
			APlayerController* PC = PlayerState->GetPlayerController();
			if (PC)
			{
				UMVE_API_PCTest_StdComponent* StdComponent = PC->FindComponentByClass<UMVE_API_PCTest_StdComponent>();
				if (StdComponent)
				{
					PRINTNETLOG(this, TEXT("GameMode: Sending URL to Player %s"), *PlayerState->GetPlayerName());
					StdComponent->Client_PrepareAudio(PresignedUrl);
				}
				else
				{
					PRINTNETLOG(this, TEXT("GameMode: StdComponent not found for Player %s"), *PlayerState->GetPlayerName());
				}
			}
		}
	}
}

void AMVE_API_GMTest::SendPlayCommandToAllClients()
{
	PRINTNETLOG(this, TEXT("GameMode: Sending Play command to all clients."));

	if (!GetWorld() || !GameState)
	{
		PRINTNETLOG(this, TEXT("GameMode: World or GameState is null. Cannot send play command."));
		return;
	}
	
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (PlayerState)
		{
			APlayerController* PC = PlayerState->GetPlayerController();
			if (PC)
			{
				UMVE_API_PCTest_StdComponent* StdComponent = PC->FindComponentByClass<UMVE_API_PCTest_StdComponent>();
				if (StdComponent)
				{
					PRINTNETLOG(this, TEXT("GameMode: Sending Play command to Player %s"), *PlayerState->GetPlayerName());
					StdComponent->Client_PlayPreparedAudio();
				}
				else
				{
					PRINTNETLOG(this, TEXT("GameMode: StdComponent not found for Player %s"), *PlayerState->GetPlayerName());
				}
			}
		}
	}
}

void AMVE_API_GMTest::SendStopCommandToAllClients()
{
	PRINTNETLOG(this, TEXT("GameMode: Sending Stop command to all clients."));

	if (!GetWorld() || !GameState)
	{
		PRINTNETLOG(this, TEXT("GameMode: World or GameState is null. Cannot send stop command."));
		return;
	}
	
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (PlayerState)
		{
			APlayerController* PC = PlayerState->GetPlayerController();
			if (PC)
			{
				UMVE_API_PCTest_StdComponent* StdComponent = PC->FindComponentByClass<UMVE_API_PCTest_StdComponent>();
				if (StdComponent)
				{
					PRINTNETLOG(this, TEXT("GameMode: Sending Stop command to Player %s"), *PlayerState->GetPlayerName());
					StdComponent->Client_StopPreparedAudio();
				}
				else
				{
					PRINTNETLOG(this, TEXT("GameMode: StdComponent not found for Player %s"), *PlayerState->GetPlayerName());
				}
			}
		}
	}
}
