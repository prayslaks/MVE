
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
		// 초기 카메라 위치 설정 (터렛테이블 방식: Yaw 고정, Pitch만 가변)
		CurrentYaw = 180.0f;  // 고정값 (정면)
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

	// 타겟 위치 (메시 중심)
	FVector TargetLocation = CaptureActor->TargetActor->GetActorLocation();

	// 터렛테이블 방식: Yaw는 고정(180도, 정면), Pitch만 가변
	// PreviewCharacter와 동일한 방식
	FRotator CameraRotation = FRotator(CurrentPitch, 180.0f, 0.0f);
	FVector Direction = CameraRotation.Vector();
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
	if (bIsDragging && CaptureActor && CaptureActor->TargetActor)
	{
		FVector2D CurrentMousePosition = InMouseEvent.GetScreenSpacePosition();
		FVector2D MouseDelta = CurrentMousePosition - LastMousePosition;
		LastMousePosition = CurrentMousePosition;

		// 터렛테이블 방식 (PreviewCharacter와 동일)
		// X 델타 → 메시를 Yaw 회전 (제자리 회전)
		AActor* TargetMesh = CaptureActor->TargetActor;
		FRotator MeshRotation = TargetMesh->GetActorRotation();
		MeshRotation.Yaw += MouseDelta.X * RotationSpeed;
		TargetMesh->SetActorRotation(MeshRotation);

		// Y 델타 → 카메라 Pitch만 조정 (위/아래 시점)
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
