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
#include "MVE_GS_StageLevel.h"
#include "MVE_WC_StageLevel_AudInputHelp.h"
#include "GameFramework/PlayerState.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "StageLevel/Default/Public/MVE_PS_StageLevel.h"
#include "SenderReceiver.h"

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

	// ⭐ 서버 API 로드 제거 - 로컬 CustomizationManager 메모리 사용
	// LoadCustomizationPresets();  // 주석 처리: 타이밍 문제로 인해 로컬 메모리 사용

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
	PRINTLOG(TEXT("=== Initialize called ==="));

	// CustomizationManager 가져오기
	UMVE_AUD_CustomizationManager* CustomizationManager = GetGameInstance()->GetSubsystem<UMVE_AUD_CustomizationManager>();

	if (CustomizationManager)
	{
		// ⭐ 액세서리 데이터 가져오기
		FCustomizationData SavedAccessory = CustomizationManager->GetSavedCustomization();
		PRINTLOG(TEXT("📦 SavedAccessory - ModelUrl: %s, Socket: %s"),
			*SavedAccessory.ModelUrl, *SavedAccessory.SocketName);

		// ⭐ 던지기 메시 데이터 가져오기
		FCustomizationData SavedThrowMesh = CustomizationManager->GetSavedThrowMeshData();
		PRINTLOG(TEXT("📦 SavedThrowMesh - ModelUrl: %s, Socket: %s"),
			*SavedThrowMesh.ModelUrl, *SavedThrowMesh.SocketName);

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

		// ⭐ 모든 커스터마이징을 배열로 한 번에 서버에 전송 (JSON 없이 직접 전달)
		PRINTLOG(TEXT("🔍 AllCustomizations count: %d"), AllCustomizations.Num());

		for (const FCustomizationData& Data : AllCustomizations)
		{
			PRINTLOG(TEXT("  - Socket: %s, ModelUrl: %s"), *Data.SocketName, *Data.ModelUrl);
		}

		PRINTLOG(TEXT("📤 Sending all customizations to server in one RPC call"));
		ServerRPC_RegisterAccessory(UserName, AllCustomizations);
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

// 서버에 액세서리 정보 등록 (배열로 한 번에 받아서 처리)
void AMVE_PC_StageLevel::ServerRPC_RegisterAccessory_Implementation(const FString& UserID, const TArray<FCustomizationData>& CustomizationDataArray)
{
	PRINTLOG(TEXT("=== ServerRPC_RegisterAccessory (Server) ==="));
	PRINTLOG(TEXT("UserID: %s"), *UserID);
	PRINTLOG(TEXT("Received %d customizations"), CustomizationDataArray.Num());

	if (!HasAuthority())
	{
		PRINTLOG(TEXT("❌ Not server authority"));
		return;
	}

	// GameState 가져오기
	AMVE_GS_StageLevel* StageGS = GetWorld()->GetGameState<AMVE_GS_StageLevel>();
	if (!StageGS)
	{
		PRINTLOG(TEXT("❌ StageGameState not found"));
		return;
	}

	// 각 커스터마이징을 모든 클라이언트에게 브로드캐스트
	for (const FCustomizationData& CustomizationData : CustomizationDataArray)
	{
		PRINTLOG(TEXT("📤 Broadcasting - Socket: %s, ModelUrl: %s"), *CustomizationData.SocketName, *CustomizationData.ModelUrl);
		StageGS->MulticastRPC_BroadcastAccessory(UserID, CustomizationData);
	}

	PRINTLOG(TEXT("✅ All %d customizations broadcasted to all clients"), CustomizationDataArray.Num());
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

		PRINTLOG(TEXT("Applying existing accessories for UserID: %s"), *UserID);
		PRINTLOG(TEXT("PresetJSON: %s"), *PresetJSON);

		// JSON 배열 파싱
		TArray<TSharedPtr<FJsonValue>> JsonArray;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(PresetJSON);

		if (!FJsonSerializer::Deserialize(Reader, JsonArray))
		{
			PRINTLOG(TEXT("❌ Failed to parse PresetJSON for UserID: %s"), *UserID);
			continue;
		}

		PRINTLOG(TEXT("✅ Parsed %d accessories for UserID: %s"), JsonArray.Num(), *UserID);

		// 각 액세서리/던지기 메시 처리
		for (const TSharedPtr<FJsonValue>& JsonValue : JsonArray)
		{
			const TSharedPtr<FJsonObject>& JsonObject = JsonValue->AsObject();
			if (!JsonObject.IsValid())
			{
				PRINTLOG(TEXT("⚠️ Invalid JSON object, skipping"));
				continue;
			}

			// FCustomizationData로 역직렬화
			FCustomizationData Data;
			Data.SocketName = JsonObject->GetStringField(TEXT("socketName"));
			Data.ModelUrl = JsonObject->GetStringField(TEXT("modelUrl"));
			Data.RelativeScale = JsonObject->GetNumberField(TEXT("relativeScale"));

			const TSharedPtr<FJsonObject>* LocationObj;
			if (JsonObject->TryGetObjectField(TEXT("relativeLocation"), LocationObj))
			{
				Data.RelativeLocation.X = (*LocationObj)->GetNumberField(TEXT("x"));
				Data.RelativeLocation.Y = (*LocationObj)->GetNumberField(TEXT("y"));
				Data.RelativeLocation.Z = (*LocationObj)->GetNumberField(TEXT("z"));
			}

			const TSharedPtr<FJsonObject>* RotationObj;
			if (JsonObject->TryGetObjectField(TEXT("relativeRotation"), RotationObj))
			{
				Data.RelativeRotation.Pitch = (*RotationObj)->GetNumberField(TEXT("pitch"));
				Data.RelativeRotation.Yaw = (*RotationObj)->GetNumberField(TEXT("yaw"));
				Data.RelativeRotation.Roll = (*RotationObj)->GetNumberField(TEXT("roll"));
			}

			PRINTLOG(TEXT("  - Socket: %s, ModelUrl: %s"), *Data.SocketName, *Data.ModelUrl);

			// GameState를 통해 다운로드 시작
			if (AMVE_GS_StageLevel* GameState = GetWorld()->GetGameState<AMVE_GS_StageLevel>())
			{
				// GameState의 MulticastRPC 로직을 로컬에서 직접 실행
				// (이미 모든 클라이언트가 받았으므로 Multicast 불필요)
				GameState->MulticastRPC_BroadcastAccessory(UserID, Data);
			}
		}
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

void AMVE_PC_StageLevel::LoadCustomizationPresets()
{
	PRINTLOG(TEXT("=== LoadCustomizationPresets ==="));

	// CustomizationManager 가져오기
	UMVE_AUD_CustomizationManager* CustomizationManager = GetGameInstance()->GetSubsystem<UMVE_AUD_CustomizationManager>();

	if (!CustomizationManager)
	{
		PRINTLOG(TEXT("❌ CustomizationManager not found"));
		return;
	}

	// 1. 액세서리 프리셋 로드 (서버에서)
	CustomizationManager->LoadAccessoryPresetFromServer();
	PRINTLOG(TEXT("✅ LoadAccessoryPresetFromServer called"));

	// 2. 던지기 메시 프리셋 로드 (서버에서)
	CustomizationManager->LoadThrowMeshPreset();
	PRINTLOG(TEXT("✅ LoadThrowMeshPreset called"));

	// ⭐ Note: 실제 로딩은 비동기이므로, CustomizationManager의 콜백에서
	//    SavedCustomization과 SavedThrowMeshData가 채워집니다.
	//    Initialize()는 OnSetUserInfoFinished 콜백으로 호출되므로,
	//    타이밍 상 프리셋 로딩이 완료된 후에 호출될 가능성이 높습니다.
}