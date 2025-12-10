
#include "MVE_AUD_WidgetClass_PreviewWidget.h"

#include "MVE_AUD_CustomizationManager.h"
#include "Components/Image.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Preview/Public/MVE_AUD_PreviewCaptureActor.h"

void UMVE_AUD_WidgetClass_PreviewWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UMVE_AUD_CustomizationManager* CustomizationManager = GetGameInstance()->GetSubsystem<UMVE_AUD_CustomizationManager>())
	{
		CustomizationManager->StartMeshPreview(FString(""), this);
	}
}

void UMVE_AUD_WidgetClass_PreviewWidget::SetRenderTarget(UTextureRenderTarget2D* InRenderTarget)
{
	if (PreviewImage && InRenderTarget)
	{
		// Render Target을 Brush로 변환
		FSlateBrush Brush;
		Brush.SetResourceObject(InRenderTarget);
		Brush.ImageSize = FVector2D(InRenderTarget->SizeX, InRenderTarget->SizeY);
		Brush.DrawAs = ESlateBrushDrawType::Image;
        
		PreviewImage->SetBrush(Brush);
        
		UE_LOG(LogTemp, Log, TEXT("✅ Render Target set to PreviewImage"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ PreviewImage or RenderTarget is null!"));
	}
}

void UMVE_AUD_WidgetClass_PreviewWidget::SetInitialDistance(float Distance)
{
	CurrentDistance = FMath::Clamp(Distance, MinDistance, MaxDistance);
	UpdateCameraPosition();
    
	UE_LOG(LogTemp, Log, TEXT("Initial camera distance set to: %f"), CurrentDistance);
}

void UMVE_AUD_WidgetClass_PreviewWidget::SetCaptureActor(AMVE_AUD_PreviewCaptureActor* InCaptureActor)
{
	CaptureActor = InCaptureActor;
    
	if (CaptureActor)
	{
		// 초기 카메라 위치 설정
		CurrentYaw = 180.0f;
		CurrentPitch = 0.0f;
		UpdateCameraPosition();
	}
}

void UMVE_AUD_WidgetClass_PreviewWidget::UpdateCameraPosition()
{
	if (!CaptureActor || !CaptureActor->TargetActor)
	{
		return;
	}
    
	// 타겟 위치 (캐릭터 중심)
	FVector TargetLocation = CaptureActor->TargetActor->GetActorLocation();
    
	// 구면 좌표계로 카메라 위치 계산
	FRotator Rotation = FRotator(CurrentPitch, CurrentYaw, 0.0f);
	FVector Direction = Rotation.Vector();
	FVector CameraLocation = TargetLocation - Direction * CurrentDistance;
    
	// Scene Capture 위치/회전 업데이트
	CaptureActor->SetActorLocation(CameraLocation);
	CaptureActor->SetActorRotation((TargetLocation - CameraLocation).Rotation());
}

FReply UMVE_AUD_WidgetClass_PreviewWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry,
                                                                   const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = true;
		LastMousePosition = InMouseEvent.GetScreenSpacePosition();

		
        
		return FReply::Handled().CaptureMouse(TakeWidget());
	}
	
	return FReply::Unhandled();
}

FReply UMVE_AUD_WidgetClass_PreviewWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = false;
        
		return FReply::Handled().ReleaseMouseCapture();
	}
    
	return FReply::Unhandled();
}

FReply UMVE_AUD_WidgetClass_PreviewWidget::NativeOnMouseMove(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (bIsDragging && CaptureActor)
	{
		FVector2D CurrentMousePosition = InMouseEvent.GetScreenSpacePosition();
		FVector2D MouseDelta = CurrentMousePosition - LastMousePosition;
		LastMousePosition = CurrentMousePosition;
        
		// Yaw (좌우 회전)
		CurrentYaw += MouseDelta.X * RotationSpeed;
        
		// Pitch (상하 회전) - 범위 제한
		CurrentPitch = FMath::Clamp(CurrentPitch - MouseDelta.Y * RotationSpeed, -80.0f, 80.0f);
        
		UpdateCameraPosition();
        
		return FReply::Handled();
	}
    
	return FReply::Unhandled();
}

FReply UMVE_AUD_WidgetClass_PreviewWidget::NativeOnMouseWheel(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (CaptureActor)
	{
		float WheelDelta = InMouseEvent.GetWheelDelta();
        
		// 줌 인/아웃
		CurrentDistance = FMath::Clamp(CurrentDistance - WheelDelta * ZoomSpeed, MinDistance, MaxDistance);
        
		UpdateCameraPosition();
        
		return FReply::Handled();
	}
    
	return FReply::Unhandled();
}
