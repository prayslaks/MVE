// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_AUD_WidgetClass_PreviewWidget.generated.h"

class AMVE_AUD_PreviewCaptureActor;
class UImage;

UCLASS()
class MVE_API UMVE_AUD_WidgetClass_PreviewWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta = (BindWidget))
	UImage* PreviewImage;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview")
	float RotationSpeed = 0.5f;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview")
	float ZoomSpeed = 50.0f;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview")
	float MinDistance = 50.0f;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview")
	float MaxDistance = 10000.0f;

	// Render Target 설정 (추가)
	UFUNCTION(BlueprintCallable)
	void SetRenderTarget(UTextureRenderTarget2D* InRenderTarget);
    
	// 초기 카메라 거리 설정 (추가)
	UFUNCTION(BlueprintCallable)
	void SetInitialDistance(float Distance);
    
	void SetCaptureActor(AMVE_AUD_PreviewCaptureActor* InCaptureActor);
	void UpdateCameraPosition();

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    
private:
	UPROPERTY()
	AMVE_AUD_PreviewCaptureActor* CaptureActor;
    
	bool bIsDragging = false;
	FVector2D LastMousePosition;
	float CurrentDistance = 500.0f;
	float CurrentYaw = 0.0f;
	float CurrentPitch = 0.0f;
	
};
