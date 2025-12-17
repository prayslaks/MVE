#include "MVE_PC_StageLevel_AudienceComponent.h"
#include "MVE.h"
#include "MVE_PC_StageLevel.h"
#include "MVE_StageLevel_AudCharacterShooterComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"

class AMVE_PC_StageLevel;

UMVE_PC_StageLevel_AudienceComponent::UMVE_PC_StageLevel_AudienceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UMVE_PC_StageLevel_AudienceComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMVE_PC_StageLevel_AudienceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UMVE_PC_StageLevel_AudienceComponent::SwingLightStickOnServer_Implementation(const FVector& Location)
{
	// 서버에서만 멀티캐스트 RPC를 호출하도록 보장합니다.
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		PRINTNETLOG(this, TEXT("서버: 야광봉 흔들기 요청 수신. 모든 클라이언트에게 전파합니다."));
		MulticastSwingLightStick(Location);
	}
}

void UMVE_PC_StageLevel_AudienceComponent::MulticastSwingLightStick_Implementation(const FVector& Location)
{
	// 사운드 에셋이 유효한지 확인합니다.
	if (SwingStickSound && GetWorld())
	{
		// 이 컴포넌트의 소유자인 플레이어 컨트롤러를 가져옵니다.
		// 플레이어 컨트롤러와 그 안의 오디오 컴포넌트가 유효한지 확인합니다.
		if (const AMVE_PC_StageLevel* PC = Cast<AMVE_PC_StageLevel>(GetOwner()))
		{
			if (const auto AudioComp = PC->GetAudioComponent())
			{
				AudioComp->SetSound(SwingStickSound);
				AudioComp->Play();	
			}
			PRINTNETLOG(this, TEXT("클라이언트: 야광봉 흔들기 사운드를 %s 위치에서 재생합니다."), *Location.ToString());
		}
	}
	else
	{
		PRINTNETLOG(this, TEXT("경고: 야광봉 흔들기 사운드(SwingStickSound)가 지정되지 않았거나 World가 유효하지 않습니다."));
	}
}

void UMVE_PC_StageLevel_AudienceComponent::TakePhotoOnServer_Implementation(const FVector& Location)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		PRINTNETLOG(this, TEXT("서버: 사진 찍기 요청 수신. 모든 클라이언트에게 전파합니다."));
		MulticastTakePhoto(Location);
	}
}

void UMVE_PC_StageLevel_AudienceComponent::MulticastTakePhoto_Implementation(const FVector& Location)
{
	if (TakePhotoSound && GetWorld())
	{
		if (const AMVE_PC_StageLevel* PC = Cast<AMVE_PC_StageLevel>(GetOwner()))
		{
			if (const auto AudioComp = PC->GetAudioComponent())
			{
				AudioComp->SetSound(TakePhotoSound);
				AudioComp->Play();	
			}
			PRINTNETLOG(this, TEXT("클라이언트: 사진 찍기 사운드를 %s 위치에서 재생합니다."), *Location.ToString());
		}
	}
	else
	{
		PRINTNETLOG(this, TEXT("경고: 사진 찍기 사운드(TakePhotoSound)가 지정되지 않았거나 World가 유효하지 않습니다."));
	}
}

void UMVE_PC_StageLevel_AudienceComponent::CheerUpOnServer_Implementation(const FVector& Location)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		PRINTNETLOG(this, TEXT("서버: 환호 요청 수신. 모든 클라이언트에게 전파합니다."));
		MulticastCheerUp(Location);
	}
}

void UMVE_PC_StageLevel_AudienceComponent::MulticastCheerUp_Implementation(const FVector& Location)
{
	if (CheerUpSound && GetWorld())
	{
		if (const AMVE_PC_StageLevel* PC = Cast<AMVE_PC_StageLevel>(GetOwner()))
		{
			if (const auto AudioComp = PC->GetAudioComponent())
			{
				AudioComp->SetSound(CheerUpSound);
				AudioComp->Play();	
			}
			PRINTNETLOG(this, TEXT("클라이언트: 환호 사운드를 %s 위치에서 재생합니다."), *Location.ToString());
		}
	}
	else
	{
		PRINTNETLOG(this, TEXT("경고: 환호 사운드(CheerUpSound)가 지정되지 않았거나 World가 유효하지 않습니다."));
	}
}

void UMVE_PC_StageLevel_AudienceComponent::RequestThrowProjectile(const FVector& Location, const FVector& Direction)
{
	if (const AMVE_PC_StageLevel* PC = Cast<AMVE_PC_StageLevel>(GetOwner()))
	{
		if (const auto ShooterComp = PC->GetShooterComponent())
		{
			ShooterComp->Fire(Location, Direction);
		}
	}
	else
	{
		PRINTNETLOG(this, TEXT("경고: 소유자 PC 또는 ShooterComponent를 찾을 수 없어 발사 요청에 실패했습니다."));
	}
}