#include "../Public/MVE_AUD_PreviewCameraPawn.h"
#include "MVE.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "MVE_AUD_CustomizationManager.h"
#include "MVE_GM_PreviewMesh.h"

AMVE_AUD_PreviewCameraPawn::AMVE_AUD_PreviewCameraPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root Component (빈 SceneComponent)
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Spring Arm 생성
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = DefaultArmLength;
	SpringArm->bUsePawnControlRotation = false;  // Pawn 회전 사용 안 함
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bDoCollisionTest = false;  // 프리뷰 전용이므로 충돌 검사 불필요

	// Camera 생성
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
}

void AMVE_AUD_PreviewCameraPawn::BeginPlay()
{
	Super::BeginPlay();

	// 초기 회전 설정
	UpdateCameraRotation();

	// ⭐ PreviewCharacter 자동 찾기 (Timer로 재시도)
	TryFindPreviewCharacter();
}

void AMVE_AUD_PreviewCameraPawn::TryFindPreviewCharacter()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	AMVE_GM_PreviewMesh* GM = World->GetAuthGameMode<AMVE_GM_PreviewMesh>();
	if (!GM)
	{
		return;
	}

	AActor* PreviewCharacter = GM->GetPreviewCharacter();
	if (PreviewCharacter)
	{
		SetTargetActor(PreviewCharacter);
		PRINTLOG(TEXT("✅ CameraPawn auto-found and targeted PreviewCharacter"));
	}
	else
	{
		// 아직 스폰 안 됨, 0.1초 후 재시도
		PRINTLOG(TEXT("⚠️ PreviewCharacter not spawned yet, retrying..."));

		FTimerHandle RetryTimer;
		World->GetTimerManager().SetTimer(RetryTimer, this, &AMVE_AUD_PreviewCameraPawn::TryFindPreviewCharacter, 0.1f, false);
	}
}

void AMVE_AUD_PreviewCameraPawn::RotateCamera(float DeltaYaw, float DeltaPitch)
{
	CurrentYaw += DeltaYaw * RotationSpeed;
	CurrentPitch = FMath::Clamp(CurrentPitch + DeltaPitch * RotationSpeed, -80.0f, 80.0f);

	UpdateCameraRotation();
}

void AMVE_AUD_PreviewCameraPawn::ZoomCamera(float Delta)
{
	float NewLength = SpringArm->TargetArmLength - Delta * ZoomSpeed;
	SpringArm->TargetArmLength = FMath::Clamp(NewLength, MinArmLength, MaxArmLength);

	PRINTLOG(TEXT("Zoom: %f"), SpringArm->TargetArmLength);
}

void AMVE_AUD_PreviewCameraPawn::ResetCamera()
{
	CurrentYaw = 180.0f;
	CurrentPitch = 0.0f;
	CurrentCharacterYaw = 0.0f;
	SpringArm->TargetArmLength = DefaultArmLength;

	UpdateCameraRotation();

	// 캐릭터 회전도 리셋
	if (TargetActor)
	{
		FRotator NewRotation = TargetActor->GetActorRotation();
		NewRotation.Yaw = 0.0f;
		TargetActor->SetActorRotation(NewRotation);
	}
}

void AMVE_AUD_PreviewCameraPawn::SetTargetActor(AActor* InTargetActor)
{
	TargetActor = InTargetActor;

	if (TargetActor)
	{
		// Pawn을 타겟 위치로 이동
		FVector TargetLocation = TargetActor->GetActorLocation();
		TargetLocation.Z += 50.0f;  // 캐릭터 중심보다 약간 위
		SetActorLocation(TargetLocation);

		// 현재 캐릭터의 Yaw 값을 저장
		CurrentCharacterYaw = TargetActor->GetActorRotation().Yaw;

		PRINTLOG(TEXT("✅ CameraPawn target set: %s at %s (Yaw: %.1f)"),
			*TargetActor->GetName(),
			*TargetLocation.ToString(),
			CurrentCharacterYaw);
	}
}

void AMVE_AUD_PreviewCameraPawn::UpdateCameraRotation()
{
	FRotator NewRotation = FRotator(CurrentPitch, CurrentYaw, 0.0f);
	SpringArm->SetRelativeRotation(NewRotation);
}

void AMVE_AUD_PreviewCameraPawn::RotateCharacter(float DeltaYaw, float DeltaPitch)
{
	if (!TargetActor)
	{
		// TargetActor가 없으면 다시 찾기 시도 (한 번만)
		TryFindPreviewCharacter();
		return;
	}

	// 캐릭터 Yaw 회전 (좌우)
	CurrentCharacterYaw += DeltaYaw * RotationSpeed;

	// 캐릭터 회전 적용
	FRotator NewRotation = TargetActor->GetActorRotation();
	NewRotation.Yaw = CurrentCharacterYaw;
	TargetActor->SetActorRotation(NewRotation);

	// Pitch는 카메라 각도로 적용 (위아래 시점)
	CurrentPitch = FMath::Clamp(CurrentPitch + DeltaPitch * RotationSpeed, -80.0f, 80.0f);
	UpdateCameraRotation();
}

