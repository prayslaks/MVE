
#include "MVE_StageLevel_ChatManager.h"
#include "HttpModule.h"
#include "MVE.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Net/UnrealNetwork.h"
#include "MVE_PC_StageLevel.h"

AMVE_StageLevel_ChatManager::AMVE_StageLevel_ChatManager()
{
	// 리플리케이션 활성화
	bReplicates = true;
	bAlwaysRelevant = true;
	
	// Tick 비활성화 (필요 없음)
	PrimaryActorTick.bCanEverTick = false;
}

void AMVE_StageLevel_ChatManager::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMVE_StageLevel_ChatManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 채팅 히스토리 리플리케이트
	DOREPLIFETIME(AMVE_StageLevel_ChatManager, ChatHistory);
}

void AMVE_StageLevel_ChatManager::SendMessage(const FString& MessageContent, const FGuid& ClientMessageID)
{
	// 빈 메시지 무시
	if (MessageContent.IsEmpty())
	{
		return;
	}

	// PlayerController 가져오기
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		PRINTLOG_CHAT(TEXT("Client: Cannot find PlayerController"));
		return;
	}

	// PlayerController를 통해 Server RPC 호출
	if (AMVE_PC_StageLevel* StagePC = Cast<AMVE_PC_StageLevel>(PC))
	{
		StagePC->ServerSendChatMessage(MessageContent, ClientMessageID);

		PRINTLOG_CHAT(TEXT("Client: Sending message '%s' with ID %s via PlayerController"),
			*MessageContent, *ClientMessageID.ToString());
	}
	else
	{
		PRINTLOG_CHAT(TEXT("Client: PlayerController is not AMVE_PC_StageLevel"));
	}
}

void AMVE_StageLevel_ChatManager::BroadcastSystemMessage(const FString& SystemMessage)
{
	if (!HasAuthority())
	{
		PRINTLOG_CHAT(TEXT("BroadcastSystemMessage can only be called on server!"));
		return;
	}

	// 시스템 메시지 생성
	FChatMessage Message = FChatMessage::CreateSystemMessage(SystemMessage);

	// 즉시 브로드캐스트
	MulticastBroadcastMessage(Message);

	// 히스토리에 추가
	AddToChatHistory(Message);

	PRINTLOG_CHAT(TEXT("Server: Broadcasting system message '%s'"), *SystemMessage);
}

void AMVE_StageLevel_ChatManager::ServerSendMessage_Implementation(APlayerState* SenderPlayerState, const FString& MessageContent, const FGuid& ClientMessageID)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!SenderPlayerState)
	{
		PRINTLOG_CHAT(TEXT("Server: Invalid PlayerState"));
		return;
	}

	// 파라미터로 받은 PlayerState 사용
	APlayerState* PlayerState = SenderPlayerState;
	
	// 1. Rate Limit 체크
	if (!CheckRateLimit(PlayerState))
	{
		PRINTLOG_CHAT(TEXT("Server: Rate limit exceeded for player %s"), *PlayerState->GetPlayerName());

		ClientNotifyMessageRejected(
			TEXT("메시지를 너무 빠르게 전송하고 있습니다. 잠시 후 다시 시도해주세요."),
			EChatRejectReason::RateLimit
		);
		return;
	}

	// 2. 메시지 기본 검증
	FString RejectReason;
	if (!ValidateMessage(MessageContent, RejectReason))
	{
		PRINTLOG_CHAT(TEXT("Server: Message validation failed: %s"), *RejectReason);

		ClientNotifyMessageRejected(RejectReason, EChatRejectReason::InvalidContent);
		return;
	}

	// 3. 메시지 객체 생성
	FString SenderName, SenderID;
	GetSenderInfo(PlayerState, SenderName, SenderID);

	FChatMessage Message(SenderName, SenderID, MessageContent);
	Message.MessageID = ClientMessageID;
	Message.State = EChatMessageState::Confirmed;

	// 4. 즉시 브로드캐스트 (실시간성)
	MulticastBroadcastMessage(Message);

	PRINTLOG_CHAT(TEXT("Server: Broadcasting message '%s' from %s"), *MessageContent, *SenderName);

	// 5. AI 필터링 시작 (비동기)
	if (bEnableAIFilter)
	{
		PendingMessages.Add(Message.MessageID, Message);
		FilterMessageAsync(Message);
	}
	else
	{
		// AI 필터링 비활성화 시 즉시 히스토리 추가
		AddToChatHistory(Message);
	}
}

bool AMVE_StageLevel_ChatManager::ServerSendMessage_Validate(APlayerState* SenderPlayerState, const FString& MessageContent, const FGuid& ClientMessageID)
{
	// 기본 검증 (악의적인 요청 차단)
	return SenderPlayerState != nullptr && !MessageContent.IsEmpty() && MessageContent.Len() <= MaxMessageLength * 2;
}

