#include "StageLevel/Actor/Public/MVE_StageLevel_AudCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "MVE.h"
#include "MVE_WC_StageLevel_AudInputHelp.h"
#include "Components/ArrowComponent.h"
#include "StageLevel/Actor/Public/MVE_StageLevel_AudCharacterShooterComponent.h"
#include "StageLevel/Default/Public/MVE_PC_StageLevel.h"
#include "Curves/CurveFloat.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "StageLevel/Actor/Public/MVE_StageLevel_AudCamera.h"
#include "StageLevel/Actor/Public/MVE_ThrowObject.h"

AMVE_StageLevel_AudCharacter::AMVE_StageLevel_AudCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(true);
	ACharacter::SetReplicateMovement(true);
	
	// 스프링 암
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 300.f;
	SpringArm->bUsePawnControlRotation = true; 
	SpringArm->bDoCollisionTest = true;
	
	// 카메라
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	
	// 오디오 컴포넌트
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);
	
	// 사격 컴포넌트
	ShooterComponent = CreateDefaultSubobject<UMVE_StageLevel_AudCharacterShooterComponent>(TEXT("ShooterComponent"));
	
	// 던지기 애로우 컴포넌트
	RightThrowArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("RightThrowArrowComp"));
	RightThrowArrowComp->SetupAttachment(GetMesh(), FName("HandGrip_R"));
	
	// 카메라 전환 타임라인
	CameraTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("CameraTimeline"));
	
	// 오디언스 카메라 차일드 액터 컴포넌트
	AudCameraChildActorComp = CreateDefaultSubobject<UChildActorComponent>(TEXT("AudCameraChildActorComp"));
	AudCameraChildActorComp->SetupAttachment(GetMesh(), TEXT("HandGrip_R"));
	
	// 기본적으로 컨트롤러의 회전값이 캐릭터에 영향을 주지 않고 카메라만 독립적으로 회전하도록 설정
	bUseControllerRotationYaw = false;
	
	// 이동 방향으로 캐릭터가 자동 회전하는 기능을 비활성화
	GetCharacterMovement()->bOrientRotationToMovement = false;
	
	// 컨트롤 모드 기본
	CurrentControlMode = EAudienceControlMode::Default;
	
	// 모드 별 카메라 위치
	DefaultCameraPosition = FMVE_StageLevel_AudCharacterCameraPosition(250.0f, { 0.0f, 0.0f, 0.0f });
	PhotoActionCameraPosition = FMVE_StageLevel_AudCharacterCameraPosition(140.0f, { 0.0f, 50.0f, 50.0f });
	ThrowActionCameraPosition = FMVE_StageLevel_AudCharacterCameraPosition(150.0f, { 0.0f, 70.0f, 70.0f });
	CheerActionCameraPosition = FMVE_StageLevel_AudCharacterCameraPosition(180.0f, { 0.0f, 40.0f, 40.0f });
	WaveLightStickActionCameraPosition = FMVE_StageLevel_AudCharacterCameraPosition(200.0f, { 0.0f, 60.0f, 60.0f });
	ClapActionCameraPosition = FMVE_StageLevel_AudCharacterCameraPosition(190.0f, { 0.0f, 80.0f, 80.0f });
	
	// 상태 별 카메라 위치
	PhotoAimCameraPosition = FMVE_StageLevel_AudCharacterCameraPosition(110.0f, {0.0f, 50.0f, 50.0f}); 
	ThrowAimCameraPosition = FMVE_StageLevel_AudCharacterCameraPosition(120.0f, { 0.0f, 70.0f, 70.0f });
	
	// 시점 변경
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->bOrientRotationToMovement = true;
		MovementComp->RotationRate = FRotator(0.f, 500.f, 0.f);
	}

	TargetArmLength = ThirdPersonArmLength;
	ZoomTargetArmLength = ThirdPersonArmLength;

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
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AMVE_StageLevel_AudCharacter::HandleMove);
		EnhancedInputComponent->BindAction(IA_ToggleViewpoint, ETriggerEvent::Started, this, &AMVE_StageLevel_AudCharacter::HandleToggleViewpoint);
		EnhancedInputComponent->BindAction(IA_Zoom, ETriggerEvent::Triggered, this, &AMVE_StageLevel_AudCharacter::HandleZoom);
		// 카메라 관련이랑 이동 추가함
		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AMVE_StageLevel_AudCharacter::OnInputActionLookTriggered);	
		EnhancedInputComponent->BindAction(IA_Aim, ETriggerEvent::Started, this, &AMVE_StageLevel_AudCharacter::OnInputActionAimStarted);
		EnhancedInputComponent->BindAction(IA_Aim, ETriggerEvent::Completed, this, &AMVE_StageLevel_AudCharacter::OnInputActionAimCompleted);
		EnhancedInputComponent->BindAction(IA_Execute, ETriggerEvent::Started, this, &AMVE_StageLevel_AudCharacter::OnInputActionExecuteStarted);
		EnhancedInputComponent->BindAction(IA_SwitchAudienceMode, ETriggerEvent::Started, this, &AMVE_StageLevel_AudCharacter::OnInputActionSwitchAudienceModeStarted);
		EnhancedInputComponent->BindAction(IA_SwitchAudienceMode, ETriggerEvent::Completed, this, &AMVE_StageLevel_AudCharacter::OnInputActionSwitchAudienceModeCompleted);
	}
}

