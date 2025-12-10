#include "MVE_STD_WidgetClass_StudioCharacterCustomize.h"

#include "AvatarPreviewActor.h"
#include "MVE.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Engine/TextureRenderTarget2D.h"
#include "MVE/UI/Widget/Studio/Public/AvatarPreviewActor.h"
#include "PresetButtonWidget.h"
#include "Data/AvatarStorageSubsystem.h"

void UMVE_STD_WidgetClass_StudioCharacterCustomize::NativeConstruct()
{
	Super::NativeConstruct();

	// 카메라 초기값
	CurrentDistance = DefaultDistance;
	CurrentYaw = 180.0f;
	CurrentPitch = 0.0f;
	bIsDragging = false;

	// Subsystem 가져오기
	if (UGameInstance* GI = GetGameInstance())
	{
		StorageSubsystem = GI->GetSubsystem<UAvatarStorageSubsystem>();
	}
	
	// 프리뷰 초기화
	InitializePreview();

	// 버튼 바인딩
	if (AttachFileButton)
	{
		AttachFileButton->OnClicked.AddDynamic(this, &UMVE_STD_WidgetClass_StudioCharacterCustomize::OnAttachFileClicked);
	}

	if (NextButton)
	{
		NextButton->OnClicked.AddDynamic(this, &UMVE_STD_WidgetClass_StudioCharacterCustomize::OnNextClicked);
	}

	// 초기 텍스트
	if (FileNameText)
	{
		FileNameText->SetText(FText::FromString(TEXT("파일 명")));
	}

	// 프리셋 목록 로드
	RefreshPresetList();

	PRINTLOG(TEXT("MVE_STD_WidgetClass_StudioCharacterCustomize NativeConstruct"));
}

void UMVE_STD_WidgetClass_StudioCharacterCustomize::NativeDestruct()
{
	PresetButtons.Empty();

	// 프리뷰 액터 정리
	if (PreviewActor)
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}

	Super::NativeDestruct();
}

void UMVE_STD_WidgetClass_StudioCharacterCustomize::InitializePreview()
{
	if (!StorageSubsystem)
	{
		PRINTLOG(TEXT("StorageSubsystem is null"));
		return;
	}

	// 프리뷰 액터 생성
	PreviewActor = StorageSubsystem->CreatePreviewActor(GetWorld());
	if (!PreviewActor)
	{
		PRINTLOG(TEXT("Failed to create PreviewActor"));
		return;
	}

	// RenderTarget 가져오기
	UTextureRenderTarget2D* RenderTarget = PreviewActor->GetRenderTarget();
	
	// RenderTarget 포맷 확인 및 수정 (검은색 투명 문제 해결)
	if (RenderTarget)
	{
		// 포맷이 잘못되어 있으면 수정
		if (RenderTarget->RenderTargetFormat != RTF_RGBA8 && 
		    RenderTarget->RenderTargetFormat != RTF_RGBA8_SRGB)
		{
			RenderTarget->RenderTargetFormat = RTF_RGBA8;
			RenderTarget->ClearColor = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Alpha=1.0 (불투명)
			RenderTarget->UpdateResourceImmediate(true);
			
			PRINTLOG(TEXT("RenderTarget format fixed to RGBA8"));
		}
		
		// Alpha가 0이면 수정
		if (RenderTarget->ClearColor.A < 0.5f)
		{
			RenderTarget->ClearColor.A = 1.0f;
			RenderTarget->UpdateResourceImmediate(true);
			
			PRINTLOG(TEXT("RenderTarget Alpha fixed to 1.0"));
		}
		
		SetRenderTarget(RenderTarget);
	}
	else
	{
		PRINTLOG(TEXT("RenderTarget is null - will be created by AvatarPreviewActor"));
	}

	// 초기 카메라 위치 설정
	UpdateCameraPosition();

	// 테스트용 메쉬 로드
	UStaticMesh* TestMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (TestMesh && PreviewActor)
	{
		PreviewActor->SetPreviewMesh(TestMesh);
		UpdateCameraPosition();
		PRINTLOG(TEXT("Test mesh (Sphere) loaded"));
	}

	PRINTLOG(TEXT("Preview initialized successfully"));
}

