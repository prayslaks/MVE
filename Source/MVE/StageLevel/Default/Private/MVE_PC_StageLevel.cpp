#include "StageLevel/Default/Public/MVE_PC_StageLevel.h"

#include "Components/TimelineComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "MVE.h"
#include "MVE_AUD_WC_InteractionPanel.h"
#include "MVE_StageLevel_WidgetController_Chat.h"
#include "MVE_STU_WC_ConcertStudioPanel.h"
#include "MVE_WC_Chat.h"
#include "UIManagerSubsystem.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/PostProcessComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "StageLevel/Actor/Public/MVE_StageLevel_AudCharacter.h"
#include "StageLevel/Default/Public/MVE_PC_StageLevel_AudienceComponent.h"
#include "StageLevel/Default/Public/MVE_PC_StageLevel_StudioComponent.h"
#include "StageLevel/Widget/Public/MVE_WC_StageLevel_AudRadialMenu.h"
#include "StageLevel/Default/Public/MVE_StageLevel_ChatManager.h"
#include "EngineUtils.h"
#include "MVE_API_Helper.h"
#include "MVE_AUD_CustomizationManager.h"
#include "MVE_GM_StageLevel.h"
#include "MVE_WC_StageLevel_AudInputHelp.h"
#include "GameFramework/PlayerState.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "StageLevel/Default/Public/MVE_PS_StageLevel.h"

class UMVE_AUD_CustomizationManager;

AMVE_PC_StageLevel::AMVE_PC_StageLevel()
{
	// 관객(Audience) 역할을 수행하는 플레이어의 상호작용을 처리하는 컴포넌트입니다.
	AudComponent = CreateDefaultSubobject<UMVE_PC_StageLevel_AudienceComponent>(TEXT("AudComponent"));

	// 스튜디오(Studio) 역할을 수행하는 플레이어의 기능을 처리하는 컴포넌트입니다.
	StdComponent = CreateDefaultSubobject<UMVE_PC_StageLevel_StudioComponent>(TEXT("StdComponent"));

	// 플래시 효과를 위한 타임라인 컴포넌트 생성
	FlashPostProcessTimelineComp = CreateDefaultSubobject<UTimelineComponent>(TEXT("FlashPostProcessTimelineComp"));
}

void AMVE_PC_StageLevel::BeginPlay()
{
	Super::BeginPlay();

	// 로컬 플레이어 컨트롤러
	if (!IsLocalController()) return;

	// --- 플래시 포스트 프로세스 효과 초기화 ---
	if (FlashPostProcessCurve && FlashPostProcessMaterialBase)
	{
		// 1. 타임라인 콜백 바인딩
		FOnTimelineFloat FlashUpdateCallback;
		FlashUpdateCallback.BindUFunction(this, FName("OnFlashPostProcessUpdate"));
		FlashPostProcessTimelineComp->AddInterpFloat(FlashPostProcessCurve, FlashUpdateCallback);

		// 2. 동적 머티리얼 인스턴스(MID) 생성
		FlashMID = UMaterialInstanceDynamic::Create(FlashPostProcessMaterialBase, this);

		// 3. 포스트 프로세스 볼륨 찾아서 MID 적용
		bool bVolumeFound = false;
		for (TActorIterator<APostProcessVolume> It(GetWorld()); It; ++It)
		{
			if (APostProcessVolume* PPVolume = *It)
			{
				if (PPVolume->ActorHasTag(FName("FlashPPVolume")))
				{
					PPVolume->Settings.AddBlendable(FlashMID, 1.0f);
					bVolumeFound = true;
					PRINTLOG(TEXT("Flash Post Process Volume 찾음, 동적 머티리얼 적용 완료."));
					break;
				}
			}
		}
		if (!bVolumeFound)
		{
			PRINTLOG(TEXT("경고: 'FlashPPVolume' 태그를 가진 PostProcessVolume을 찾을 수 없습니다."));
		}
	}
	else
	{
		PRINTERROR(TEXT("경고: FlashPostProcessCurve 또는 FlashPostProcessMaterialBase가 설정되지 않았습니다."));
	}
	// ------------------------------------

	// 유저 정보가 저장이 완료되면 호출할 콜백 함수 바인딩
	OnSetUserInfoFinished.BindUObject(this, &AMVE_PC_StageLevel::Initialize);

	// 위젯 생성
	CreateWidgets();

	// 유저 정보 저장
	FOnGetProfileComplete OnGetProfileComplete;
	OnGetProfileComplete.BindUObject(this, &AMVE_PC_StageLevel::SetUserInfo);
	UMVE_API_Helper::GetProfile(OnGetProfileComplete);
}