void AMVE_StageLevel_AudCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMVE_StageLevel_AudCharacter, bIsAiming);
	DOREPLIFETIME(AMVE_StageLevel_AudCharacter, bIsExecuting);
	DOREPLIFETIME(AMVE_StageLevel_AudCharacter, CurrentControlMode);
}

#pragma region 조준 여부 변수 할당 & 리플리케이션 처리

void AMVE_StageLevel_AudCharacter::OnInputActionAimStarted(const FInputActionValue& Value)
{
	const bool Input = Value.Get<bool>();
	RequestSetIsAiming(Input);
}

void AMVE_StageLevel_AudCharacter::OnInputActionAimCompleted(const FInputActionValue& Value)
{
	RequestSetIsAiming(false);
}

void AMVE_StageLevel_AudCharacter::RequestSetIsAiming(const bool Value)
{
	// 조준 가능한 모드인지 확인
	if (CurrentControlMode != EAudienceControlMode::Throw && CurrentControlMode != EAudienceControlMode::Photo)
	{
		return;
	}
	
	// 네트워크 대역폭 관리
	if (bIsAiming == Value)
	{
		return;
	}
	
	// 서버를 거쳐서 리플리케이션을 통해 최종 업데이트
	Server_SetIsAiming(Value);
}

void AMVE_StageLevel_AudCharacter::Server_SetIsAiming_Implementation(const bool Value)
{
	bIsAiming = Value;
}

void AMVE_StageLevel_AudCharacter::OnRep_IsAiming()
{
	PRINTNETLOG(this, TEXT("서버에 의해 리플리케이션됐음!"));
	
	// 조준이 풀리면 다시 조준을 하라고 도움말이 뜨고, 조준이 걸리면 발사를 하라고 도움말이 뜬다
	GetBindingPC()->SwitchInputHelpWidget(bIsAiming ? EAudienceInputHelpState::Executable : EAudienceInputHelpState::Aimable);
	
	switch (CurrentControlMode)
	{
		case EAudienceControlMode::Photo:
			{
				CameraInterpStartPosition.Copy(SpringArm);
				CameraInterpEndPosition = bIsAiming ? PhotoAimCameraPosition : PhotoActionCameraPosition;
				PRINTNETLOG(this, TEXT("포지션 : %s"), (bIsAiming ? TEXT("PhotoAimCameraPosition") : TEXT("PhotoActionCameraPosition")));
				if (CameraTimeline)
				{
					CameraTimeline->PlayFromStart();
				}
				break;
			}
		case EAudienceControlMode::Throw:
			{
				CameraInterpStartPosition.Copy(SpringArm);
				CameraInterpEndPosition = bIsAiming ? ThrowAimCameraPosition : ThrowActionCameraPosition;
				PRINTNETLOG(this, TEXT("포지션 : %s"), (bIsAiming ? TEXT("ThrowAimCameraPosition") : TEXT("PhotoActionCameraPosition")));
				if (CameraTimeline)
				{
					CameraTimeline->PlayFromStart();
				}
				break;
			}
		default:
			{
				break;
			}
	}
}

