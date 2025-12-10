// Fill out your copyright notice in the Description page of Project Settings.

#include "MVE_AUD_WC_InteractionPanel.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "API/Public/MVE_API_PCTest.h"
#include "API/Public/MVE_API_PCTest_AudComponent.h"
#include "MVE.h"
#include "MVE_AUD_InteractionComponent.h"

void UMVE_AUD_WC_InteractionPanel::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (SwingLightStickButton)
	{
		SwingLightStickButton->OnClicked.AddDynamic(this, &UMVE_AUD_WC_InteractionPanel::OnSwingLightStickButtonClicked);
	}
	if (TakePhotoButton)
	{
		TakePhotoButton->OnClicked.AddDynamic(this, &UMVE_AUD_WC_InteractionPanel::OnTakePhotoButtonClicked);
	}
	if (CheerUpButton)
	{
		CheerUpButton->OnClicked.AddDynamic(this, &UMVE_AUD_WC_InteractionPanel::OnCheerUpButtonClicked);
	}
	if (SwingHandButton)
	{
		SwingHandButton->OnClicked.AddDynamic(this, &UMVE_AUD_WC_InteractionPanel::OnSwingHandButtonClicked);
	}
}

void UMVE_AUD_WC_InteractionPanel::OnSwingLightStickButtonClicked()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		PRINTLOG(TEXT("PlayerController not found."));
		return;
	}

	AMVE_API_PCTest* PCTest = Cast<AMVE_API_PCTest>(PC);
	if (!PCTest)
	{
		PRINTLOG(TEXT("PlayerController is not of type AMVE_API_PCTest."));
		return;
	}

	UMVE_API_PCTest_AudComponent* AudComponent = PCTest->FindComponentByClass<UMVE_API_PCTest_AudComponent>();
	if (!AudComponent)
	{
		PRINTLOG(TEXT("AudComponent not found on PlayerController."));
		return;
	}

	FVector PlayerLocation = FVector::ZeroVector;
	if (APawn* PlayerPawn = PC->GetPawn())
	{
		PlayerLocation = PlayerPawn->GetActorLocation();
	}

	PRINTLOG(TEXT("Calling SwingLightStickOnServer RPC."));
	AudComponent->SwingLightStickOnServer(PlayerLocation);
}

void UMVE_AUD_WC_InteractionPanel::OnTakePhotoButtonClicked()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		PRINTLOG(TEXT("PlayerController not found."));
		return;
	}

	AMVE_API_PCTest* PCTest = Cast<AMVE_API_PCTest>(PC);
	if (!PCTest)
	{
		PRINTLOG(TEXT("PlayerController is not of type AMVE_API_PCTest."));
		return;
	}

	UMVE_API_PCTest_AudComponent* AudComponent = PCTest->FindComponentByClass<UMVE_API_PCTest_AudComponent>();
	if (!AudComponent)
	{
		PRINTLOG(TEXT("AudComponent not found on PlayerController."));
		return;
	}

	FVector PlayerLocation = FVector::ZeroVector;
	if (APawn* PlayerPawn = PC->GetPawn())
	{
		PlayerLocation = PlayerPawn->GetActorLocation();
	}
	
	PRINTLOG(TEXT("Calling TakePhotoOnServer RPC."));
	AudComponent->TakePhotoOnServer(PlayerLocation);
}

void UMVE_AUD_WC_InteractionPanel::OnCheerUpButtonClicked()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		PRINTLOG(TEXT("PlayerController not found."));
		return;
	}

	AMVE_API_PCTest* PCTest = Cast<AMVE_API_PCTest>(PC);
	if (!PCTest)
	{
		PRINTLOG(TEXT("PlayerController is not of type AMVE_API_PCTest."));
		return;
	}

	UMVE_API_PCTest_AudComponent* AudComponent = PCTest->FindComponentByClass<UMVE_API_PCTest_AudComponent>();
	if (!AudComponent)
	{
		PRINTLOG(TEXT("AudComponent not found on PlayerController."));
		return;
	}
	
	FVector PlayerLocation = FVector::ZeroVector;
	if (APawn* PlayerPawn = PC->GetPawn())
	{
		PlayerLocation = PlayerPawn->GetActorLocation();
	}

	PRINTLOG(TEXT("Calling CheerUpOnServer RPC."));
	AudComponent->CheerUpOnServer(PlayerLocation);
}

void UMVE_AUD_WC_InteractionPanel::OnSwingHandButtonClicked()
{
	if (const auto Pawn = GetWorld()->GetFirstPlayerController()->GetPawn())
	{
		if (const auto Comp = Pawn->GetComponentByClass<UMVE_AUD_InteractionComponent>())
		{
			Comp->RequestWaveLightStick();
		}
	}
}
