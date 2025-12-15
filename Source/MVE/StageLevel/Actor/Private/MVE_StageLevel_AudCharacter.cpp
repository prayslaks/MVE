#include "StageLevel/Actor/Public/MVE_StageLevel_AudCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "MVE.h"
#include "StageLevel/Actor/Public/MVE_StageLevel_AudCharacterShooterComponent.h"
#include "StageLevel/Default/Public/MVE_PC_StageLevel.h"
#include "Curves/CurveFloat.h"

AMVE_StageLevel_AudCharacter::AMVE_StageLevel_AudCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// 스프링 암
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 300.f;
	SpringArm->bUsePawnControlRotation = true; 
	
	// 카메라
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	
	// 오디오 컴포넌트
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);
	
	// 사격 컴포넌트
	ShooterComponent = CreateDefaultSubobject<UMVE_StageLevel_AudCharacterShooterComponent>(TEXT("ShooterComponent"));
	
	// 카메라 전환 타임라인
	CameraTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("CameraTimeline"));
	
	// 기본적으로 컨트롤러의 회전값이 캐릭터에 영향을 주지 않고 카메라만 독립적으로 회전하도록 설정
	bUseControllerRotationYaw = false;
	
	// 이동 방향으로 캐릭터가 자동 회전하는 기능을 비활성화
	GetCharacterMovement()->bOrientRotationToMovement = false;
	
	// 컨트롤 모드 기본
	CurrentControlMode = EAudienceControlMode::Default;
}

void AMVE_StageLevel_AudCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 플레이어 컨트롤러 획득
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		// 향상된 입력 로컬 플레이어 서브시스템 획득 
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// 우선순위 0으로 입력 매핑 컨텍스트 추가 
			if (IMC_AudCharacter)
			{
				Subsystem->AddMappingContext(IMC_AudCharacter, 0);
			}
			const FInputModeGameOnly InputModeGameOnly; 
			PlayerController->SetInputMode(InputModeGameOnly);
			PlayerController->SetShowMouseCursor(false);
			PRINTNETLOG(this, TEXT("입력모드 : GameOnly"))
		}
	}
	// 타임라인 설정
	if (CameraTransitionCurve)
	{
		FOnTimelineFloat UpdateCallback;
		UpdateCallback.BindUFunction(this, FName("UpdateCameraTimeline"));
		CameraTimeline->AddInterpFloat(CameraTransitionCurve, UpdateCallback);
	}
}

void AMVE_StageLevel_AudCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (const auto EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 이동이나 점프는 아직 보류
		// EnhancedInputComponent->BindAction(IA_Move)
		// EnhancedInputComponent->BindAction(IA_Jump)
		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AMVE_StageLevel_AudCharacter::OnInputActionLookTriggered);	
		EnhancedInputComponent->BindAction(IA_Aim, ETriggerEvent::Started, this, &AMVE_StageLevel_AudCharacter::OnInputActionAimStarted);
		EnhancedInputComponent->BindAction(IA_Aim, ETriggerEvent::Completed, this, &AMVE_StageLevel_AudCharacter::OnInputActionAimCompleted);
		EnhancedInputComponent->BindAction(IA_SwitchAudienceMode, ETriggerEvent::Started, this, &AMVE_StageLevel_AudCharacter::OnInputActionSwitchAudienceModeStarted);
		EnhancedInputComponent->BindAction(IA_SwitchAudienceMode, ETriggerEvent::Completed, this, &AMVE_StageLevel_AudCharacter::OnInputActionSwitchAudienceModeCompleted);
	}
}

