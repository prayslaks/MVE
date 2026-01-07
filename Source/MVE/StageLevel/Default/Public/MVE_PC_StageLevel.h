#pragma once

#include "CoreMinimal.h"
#include "MVE_API_ResponseData.h"
#include "GameFramework/PlayerController.h"
#include "MVE_PC_StageLevel.generated.h"

enum class EAudienceInputHelpState : uint8;
class UMVE_WC_StageLevel_AudInputHelp;
class UMVE_StageLevel_WidgetController_Chat;
struct FPlayerAccessoryInfo;
struct FCustomizationData;
class AMVE_StageLevel_AudCharacter;
class UMVE_WC_StageLevel_AudRadialMenu;
class UMVE_WC_Chat;
class UTimelineComponent;
class UCurveFloat;

DECLARE_DELEGATE(FOnSetUserInfoFinished);

UCLASS()
class MVE_API AMVE_PC_StageLevel : public APlayerController
{
	GENERATED_BODY()

public:
	AMVE_PC_StageLevel();

	/** StudioComponent 등에서 오디오 로딩이 완료되었을 때 호출하여 서버에 준비 상태를 알립니다. */
	void NotifyAudioReady();
	
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	
	UFUNCTION()
	AMVE_StageLevel_AudCharacter* GetBindingAudCharacter() const;

	// 플래시 포스트 프로세스 효과를 클라이언트에서 트리거합니다.
	UFUNCTION(Client, Reliable)
	void Client_TriggerFlashPostProcess();
	
	// 서버에 액세서리 정보 등록 (배열로 한 번에 전송)
	UFUNCTION(Server, Reliable)
	void ServerRPC_RegisterAccessory(const FString& UserID, const TArray<FCustomizationData>& CustomizationDataArray);
	
	// 신규 입장 시 기존 참여자들의 액세서리 정보 받기
	UFUNCTION(Client, Reliable)
	void ClientRPC_ReceiveExistingAccessories(const TArray<FPlayerAccessoryInfo>& ExistingAccessories);
	
	// 래디얼 메뉴의 표시 여부를 토글하고 마우스 커서 및 입력 모드를 설정한다
	void ToggleRadialMenu(const bool bShow);

	// 래디얼 메뉴에서 현재 선택된 섹터의 인덱스를 반환한다
	int32 GetRadialMenuSelection() const;
	
	void SwitchInputHelpWidget(const EAudienceInputHelpState NewState) const;
protected:
	// 플래시 효과를 일으키는 포스트 프로세스 머터리얼의 베이스
	UPROPERTY(EditDefaultsOnly, Category = "MVE|Flash Effect")
	TObjectPtr<UMaterialInterface> FlashPostProcessMaterialBase;

	// 플래시 효과를 일으키는 포스트 프로세스 MID 패러미터에 개입하는 커브 플로트
	UPROPERTY(EditDefaultsOnly, Category = "MVE|Flash Effect")
	TObjectPtr<UCurveFloat> FlashPostProcessCurve;
	
	// 플래시 효과를 일으키는 포스트 프로세스 머터리얼 베이스로부터 얻은 MID
	UPROPERTY(VisibleAnywhere, Category = "MVE|Flash Effect")
	TObjectPtr<UMaterialInstanceDynamic> FlashMID;

private:
	// 플래시 효과를 일으키는 포스트 프로세스 MID 패러미터에 개입하는 타임라인 컴포넌트
	UPROPERTY()
	TObjectPtr<UTimelineComponent> FlashPostProcessTimelineComp;

	// 플래시 효과를 일으키는 타임라인 컴포넌트 업데이트 콜백
	UFUNCTION()
	void OnFlashPostProcessUpdate(float Value) const;
	
protected:
	// 호스트용 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MVE|UI")
	TSubclassOf<UUserWidget> HostStandardMenuWidgetClass;
	
	// 호스트 위젯
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MVE|UI")
	TObjectPtr<UUserWidget> HostStandardMenuWidget;
	
	// 클라이언트용 원형 메뉴 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MVE|UI")
	TSubclassOf<UMVE_WC_StageLevel_AudRadialMenu> AudRadialMenuWidgetClass;
	
	// 클라이언트 원형 위젯
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MVE|UI")
	TObjectPtr<UMVE_WC_StageLevel_AudRadialMenu> AudRadialMenuWidget;
	
	// 클라이언트용 입력 헬프 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MVE|UI")
	TSubclassOf<UMVE_WC_StageLevel_AudInputHelp> AudInputHelpWidgetClass;
	
	// 클라이언트 입력 헬프 위젯
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MVE|UI")
	TObjectPtr<UMVE_WC_StageLevel_AudInputHelp> AudInputHelpWidget;
	
	// 위젯들을 생성만 하고 뷰포트에는 추가하지 않는다
	void CreateWidgets();
	
	// 호스트인지 확인하는 헬퍼 함수
	bool IsHost() const;

private:
	// 마우스 커서를 뷰포트 중앙으로 이동시킵니다.
	void CenterMouseCursor();


public:
	// 오디언스 역할을 수행하는 플레이어의 상호작용을 처리하는 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MVE|Component")
	TObjectPtr<class UMVE_PC_StageLevel_AudienceComponent> AudComponent;

	// 스튜디오 역할을 수행하는 플레이어의 기능을 처리하는 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MVE|Component")
	TObjectPtr<class UMVE_PC_StageLevel_StudioComponent> StdComponent;

	void SetUserInfo(bool bSuccess, const FProfileResponseData& Data, const FString& ErrorCode);
	
	FString GetUserEmail() { return UserEmail; }
	FString GetUserName() { return UserName; }

	

public:
	/** 캐릭터에 부착된 오디오 컴포넌트를 가져옵니다. */
	UFUNCTION(BlueprintCallable, Category = "MVE|Component")
	UAudioComponent* GetAudioComponent() const;

public:
	/** 채팅 메시지 전송 (Server RPC) */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSendChatMessage(const FString& MessageContent, const FGuid& ClientMessageID);

	/** 서버에 PlayerState 이름 설정 (Server RPC) */
	UFUNCTION(Server, Reliable)
	void ServerSetPlayerName(const FString& InPlayerName);

private:
	void SetupChatUI(UMVE_WC_Chat* InWidget);

	UPROPERTY()
	TObjectPtr<UMVE_StageLevel_WidgetController_Chat> ChatController;

	UPROPERTY()
	TObjectPtr<UMVE_WC_Chat> ChatWidget;
	
	FString UserEmail;
	FString UserName;


	void Initialize();

	// 서버에서 액세서리 & 던지기 메시 프리셋 로드
	void LoadCustomizationPresets();

	FOnSetUserInfoFinished OnSetUserInfoFinished;

};
