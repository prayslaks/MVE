
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MVE_AUD_AudienceCharacter.generated.h"

class UCameraComponent;
class UMVE_AUD_InteractionComponent;
class USpringArmComponent;

UCLASS()
class MVE_API AMVE_AUD_AudienceCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMVE_AUD_AudienceCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	FORCEINLINE UMVE_AUD_InteractionComponent* GetInteractionComponent() const { return InteractionComponent; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USpringArmComponent> SpringArm;
	
	UPROPERTY()
	TObjectPtr<UCameraComponent> Camera;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UMVE_AUD_InteractionComponent* InteractionComponent;

	// 저장된 커스터마이징 적용
	void ApplyCustomization();

	// GLB 로딩 완료 콜백
	void OnCustomizationMeshLoaded(class AActor* LoadedActor, FName SocketName, FTransform RelativeTransform);

};
