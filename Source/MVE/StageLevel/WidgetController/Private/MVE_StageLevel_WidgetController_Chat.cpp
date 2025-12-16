
#include "../Public/MVE_StageLevel_WidgetController_Chat.h"

#include "EngineUtils.h"
#include "StageLevel/Default/Public/MVE_StageLevel_ChatManager.h"
#include "MVE.h"

UMVE_StageLevel_WidgetController_Chat::UMVE_StageLevel_WidgetController_Chat()
{
}

void UMVE_StageLevel_WidgetController_Chat::Initialize(bool bAutoFindManager)
{
	if (!GetWorld())
	{
		PRINTLOG(TEXT("Initialize failed: Invalid World"));
		return;
	}

	if (bAutoFindManager)
	{
		// ChatManager 즉시 검색
		AMVE_StageLevel_ChatManager* FoundManager = FindChatManager();
		
		if (FoundManager)
		{
			SetChatManager(FoundManager);
		}

		GetWorld()->GetTimerManager().SetTimer(
				FindManagerTimerHandle,
				FTimerDelegate::CreateUObject(this, &UMVE_StageLevel_WidgetController_Chat::Initialize, true),
				0.5f,
				false);
	}
}

void UMVE_StageLevel_WidgetController_Chat::Shutdown()
{
	// 타이머 정리
	if (GetWorld() && FindManagerTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(FindManagerTimerHandle);
	}

	// 델리게이트 언바인드
	if (ChatManager)
	{
		ChatManager.Get()->OnChatMessageReceived.RemoveDynamic(this, &UMVE_StageLevel_WidgetController_Chat::OnMessageReceived);
		ChatManager.Get()->OnChatMessageRejected.RemoveDynamic(this, &UMVE_StageLevel_WidgetController_Chat::OnMessageRejected);
		ChatManager.Get()->OnChatMessageRemoved.RemoveDynamic(this, &UMVE_StageLevel_WidgetController_Chat::OnMessageRemovedFromServer);
	}

	ChatManager = nullptr;
	PendingMessages.Empty();

	PRINTLOG(TEXT("ChatWidgetController shutdown"));
}

void UMVE_StageLevel_WidgetController_Chat::SetChatManager(AMVE_StageLevel_ChatManager* InChatManager)
{
	// 기존 연결 정리
	if (ChatManager)
	{
		ChatManager->OnChatMessageReceived.RemoveDynamic(this, &UMVE_StageLevel_WidgetController_Chat::OnMessageReceived);
		ChatManager->OnChatMessageRejected.RemoveDynamic(this, &UMVE_StageLevel_WidgetController_Chat::OnMessageRejected);
		ChatManager->OnChatMessageRemoved.RemoveDynamic(this, &UMVE_StageLevel_WidgetController_Chat::OnMessageRemovedFromServer);
	}

	ChatManager = InChatManager;

	if (ChatManager)
	{
		// 델리게이트 바인딩
		ChatManager.Get()->OnChatMessageReceived.AddDynamic(this, &UMVE_StageLevel_WidgetController_Chat::OnMessageReceived);
		ChatManager.Get()->OnChatMessageRejected.AddDynamic(this, &UMVE_StageLevel_WidgetController_Chat::OnMessageRejected);
		ChatManager.Get()->OnChatMessageRemoved.AddDynamic(this, &UMVE_StageLevel_WidgetController_Chat::OnMessageRemovedFromServer);

		// 타이머 정리
		if (GetWorld() && FindManagerTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(FindManagerTimerHandle);
		}

		PRINTLOG(TEXT("Connected to ChatManager"));
	}
}

bool UMVE_StageLevel_WidgetController_Chat::SendMessage(const FString& MessageContent)
{
	if (!ChatManager)
	{
		PRINTLOG(TEXT("SendMessage failed: ChatManager not connected"));
		OnChatSendFailed.Broadcast(TEXT("채팅 시스템에 연결되지 않았습니다."), EChatRejectReason::None);
		return false;
	}

	// 로컬 검증
	FString RejectReason;
	if (!ValidateMessageLocally(MessageContent, RejectReason))
	{
		PRINTLOG(TEXT("Local validation failed: %s"), *RejectReason);
		OnChatSendFailed.Broadcast(RejectReason, EChatRejectReason::InvalidContent);
		return false;
	}

	// Pending 큐 크기 제한
	if (PendingMessages.Num() >= MAX_PENDING_MESSAGES)
	{
		PRINTLOG(TEXT("Pending queue full"));
		OnChatSendFailed.Broadcast(
			TEXT("이전 메시지가 처리 중입니다. 잠시 후 다시 시도해주세요."),
			EChatRejectReason::RateLimit
		);
		return false;
	}

	// Pending 메시지 생성 및 추가
	FChatMessage PendingMsg;
	PendingMsg.MessageContent = MessageContent;
	PendingMsg.SenderName = TEXT("나"); // 임시, 서버에서 실제 이름 설정
	PendingMsg.State = EChatMessageState::Pending;
	PendingMsg.Timestamp = FDateTime::UtcNow();
	PendingMsg.MessageID = FGuid::NewGuid();

	AddPendingMessage(PendingMsg);

	// 서버로 전송
	ChatManager.Get()->SendMessage(MessageContent);

	PRINTLOG(TEXT("Message sent: '%s' (ID: %s)"), 
		*MessageContent, *PendingMsg.MessageID.ToString());

	return true;
}

