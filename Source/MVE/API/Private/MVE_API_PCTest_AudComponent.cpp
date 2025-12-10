// Fill out your copyright notice in the Description page of Project Settings.

#include "API/Public/MVE_API_PCTest_AudComponent.h"

#include "MVE_API_PCTest.h"
#include "Net/UnrealNetwork.h" // For GetLifetimeReplicatedProps
#include "Kismet/GameplayStatics.h" // For PlaySoundAtLocation
#include "Engine/World.h" // For GetWorld()

UMVE_API_PCTest_AudComponent::UMVE_API_PCTest_AudComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true); // Enable replication for this component
}

void UMVE_API_PCTest_AudComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMVE_API_PCTest_AudComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UMVE_API_PCTest_AudComponent::SwingLightStickOnServer_Implementation(FVector Location)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("Server received SwingLightStickOnServer RPC. Propagating via Multicast."));
		MulticastSwingLightStick(Location);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SwingLightStickOnServer_Implementation called on a non-authority client. This should not happen."));
	}
}

void UMVE_API_PCTest_AudComponent::MulticastSwingLightStick_Implementation(FVector Location)
{
	// Play sound
	if (SwingStickSound && GetWorld())
	{
		AMVE_API_PCTest* PCTestOwner = Cast<AMVE_API_PCTest>(GetOwner());
		if (PCTestOwner && PCTestOwner->AudioComponent)
		{
			PCTestOwner->AudioComponent->SetSound(SwingStickSound);
			PCTestOwner->AudioComponent->SetWorldLocation(Location);
			PCTestOwner->AudioComponent->Play();
			UE_LOG(LogTemp, Log, TEXT("MulticastSwingLightStick playing sound at %s."), *Location.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MulticastSwingLightStick failed: SwingStickSound is not set or World is invalid."));
	}

	// Also log for the animation/effect part
	if (GetOwner())
	{
		UE_LOG(LogTemp, Log, TEXT("MulticastSwingLightStick_Implementation on Role: %s"), *UEnum::GetValueAsString(GetOwner()->GetLocalRole()));
		// Placeholder for animation logic remains here
	}
}

void UMVE_API_PCTest_AudComponent::TakePhotoOnServer_Implementation(FVector Location)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		MulticastTakePhoto(Location);
	}
}

void UMVE_API_PCTest_AudComponent::MulticastTakePhoto_Implementation(FVector Location)
{
	if (TakePhotoSound && GetWorld())
	{
		AMVE_API_PCTest* PCTestOwner = Cast<AMVE_API_PCTest>(GetOwner());
		if (PCTestOwner && PCTestOwner->AudioComponent)
		{
			PCTestOwner->AudioComponent->SetSound(TakePhotoSound);
			PCTestOwner->AudioComponent->SetWorldLocation(Location);
			PCTestOwner->AudioComponent->Play();
			UE_LOG(LogTemp, Log, TEXT("MulticastTakePhoto playing sound at %s."), *Location.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MulticastTakePhoto failed: TakePhotoSound is not set or World is invalid."));
	}
}

void UMVE_API_PCTest_AudComponent::CheerUpOnServer_Implementation(FVector Location)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		MulticastCheerUp(Location);
	}
}

void UMVE_API_PCTest_AudComponent::MulticastCheerUp_Implementation(FVector Location)
{
	if (CheerUpSound && GetWorld())
	{
		AMVE_API_PCTest* PCTestOwner = Cast<AMVE_API_PCTest>(GetOwner());
		if (PCTestOwner && PCTestOwner->AudioComponent)
		{
			PCTestOwner->AudioComponent->SetSound(CheerUpSound);
			PCTestOwner->AudioComponent->SetWorldLocation(Location);
			PCTestOwner->AudioComponent->Play();
			UE_LOG(LogTemp, Log, TEXT("MulticastCheerUp playing sound at %s."), *Location.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MulticastCheerUp failed: CheerUpSound is not set or World is invalid."));
	}
}
