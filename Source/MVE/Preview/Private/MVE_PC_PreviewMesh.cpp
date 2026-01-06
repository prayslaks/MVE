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
	}
	
	PRINTLOG(TEXT("✅ PreviewMesh PlayerController initialized"));
}

void AMVE_PC_PreviewMesh::SetupInputComponent()
{
	Super::SetupInputComponent();

	// EnhancedInputComponent로 캐스팅
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{

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
	InputMode.SetHideCursorDuringCapture(false);
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
	PRINTLOG(TEXT("⭐ OnMouseClickStarted CALLED!"));  // 디버그 로그 추가

	AMVE_AUD_PreviewCameraPawn* CameraPawn = GetCameraPawn();
	if (!CameraPawn)
	{
		PRINTLOG(TEXT("❌ CameraPawn is null"));
		return;
	}

	PRINTLOG(TEXT("🔍 IsGizmoMode: %s"), CameraPawn->IsGizmoMode() ? TEXT("TRUE") : TEXT("FALSE"));

	if (CameraPawn->IsGizmoMode())
	{
		// ✅ Gizmo 모드: PressPointer 1회 호출
		CameraPawn->OnMousePressed();

		// ✅ 카메라 회전 입력 차단 (기즈모 조작 중 카메라 움직임 방지)
		SetIgnoreLookInput(true);

		PRINTLOG(TEXT("🖱️ Gizmo Mode: Mouse Pressed → Look Input Disabled"));

		// ⚠️ Gizmo 모드에서는 입력을 소비하지 않음 (기즈모가 legacy 이벤트를 받을 수 있도록)
		// 참고: Enhanced Input이 ConsumeInput하면 기즈모가 마우스 추적을 못 함
		return;  // 입력을 소비하지 않고 통과시킴
	}
	else
	{
		// 뷰 모드: 드래그 시작
		bIsDragging = true;
		PRINTLOG(TEXT("🖱️ View Mode: Drag started"));
	}
}

void AMVE_PC_PreviewMesh::OnMouseClickCompleted(const FInputActionValue& Value)
{
	PRINTLOG(TEXT("⭐ OnMouseClickCompleted CALLED!"));  // 디버그 로그 추가

	AMVE_AUD_PreviewCameraPawn* CameraPawn = GetCameraPawn();
	if (!CameraPawn)
	{
		PRINTLOG(TEXT("❌ CameraPawn is null"));
		return;
	}

	PRINTLOG(TEXT("🔍 IsGizmoMode: %s"), CameraPawn->IsGizmoMode() ? TEXT("TRUE") : TEXT("FALSE"));

	if (CameraPawn->IsGizmoMode())
	{
		// ✅ Gizmo 모드: Blueprint 이벤트 호출 (기즈모 플러그인의 ReleasePointer 호출용)
		CameraPawn->OnMouseReleased();

		// ✅ 카메라 회전 입력 복원 (기즈모 조작 끝)
		SetIgnoreLookInput(false);

		PRINTLOG(TEXT("🖱️ Gizmo Mode: Mouse Released → Look Input Enabled"));
	}
	else
	{
		// 뷰 모드: 드래그 종료
		bIsDragging = false;
		PRINTLOG(TEXT("🖱️ View Mode: Drag ended"));
	}
}
