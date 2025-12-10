#include "../Public/MVE_PC_PreviewMesh.h"
#include "MVE.h"
#include "MVE_AUD_PreviewCameraPawn.h"
#include "MVE_GM_PreviewMesh.h"
#include "UIManagerSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

AMVE_PC_PreviewMesh::AMVE_PC_PreviewMesh()
{
	bShowMouseCursor = true;
}

void AMVE_PC_PreviewMesh::BeginPlay()
{
	Super::BeginPlay();

	// UI와 게임 모두 입력 받도록 설정
	SetGameAndUIMode();

	// EnhancedInput MappingContext 추가
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (PreviewMappingContext)
		{
			Subsystem->AddMappingContext(PreviewMappingContext, 0);
			PRINTLOG(TEXT("✅ Preview MappingContext added"));
		}
		else
		{
			PRINTLOG(TEXT("⚠️ PreviewMappingContext is not set!"));
		}
	}

	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UIManager->ShowScreen(EUIScreen::AudienceStation);
		//UIManager->ShowPopup(EUIPopup::AudienceCustomizing);
	}

	PRINTLOG(TEXT("✅ PreviewMesh PlayerController initialized"));
}

void AMVE_PC_PreviewMesh::SetupInputComponent()
{
	Super::SetupInputComponent();

	// EnhancedInputComponent로 캐스팅
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// 카메라 Look (마우스 드래그)
		if (CameraLookAction)
		{
			EnhancedInput->BindAction(CameraLookAction, ETriggerEvent::Triggered, this, &AMVE_PC_PreviewMesh::OnCameraLook);
		}

		// 카메라 Zoom (마우스 휠)
		if (CameraZoomAction)
		{
			EnhancedInput->BindAction(CameraZoomAction, ETriggerEvent::Triggered, this, &AMVE_PC_PreviewMesh::OnCameraZoom);
		}

		// 마우스 클릭 (Gizmo용)
		if (MouseClickAction)
		{
			EnhancedInput->BindAction(MouseClickAction, ETriggerEvent::Started, this, &AMVE_PC_PreviewMesh::OnMouseClickStarted);
			EnhancedInput->BindAction(MouseClickAction, ETriggerEvent::Completed, this, &AMVE_PC_PreviewMesh::OnMouseClickCompleted);
		}

		PRINTLOG(TEXT("✅ Enhanced Input actions bound"));
	}
	else
	{
		PRINTLOG(TEXT("❌ Failed to cast to EnhancedInputComponent"));
	}
}

AMVE_AUD_PreviewCameraPawn* AMVE_PC_PreviewMesh::GetCameraPawn() const
{
	return Cast<AMVE_AUD_PreviewCameraPawn>(GetPawn());
}

AActor* AMVE_PC_PreviewMesh::GetPreviewCharacter() const
{
	AMVE_GM_PreviewMesh* GM = GetWorld()->GetAuthGameMode<AMVE_GM_PreviewMesh>();
	if (GM)
	{
		return GM->GetPreviewCharacter();
	}
	return nullptr;
}

void AMVE_PC_PreviewMesh::SetUIOnlyMode()
{
	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void AMVE_PC_PreviewMesh::SetGameAndUIMode()
{
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void AMVE_PC_PreviewMesh::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	AMVE_AUD_PreviewCameraPawn* CameraPawn = GetCameraPawn();
	if (!CameraPawn)
	{
		return;
	}

	// Gizmo 모드일 때는 회전 비활성화
	if (CameraPawn->IsGizmoMode())
	{
		return;
	}

	// 마우스 드래그 중일 때만 캐릭터 회전 (터렛테이블 방식)
	if (bIsDragging)
	{
		// 마우스 델타 직접 가져오기
		float MouseX, MouseY;
		GetInputMouseDelta(MouseX, MouseY);

		if (FMath::Abs(MouseX) > 0.01f || FMath::Abs(MouseY) > 0.01f)
		{
			// 캐릭터를 회전시킴 (카메라는 고정)
			CameraPawn->RotateCharacter(MouseX, -MouseY);
		}
	}
}

void AMVE_PC_PreviewMesh::OnCameraLook(const FInputActionValue& Value)
{
	// 이제 사용 안 함 (PlayerTick에서 처리)
	// EnhancedInput의 Mouse XY는 절대 좌표만 주므로 델타를 직접 가져와야 함
}

void AMVE_PC_PreviewMesh::OnCameraZoom(const FInputActionValue& Value)
{
	AMVE_AUD_PreviewCameraPawn* CameraPawn = GetCameraPawn();
	if (!CameraPawn)
	{
		return;
	}

	// Gizmo 모드일 때는 줌 비활성화
	if (CameraPawn->IsGizmoMode())
	{
		return;
	}

	float ZoomValue = Value.Get<float>();
	CameraPawn->ZoomCamera(ZoomValue);
}

void AMVE_PC_PreviewMesh::OnMouseClickStarted(const FInputActionValue& Value)
{
	AMVE_AUD_PreviewCameraPawn* CameraPawn = GetCameraPawn();
	if (!CameraPawn)
	{
		return;
	}

	if (CameraPawn->IsGizmoMode())
	{
		// Gizmo 모드: Blueprint의 PressPointer 호출 (Blueprint에서 처리)
		// C++에서는 아무것도 안 함 (Blueprint Event로 처리)
	}
	else
	{
		// 뷰 모드: 드래그 시작
		bIsDragging = true;
	}
}

void AMVE_PC_PreviewMesh::OnMouseClickCompleted(const FInputActionValue& Value)
{
	AMVE_AUD_PreviewCameraPawn* CameraPawn = GetCameraPawn();
	if (!CameraPawn)
	{
		return;
	}

	if (CameraPawn->IsGizmoMode())
	{
		// Gizmo 모드: Blueprint의 ReleasePointer 호출 (Blueprint에서 처리)
		// C++에서는 아무것도 안 함
	}
	else
	{
		// 뷰 모드: 드래그 종료
		bIsDragging = false;
	}
}
