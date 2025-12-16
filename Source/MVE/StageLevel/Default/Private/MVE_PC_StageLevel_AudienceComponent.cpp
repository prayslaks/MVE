#include "MVE_PC_StageLevel_AudienceComponent.h"
#include "MVE.h"
#include "MVE_PC_StageLevel.h"
#include "MVE_StageLevel_AudCamera.h"
#include "MVE_StageLevel_AudCharacter.h"
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

AMVE_PC_StageLevel* UMVE_PC_StageLevel_AudienceComponent::GetBindingPC() const
{
	return Cast<AMVE_PC_StageLevel>(GetOwner());
}

void UMVE_PC_StageLevel_AudienceComponent::Server_ThrowSomething_Implementation(const FVector& Location, const FVector& Direction)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		PRINTNETLOG(this, TEXT("물건 던지기 요청을 모든 클라이언트에게 전파."));
	}	
}

void UMVE_PC_StageLevel_AudienceComponent::Server_WaveLightStick_Implementation(const FVector& Location)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		PRINTNETLOG(this, TEXT("응원봉 흔들기 요청을 모든 클라이언트에게 전파."));
	}
}

void UMVE_PC_StageLevel_AudienceComponent::Server_TakePhoto_Implementation()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		PRINTNETLOG(this, TEXT("사진 찍기 요청을 모든 클라이언트에게 전파."));
		GetBindingPC()->GetBindingAudCharacter()->GetAudCamera()->Multicast_TakePhoto();
	}
}

void UMVE_PC_StageLevel_AudienceComponent::Server_CheerUp_Implementation(const FVector& Location)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		PRINTNETLOG(this, TEXT("환호하기 요청을 모든 클라이언트에게 전파."));
	}
}