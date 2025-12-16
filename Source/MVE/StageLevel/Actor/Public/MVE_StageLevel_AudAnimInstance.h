#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MVE_StageLevel_AudAnimInstance.generated.h"

enum class EAudienceControlMode : uint8;
class AMVE_StageLevel_AudCharacter;

UCLASS()
class MVE_API UMVE_StageLevel_AudAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<AMVE_StageLevel_AudCharacter> OwningCharacter;
	
	// 애님 블루프린트의 블렌드 스페이스에서 사용하는 값
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aim Offset")
	float AimYaw;

	// 애님 블루프린트의 블렌드 스페이스에서 사용하는 값
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aim Offset")
	float AimPitch;
	
	// 애님 블루프린트의 블렌드 스페이스 활성화
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aim Offset")
	bool bAimEnabled;
	
	// 캐릭터 모드에 따라 다른 애니메이션 포즈 선택
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Character");
	EAudienceControlMode AudienceControlMode;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	bool bIsExecuting;
	
	UFUNCTION()
	void AnimNotify_EndExecute() const;
};