void AMVE_PC_StageLevel::Client_TriggerFlashPostProcess_Implementation()
{
	if (FlashPostProcessTimelineComp)
	{
		FlashPostProcessTimelineComp->PlayFromStart();
		PRINTNETLOG(this, TEXT("타임라인으로 플래시 포스트 프로세스!"))
	}
}

void AMVE_PC_StageLevel::OnFlashPostProcessUpdate(const float Value) const
{
	if (FlashMID)
	{
		FlashMID->SetScalarParameterValue(FName("Strength"), Value);
	}
}

void AMVE_PC_StageLevel::SetUserInfo(const bool bSuccess, const FProfileResponseData& Data, const FString& ErrorCode)
{
	PRINTNETLOG(this, TEXT("SetUserInfo Called"));
	
	if (!bSuccess)
	{
		PRINTLOG(TEXT("Failed to get user info: %s"), *ErrorCode);
		UserName = TEXT("Guest");
		UserEmail = TEXT("");
		return;
	}

	// UserEmail 설정
	UserEmail = Data.User.Email;

	int32 AtIndex;
	if (UserEmail.FindChar(TEXT('@'), AtIndex))
	{
		UserName = UserEmail.Left(AtIndex);
	}
	else
	{
		UserName = FString("No Name");
	}

	// 서버에 PlayerName 설정 요청 (Server RPC)
	ServerSetPlayerName(UserName);
	PRINTNETLOG(this, TEXT("Requesting server to set PlayerName: %s"), *UserName);

	// 브로드캐스트
	OnSetUserInfoFinished.ExecuteIfBound();
}


UAudioComponent* AMVE_PC_StageLevel::GetAudioComponent() const
{
	if (const AMVE_StageLevel_AudCharacter* AudCharacter = GetPawn<AMVE_StageLevel_AudCharacter>())
	{
		return AudCharacter->GetAudioComponent();
	}
	return nullptr;
}

UMVE_StageLevel_AudCharacterShooterComponent* AMVE_PC_StageLevel::GetShooterComponent() const
{
	if (const AMVE_StageLevel_AudCharacter* AudCharacter = GetPawn<AMVE_StageLevel_AudCharacter>())
	{
		return AudCharacter->GetShooterComponent();
	}
	return nullptr;
}

void AMVE_PC_StageLevel::SetupChatUI(UMVE_WC_Chat* InWidget)
{
	// 1. 컨트롤러 생성
	ChatController = NewObject<UMVE_StageLevel_WidgetController_Chat>(this);
	ChatController->Initialize(true); // 자동으로 ChatManager 찾기
    
	// 2. 위젯에 컨트롤러 설정
	ChatWidget = InWidget;
    
	if (ChatWidget)
	{
		ChatWidget->SetController(ChatController);
	}
}

