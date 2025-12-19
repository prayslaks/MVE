#pragma once

#include "CoreMinimal.h"
#include "MVE_AUD_PreviewCameraPawn.h"
#include "GameFramework/PlayerController.h"
#include "MVE_PC_PreviewMesh.generated.h"

class AMVE_AUD_PreviewCameraPawn;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * PreviewMesh 전용 PlayerController
 * CameraPawn의 카메라를 제어하고 입력을 처리
 */
UCLASS()
class MVE_API AMVE_PC_PreviewMesh : public APlayerController
{
	GENERATED_BODY()

public:
	AMVE_PC_PreviewMesh();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Test")
	bool bTestMode = false;

	// CameraPawn 가져오기
	UFUNCTION(BlueprintCallable, Category = "Camera")
	AMVE_AUD_PreviewCameraPawn* GetCameraPawn() const;

	// 프리뷰 캐릭터 가져오기
	UFUNCTION(BlueprintCallable, Category = "Preview")
	AActor* GetPreviewCharacter() const;

	// 입력 모드 설정
	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetUIOnlyMode();

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetGameAndUIMode();

protected:
	// EnhancedInput 관련
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* PreviewMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* CameraLookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* CameraZoomAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MouseClickAction;

	// 입력 처리 함수들
	void OnCameraZoom(const FInputActionValue& Value);
	void OnMouseClickStarted(const FInputActionValue& Value);
	void OnMouseClickCompleted(const FInputActionValue& Value);

private:
	// 마우스 드래그 상태
	bool bIsDragging = false;
};
