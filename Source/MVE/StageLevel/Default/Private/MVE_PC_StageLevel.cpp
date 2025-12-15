#include "StageLevel/Default/Public/MVE_PC_StageLevel.h"

#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "MVE.h"
#include "MVE_AUD_WC_InteractionPanel.h"
#include "MVE_StageLevel_WidgetController_Chat.h"
#include "MVE_STU_WC_ConcertStudioPanel.h"
#include "MVE_WC_Chat.h"
#include "UIManagerSubsystem.h"
#include "StageLevel/Actor/Public/MVE_StageLevel_AudCharacter.h"
#include "StageLevel/Default/Public/MVE_PC_StageLevel_AudienceComponent.h"
#include "StageLevel/Default/Public/MVE_PC_StageLevel_StudioComponent.h"
#include "StageLevel/Widget/Public/MVE_WC_StageLevel_AudRadialMenu.h"
#include "StageLevel/Default/Public/MVE_StageLevel_ChatManager.h"
#include "EngineUtils.h"
#include "MVE_API_Helper.h"
#include "GameFramework/PlayerState.h"

AMVE_PC_StageLevel::AMVE_PC_StageLevel()
{
	// 관객(Audience) 역할을 수행하는 플레이어의 상호작용을 처리하는 컴포넌트입니다.
	AudComponent = CreateDefaultSubobject<UMVE_PC_StageLevel_AudienceComponent>(TEXT("AudComponent"));

	// 스튜디오(Studio) 역할을 수행하는 플레이어의 기능을 처리하는 컴포넌트입니다.
	StdComponent = CreateDefaultSubobject<UMVE_PC_StageLevel_StudioComponent>(TEXT("StdComponent"));
}

void AMVE_PC_StageLevel::SetUserInfo(bool bSuccess, const FProfileResponseData& Data, const FString& ErrorCode)
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

void AMVE_PC_StageLevel::BeginPlay()
{
	Super::BeginPlay();

	// 로컬 플레이어 컨트롤러인 경우에만 위젯 생성
	if (IsLocalPlayerController())
	{
		CreateWidgets();
	}

	// 유저 정보 저장
	FOnGetProfileComplete OnGetProfileComplete;
	OnGetProfileComplete.BindUObject(this, &AMVE_PC_StageLevel::SetUserInfo);
	UMVE_API_Helper::GetProfile(OnGetProfileComplete);
}

void AMVE_PC_StageLevel::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (InPawn)
	{
		PRINTNETLOG(this, TEXT("새로운 폰 빙의 : %s"), *InPawn->GetName());
	}
}

void AMVE_PC_StageLevel::ToggleRadialMenu(const bool bShow)
{
	if (!AudRadialMenuWidget)
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