void AMVE_PC_StageLevel::Initialize()
{
	// CustomizationManager 가져오기
	UMVE_AUD_CustomizationManager* CustomizationManager = GetGameInstance()->GetSubsystem<UMVE_AUD_CustomizationManager>();

	if (CustomizationManager)
	{
		// ⭐ 액세서리 데이터 가져오기
		FCustomizationData SavedAccessory = CustomizationManager->GetSavedCustomization();

		// ⭐ 던지기 메시 데이터 가져오기
		FCustomizationData SavedThrowMesh = CustomizationManager->GetSavedThrowMeshData();

		// 액세서리와 던지기 메시 모두 체크
		bool bHasAccessory = !SavedAccessory.ModelUrl.IsEmpty();
		bool bHasThrowMesh = !SavedThrowMesh.ModelUrl.IsEmpty();

		if (!bHasAccessory && !bHasThrowMesh)
		{
			PRINTLOG(TEXT("⚠️ No saved customization data (user hasn't customized yet)"));
			return;
		}

		// ⭐ 배열로 직렬화 (액세서리 + 던지기 메시)
		TArray<FCustomizationData> AllCustomizations;

		if (bHasAccessory)
		{
			AllCustomizations.Add(SavedAccessory);
			PRINTLOG(TEXT("✅ Accessory added to sync"));
			PRINTLOG(TEXT("   Model URL: %s"), *SavedAccessory.ModelUrl);
			PRINTLOG(TEXT("   Socket: %s"), *SavedAccessory.SocketName);
		}

		// ⭐ 던지기 메시 추가
		if (bHasThrowMesh)
		{
			AllCustomizations.Add(SavedThrowMesh);
			PRINTLOG(TEXT("✅ Throw mesh added to sync"));
			PRINTLOG(TEXT("   Model URL: %s"), *SavedThrowMesh.ModelUrl);
			PRINTLOG(TEXT("   Socket: %s"), *SavedThrowMesh.SocketName);

			// ⚠️ 로컬 다운로드는 GameState의 OnAccessoryLoaded()가 처리하므로 여기서는 하지 않음
			// CustomizationManager->LoadThrowMeshFromURL()는 호출하지 않음
		}

		// ⭐ JSON 직렬화 (올바른 배열 형식)
		PRINTLOG(TEXT("🔍 AllCustomizations count: %d"), AllCustomizations.Num());

		TArray<TSharedPtr<FJsonValue>> JsonArray;
		for (const FCustomizationData& Data : AllCustomizations)
		{
			PRINTLOG(TEXT("🔍 Processing customization: Socket=%s, ModelUrl=%s"), *Data.SocketName, *Data.ModelUrl);
			TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
			JsonObject->SetStringField(TEXT("socketName"), Data.SocketName);

			TSharedPtr<FJsonObject> LocationObj = MakeShareable(new FJsonObject);
			LocationObj->SetNumberField(TEXT("x"), Data.RelativeLocation.X);
			LocationObj->SetNumberField(TEXT("y"), Data.RelativeLocation.Y);
			LocationObj->SetNumberField(TEXT("z"), Data.RelativeLocation.Z);
			JsonObject->SetObjectField(TEXT("relativeLocation"), LocationObj);

			TSharedPtr<FJsonObject> RotationObj = MakeShareable(new FJsonObject);
			RotationObj->SetNumberField(TEXT("pitch"), Data.RelativeRotation.Pitch);
			RotationObj->SetNumberField(TEXT("yaw"), Data.RelativeRotation.Yaw);
			RotationObj->SetNumberField(TEXT("roll"), Data.RelativeRotation.Roll);
			JsonObject->SetObjectField(TEXT("relativeRotation"), RotationObj);

			JsonObject->SetNumberField(TEXT("relativeScale"), Data.RelativeScale);
			JsonObject->SetStringField(TEXT("modelUrl"), Data.ModelUrl);

			JsonArray.Add(MakeShareable(new FJsonValueObject(JsonObject)));
		}

		FString PresetJSON;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&PresetJSON);
		FJsonSerializer::Serialize(JsonArray, Writer);

		PRINTLOG(TEXT("PresetJSON: %s"), *PresetJSON);

		// 서버에 등록 요청
		PRINTLOG(TEXT("Calling ServerRPC_RegisterAccessory..."));
		ServerRPC_RegisterAccessory(UserName, PresetJSON);
	}
}

void AMVE_PC_StageLevel::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (InPawn)
	{
		PRINTNETLOG(this, TEXT("새로운 폰 빙의 : %s"), *InPawn->GetName());
	}
}

AMVE_StageLevel_AudCharacter* AMVE_PC_StageLevel::GetBindingAudCharacter() const
{
	return Cast<AMVE_StageLevel_AudCharacter>(GetPawn());
}

// 서버에 액세서리 정보 등록
void AMVE_PC_StageLevel::ServerRPC_RegisterAccessory_Implementation(const FString& UserID, const FString& PresetJSON)
{
	PRINTLOG(TEXT("=== ServerRPC_RegisterAccessory (Server) ==="));
	PRINTLOG(TEXT("UserID: %s"), *UserID);
	PRINTLOG(TEXT("PresetJSON: %s"), *PresetJSON);
	
	if (!HasAuthority())
	{
		PRINTLOG(TEXT("❌ Not server authority"));
		return;
	}
	
	// GameMode 가져오기
	AMVE_GM_StageLevel* StageGM = Cast<AMVE_GM_StageLevel>(GetWorld()->GetAuthGameMode());
	if (!StageGM)
	{
		PRINTLOG(TEXT("❌ StageGameMode not found"));
		return;
	}
	
	// GameMode에 액세서리 정보 전달
	StageGM->RegisterPlayerAccessory(UserID, PresetJSON);
	
	PRINTLOG(TEXT("✅ Accessory registered to GameMode"));
}