void AMVE_StageLevel_ChatManager::MulticastBroadcastMessage_Implementation(const FChatMessage& Message)
{
	PRINTLOG_CHAT(TEXT("Received message: [%s] %s"), *Message.SenderName, *Message.MessageContent);

	// 델리게이트 브로드캐스트
	OnChatMessageReceived.Broadcast(Message);
}

void AMVE_StageLevel_ChatManager::MulticastRemoveMessage_Implementation(const FGuid& MessageID, const FString& Reason)
{
	PRINTLOG_CHAT(TEXT("Message %s removed: %s"),
		*MessageID.ToString(), *Reason);

	// 델리게이트 브로드캐스트
	OnChatMessageRemoved.Broadcast(MessageID, Reason);

	// 서버에서 히스토리에서도 제거
	if (HasAuthority())
	{
		ChatHistory.RemoveAll([&MessageID](const FChatMessage& Msg) {
			return Msg.MessageID == MessageID;
		});
	}
}

void AMVE_StageLevel_ChatManager::ClientNotifyMessageRejected_Implementation(const FString& Reason,
	EChatRejectReason RejectType)
{
	PRINTLOG_CHAT(TEXT("Message rejected: %s"), *Reason);

	// 델리게이트 브로드캐스트
	OnChatMessageRejected.Broadcast(Reason, RejectType);
}

void AMVE_StageLevel_ChatManager::OnRep_ChatHistory()
{
	PRINTLOG_CHAT(TEXT("ChatHistory replicated: %d messages"), ChatHistory.Num());

	// OnRep_ChatHistory는 초기 동기화 시에만 사용
	// 새 메시지는 이미 MulticastBroadcastMessage로 받았으므로 여기서 다시 브로드캐스트하지 않음
	//
	// 신규 접속자의 경우, 접속 직후에만 히스토리를 로드하면 되므로
	// 별도 플래그로 관리하거나, 이 함수는 빈 상태로 둠

	// TODO: 신규 접속 시에만 히스토리 로드 필요 시 플래그 추가
}

/*
 * 내부 메서드 구현
 */

bool AMVE_StageLevel_ChatManager::CheckRateLimit(APlayerState* Player)
{
	if (!Player)
	{
		return false;
	}

	// Rate Limit 버킷 가져오기 (없으면 생성)
	FRateLimitBucket& Bucket = RateLimits.FindOrAdd(Player);

	// 시간 경과에 따른 토큰 충전
	FDateTime Now = FDateTime::UtcNow();
	FTimespan Elapsed = Now - Bucket.LastRefill;
	float RefillAmount = Elapsed.GetTotalSeconds() * TokenRefillRate;
	
	Bucket.Tokens = FMath::Min(Bucket.Tokens + RefillAmount, MaxTokens);
	Bucket.LastRefill = Now;

	// 토큰 소비 시도
	if (Bucket.Tokens >= 1.0f)
	{
		Bucket.Tokens -= 1.0f;

		PRINTLOG_CHAT(TEXT("Rate limit check passed for %s (Tokens: %.2f)"),
			*Player->GetPlayerName(), Bucket.Tokens);

		return true;
	}

	PRINTLOG_CHAT(TEXT("Rate limit exceeded for %s (Tokens: %.2f)"),
		*Player->GetPlayerName(), Bucket.Tokens);

	return false;
}

bool AMVE_StageLevel_ChatManager::ValidateMessage(const FString& MessageContent, FString& OutReason)
{
	// 빈 메시지
	if (MessageContent.IsEmpty())
	{
		OutReason = TEXT("메시지가 비어있습니다.");
		return false;
	}

	// 길이 검증
	if (MessageContent.Len() > MaxMessageLength)
	{
		OutReason = FString::Printf(TEXT("메시지가 너무 깁니다 (최대 %d자)."), MaxMessageLength);
		return false;
	}

	// 공백만 있는 메시지
	if (MessageContent.TrimStartAndEnd().IsEmpty())
	{
		OutReason = TEXT("유효한 내용이 없습니다.");
		return false;
	}

	// 추가 검증 로직 (예: 특정 문자 차단 등)
	// ...

	return true;
}

