#pragma once

#include "CoreMinimal.h"
#include "MVE_PC_StageLevel.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "MVE_StageLevel_AudCharacter.generated.h"

class AMVE_StageLevel_AudObject;
class AMVE_StageLevel_AudCamera;
struct FInputActionValue;
class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class UAudioComponent;
class UMVE_StageLevel_AudCharacterShooterComponent;
class AMVE_PC_StageLevel;
class UCurveFloat;


UENUM(BlueprintType)
enum class EAudienceControlMode : uint8
{
 	// 기본 상태. 카메라만 자유롭게 움직임
 	Default = 0 UMETA(DisplayName="기본"),
	// 조준 상태. 캐릭터가 카메라 방향을 따라 회전, 에임 오프셋 사용                  
	Throw = 1 UMETA(DisplayName="던지기"),
	// 사진 상태. 캐릭터가 카메라 방향을 따라 회전, 에임 오프셋 사용
	Photo = 2 UMETA(DisplayName="사진"),
	// 환호 상태. 캐릭터가 카메라 방향을 따라 회전
	Cheer = 3 UMETA(DisplayName="환호"),
	// 응원 상태. 캐릭터가 카메라 방향을 따라 회전
	WaveLightStick = 4 UMETA(DisplayName="응원"),
	// 박수 상태. 캐릭터가 카메라 방향을 따라 회전
	Clap = 5 UMETA(DisplayName="박수"),
	// 모드 선택 위젯이 활성화된 상태. 모든 입력이 중지됨
	WidgetSelection = 6 UMETA(DisplayName="위젯 선택"),
};

USTRUCT(BlueprintType)
struct FMVE_StageLevel_AudCharacterCameraPosition
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float SpringArmTargetArmLength;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FVector SpringArmRelativeLocation;
	
	// 기본 생성자
	FMVE_StageLevel_AudCharacterCameraPosition() : 
		SpringArmTargetArmLength(0.0f), 
		SpringArmRelativeLocation(FVector::ZeroVector) { }
	
	// 초기화 생성자
	FMVE_StageLevel_AudCharacterCameraPosition(const float& InTargetArmLength, const FVector& InRelativeLocation) : 
		SpringArmTargetArmLength(InTargetArmLength),
		SpringArmRelativeLocation(InRelativeLocation) { }
	
	// 복사 함수
	void Copy(const USpringArmComponent* Other)
	{
		SpringArmTargetArmLength = Other->TargetArmLength;
		SpringArmRelativeLocation = Other->GetRelativeLocation();
	};
};

UCLASS()
class MVE_API AMVE_StageLevel_AudCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	AMVE_StageLevel_AudCharacter();
	
	virtual void BeginPlay() override;
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	UAudioComponent* GetAudioComponent() const { return AudioComponent; }
	
	UFUNCTION()
	UMVE_StageLevel_AudCharacterShooterComponent* GetShooterComponent() const { return ShooterComponent; }
	
	UFUNCTION() 
	EAudienceControlMode GetControlMode() const { return CurrentControlMode; }
	
	UFUNCTION()
	FORCEINLINE bool GetIsAiming() const { return bIsAiming; }
	
	UFUNCTION()
	FORCEINLINE bool GetIsExecuting() const { return bIsExecuting; }
	
	UFUNCTION()
	AMVE_StageLevel_AudCamera* GetAudCamera() const;
	
#pragma region 던지기 액션 관련 함수 & RPC 선언

	UPROPERTY(EditDefaultsOnly, Category="Throw")
	TSubclassOf<class AMVE_ThrowObject> ThrowObjectClass;

	UPROPERTY(EditDefaultsOnly, Category="Throw")
	float ThrowSpeed = 1000.0f;
	
	UFUNCTION(Server, Reliable)
	void Server_ExecuteThrow();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ExecuteThrow();

	UFUNCTION()
	void ThrowObject();
	
#pragma endregion
	
#pragma region 사진 액션 관련 함수 & RPC 선언
	
	UFUNCTION(Server, Reliable)
	void Server_ExecutePhoto();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ExecutePhoto();
	
#pragma endregion
	
#pragma region 환호 액션 관련 함수 & RPC 선언
	
	UFUNCTION(Server, Reliable)
	void Server_ExecuteCheerUp();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ExecuteCheerUp();
	
#pragma endregion
	
#pragma region 응원 액션 관련 함수 & RPC 선언
	
	UFUNCTION(Server, Reliable)
	void Server_ExecuteSwingLightStick();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ExecuteSwingLightStick();
	
#pragma endregion
	
#pragma region 박수 액션 관련 함수 & RPC 선언
	
	UFUNCTION(Server, Reliable)
	void Server_ExecuteClap();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ExecuteClap();
	
#pragma endregion 

private:
	// 이 캐릭터와 바인딩된 플레이어 컨트롤러
	UFUNCTION(Category = "MVE|Control")
	AMVE_PC_StageLevel* GetBindingPC() const { return Cast<AMVE_PC_StageLevel>(GetWorld()->GetFirstPlayerController()); }
	
	// 오디언스 상호작용 관련 필드
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MVE|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAudioComponent> AudioComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MVE|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMVE_StageLevel_AudCharacterShooterComponent> ShooterComponent;
	