// 신규 입장 시 기존 참여자들의 액세서리 정보 받기
void AMVE_PC_StageLevel::ClientRPC_ReceiveExistingAccessories_Implementation(
	const TArray<FPlayerAccessoryInfo>& ExistingAccessories)
{
	PRINTLOG(TEXT("=== ClientRPC_ReceiveExistingAccessories ==="));
	PRINTLOG(TEXT("Received %d existing accessories"), ExistingAccessories.Num());
	
	// CustomizationManager 가져오기
	UMVE_AUD_CustomizationManager* CustomizationManager = 
		GetGameInstance()->GetSubsystem<UMVE_AUD_CustomizationManager>();
	
	if (!CustomizationManager)
	{
		PRINTLOG(TEXT("❌ CustomizationManager not found"));
		return;
	}
	
	// 각 기존 참여자의 액세서리 적용
	for (const FPlayerAccessoryInfo& Info : ExistingAccessories)
	{
		const FString& UserID = Info.UserID;
		const FString& PresetJSON = Info.PresetJSON;
		
		PRINTLOG(TEXT("Applying existing accessory for UserID: %s"), *UserID);
		
		// JSON 역직렬화
		FCustomizationData Data = CustomizationManager->DeserializeCustomizationData(PresetJSON);
		
		// TODO: 다음 Step에서 구현
		// 해당 UserID의 Character 찾기 → 액세서리 적용
		PRINTLOG(TEXT("TODO: Find character and apply accessory"));
	}
}

void AMVE_PC_StageLevel::ToggleRadialMenu(const bool bShow)
{
	if (AudRadialMenuWidget == false)
	{
		return;
	}
	
	if (bShow)
	{
		AudRadialMenuWidget->SetVisibility(ESlateVisibility::Visible);
		CenterMouseCursor();
		SetShowMouseCursor(true);

		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		SetInputMode(InputMode);
		PRINTNETLOG(this, TEXT("입력모드 : GameAndUI"))
	}
	else
	{
		AudRadialMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
		SetShowMouseCursor(false);

		const FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		PRINTNETLOG(this, TEXT("입력모드 : GameOnly"))
	}
}

int32 AMVE_PC_StageLevel::GetRadialMenuSelection() const
{
	if (AudRadialMenuWidget)
	{
		return AudRadialMenuWidget->GetCurrentSectorIndex();
	}
	return -1; // 유효하지 않은 인덱스
}

void AMVE_PC_StageLevel::SwitchInputHelpWidget(const EAudienceInputHelpState NewState) const
{
	if (AudInputHelpWidget)
	{
		AudInputHelpWidget->SetInputHelpState(NewState);	
	}
	else
	{
		PRINTNETLOG(this, TEXT("위험한 접근!"));
	}
}

void AMVE_PC_StageLevel::CenterMouseCursor()
{
	if (const UGameViewportClient* ViewportClient = GetGameInstance()->GetGameViewportClient())
	{
		FVector2D ViewportSize;
		ViewportClient->GetViewportSize(ViewportSize);
		const int32 CenterX = FMath::RoundToInt(ViewportSize.X * 0.5f);
		const int32 CenterY = FMath::RoundToInt(ViewportSize.Y * 0.5f);
		SetMouseLocation(CenterX, CenterY);
	}
}