#pragma endregion

#pragma region 모드 변수 할당 & 리플리케이션 처리

void AMVE_StageLevel_AudCharacter::OnInputActionSwitchAudienceModeStarted()
{
	RequestSetControlMode(EAudienceControlMode::WidgetSelection);
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
		
		// 선택한 메뉴 섹션이 없다면 취소
		if (Selection == -1)
		{
			return;
		}
		RequestSetControlMode(static_cast<EAudienceControlMode>(Selection));
	}
	else
	{
		// 예외 처리
		RequestSetControlMode(EAudienceControlMode::Default);
	}
}

void AMVE_StageLevel_AudCharacter::RequestSetControlMode(const EAudienceControlMode Value)
{
	// 네트워크 대역폭 관리
	if (CurrentControlMode == Value)
	{
		return;
	}
	
	// 서버를 거쳐서 리플리케이션을 통해 최종 업데이트
	Server_SetControlMode(Value);
}

void AMVE_StageLevel_AudCharacter::Server_SetControlMode_Implementation(const EAudienceControlMode Value)
{
	CurrentControlMode = Value;
	OnRep_ControlMode();
}

void AMVE_StageLevel_AudCharacter::OnRep_ControlMode()
{
	if (GetBindingPC() == nullptr)
	{
		return;
	}
	
	// 오디언스 오브젝트 세팅 람다 함수
	const auto SetAudObject = [this](AActor* Actor, const FString& Message)
	{
		if (const auto TargetAudObject = Cast<AMVE_StageLevel_AudObject>(Actor))
		{
			PRINTNETLOG(this, TEXT("%s"), *Message)
			
			// 비가시화
			if (CurrentAudObject && CurrentAudObject != TargetAudObject)
			{
				CurrentAudObject->SetIsVisible(false);
				CurrentAudObject = TargetAudObject;
			}
			
			// 가시화
			if (TargetAudObject)
			{
				CurrentAudObject = TargetAudObject;
				CurrentAudObject->SetIsVisible(true);
			}
		}
		else
		{
			PRINTNETLOG(this, TEXT("%s"), *Message)
			
			// 비가시화
			if (CurrentAudObject)
			{
				CurrentAudObject->SetIsVisible(false);
				CurrentAudObject = nullptr;
			}
		}
	};
	
	// 카메라 모드 람다 함수
	const auto CameraMode1 = [this]()
	{
		// 위젯 선택 중에는 캐릭터가 카메라를 따라 회전하지 않는다
		// 카메라 포지션 컨트롤도 존재하지 않는다
		bUseControllerRotationYaw = false;
		Server_SetUseControllerRotationYaw(false);
		
		// 화면 회전 잠금 설정
		bLockCamera = true;
	};
	const auto CameraMode2 = [this]()
	{
		// 이 모드들에서는 캐릭터가 컨트롤러의 Yaw 회전값을 따라야 한다
		// 목표를 보기에 적절한 위치로 카메라 포지션을 컨트롤한다 
		bUseControllerRotationYaw = true;
		Server_SetUseControllerRotationYaw(true);
		if (CameraTimeline)
		{
			CameraTimeline->PlayFromStart();
		}
		
		// 화면 회전 잠금 해제
		bLockCamera = false;
	};
	const auto CameraMode3 = [this]()
	{
		// 기본 상태에서는 캐릭터가 카메라를 따라 회전하지 않는다
		// 주변을 둘러보기 좋도록 카메라 포지션을 컨트롤한다
		bUseControllerRotationYaw = false;
		Server_SetUseControllerRotationYaw(false);
		if (CameraTimeline)
		{
			CameraTimeline->PlayFromStart();
		}
		
		// 화면 회전 잠금 해제
		bLockCamera = false;
	};
	
	switch (CurrentControlMode)
	{
	case EAudienceControlMode::WidgetSelection:
		{
			CameraMode1();
			GetBindingPC()->SwitchInputHelpWidget(EAudienceInputHelpState::Unvisible);
			break;
		}
	case EAudienceControlMode::Throw:
		{
			// 카메라 보간 위치
			CameraInterpStartPosition.Copy(SpringArm);
			CameraInterpEndPosition = ThrowActionCameraPosition;
			SetAudObject(nullptr, TEXT("오디언스 오브젝트 해제!"));
			CameraMode2();
			GetBindingPC()->SwitchInputHelpWidget(EAudienceInputHelpState::Aimable);
			break;
		}
	case EAudienceControlMode::Photo:
		{
			// 카메라 보간 위치
			CameraInterpStartPosition.Copy(SpringArm);
			CameraInterpEndPosition = PhotoActionCameraPosition;
			SetAudObject(AudCameraChildActorComp->GetChildActor(), TEXT("오디언스 카메라 준비 완료!"));
			CameraMode2();
			GetBindingPC()->SwitchInputHelpWidget(EAudienceInputHelpState::Aimable);
			break;
		}
	case EAudienceControlMode::Cheer:
		{
			// 카메라 보간 위치
			CameraInterpStartPosition.Copy(SpringArm);
			CameraInterpEndPosition = CheerActionCameraPosition;
			SetAudObject(nullptr, TEXT("오디언스 오브젝트 해제!"));
			CameraMode2();
			GetBindingPC()->SwitchInputHelpWidget(EAudienceInputHelpState::Executable);
			break;
		}
	case EAudienceControlMode::WaveLightStick:
		{
			// 카메라 보간 위치
			CameraInterpStartPosition.Copy(SpringArm);
			CameraInterpEndPosition = WaveLightStickActionCameraPosition;
			SetAudObject(nullptr, TEXT("오디언스 오브젝트 해제!"));
			CameraMode2();
			GetBindingPC()->SwitchInputHelpWidget(EAudienceInputHelpState::Executable);
			break;
		}
	case EAudienceControlMode::Clap:
		{
			// 카메라 보간 위치
			CameraInterpStartPosition.Copy(SpringArm);
			CameraInterpEndPosition = ClapActionCameraPosition;
			SetAudObject(nullptr, TEXT("오디언스 오브젝트 해제!"));
			CameraMode2();
			GetBindingPC()->SwitchInputHelpWidget(EAudienceInputHelpState::Executable);
			break;
		}
	case EAudienceControlMode::Default:
		{
			// 카메라 보간 위치
			CameraInterpStartPosition.Copy(SpringArm);
			CameraInterpEndPosition = DefaultCameraPosition;
			SetAudObject(nullptr, TEXT("오디언스 오브젝트 해제!"));
			CameraMode3();
			GetBindingPC()->SwitchInputHelpWidget(EAudienceInputHelpState::Selectable);
			break;
		}
	default:
		{
			SetAudObject(nullptr, TEXT("오디언스 오브젝트 해제!"));
			CameraMode3();
			break;
		}
	}
	PRINTNETLOG(this, TEXT("현재 모드 %s"), *StaticEnum<EAudienceControlMode>()->GetNameStringByValue(static_cast<int64>(CurrentControlMode)));
}

