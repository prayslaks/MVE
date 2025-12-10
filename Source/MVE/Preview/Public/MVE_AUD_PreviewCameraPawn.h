#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MVE_AUD_PreviewCameraPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;


UCLASS()
class MVE_API AMVE_AUD_PreviewCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	AMVE_AUD_PreviewCameraPawn();

	virtual void BeginPlay() override;

	// 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* Camera;

	// 카메라 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float DefaultArmLength = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MinArmLength = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MaxArmLength = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float RotationSpeed = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ZoomSpeed = 50.0f;

	// 카메라 제어 함수들
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void RotateCamera(float DeltaYaw, float DeltaPitch);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ZoomCamera(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ResetCamera();

	// 캐릭터 회전 (터렛테이블 방식 - 캐릭터가 제자리 회전)
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void RotateCharacter(float DeltaYaw, float DeltaPitch);

	// 타겟 액터 설정 (캐릭터를 바라볼 대상)
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetTargetActor(AActor* InTargetActor);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	AActor* GetTargetActor() const { return TargetActor; }

	// 모드 전환 
	UFUNCTION(BlueprintCallable, Category = "Gizmo")
	void SwitchToGizmoMode(AActor* InTargetActor);

	UFUNCTION(BlueprintCallable, Category = "Gizmo")
	void SwitchToViewMode();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Gizmo")
	bool IsGizmoMode() const { return bIsGizmoMode; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Gizmo")
	AActor* GetGizmoTargetActor() const { return GizmoTargetActor; }

	// Blueprint에서 구현할 이벤트들
	UFUNCTION(BlueprintImplementableEvent, Category = "Gizmo")
	void OnSwitchToGizmoMode(AActor* InTargetActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Gizmo")
	void OnSwitchToViewMode();

	// Transform 저장
	UFUNCTION(BlueprintCallable, Category = "Gizmo")
	void SaveAccessoryTransform(AActor* Accessory, const FTransform& NewTransform);

protected:
	UPROPERTY()
	AActor* TargetActor;

	// Gizmo 모드 관리
	UPROPERTY(BlueprintReadOnly, Category = "Gizmo")
	bool bIsGizmoMode = false;

	UPROPERTY(BlueprintReadOnly, Category = "Gizmo")
	AActor* GizmoTargetActor = nullptr;

	float CurrentYaw = 180.0f;
	float CurrentPitch = 0.0f;

	// 캐릭터 회전값 (터렛테이블용)
	float CurrentCharacterYaw = 0.0f;

	void UpdateCameraRotation();

	// PreviewCharacter 찾기 (Timer 재시도용)
	void TryFindPreviewCharacter();
};
