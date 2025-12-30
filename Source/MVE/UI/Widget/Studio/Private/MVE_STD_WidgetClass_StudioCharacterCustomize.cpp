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
#include "Components/TileView.h"
#include "Data/AvatarStorageSubsystem.h"
#include "Data/AvatarDataObject.h"

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

	// TileView 이벤트 바인딩
	if (PresetContainer)
	{
		PresetContainer->OnItemClicked().AddUObject(this, &UMVE_STD_WidgetClass_StudioCharacterCustomize::OnPresetItemClicked);
	}

	// 버튼 바인딩
	if (AttachFileButton)
	{
		AttachFileButton->OnClicked.AddDynamic(this, &UMVE_STD_WidgetClass_StudioCharacterCustomize::OnAttachFileClicked);
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

UTexture2D* UMVE_STD_WidgetClass_StudioCharacterCustomize::CapturePreviewThumbnail()
{
	if (!PreviewActor || !PreviewActor->RenderTarget)
	{
		PRINTLOG(TEXT("FileNameText PreviewActor or RenderTarget is null"));
		return nullptr;
	}

	UTextureRenderTarget2D* RT = PreviewActor->RenderTarget;
	
	// RenderTarget을 읽기 위한 준비
	FTextureRenderTargetResource* RTResource = RT->GameThread_GetRenderTargetResource();
	if (!RTResource)
	{
		PRINTLOG(TEXT("FileNameText RenderTarget Resource is null"));
		return nullptr;
	}

	// 썸네일 크기 (작게)
	const int32 ThumbnailSize = 256;
	
	// RenderTarget의 픽셀 데이터 읽기
	TArray<FColor> SurfaceData;
	FIntRect Rect(0, 0, RT->SizeX, RT->SizeY);
	
	if (!RTResource->ReadPixels(SurfaceData, FReadSurfaceDataFlags(), Rect))
	{
		PRINTLOG(TEXT("FileNameText Failed to read RenderTarget pixels"));
		return nullptr;
	}

	// Texture2D 생성
	UTexture2D* Thumbnail = UTexture2D::CreateTransient(ThumbnailSize, ThumbnailSize, PF_B8G8R8A8);
	if (!Thumbnail)
	{
		PRINTLOG(TEXT("FileNameText Failed to create Texture2D"));
		return nullptr;
	}

	// 리샘플링 (1024x1024 → 256x256)
	TArray<FColor> ResizedData;
	ResizedData.SetNum(ThumbnailSize * ThumbnailSize);
	
	float ScaleX = (float)RT->SizeX / ThumbnailSize;
	float ScaleY = (float)RT->SizeY / ThumbnailSize;
	
	for (int32 Y = 0; Y < ThumbnailSize; Y++)
	{
		for (int32 X = 0; X < ThumbnailSize; X++)
		{
			int32 SrcX = FMath::Clamp((int32)(X * ScaleX), 0, RT->SizeX - 1);
			int32 SrcY = FMath::Clamp((int32)(Y * ScaleY), 0, RT->SizeY - 1);
			int32 SrcIndex = SrcY * RT->SizeX + SrcX;
			int32 DstIndex = Y * ThumbnailSize + X;
			
			if (SurfaceData.IsValidIndex(SrcIndex))
			{
				ResizedData[DstIndex] = SurfaceData[SrcIndex];
			}
		}
	}

	// Texture2D에 데이터 복사
	void* TextureData = Thumbnail->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, ResizedData.GetData(), ResizedData.Num() * sizeof(FColor));
	Thumbnail->GetPlatformData()->Mips[0].BulkData.Unlock();
	Thumbnail->UpdateResource();

	PRINTLOG(TEXT("FileNameText Thumbnail created: %dx%d"), ThumbnailSize, ThumbnailSize);
	
	return Thumbnail;
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
	TargetLocation.Z += 30.0f; // 약간 위로

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
				return FReply::Handled();
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
			return FReply::Handled();
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
	PRINTLOG(TEXT("FileNameText [2] OnAttachFileClicked 시작"));
	
	if (!StorageSubsystem)
	{
		PRINTLOG(TEXT("FileNameText [2] StorageSubsystem is null"));
		return;
	}

	// 파일 선택
	FString FilePath = StorageSubsystem->OpenFileDialog();
	if (FilePath.IsEmpty())
	{
		PRINTLOG(TEXT("FileNameText [2] 파일 선택 취소"));
		return;
	}

	PRINTLOG(TEXT("FileNameText [2] 파일 선택됨: %s"), *FilePath);

	// 파일 저장 (썸네일 없이 먼저)
	FAvatarData NewData;
	if (StorageSubsystem->SaveAvatarFile(FilePath, NewData))
	{
		PRINTLOG(TEXT("FileNameText [2] AvatarData 저장됨: %s"), *NewData.UniqueID);
		
		if (FileNameText)
		{
			FileNameText->SetText(FText::FromString(NewData.FileName));
		}

		CurrentSelectedID = NewData.UniqueID;
		
		// 먼저 프리뷰에 로드
		if (PreviewActor)
		{
			PRINTLOG(TEXT("FileNameText [2] 프리뷰에 메시 로드 중..."));
			
			PreviewActor->LoadAvatarMesh(FilePath);
			
			// 카메라 리셋
			CurrentYaw = 270.0f;
			CurrentPitch = 0.0f;
			CurrentDistance = DefaultDistance;
			UpdateCameraPosition();
			
			PRINTLOG(TEXT("FileNameText [2] 프리뷰 로드 완료, 0.3초 후 썸네일 캡처 시작"));
			
			// FileNameText 0.3초 후 썸네일 캡처 (렌더링 완료 대기)
			FTimerHandle ThumbnailTimer;
			GetWorld()->GetTimerManager().SetTimer(ThumbnailTimer, [this, NewData]()
			{
				PRINTLOG(TEXT("FileNameText [3] 타이머 실행 - 썸네일 캡처 시작"));
				
				// 썸네일 생성
				UTexture2D* Thumbnail = CapturePreviewThumbnail();
				
				if (Thumbnail)
				{
					PRINTLOG(TEXT("FileNameText [3] 썸네일 생성 성공!"));
					
					// 썸네일 업데이트
					if (StorageSubsystem->UpdateAvatarThumbnail(NewData.UniqueID, Thumbnail))
					{
						PRINTLOG(TEXT("FileNameText [3] 썸네일 저장 성공!"));
					}
					else
					{
						PRINTLOG(TEXT("FileNameText [3] 썸네일 저장 실패"));
					}
				}
				else
				{
					PRINTLOG(TEXT("FileNameText [3] 썸네일 생성 실패"));
				}
				
				// 프리셋 목록 갱신
				PRINTLOG(TEXT("FileNameText [3] 프리셋 목록 갱신 중..."));
				RefreshPresetList();
				
			}, 0.3f, false);
		}
		else
		{
			PRINTLOG(TEXT("FileNameText [2] PreviewActor is null"));
			RefreshPresetList();
		}
	}
	else
	{
		PRINTLOG(TEXT("FileNameText [2] AvatarData 저장 실패"));
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
	PRINTLOG(TEXT("FileNameText [5] RefreshPresetList 시작"));

	if (!PresetContainer || !StorageSubsystem)
	{
		PRINTLOG(TEXT("FileNameText [5] PresetContainer or StorageSubsystem is null"));
		return;
	}

	// TileView 초기화
	PresetContainer->ClearListItems();

	// 데이터 객체 배열 생성
	TArray<FAvatarData> Avatars = StorageSubsystem->GetSavedAvatars();

	PRINTLOG(TEXT("FileNameText [5] 저장된 Avatar 개수: %d"), Avatars.Num());

	for (int32 i = 0; i < Avatars.Num(); i++)
	{
		FAvatarData& Data = Avatars[i];
		PRINTLOG(TEXT("FileNameText [5] Avatar[%d]: %s, Thumbnail: %s"),
			i,
			*Data.FileName,
			Data.ThumbnailTexture ? TEXT("있음") : TEXT("없음"));

		// UAvatarDataObject 생성 및 TileView에 추가
		UAvatarDataObject* DataObject = NewObject<UAvatarDataObject>(this);
		DataObject->AvatarData = Data;
		PresetContainer->AddItem(DataObject);
	}

	PRINTLOG(TEXT("FileNameText [5] RefreshPresetList 완료"));
}

void UMVE_STD_WidgetClass_StudioCharacterCustomize::OnPresetItemClicked(UObject* Item)
{
	UAvatarDataObject* DataObject = Cast<UAvatarDataObject>(Item);
	if (DataObject)
	{
		OnPresetClicked(DataObject->AvatarData.UniqueID);
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
	CurrentYaw = 0.0f;
	CurrentPitch = 0.0f;
	CurrentDistance = DefaultDistance;
	UpdateCameraPosition();

	PRINTLOG(TEXT("Update preview for: %s"), *Data.FilePath);
}