#pragma endregion 

#pragma region 액션 실행 여부 변수 할당 & 리플리케이션 처리

void AMVE_StageLevel_AudCharacter::Server_SetIsExecuting_Implementation(const bool Value)
{
	bIsExecuting = Value;
}

void AMVE_StageLevel_AudCharacter::Server_ActiveExecuteTimer_Implementation(const float& Time)
{
	// 연속 입력 방지
	if (bIsExecuting)
	{
		return;
	}
	
	bIsExecuting = true;
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindWeakLambda(this, [this]()
	{
		bIsExecuting = false;
		PRINTNETLOG(this, TEXT("실행 입력 액션 타이머 종료!"))
	});
	GetWorldTimerManager().SetTimer(ExecuteTimerHandle, TimerDelegate, Time, false);
}

void AMVE_StageLevel_AudCharacter::OnRep_IsExecuting() const
{
	PRINTNETLOG(this, TEXT("서버를 통해 리플리케이티드!"));
}

void AMVE_StageLevel_AudCharacter::RequestSetIsExecuting(const bool Value)
{
	// 네트워크 대역폭 관리
	if (bIsExecuting == Value)
	{
		return;
	}
	
	// 서버를 거쳐서 리플리케이션을 통해 최종 업데이트
	Server_SetIsExecuting(Value);
}

#pragma endregion

#pragma region 사진 액션 관련 함수 & RPC 구현

