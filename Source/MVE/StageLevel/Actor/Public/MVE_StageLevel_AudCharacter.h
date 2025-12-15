#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "MVE_StageLevel_AudCharacter.generated.h"

class UInputMappingContext;
struct FInputActionValue;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;
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
	Aiming = 1 UMETA(DisplayName="조준"),
	// 사진 촬영 상태. 조준 상태와 동일
	Photo = 2 UMETA(DisplayName="사진"),
	// 환호 상태. 캐릭터가 카메라 방향을 따라 회전
	Cheer = 3 UMETA(DisplayName="환호"),
	// 응원봉 흔들기 상태. 환호 상태와 동일
	WaveLightStick = 4 UMETA(DisplayName="응원"),
	// 모드 선택 위젯이 활성화된 상태. 모든 입력이 중지됨
	WidgetSelection = 5 UMETA(DisplayName="위젯 선택"),
};

UCLASS()
class MVE_API AMVE_StageLevel_AudCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	AMVE_StageLevel_AudCharacter();
	
	virtual void BeginPlay() override;
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION()
	void SetControlMode(const EAudienceControlMode NewMode);
	
	UPROPERTY()
	bool bAimEnabled;
	
	UFUNCTION()
	UAudioComponent* GetAudioComponent() const { return AudioComponent; }
	
	UFUNCTION()
	UMVE_StageLevel_AudCharacterShooterComponent* GetShooterComponent() const { return ShooterComponent; }
protected:
	// IA_Look : 화면 전환
	UFUNCTION()
	void OnInputActionLookTriggered(const FInputActionValue& Value);
	
	// IA_Aim : 조준 시작
	UFUNCTION()
	void OnInputActionAimStarted(const FInputActionValue& Value);
	
	// IA_Aim : 조준 종료
	UFUNCTION()
	void OnInputActionAimCompleted(const FInputActionValue& Value);
	
	// IA_SwitchAudienceMode : 래디얼 메뉴 표시
	UFUNCTION()
	void OnInputActionSwitchAudienceModeStarted();
	
	// IA_SwitchAudienceMode : 래디얼 메뉴 선택 확정
	void OnInputActionSwitchAudienceModeCompleted();
	
	// 카메라 전환 타임라인의 업데이트
	UFUNCTION()
	void UpdateCameraTimeline(float Alpha) const;
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArm;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> Camera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAudioComponent> AudioComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMVE_StageLevel_AudCharacterShooterComponent> ShooterComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTimelineComponent> CameraTimeline;
	
	UPROPERTY(EditAnywhere, Category = "Timeline", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCurveFloat> CameraTransitionCurve;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Control", meta = (AllowPrivateAccess = "true"))
	EAudienceControlMode CurrentControlMode;
	
	UPROPERTY(EditAnywhere, Category = "Control", meta = (AllowPrivateAccess = "true"))           
	TObjectPtr<UInputMappingContext> IMC_AudCharacter;
	                                                                                              
	UPROPERTY(EditAnywhere, Category = "Control", meta = (AllowPrivateAccess = "true"))           
	TObjectPtr<UInputAction> IA_Move;
	                                                                                              
	UPROPERTY(EditAnywhere, Category = "Control", meta = (AllowPrivateAccess = "true"))           
	TObjectPtr<UInputAction> IA_Jump;
	                                                                                              
	UPROPERTY(EditAnywhere, Category = "Control", meta = (AllowPrivateAccess = "true"))           
	TObjectPtr<UInputAction> IA_SwitchAudienceMode;
	                                                                                              
	UPROPERTY(EditAnywhere, Category = "Control", meta = (AllowPrivateAccess = "true"))           
	TObjectPtr<UInputAction> IA_Aim;
	                                                                                              
	UPROPERTY(EditAnywhere, Category = "Control", meta = (AllowPrivateAccess = "true"))      
	TObjectPtr<UInputAction> IA_Look;
};