void AMVE_StageLevel_AudCharacter::SetControlMode(const EAudienceControlMode NewMode)
{
	CurrentControlMode = NewMode;
	switch (CurrentControlMode)
	{
	case EAudienceControlMode::WidgetSelection:
		// 위젯 선택 중에는 캐릭터가 카메라를 따라 회전하지 않는다
		// 카메라 포지션 컨트롤도 존재하지 않는다
		bUseControllerRotationYaw = false;
		break;
	case EAudienceControlMode::Aiming:
	case EAudienceControlMode::Photo:
	case EAudienceControlMode::Cheer:
	case EAudienceControlMode::WaveLightStick:
		// 이 모드들에서는 캐릭터가 컨트롤러의 Yaw 회전값을 따라야 한다
		// 목표를 보기에 적절한 위치로 카메라 포지션을 컨트롤한다 
		bUseControllerRotationYaw = true;
		if (CameraTimeline)
		{
			CameraTimeline->Play();
		}
		break;
	case EAudienceControlMode::Default:
	default:
		// 기본 상태에서는 캐릭터가 카메라를 따라 회전하지 않는다
		// 주변을 둘러보기 좋도록 카메라 포지션을 컨트롤한다
		bUseControllerRotationYaw = false;
		if (CameraTimeline)
		{
			CameraTimeline->Reverse();
		}
		break;
	}
	PRINTNETLOG(this, TEXT("현재 모드 %s"), *StaticEnum<EAudienceControlMode>()->GetNameStringByValue(static_cast<int64>(CurrentControlMode)));
}

void AMVE_StageLevel_AudCharacter::OnInputActionAimStarted(const FInputActionValue& Value)
{
	// 모드에 따라서 입력 처리
	switch (CurrentControlMode)
	{
	case EAudienceControlMode::Aiming:
	case EAudienceControlMode::Photo:
		// 현재 홀드 여부를 추출
		bAimEnabled = Value.Get<bool>();
		break;
	default: 
		bAimEnabled = false;
	}
}

void AMVE_StageLevel_AudCharacter::OnInputActionAimCompleted(const FInputActionValue& Value)
{
	// 모드에 따라서 입력 처리
	switch (CurrentControlMode)
	{
	case EAudienceControlMode::Aiming:
	case EAudienceControlMode::Photo:
		// 현재 홀드 여부를 추출
		bAimEnabled = Value.Get<bool>();
		break;
	default: 
		bAimEnabled = false;
	}
}

void AMVE_StageLevel_AudCharacter::OnInputActionLookTriggered(const FInputActionValue& Value)
{
	if (Controller == nullptr)
	{
		return;
	}
	
	// Value에서 Vector2D 형태로 값을 추출
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	
	switch (CurrentControlMode)
	{
	case EAudienceControlMode::Default:
	case EAudienceControlMode::Aiming:
	case EAudienceControlMode::Photo:
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
		break;
	case EAudienceControlMode::Cheer:
	case EAudienceControlMode::WaveLightStick:
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
		break;
	case EAudienceControlMode::WidgetSelection:
		// 위젯 선택 모드에서는 카메라 회전을 막습니다.
		break;
	default:
		// 처리되지 않은 다른 모드들
		break;
	}
}

void AMVE_StageLevel_AudCharacter::OnInputActionSwitchAudienceModeStarted()
{
	SetControlMode(EAudienceControlMode::WidgetSelection);
	if (AMVE_PC_StageLevel* PC = Cast<AMVE_PC_StageLevel>(GetController()))
	{
		PC->ToggleRadialMenu(true);
	}
}

void AMVE_StageLevel_AudCharacter::OnInputActionSwitchAudienceModeCompleted()
{
	if (AMVE_PC_StageLevel* PC = Cast<AMVE_PC_StageLevel>(GetController()))
	{
		// 선택한 메뉴 섹션 번호로 모드를 변경
		const int32 Selection = PC->GetRadialMenuSelection();
		PC->ToggleRadialMenu(false);
		SetControlMode(static_cast<EAudienceControlMode>(Selection));
	}
	else
	{
		// 예외 처리
		SetControlMode(EAudienceControlMode::Default);
	}
}

void AMVE_StageLevel_AudCharacter::UpdateCameraTimeline(const float Alpha) const
{
	// 스프링 암 길이 보간
	constexpr float DefaultArmLength = 250.0;
	constexpr float ActionArmLength = 150.0;
	SpringArm->TargetArmLength = FMath::Lerp(DefaultArmLength, ActionArmLength, Alpha);
	
	// 카메라 상대 위치 보간
	const FVector DefaultLocation = FVector::ZeroVector;
	const FVector ActionLocation = FVector(0.0, 80.0, 80.0);
	SpringArm->SetRelativeLocation(FMath::Lerp(DefaultLocation, ActionLocation, Alpha));
}