AMVE_StageLevel_AudCamera* AMVE_StageLevel_AudCharacter::GetAudCamera() const
{
	return Cast<AMVE_StageLevel_AudCamera>(AudCameraChildActorComp->GetChildActor());
}

void AMVE_StageLevel_AudCharacter::Server_ExecutePhoto_Implementation()
{
	// 조준 상태가 아니면 사진 입력 액션을 발동할 수 없다
	if (GetIsAiming() == false)
	{	
		return;
	}
	
	// 서버가 모든 클라이언트에 사진 액션 실행 요청
	Multicast_ExecutePhoto();
}

void AMVE_StageLevel_AudCharacter::Multicast_ExecutePhoto_Implementation()
{
	PRINTNETLOG(this, TEXT("카메라 멀티캐스트!"));
	
	// 카메라에 접근해서 이펙트 실행
	if (AMVE_StageLevel_AudCamera* AudCamera = GetAudCamera())
	{
		AudCamera->TakePhoto(GetController());	
	}
}

#pragma endregion

#pragma region 던지기 액션 관련 함수 & RPC 구현

void AMVE_StageLevel_AudCharacter::Server_ExecuteThrow_Implementation()
{
	// 조준 상태가 아니면 던지기 입력 액션을 발동할 수 없다
	if (GetIsAiming() == false)
	{	
		return;
	}
	
	// 서버가 모든 클라이언트에 애님 몽타주 재생 요청
	Multicast_ExecuteThrow();	
}

void AMVE_StageLevel_AudCharacter::Multicast_ExecuteThrow_Implementation()
{
	// 애니메이션 몽타주 재생
	PlayAnimMontage(AudThrowAnimMontage);	
}

void AMVE_StageLevel_AudCharacter::ThrowObject()
{
	if (HasAuthority() == false)
	{
		return;
	}
	
	if (ThrowObjectClass)
	{
		// 소켓을 통해 던지는 위치 획득
		GetMesh()->RefreshBoneTransforms();
		const FVector ThrowLocation = RightThrowArrowComp->GetComponentLocation();
		const FRotator ThrowRotation = RightThrowArrowComp->GetComponentRotation();
		
		// 투척
		const FVector ThrowDirection = ThrowRotation.Vector();
		DrawDebugPoint(GetWorld(), ThrowLocation, 10.f, FColor::Yellow, false, 3.0f);
		DrawDebugDirectionalArrow(GetWorld(), ThrowLocation, ThrowDirection * 100, 10, FColor::Red, false, 3.0f);
		
		// 스폰 조건
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
		if (AMVE_ThrowObject* SpawnedObject = GetWorld()->SpawnActor<AMVE_ThrowObject>(ThrowObjectClass, ThrowLocation, ThrowRotation, SpawnParams))
		{
			// 초기 속도 설정
			if (auto* ProjectileComp = SpawnedObject->FindComponentByClass<UProjectileMovementComponent>())
			{
				ProjectileComp->InitialSpeed = ThrowSpeed;
				ProjectileComp->MaxSpeed = ThrowSpeed;
			}
			
			// 위치 이동
			SpawnedObject->FireInDirection(ThrowDirection);
		}
	}
}

#pragma endregion

#pragma region 환호 액션 관련 함수 & RPC 구현

void AMVE_StageLevel_AudCharacter::Server_ExecuteCheerUp_Implementation()
{
	// 조건 확인
	
	Multicast_ExecuteCheerUp();
}

void AMVE_StageLevel_AudCharacter::Multicast_ExecuteCheerUp_Implementation()
{
	PlayAnimMontage(AudCheerUpAnimMontage);
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), AudCheerUpSound, GetActorLocation());
}

#pragma endregion 

#pragma region 응원 액션 관련 함수 & RPC 구현

void AMVE_StageLevel_AudCharacter::Server_ExecuteSwingLightStick_Implementation()
{
	// 조건 확인
	
	Multicast_ExecuteSwingLightStick();
}

void AMVE_StageLevel_AudCharacter::Multicast_ExecuteSwingLightStick_Implementation()
{
	PlayAnimMontage(AudWaveLightStickAnimMontage);
}

#pragma endregion

#pragma region 박수 액션 관련 함수 & RPC 구현