void UMVE_STD_WidgetClass_StudioCharacterCustomize::SetRenderTarget(UTextureRenderTarget2D* InRenderTarget)
{
	if (PreviewImage && InRenderTarget)
	{
		// RenderTarget을 FSlateBrush로 변환
		FSlateBrush Brush;
		Brush.SetResourceObject(InRenderTarget);
		Brush.ImageSize = FVector2D(InRenderTarget->SizeX, InRenderTarget->SizeY);
		Brush.DrawAs = ESlateBrushDrawType::Image;

		PreviewImage->SetBrush(Brush);
		
		PRINTLOG(TEXT("Render Target set to PreviewImage"));
	}
	else
	{
		PRINTLOG(TEXT("PreviewImage or RenderTarget is null!"));
	}
}

void UMVE_STD_WidgetClass_StudioCharacterCustomize::UpdateCameraPosition()
{
	if (!PreviewActor || !PreviewActor->TargetActor)
	{
		return;
	}

	// 타겟 위치 (메쉬 중심)
	FVector TargetLocation = PreviewActor->TargetActor->GetActorLocation();
	TargetLocation.Z += 50.0f; // 약간 위로

	// 구면 좌표계로 카메라 위치 계산
	FRotator Rotation = FRotator(CurrentPitch, CurrentYaw, 0.0f);
	FVector Direction = Rotation.Vector();
	FVector CameraLocation = TargetLocation - Direction * CurrentDistance;

	// Scene Capture 위치/회전 업데이트
	PreviewActor->SetActorLocation(CameraLocation);
	PreviewActor->SetActorRotation((TargetLocation - CameraLocation).Rotation());
}

// 우클릭으로 회전하도록 변경 (프리셋 버튼과 충돌 방지)
FReply UMVE_STD_WidgetClass_StudioCharacterCustomize::NativeOnMouseButtonDown
(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 우클릭으로 회전
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		// PreviewImage 영역 확인
		if (PreviewImage)
		{
			FGeometry PreviewGeometry = PreviewImage->GetCachedGeometry();
			FVector2D LocalMousePos = PreviewGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
			FVector2D LocalSize = PreviewGeometry.GetLocalSize();

			// 마우스가 PreviewImage 안에 있는지 확인
			if (LocalMousePos.X >= 0 && LocalMousePos.X <= LocalSize.X &&
				LocalMousePos.Y >= 0 && LocalMousePos.Y <= LocalSize.Y)
			{
				bIsDragging = true;
				LastMousePosition = InMouseEvent.GetScreenSpacePosition();
				PRINTLOG(TEXT("Preview rotation started (Right Click)"));
				return FReply::Handled().CaptureMouse(TakeWidget());
			}
		}
	}

	// 좌클릭은 통과시켜서 프리셋 버튼이 클릭되도록 함
	return FReply::Unhandled();
}

FReply UMVE_STD_WidgetClass_StudioCharacterCustomize::NativeOnMouseButtonUp
(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 우클릭 해제
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (bIsDragging)
		{
			bIsDragging = false;
			PRINTLOG(TEXT("Preview rotation ended"));
			return FReply::Handled().ReleaseMouseCapture();
		}
	}

	return FReply::Unhandled();
}

FReply UMVE_STD_WidgetClass_StudioCharacterCustomize::NativeOnMouseMove
(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDragging && PreviewActor)
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

FReply UMVE_STD_WidgetClass_StudioCharacterCustomize::NativeOnMouseWheel
(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (PreviewActor && PreviewImage)
	{
		// PreviewImage 영역 확인
		FGeometry PreviewGeometry = PreviewImage->GetCachedGeometry();
		FVector2D LocalMousePos = PreviewGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
		FVector2D LocalSize = PreviewGeometry.GetLocalSize();

		if (LocalMousePos.X >= 0 && LocalMousePos.X <= LocalSize.X &&
			LocalMousePos.Y >= 0 && LocalMousePos.Y <= LocalSize.Y)
		{
			float WheelDelta = InMouseEvent.GetWheelDelta();

			// 줌 인/아웃
			CurrentDistance = FMath::Clamp(CurrentDistance - WheelDelta * ZoomSpeed, MinDistance, MaxDistance);

			UpdateCameraPosition();

			return FReply::Handled();
		}
	}

	return FReply::Unhandled();
}