void AMVE_AUD_PreviewCameraPawn::SwitchToGizmoMode(AActor* InTargetActor)
{
	if (bIsGizmoMode)
	{
		PRINTLOG(TEXT("⚠️ Already in Gizmo mode"));
		return;
	}

	if (!InTargetActor)
	{
		PRINTLOG(TEXT("❌ No target actor for Gizmo"));
		return;
	}

	bIsGizmoMode = true;
	GizmoTargetActor = InTargetActor;

	PRINTLOG(TEXT("=== CameraPawn: Switch to GIZMO Mode ==="));
	PRINTLOG(TEXT("Target Actor: %s"), *InTargetActor->GetName());

	// InputMode를 GameOnly로 변경 (UI 입력 차단, 마우스 추적 활성화)
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		FInputModeGameOnly GameOnlyMode;
		PC->SetInputMode(GameOnlyMode);
		PC->bShowMouseCursor = true;
		PRINTLOG(TEXT("✅ InputMode changed to GameOnly for Gizmo"));
	}

	// Blueprint 이벤트 호출 (Gizmo Actor 생성/표시)
	OnSwitchToGizmoMode(InTargetActor);
}

void AMVE_AUD_PreviewCameraPawn::SwitchToViewMode()
{
	if (!bIsGizmoMode)
	{
		PRINTLOG(TEXT("⚠️ Already in View mode"));
		return;
	}

	bIsGizmoMode = false;

	PRINTLOG(TEXT("=== CameraPawn: Switch to VIEW Mode ==="));

	// InputMode를 GameAndUI로 복원
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		FInputModeGameAndUI GameAndUIMode;
		GameAndUIMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(GameAndUIMode);
		PC->bShowMouseCursor = true;
		PRINTLOG(TEXT("✅ InputMode restored to GameAndUI"));
	}

	// Blueprint 이벤트 호출 (Gizmo 숨김)
	OnSwitchToViewMode();

	GizmoTargetActor = nullptr;
}

void AMVE_AUD_PreviewCameraPawn::SaveAccessoryTransform(AActor* Accessory, const FTransform& NewTransform)
{
	if (!Accessory)
	{
		return;
	}

	PRINTLOG(TEXT("=== CameraPawn: Saving Transform ==="));
	PRINTLOG(TEXT("Actor: %s"), *Accessory->GetName());
	PRINTLOG(TEXT("Location: %s"), *NewTransform.GetLocation().ToString());
	PRINTLOG(TEXT("Rotation: %s"), *NewTransform.Rotator().ToString());
	PRINTLOG(TEXT("Scale: %s"), *NewTransform.GetScale3D().ToString());

	// CustomizationManager에 저장
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI)
	{
		UMVE_AUD_CustomizationManager* Manager = GI->GetSubsystem<UMVE_AUD_CustomizationManager>();
		if (Manager)
		{
			Manager->SaveAccessoryTransform(Accessory, NewTransform);
		}
	}
}

void AMVE_AUD_PreviewCameraPawn::DetachGizmoTarget()
{
	if (!GizmoTargetActor)
	{
		PRINTLOG(TEXT("❌ No GizmoTargetActor to detach"));
		return;
	}

	// 현재 부착 정보 저장
	if (USceneComponent* RootComp = GizmoTargetActor->GetRootComponent())
	{
		OriginalParent = RootComp->GetAttachParent();
		OriginalSocketName = RootComp->GetAttachSocketName();
		OriginalRelativeTransform = RootComp->GetRelativeTransform();

		PRINTLOG(TEXT("=== Detaching Gizmo Target ==="));
		PRINTLOG(TEXT("Actor: %s"), *GizmoTargetActor->GetName());
		PRINTLOG(TEXT("Original Parent: %s"), OriginalParent ? *OriginalParent->GetName() : TEXT("None"));
		PRINTLOG(TEXT("Original Socket: %s"), *OriginalSocketName.ToString());

		// Detach (World Transform 유지)
		FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, false);
		RootComp->DetachFromComponent(DetachRules);

		PRINTLOG(TEXT("✅ Actor detached - now free to move with Gizmo"));
	}
}

void AMVE_AUD_PreviewCameraPawn::ReattachGizmoTarget()
{
	if (!GizmoTargetActor || !OriginalParent)
	{
		PRINTLOG(TEXT("❌ Cannot reattach - missing actor or parent info"));
		return;
	}

	if (USceneComponent* RootComp = GizmoTargetActor->GetRootComponent())
	{
		PRINTLOG(TEXT("=== Reattaching Gizmo Target ==="));
		PRINTLOG(TEXT("Actor: %s"), *GizmoTargetActor->GetName());
		PRINTLOG(TEXT("Parent: %s"), *OriginalParent->GetName());
		PRINTLOG(TEXT("Socket: %s"), *OriginalSocketName.ToString());

		// 현재 World Transform 저장
		FTransform CurrentWorldTransform = RootComp->GetComponentTransform();

		// Reattach (KeepRelative로 현재 변경된 위치 유지)
		FAttachmentTransformRules AttachRules(
			EAttachmentRule::KeepRelative,
			EAttachmentRule::KeepRelative,
			EAttachmentRule::KeepRelative,
			false
		);
		RootComp->AttachToComponent(OriginalParent, AttachRules, OriginalSocketName);

		// World Transform을 Relative Transform으로 변환해서 적용
		FTransform ParentTransform = OriginalParent->GetSocketTransform(OriginalSocketName);
		FTransform NewRelativeTransform = CurrentWorldTransform.GetRelativeTransform(ParentTransform);
		RootComp->SetRelativeTransform(NewRelativeTransform);

		PRINTLOG(TEXT("✅ Actor reattached with new Relative Transform:"));
		PRINTLOG(TEXT("   Location: %s"), *NewRelativeTransform.GetLocation().ToString());
		PRINTLOG(TEXT("   Rotation: %s"), *NewRelativeTransform.Rotator().ToString());
		PRINTLOG(TEXT("   Scale: %s"), *NewRelativeTransform.GetScale3D().ToString());

		// 저장된 정보 초기화
		OriginalParent = nullptr;
		OriginalSocketName = NAME_None;
	}
}