void AMVE_StageLevel_AudCharacter::Server_ExecuteClap_Implementation()
{
	//조건 확인
	
	Multicast_ExecuteClap();
}

void AMVE_StageLevel_AudCharacter::Multicast_ExecuteClap_Implementation()
{
	PlayAnimMontage(AudClapAnimMontage);
}

#pragma endregion

void AMVE_StageLevel_AudCharacter::OnInputActionExecuteStarted(const FInputActionValue& Value)
{
	// 연속 입력 방지
	if (bIsExecuting)
	{
		return;
	}
	
	// 실행 완료 후 재입력 딜레이
	constexpr float Delay = 0.2;
	
	// 현재 모드에 따라서 다른 도구, 애니메이션, 사운드 사용
	switch (CurrentControlMode)
	{
	case EAudienceControlMode::WidgetSelection:
		{
			PRINTNETLOG(this, TEXT("오디언스 인터렉션 모드 선택 중..."))
			break;
		}
	case EAudienceControlMode::Throw:
		{
			if (GetIsAiming() == false)
			{
				return;
			}
			PRINTNETLOG(this, TEXT("던지기 입력 액션!"))
			if (AudThrowAnimMontage)
			{
				const float Length = AudThrowAnimMontage->GetPlayLength() / AudThrowAnimMontage->RateScale;
				Server_ActiveExecuteTimer(Length + Delay);
				Server_ExecuteThrow();
			}
			break;
		}
	case EAudienceControlMode::Photo:
		{
			if (GetIsAiming() == false)
			{
				return;
			}
			PRINTNETLOG(this, TEXT("카메라 입력 액션!"))
			Server_ActiveExecuteTimer(0.2f);
			Server_ExecutePhoto();
			break;
		}
	case EAudienceControlMode::Cheer:
		{
			PRINTNETLOG(this, TEXT("환호 입력 액션!"));
			if (AudCheerUpAnimMontage)
			{
				const float Length = AudCheerUpAnimMontage->GetPlayLength() / AudCheerUpAnimMontage->RateScale;
				Server_ActiveExecuteTimer(Length + Delay);
				Server_ExecuteCheerUp();
			}
			break;
		}
	case EAudienceControlMode::WaveLightStick:
		{
			PRINTNETLOG(this, TEXT("응원 입력 액션"));
			if (AudWaveLightStickAnimMontage)
			{
				const float Length = AudWaveLightStickAnimMontage->GetPlayLength() / AudWaveLightStickAnimMontage->RateScale;
				Server_ActiveExecuteTimer(Length + Delay);
				Server_ExecuteSwingLightStick();
			}
			break;
		}
	case EAudienceControlMode::Clap:
		{
			PRINTNETLOG(this, TEXT("박수 입력 액션"));
			if (AudClapAnimMontage)
			{
				const float Length = AudClapAnimMontage->GetPlayLength() / AudClapAnimMontage->RateScale;
				Server_ActiveExecuteTimer(Length + Delay);
				Server_ExecuteClap();
			}
			break;
		}
	case EAudienceControlMode::Default:
	default:
		{
			break;
		}
	}
}

