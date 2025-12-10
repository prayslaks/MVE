
#include "../Public/MVE_AUD_InteractionComponent.h"

#include "MVE.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


UMVE_AUD_InteractionComponent::UMVE_AUD_InteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicated(true);
}

void UMVE_AUD_InteractionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMVE_AUD_InteractionComponent, bIsPerformingAction);
}

void UMVE_AUD_InteractionComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeReferences();	
}

void UMVE_AUD_InteractionComponent::RequestWaveLightStick()
{
	PRINTLOG(TEXT("Request Wave Light Stick Called"));
	
	if (!CanPerformAction())
	{
		PRINTLOG(TEXT("Cannot Perform Action"));
		return;
	}

	ServerWaveLightStick();
}

void UMVE_AUD_InteractionComponent::RequestEmote(int32 EmoteID)
{
	if (!CanPerformAction())
	{
		return;
	}

	ServerPlayEmote(EmoteID);
}

void UMVE_AUD_InteractionComponent::RequestReaction(int32 ReactionID)
{
}

void UMVE_AUD_InteractionComponent::ServerWaveLightStick_Implementation()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
    
	if (CurrentTime - LastWaveTime < WaveCooldown)
	{
		return;
	}

	LastWaveTime = CurrentTime;
	MulticastWaveLightStick();
}

bool UMVE_AUD_InteractionComponent::ServerWaveLightStick_Validate()
{
	return true;
}

void UMVE_AUD_InteractionComponent::MulticastWaveLightStick_Implementation()
{
	PlayWaveLightStickAnimation();
}

void UMVE_AUD_InteractionComponent::PlayWaveLightStickAnimation()
{
	if (!WaveLightStickMontage || !AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("WaveLightStickMontage or AnimInstance is null!"));
		return;
	}

	PRINTLOG(TEXT("PlayWaveLightStickAnimation Called"));
	
	bIsPerformingAction = true;
    
	AnimInstance->Montage_Play(WaveLightStickMontage);

	/*
	// 파티클 효과
	if (WaveLightStickParticle && OwnerMesh)
	{
		UGameplayStatics::SpawnEmitterAttached(
			WaveLightStickParticle,
			OwnerMesh,
			TEXT("hand_r_socket"),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget
		);
	}

	// 사운드 재생
	if (WaveLightStickSound && OwnerCharacter)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			WaveLightStickSound,
			OwnerCharacter->GetActorLocation()
		);
	}
	*/

	// 몽타주 종료 콜백
	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindLambda([this](UAnimMontage* Montage, bool bInterrupted)
	{
		bIsPerformingAction = false;
	});
    
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, WaveLightStickMontage);
}

void UMVE_AUD_InteractionComponent::ServerPlayEmote_Implementation(int32 EmoteID)
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
    
	if (CurrentTime - LastEmoteTime < EmoteCooldown)
	{
		return;
	}

	LastEmoteTime = CurrentTime;
	MulticastPlayEmote(EmoteID);
}

bool UMVE_AUD_InteractionComponent::ServerPlayEmote_Validate(int32 EmoteID)
{
	return EmoteMontages.Contains(EmoteID);
}

void UMVE_AUD_InteractionComponent::MulticastPlayEmote_Implementation(int32 EmoteID)
{
	PlayEmoteAnimation(EmoteID);
}

void UMVE_AUD_InteractionComponent::PlayEmoteAnimation(int32 EmoteID)
{
	UAnimMontage** FoundMontage = EmoteMontages.Find(EmoteID);
	if (!FoundMontage || !*FoundMontage || !AnimInstance)
	{
		return;
	}

	bIsPerformingAction = true;
	AnimInstance->Montage_Play(*FoundMontage);
    
	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindLambda([this](UAnimMontage* Montage, bool bInterrupted)
	{
		bIsPerformingAction = false;
	});
    
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, *FoundMontage);
}

void UMVE_AUD_InteractionComponent::ServerPlayReaction_Implementation(int32 ReactionID)
{
}

bool UMVE_AUD_InteractionComponent::ServerPlayReaction_Validate(int32 ReactionID)
{
	return true;
}

void UMVE_AUD_InteractionComponent::MulticastPlayReaction_Implementation(int32 ReactionID)
{
}

bool UMVE_AUD_InteractionComponent::CanPerformAction() const
{
	return !bIsPerformingAction && AnimInstance != nullptr;
}

void UMVE_AUD_InteractionComponent::InitializeReferences()
{
	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		OwnerMesh = OwnerCharacter->GetMesh();
		if (OwnerMesh)
		{
			AnimInstance = OwnerMesh->GetAnimInstance();
		}
	}

	if (!OwnerCharacter || !OwnerMesh || !AnimInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("AudienceInteractionComponent: Failed to initialize references!"));
	}
}