#pragma region 사용자 입력 액션 콜백
	
	// IA_Look : 화면 전환
	UFUNCTION()
	void OnInputActionLookTriggered(const FInputActionValue& Value);
	// IA_Aim : 조준 시작
	UFUNCTION()
	void OnInputActionAimStarted(const FInputActionValue& Value);
	// IA_Aim : 조준 종료
	UFUNCTION()
	void OnInputActionAimCompleted(const FInputActionValue& Value);
	// IA_Execute : 실행 개시
	UFUNCTION()
	void OnInputActionExecuteStarted(const FInputActionValue& Value);
	// IA_SwitchAudienceMode : 래디얼 메뉴 표시
	UFUNCTION()
	void OnInputActionSwitchAudienceModeStarted();
	// IA_SwitchAudienceMode : 래디얼 메뉴 선택 확정
	void OnInputActionSwitchAudienceModeCompleted();
	// 카메라 전환 타임라인의 업데이트
	UFUNCTION()
	void UpdateCameraTimeline(float Alpha) const;
	
#pragma endregion 
	
#pragma region 조준 입력 변수 할당 & 리플리케이션 처리
	
	UPROPERTY(VisibleAnywhere, Category = "MVE|Control", ReplicatedUsing=OnRep_IsAiming)
	bool bIsAiming;	
	UFUNCTION(Server, Reliable)
	void Server_SetIsAiming(const bool Value);
	UFUNCTION()
	void OnRep_IsAiming();
	UFUNCTION()
	void RequestSetIsAiming(const bool Value);
	
#pragma endregion
	
#pragma region 모드 변수 할당 & 리플리케이션 처리
	
	UPROPERTY(VisibleAnywhere, Category = "MVE|Control", ReplicatedUsing=OnRep_ControlMode)
	EAudienceControlMode CurrentControlMode;
	UFUNCTION(Server, Reliable)
	void Server_SetControlMode(const EAudienceControlMode Value);
	UFUNCTION()
	void OnRep_ControlMode();
	UFUNCTION()
	void RequestSetControlMode(const EAudienceControlMode Value);
	
#pragma endregion
	
#pragma region 액션 실행 변수 할당 & 리플리케이션 처리
	
	FTimerHandle ExecuteTimerHandle;
	UPROPERTY(VisibleAnywhere, Category = "MVE|Control", ReplicatedUsing=OnRep_IsExecuting)
	bool bIsExecuting;
	UFUNCTION(Server, Reliable)
	void Server_ActiveExecuteTimer(const float& Time);
	
	
	UFUNCTION(Server, Reliable)
	void Server_SetIsExecuting(const bool Value);
	UFUNCTION()
	void OnRep_IsExecuting() const;
	UFUNCTION()
	void RequestSetIsExecuting(const bool Value);
	
#pragma endregion 
	
	UFUNCTION(Server, Reliable, Category = "MVE|Control")
	void Server_SetUseControllerRotationYaw(const bool Value);
	