void AMVE_StageLevel_AudCharacter::OnInputActionLookTriggered(const FInputActionValue& Value)
{
	if (Controller == nullptr)
	{
		return;
	}
	
	if (bLockCamera)
	{
		return;
	}
	
	// Value에서 Vector2D 형태로 값을 추출
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void AMVE_StageLevel_AudCharacter::UpdateCameraTimeline(const float Alpha) const
{
	{
		// 스프링 암 길이 보간
		const float Start = CameraInterpStartPosition.SpringArmTargetArmLength;
		const float End = CameraInterpEndPosition.SpringArmTargetArmLength;
		SpringArm->TargetArmLength = FMath::Lerp(Start, End, Alpha);
	}
	
	{
		// 카메라 상대 위치 보간
		const FVector Start = CameraInterpStartPosition.SpringArmRelativeLocation;
		const FVector End = CameraInterpEndPosition.SpringArmRelativeLocation;
		SpringArm->SetRelativeLocation(FMath::Lerp(Start, End, Alpha));
	}
}

void AMVE_StageLevel_AudCharacter::Server_SetUseControllerRotationYaw_Implementation(const bool Value)
{
	bUseControllerRotationYaw = Value;
}


// 움직임
void AMVE_StageLevel_AudCharacter::HandleMove(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMVE_StageLevel_AudCharacter::HandleLook(const FInputActionValue& Value)
{
	if (Controller == nullptr || bLockCamera)
	{
		return;
	}
	
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxisVector.X * LookSensitivity);
	AddControllerPitchInput(LookAxisVector.Y * LookSensitivity);
}

void AMVE_StageLevel_AudCharacter::HandleToggleViewpoint(const FInputActionValue& Value)
{
	bIsFirstPerson = !bIsFirstPerson;

	if (bIsFirstPerson)
	{
		TargetArmLength = FirstPersonArmLength;
		bUseControllerRotationYaw = true;
		Server_SetUseControllerRotationYaw(true);
		
		if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
		{
			MovementComp->bOrientRotationToMovement = false;
		}
		
		if (USkeletalMeshComponent* MeshComp = GetMesh())
		{
			MeshComp->SetOwnerNoSee(true);
		}
		
		PRINTNETLOG(this, TEXT("1인칭 시점으로 전환"));
	}
	else
	{
		TargetArmLength = ZoomTargetArmLength;
		bUseControllerRotationYaw = false;
		Server_SetUseControllerRotationYaw(false);
		
		if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
		{
			MovementComp->bOrientRotationToMovement = true;
		}
		
		if (USkeletalMeshComponent* MeshComp = GetMesh())
		{
			MeshComp->SetOwnerNoSee(false);
		}
		
		PRINTNETLOG(this, TEXT("3인칭 시점으로 전환"));
	}

	// 보간 타이머 시작 (람다 사용)
	GetWorldTimerManager().SetTimer(
		ViewpointInterpTimerHandle,
		FTimerDelegate::CreateLambda([this]()
		{
			UpdateCameraInterpolation(0.016f);
		}),
		0.016f,
		true
	);
}

void AMVE_StageLevel_AudCharacter::HandleZoom(const FInputActionValue& Value)
{
	const float ZoomValue = Value.Get<float>();

	if (bIsFirstPerson)
	{
		//fov 만 조절
		TargetFOV = FMath::Clamp(
			TargetFOV - (ZoomValue * FOVZoomStep),
			ZoomFOVMin,
			ZoomFOVMax
		);
	}
	else
	{
		// 스프링암 있으니 스프링암 조절
		ZoomTargetArmLength = FMath::Clamp(
			ZoomTargetArmLength - (ZoomValue * ZoomStep),
			ZoomMin,
			ZoomMax
		);
		TargetArmLength = ZoomTargetArmLength;
	}
	
	// 보간 tick 안쓸려고
	if (!GetWorldTimerManager().IsTimerActive(ViewpointInterpTimerHandle))
	{
		GetWorldTimerManager().SetTimer(
			ViewpointInterpTimerHandle,
			FTimerDelegate::CreateLambda([this]()
			{
				UpdateCameraInterpolation(0.016f);
			}),
			0.016f,
			true
		);
	}
}

void AMVE_StageLevel_AudCharacter::UpdateCameraInterpolation(float DeltaTime)
{
	bool bStillInterpolating = false;

	// 스플링 암 보간
	if (SpringArm && FMath::Abs(SpringArm->TargetArmLength - TargetArmLength) > 0.1f)
	{
		const float InterpSpeed = bIsFirstPerson ? ViewpointInterpSpeed : ZoomInterpSpeed;
		SpringArm->TargetArmLength = FMath::FInterpTo(
			SpringArm->TargetArmLength,
			TargetArmLength,
			DeltaTime,
			InterpSpeed
		);
		bStillInterpolating = true;
	}

	// fove 보간
	if (Camera && FMath::Abs(Camera->FieldOfView - TargetFOV) > 0.1f)
	{
		Camera->FieldOfView = FMath::FInterpTo(
			Camera->FieldOfView,
			TargetFOV,
			DeltaTime,
			ZoomInterpSpeed
		);
		bStillInterpolating = true;
	}

	// 보간끝나면 타이머 종료
	if (!bStillInterpolating)
	{
		GetWorldTimerManager().ClearTimer(ViewpointInterpTimerHandle);
	}
}