TArray<FChatMessage> UMVE_StageLevel_WidgetController_Chat::GetChatHistory() const
{
	if (ChatManager)
	{
		return ChatManager->GetChatHistory();
	}

	return TArray<FChatMessage>();
}

void UMVE_StageLevel_WidgetController_Chat::OnMessageReceived(const FChatMessage& Message)
{
	PRINTLOG(TEXT("Message received: [%s] %s"), 
		*Message.SenderName, *Message.MessageContent);

	// Pending 큐에서 매칭되는 메시지 찾기
	TOptional<FChatMessage> PendingMsg = RemovePendingMessage(Message.MessageID);

	if (PendingMsg.IsSet())
	{
		// 내가 보낸 메시지가 확정됨
		PRINTLOG(TEXT("Pending message confirmed: %s"), 
			*Message.MessageID.ToString());
	}

	// UI에 브로드캐스트
	OnChatMessageAdded.Broadcast(Message);
}

void UMVE_StageLevel_WidgetController_Chat::OnMessageRejected(const FString& Reason, EChatRejectReason RejectType)
{
	PRINTLOG(TEXT("Message rejected: %s"), *Reason);

	// UI에 브로드캐스트
	OnChatSendFailed.Broadcast(Reason, RejectType);

	// TODO: 가장 최근 Pending 메시지 제거?
}

void UMVE_StageLevel_WidgetController_Chat::OnMessageRemovedFromServer(const FGuid& MessageID, const FString& Reason)
{
	PRINTLOG(TEXT("Message removed: %s (Reason: %s)"), 
		*MessageID.ToString(), *Reason);

	// Pending 큐에서도 제거
	RemovePendingMessage(MessageID);

	// UI에 브로드캐스트
	OnChatMessageRemoved.Broadcast(MessageID);
}

AMVE_StageLevel_ChatManager* UMVE_StageLevel_WidgetController_Chat::FindChatManager()
{
	if (!GetWorld())
	{
		return nullptr;
	}

	// 월드에서 ChatManager 검색
	for (TActorIterator<AMVE_StageLevel_ChatManager> It(GetWorld()); It; ++It)
	{
		AMVE_StageLevel_ChatManager* Manager = *It;
		if (Manager)
		{
			PRINTLOG(TEXT("Found ChatManager: %s"), *Manager->GetName());
			return Manager;
		}
	}

	return nullptr;
}

void UMVE_StageLevel_WidgetController_Chat::AddPendingMessage(const FChatMessage& Message)
{
	PendingMessages.Add(Message);

	// Pending 메시지도 UI에 즉시 표시 (Pending 상태로)
	OnChatMessageAdded.Broadcast(Message);

	PRINTLOG(TEXT("Added to pending queue (Total: %d)"), PendingMessages.Num());
}

TOptional<FChatMessage> UMVE_StageLevel_WidgetController_Chat::RemovePendingMessage(const FGuid& MessageID)
{
	int32 FoundIndex = PendingMessages.IndexOfByPredicate([&MessageID](const FChatMessage& Msg) {
		return Msg.MessageID == MessageID;
	});

	if (FoundIndex != INDEX_NONE)
	{
		FChatMessage RemovedMsg = PendingMessages[FoundIndex];
		PendingMessages.RemoveAt(FoundIndex);

		PRINTLOG(TEXT("Removed from pending queue (Remaining: %d)"), PendingMessages.Num());

		return RemovedMsg;
	}

	return TOptional<FChatMessage>();
}

bool UMVE_StageLevel_WidgetController_Chat::ValidateMessageLocally(const FString& MessageContent, FString& OutReason)
{
	// 빈 메시지
	if (MessageContent.IsEmpty())
	{
		OutReason = TEXT("메시지를 입력해주세요.");
		return false;
	}

	// 공백만 있는 메시지
	if (MessageContent.TrimStartAndEnd().IsEmpty())
	{
		OutReason = TEXT("유효한 내용을 입력해주세요.");
		return false;
	}

	// 길이 검증 (서버 설정보다 조금 여유있게)
	constexpr int32 MaxLength = 200;
	if (MessageContent.Len() > MaxLength)
	{
		OutReason = FString::Printf(TEXT("메시지가 너무 깁니다 (최대 %d자)."), MaxLength);
		return false;
	}

	return true;
}