void AMVE_PC_StageLevel::CreateWidgets()
{
	if (!IsLocalPlayerController())
	{
		return;
	}
	
	if (IsHost())
	{
		if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
		{
			UIManager->ShowScreen(EUIScreen::Studio_OnLive);
			UUserWidget* Widget = UIManager->GetCurrentWidget();
			UMVE_STU_WC_ConcertStudioPanel* StudioWidget = Cast<UMVE_STU_WC_ConcertStudioPanel>(Widget);
			SetupChatUI(StudioWidget->ChatWidget);

			// StdComponent에 AudioPlayer 연결 (재생 진행률 업데이트용)
			if (StdComponent && StudioWidget->AudioPlayer)
			{
				StdComponent->SetAudioPlayer(StudioWidget->AudioPlayer);
				PRINTNETLOG(this, TEXT("AudioPlayer가 StdComponent에 연결되었습니다."));
			}
		}
	}
	else
	{
		if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
		{
			UIManager->ShowScreen(EUIScreen::AudienceConcertRoom);
			UUserWidget* Widget = UIManager->GetCurrentWidget();
			UMVE_AUD_WC_InteractionPanel* AudienceWidget = Cast<UMVE_AUD_WC_InteractionPanel>(Widget); 
			SetupChatUI(AudienceWidget->ChatWidget);
		}
		
		// 원형 메뉴
		if (AudRadialMenuWidget)
		{
			AudRadialMenuWidget->RemoveFromParent();
			AudRadialMenuWidget = nullptr;
		}
		if (AudRadialMenuWidgetClass)
		{
			AudRadialMenuWidget = CreateWidget<UMVE_WC_StageLevel_AudRadialMenu>(this, AudRadialMenuWidgetClass);	
		}
		if (AudRadialMenuWidget)
		{
			AudRadialMenuWidget->AddToViewport();
			AudRadialMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
			PRINTNETLOG(this, TEXT("%s 위젯을 생성했지만, 숨겨진 상태입니다."), *AudRadialMenuWidgetClass->GetName());
		}
		
		// 입력 헬프 위젯
		if (AudInputHelpWidget)
		{
			AudInputHelpWidget->RemoveFromParent();
			AudInputHelpWidget = nullptr;
		}
		if (AudInputHelpWidgetClass)
		{
			AudInputHelpWidget = CreateWidget<UMVE_WC_StageLevel_AudInputHelp>(this, AudInputHelpWidgetClass);	
		}
		if (AudInputHelpWidget)
		{
			AudInputHelpWidget->AddToViewport();
			AudInputHelpWidget->SetInputHelpState(EAudienceInputHelpState::Selectable);
			PRINTNETLOG(this, TEXT("%s 위젯을 생성했고, 볼 수 있는 상태입니다."), *AudInputHelpWidgetClass->GetName());
		}
	}
}

bool AMVE_PC_StageLevel::IsHost() const
{
	if (const ENetMode NetMode = GetNetMode(); NetMode == NM_ListenServer)
	{
		// 로컬 플레이어인지 확인 (NetConnection이 없으면 로컬 플레이어)
		return IsLocalPlayerController();
	}
	else if (NetMode == NM_Standalone)
	{
		// 스탠드얼론 = 호스트
		return true;
	}

	return false;
}

void AMVE_PC_StageLevel::ServerSendChatMessage_Implementation(const FString& MessageContent, const FGuid& ClientMessageID)
{
	if (!HasAuthority())
	{
		return;
	}

	// ChatManager 찾기
	AMVE_StageLevel_ChatManager* ChatManager = nullptr;
	for (TActorIterator<AMVE_StageLevel_ChatManager> It(GetWorld()); It; ++It)
	{
		ChatManager = *It;
		break;
	}

	if (!ChatManager)
	{
		PRINTLOG_CHAT(TEXT("ServerSendChatMessage: ChatManager not found"));
		return;
	}

	// ChatManager를 통해 메시지 처리
	ChatManager->ServerSendMessage(PlayerState, MessageContent, ClientMessageID);
}

bool AMVE_PC_StageLevel::ServerSendChatMessage_Validate(const FString& MessageContent, const FGuid& ClientMessageID)
{
	// 기본 검증
	return !MessageContent.IsEmpty() && MessageContent.Len() <= 400;
}

void AMVE_PC_StageLevel::ServerSetPlayerName_Implementation(const FString& InPlayerName)
{
	if (!HasAuthority())
	{
		return;
	}

	// 서버의 PlayerState에 이름 설정
	if (PlayerState)
	{
		PlayerState->SetPlayerName(InPlayerName);
		PRINTNETLOG(this, TEXT("Server set PlayerName to: %s"), *InPlayerName);
	}
	
}

void AMVE_PC_StageLevel::NotifyAudioReady()
{
	if (IsLocalController())
	{
		if (AMVE_PS_StageLevel* PS = GetPlayerState<AMVE_PS_StageLevel>())
		{
			PRINTNETLOG(this, TEXT("Local player audio is ready. Notifying server."));
			PS->Server_SetIsAudioReady(true);
		}
	}
}