#pragma region  입력 매핑 컨텍스트와 입력 액션
	
	UPROPERTY(EditAnywhere, Category = "MVE|Control", meta = (AllowPrivateAccess = "true"))           
	TObjectPtr<UInputMappingContext> IMC_AudCharacter;
	UPROPERTY(EditAnywhere, Category = "MVE|Control", meta = (AllowPrivateAccess = "true"))           
	TObjectPtr<UInputAction> IA_Move;
	UPROPERTY(EditAnywhere, Category = "MVE|Control", meta = (AllowPrivateAccess = "true"))           
	TObjectPtr<UInputAction> IA_Jump;
	UPROPERTY(EditAnywhere, Category = "MVE|Control", meta = (AllowPrivateAccess = "true"))           
	TObjectPtr<UInputAction> IA_SwitchAudienceMode;
	UPROPERTY(EditAnywhere, Category = "MVE|Control", meta = (AllowPrivateAccess = "true"))           
	TObjectPtr<UInputAction> IA_Aim;
	UPROPERTY(EditAnywhere, Category = "MVE|Control", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_Execute;
	UPROPERTY(EditAnywhere, Category = "MVE|Control", meta = (AllowPrivateAccess = "true"))      
	TObjectPtr<UInputAction> IA_Look;
	
#pragma endregion 

#pragma region 상호작용 모드 & 특수 상태 별 카메라 컨트롤 관련
	
	// 카메라 관련 필드
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MVE|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MVE|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MVE|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTimelineComponent> CameraTimeline;
	UPROPERTY(EditAnywhere, Category = "MVE|Control", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCurveFloat> CameraTransitionCurve;
	UPROPERTY(VisibleAnywhere, Category = "MVE|Control") 
	bool bLockCamera;
	
	// 상호작용 모드 별 카메라 위치 보간 기준
	UPROPERTY(VisibleAnywhere, Category = "MVE|Control", meta = (AllowPrivateAccess = "true"))
	FMVE_StageLevel_AudCharacterCameraPosition CameraInterpStartPosition;
	UPROPERTY(VisibleAnywhere, Category = "MVE|Control", meta = (AllowPrivateAccess = "true"))
	FMVE_StageLevel_AudCharacterCameraPosition CameraInterpEndPosition;
	
	// 상호작용 모드 별 카메라 위치
	UPROPERTY(VisibleAnywhere, Category = "MVE|Control", meta = (AllowPrivateAccess = "true"))
	FMVE_StageLevel_AudCharacterCameraPosition DefaultCameraPosition;
	UPROPERTY(VisibleAnywhere, Category = "MVE|Control", meta = (AllowPrivateAccess = "true"))
	FMVE_StageLevel_AudCharacterCameraPosition PhotoActionCameraPosition;
	UPROPERTY(VisibleAnywhere, Category = "MVE|Control", meta = (AllowPrivateAccess = "true"))
	FMVE_StageLevel_AudCharacterCameraPosition ThrowActionCameraPosition;
	UPROPERTY(VisibleAnywhere, Category = "MVE|Control", meta = (AllowPrivateAccess = "true"))
	FMVE_StageLevel_AudCharacterCameraPosition CheerActionCameraPosition;
	UPROPERTY(VisibleAnywhere, Category = "MVE|Control", meta = (AllowPrivateAccess = "true"))
	FMVE_StageLevel_AudCharacterCameraPosition WaveLightStickActionCameraPosition;
	UPROPERTY(VisibleAnywhere, Category = "MVE|Control", meta = (AllowPrivateAccess = "true"))
	FMVE_StageLevel_AudCharacterCameraPosition ClapActionCameraPosition;
	
	// 특수 상태 별 카메라 위치
	UPROPERTY(VisibleAnywhere, Category = "MVE|Control", meta = (AllowPrivateAccess = "true"))
	FMVE_StageLevel_AudCharacterCameraPosition PhotoAimCameraPosition;
	UPROPERTY(VisibleAnywhere, Category = "MVE|Control", meta = (AllowPrivateAccess = "true"))
	FMVE_StageLevel_AudCharacterCameraPosition ThrowAimCameraPosition;
	
#pragma endregion
	
	UPROPERTY(VisibleAnywhere, Category = "MVE|Objects", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AMVE_StageLevel_AudObject> CurrentAudObject;
	UPROPERTY(VisibleAnywhere, Category = "MVE|Objects", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UChildActorComponent> AudCameraChildActorComp;
	UPROPERTY(VisibleAnywhere, Category = "MVE|Objects", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UChildActorComponent> AudLightStickChildActorComp;
	UPROPERTY(EditDefaultsOnly, Category = "MVE|Objects", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UArrowComponent> RightThrowArrowComp;

#pragma region 상호작용 애니메이션 & VFX & SFX
	
	UPROPERTY(EditAnywhere, Category = "MVE|Interactions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> AudThrowAnimMontage;
	UPROPERTY(EditAnywhere, Category = "MVE|Interactions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> AudCheerUpAnimMontage;
	UPROPERTY(EditAnywhere, Category = "MVE|Interactions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> AudWaveLightStickAnimMontage;
	UPROPERTY(EditAnywhere, Category = "MVE|Interactions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> AudClapAnimMontage;
	UPROPERTY(EditAnywhere, Category = "MVE|Interactions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> AudCheerUpSound;
	
#pragma endregion

public:
	// Enhanced input

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_ToggleViewpoint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Zoom;
	
	// 카메라 셋팅
	UPROPERTY(EditDefaultsOnly, Category = "MVE|Viewpoint")
	float ThirdPersonArmLength = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "MVE|Viewpoint")
	float FirstPersonArmLength = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "MVE|Viewpoint")
	float ViewpointInterpSpeed = 8.f;

	UPROPERTY(EditDefaultsOnly, Category = "MVE|Zoom")
	float ZoomMin = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "MVE|Zoom")
	float ZoomMax = 800.f;

	UPROPERTY(EditDefaultsOnly, Category = "MVE|Zoom")
	float ZoomStep = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = "MVE|Zoom")
	float ZoomInterpSpeed = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "MVE|Look")
	float LookSensitivity = 1.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "MVE|Zoom")
	float DefaultFOV = 90.f;

	UPROPERTY(EditDefaultsOnly, Category = "MVE|Zoom")
	float ZoomFOVMin = 30.f; 
	
	// 기본시야
	UPROPERTY(EditDefaultsOnly, Category = "MVE|Zoom")
	float ZoomFOVMax = 90.f;

	UPROPERTY(EditDefaultsOnly, Category = "MVE|Zoom")
	float FOVZoomStep = 5.f;
	
	// 런타임 
	float TargetFOV = 90.f;
	bool bIsFirstPerson = false;
	float TargetArmLength = 300.f;
	float ZoomTargetArmLength = 300.f;

	// input 핸들러
	void HandleMove(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void HandleToggleViewpoint(const FInputActionValue& Value);
	void HandleZoom(const FInputActionValue& Value);
	void UpdateCameraInterpolation(float DeltaTime);
	
	FTimerHandle ViewpointInterpTimerHandle;

};