void AMVE_StageLevel_ChatManager::FilterMessageAsync(const FChatMessage& Message)
{
	if (!HasAuthority())
	{
		return;
	}

	// HTTP 모듈 가져오기
	FHttpModule& HttpModule = FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = HttpModule.CreateRequest();

	// 요청 설정
	Request->SetURL(AIFilterServerURL);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetTimeout(AIFilterTimeout);

	// JSON 페이로드 생성
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField(TEXT("message"), Message.MessageContent);
	JsonObject->SetStringField(TEXT("sender"), Message.SenderName);
	JsonObject->SetStringField(TEXT("message_id"), Message.MessageID.ToString());

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	Request->SetContentAsString(JsonString);

	// 응답 콜백 바인딩
	Request->OnProcessRequestComplete().BindLambda(
		[this, MessageID = Message.MessageID](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
		{
			if (!bSucceeded || !HttpResponse.IsValid())
			{
				PRINTLOG_CHAT(TEXT("AI Filter request failed for message %s"),
					*MessageID.ToString());

				// 실패 시 일단 통과 처리 (서비스 가용성 우선)
				OnAIFilterCompleted(MessageID, true, TEXT(""));
				return;
			}

			// 응답 파싱
			FString ResponseString = HttpResponse->GetContentAsString();
			TSharedPtr<FJsonObject> ResponseJson;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);

			if (FJsonSerializer::Deserialize(Reader, ResponseJson) && ResponseJson.IsValid())
			{
				bool bPassed = ResponseJson->GetBoolField(TEXT("passed"));
				FString Reason = ResponseJson->GetStringField(TEXT("reason"));

				PRINTLOG_CHAT(TEXT("AI Filter result for %s: %s (Reason: %s)"),
					*MessageID.ToString(),
					bPassed ? TEXT("PASS") : TEXT("BLOCK"),
					*Reason);

				OnAIFilterCompleted(MessageID, bPassed, Reason);
			}
			else
			{
				PRINTLOG_CHAT(TEXT("Failed to parse AI Filter response"));
				OnAIFilterCompleted(MessageID, true, TEXT(""));
			}
		}
	);

	// 요청 전송
	Request->ProcessRequest();

	PRINTLOG_CHAT(TEXT("AI Filter request sent for message %s"),
		*Message.MessageID.ToString());
}

void AMVE_StageLevel_ChatManager::OnAIFilterCompleted(const FGuid& MessageID, bool bPassed, const FString& Reason)
{
	if (!HasAuthority())
	{
		return;
	}

	// Pending 메시지 찾기
	FChatMessage* FoundMessage = PendingMessages.Find(MessageID);
	if (!FoundMessage)
	{
		PRINTLOG_CHAT(TEXT("Message %s not found in pending list"),
			*MessageID.ToString());
		return;
	}

	if (bPassed)
	{
		// 통과: 히스토리에 추가
		FoundMessage->bIsFiltered = false;
		AddToChatHistory(*FoundMessage);

		PRINTLOG_CHAT(TEXT("Message %s passed AI filter, added to history"),
			*MessageID.ToString());
	}
	else
	{
		// 차단: 메시지 삭제 브로드캐스트
		FoundMessage->bIsFiltered = true;

		FString RemoveReason = FString::Printf(
			TEXT("부적절한 내용이 감지되었습니다: %s"),
			*Reason
		);

		MulticastRemoveMessage(MessageID, RemoveReason);

		// 발신자에게만 상세 이유 전달
		auto* SenderPtr = GetWorld()->GetGameState()->PlayerArray.FindByPredicate(
	[&](APlayerState* PS)
		{
		 return PS && FString::FromInt(PS->GetPlayerId()) == FoundMessage->SenderID;
		});

		if (SenderPtr)
		{
			APlayerState* Sender = *SenderPtr;
			APlayerController* PC = Cast<APlayerController>(Sender->GetOwner());
			if (PC)
			{
				// TODO: 특정 클라이언트에게만 RPC 전송하는 방법 개선 필요
			}
		}

		PRINTLOG_CHAT(TEXT("Message %s blocked by AI filter: %s"),
			*MessageID.ToString(), *Reason);
	}

	// Pending에서 제거
	PendingMessages.Remove(MessageID);
}

void AMVE_StageLevel_ChatManager::AddToChatHistory(const FChatMessage& Message)
{
	if (!HasAuthority())
	{
		return;
	}

	// 히스토리에 추가
	ChatHistory.Add(Message);

	// 최대 크기 초과 시 오래된 메시지 제거 (순환 버퍼)
	if (ChatHistory.Num() > MaxChatHistory)
	{
		int32 RemoveCount = ChatHistory.Num() - MaxChatHistory;
		ChatHistory.RemoveAt(0, RemoveCount);

		PRINTLOG_CHAT(TEXT("Removed %d old messages from history"), RemoveCount);
	}

	PRINTLOG_CHAT(TEXT("Message added to history (Total: %d)"), ChatHistory.Num());
}

void AMVE_StageLevel_ChatManager::GetSenderInfo(APlayerState* Player, FString& OutName, FString& OutID)
{
	if (!Player)
	{
		OutName = TEXT("Unknown");
		OutID = TEXT("UNKNOWN");
		return;
	}

	OutName = Player->GetPlayerName();
	
	// UniqueID가 유효하지 않으면 PlayerState 포인터를 ID로 사용
	if (Player->GetUniqueId().IsValid())
	{
		OutID = FString::FromInt(Player->GetPlayerId());
	}
	else
	{
		OutID = FString::Printf(TEXT("PS_%p"), Player);
	}
}