void UMVE_STD_WidgetClass_StudioCharacterCustomize::OnAttachFileClicked()
{
	if (!StorageSubsystem)
	{
		PRINTLOG(TEXT("StorageSubsystem is null"));
		return;
	}

	// 파일 선택
	FString FilePath = StorageSubsystem->OpenFileDialog();
	if (FilePath.IsEmpty())
	{
		PRINTLOG(TEXT("No file selected"));
		return;
	}

	// 파일 저장
	FAvatarData NewData;
	if (StorageSubsystem->SaveAvatarFile(FilePath, NewData))
	{
		if (FileNameText)
		{
			FileNameText->SetText(FText::FromString(NewData.FileName));
		}

		CurrentSelectedID = NewData.UniqueID;
		RefreshPresetList();
		UpdatePreview(NewData);

		PRINTLOG(TEXT("Avatar saved and selected: %s"), *NewData.UniqueID);
	}
	else
	{
		PRINTLOG(TEXT("Failed to save avatar file"));
	}
}

void UMVE_STD_WidgetClass_StudioCharacterCustomize::OnNextClicked()
{
	if (CurrentSelectedID.IsEmpty())
	{
		PRINTLOG(TEXT("No avatar selected"));
		return;
	}

	// TODO: 다음 화면으로 이동
	PRINTLOG(TEXT("Next button clicked with avatar: %s"), *CurrentSelectedID);
}

void UMVE_STD_WidgetClass_StudioCharacterCustomize::RefreshPresetList()
{
	if (!PresetContainer || !StorageSubsystem)
	{
		return;
	}

	PresetContainer->ClearChildren();
	PresetButtons.Empty();

	TArray<FAvatarData> Avatars = StorageSubsystem->GetSavedAvatars();
	for (const FAvatarData& Data : Avatars)
	{
		CreatePresetButton(Data);
	}

	PRINTLOG(TEXT("Refreshed preset list: %d items"), Avatars.Num());
}

void UMVE_STD_WidgetClass_StudioCharacterCustomize::CreatePresetButton(const FAvatarData& Data)
{
	if (!PresetButtonClass)
	{
		PRINTLOG(TEXT("PresetButtonClass not set"));
		return;
	}

	UPresetButtonWidget* PresetButton = CreateWidget<UPresetButtonWidget>(this, PresetButtonClass);
	if (PresetButton)
	{
		PresetButton->SetAvatarData(Data);
		PresetButton->OnButtonClicked.BindUObject(this, &UMVE_STD_WidgetClass_StudioCharacterCustomize::OnPresetClicked);

		PresetButtons.Add(PresetButton);
		PresetContainer->AddChild(PresetButton);
	}
}

void UMVE_STD_WidgetClass_StudioCharacterCustomize::OnPresetClicked(const FString& UniqueID)
{
	CurrentSelectedID = UniqueID;

	FAvatarData Data;
	if (StorageSubsystem && StorageSubsystem->GetAvatarData(UniqueID, Data))
	{
		if (FileNameText)
		{
			FileNameText->SetText(FText::FromString(Data.FileName));
		}
		UpdatePreview(Data);
	}

	PRINTLOG(TEXT("Preset clicked: %s"), *UniqueID);
}

void UMVE_STD_WidgetClass_StudioCharacterCustomize::UpdatePreview(const FAvatarData& Data)
{
	if (!PreviewActor)
	{
		PRINTLOG(TEXT("PreviewActor is null"));
		return;
	}

	// 실제 GLB 파일 로드
	PreviewActor->LoadAvatarMesh(Data.FilePath);

	// 카메라 리셋
	CurrentYaw = 180.0f;
	CurrentPitch = 0.0f;
	CurrentDistance = DefaultDistance;
	UpdateCameraPosition();

	PRINTLOG(TEXT("Update preview for: %s"), *Data.FilePath);
}