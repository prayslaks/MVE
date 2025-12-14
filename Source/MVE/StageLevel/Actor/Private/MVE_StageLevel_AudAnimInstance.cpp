#include "StageLevel/Actor/Public/MVE_StageLevel_AudAnimInstance.h"

#include "StageLevel/Actor/Public/MVE_StageLevel_AudCharacter.h"
#include "Kismet/KismetMathLibrary.h"

void UMVE_StageLevel_AudAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	// 오너 폰을 가져와서 AudCharacter 타입으로 캐스팅하여 저장합니다.
	OwningCharacter = Cast<AMVE_StageLevel_AudCharacter>(TryGetPawnOwner());
}

void UMVE_StageLevel_AudAnimInstance::NativeUpdateAnimation(const float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	// 유효하지 않은 경우 오너 폰 재획득
	if (OwningCharacter == nullptr)
	{
		OwningCharacter = Cast<AMVE_StageLevel_AudCharacter>(TryGetPawnOwner());
		if (OwningCharacter == nullptr)
		{
			return;
		}
	}
	
	// 에임 활성화 여부를 캐릭터에서 가져옵니다.
	bAimEnabled = OwningCharacter->bAimEnabled;
	
	// 에임이 활성화된 경우 에임 오프셋을 계산합니다.
	if (bAimEnabled)
	{
		// 캐릭터의 조준 회전값과 액터의 회전값 차이를 계산합니다.
		const FRotator AimRotation = OwningCharacter->GetBaseAimRotation();
		const FRotator MovementRotation = OwningCharacter->GetActorRotation();
		const FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, MovementRotation);
	
		// 부드러운 전환을 위해 값을 보간합니다.
		AimYaw = FMath::FInterpTo(AimYaw, DeltaRot.Yaw, DeltaSeconds, 15.f);
		AimPitch = FMath::FInterpTo(AimPitch, DeltaRot.Pitch, DeltaSeconds, 15.f);
	}
	else
	{
		// 에임이 비활성화된 경우 값들을 0으로 부드럽게 복원합니다.
		AimYaw = FMath::FInterpTo(AimYaw, 0.f, DeltaSeconds, 15.f);
		AimPitch = FMath::FInterpTo(AimPitch, 0.f, DeltaSeconds, 15.f);
	}
}