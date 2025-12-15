
#include "../Public/MVE_WC_Chat.h"

#include "MVE.h"
#include "MVE_StageLevel_WidgetController_Chat.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"

void UMVE_WC_Chat::NativeConstruct()
{
	Super::NativeConstruct();

	// 입력 위젯 초기화
	SetupInputBox();
	SetupSendButton();

	PRINTLOG_CHAT(TEXT("ChatWidget constructed"));
}

void UMVE_WC_Chat::NativeDestruct()
{
	// 델리게이트 언바인드
	if (Controller)
	{
		Controller->OnChatMessageAdded.RemoveDynamic(this, &UMVE_WC_Chat::OnMessageAdded);
		Controller->OnChatSendFailed.RemoveDynamic(this, &UMVE_WC_Chat::OnSendFailed);
		Controller->OnChatMessageRemoved.RemoveDynamic(this, &UMVE_WC_Chat::OnMessageRemoved);
	}

	// 버튼 이벤트 언바인드
	if (SendButton)
	{
		SendButton->OnClicked.RemoveDynamic(this, &UMVE_WC_Chat::HandleSendMessage);
	}

	// 타이머 정리
	if (GetWorld() && ErrorTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(ErrorTimerHandle);
	}

	Super::NativeDestruct();

	PRINTLOG_CHAT(TEXT("ChatWidget destructed"));
}

void UMVE_WC_Chat::SetController(UMVE_StageLevel_WidgetController_Chat* InController)
{
	// 기존 컨트롤러 언바인드
	if (Controller)
	{
		Controller->OnChatMessageAdded.RemoveDynamic(this, &UMVE_WC_Chat::OnMessageAdded);
		Controller->OnChatSendFailed.RemoveDynamic(this, &UMVE_WC_Chat::OnSendFailed);
		Controller->OnChatMessageRemoved.RemoveDynamic(this, &UMVE_WC_Chat::OnMessageRemoved);
	}

	Controller = InController;

	// 새 컨트롤러 바인드
	if (Controller)
	{
		Controller->OnChatMessageAdded.AddDynamic(this, &UMVE_WC_Chat::OnMessageAdded);
		Controller->OnChatSendFailed.AddDynamic(this, &UMVE_WC_Chat::OnSendFailed);
		Controller->OnChatMessageRemoved.AddDynamic(this, &UMVE_WC_Chat::OnMessageRemoved);

		// 기존 히스토리 로드
		LoadChatHistory();

		PRINTLOG_CHAT(TEXT("Controller set and history loaded"));
	}
}

void UMVE_WC_Chat::OnMessageAdded(const FChatMessage& Message)
{
	// ListView에 추가
	AddMessageToListView(Message);

	PRINTLOG_CHAT(TEXT("Message added to ListView: [%s] %s"), *Message.SenderName, *Message.MessageContent);
}

void UMVE_WC_Chat::OnSendFailed(const FString& Reason, EChatRejectReason RejectType)
{
	// 에러 표시
	ShowError(Reason);

	PRINTLOG_CHAT(TEXT("Send failed: %s"), *Reason);
}

void UMVE_WC_Chat::OnMessageRemoved(const FGuid& MessageID)
{
	// ListView에서 제거
	RemoveMessageFromListView(MessageID);

	PRINTLOG_CHAT(TEXT("Message removed from ListView: %s"), *MessageID.ToString());

}

bool UMVE_WC_Chat::SendMessage(const FString& MessageContent)
{
	if (!Controller)
	{
		PRINTLOG_CHAT(TEXT("SendMessage failed: Controller not set"));
		ShowError(TEXT("채팅 컨트롤러가 초기화되지 않았습니다."));
		return false;
	}

	bool bSuccess = Controller->SendMessage(MessageContent);

	// 전송 성공 시 입력창 초기화
	if (bSuccess && MessageInputBox)
	{
		MessageInputBox->SetText(FText::GetEmpty());
	}

	return bSuccess;
}

void UMVE_WC_Chat::LoadChatHistory()
{
	if (!Controller)
	{
		PRINTLOG_CHAT(TEXT("LoadChatHistory: Controller not set"));
		return;
	}

	// 히스토리 가져오기
	TArray<FChatMessage> History = Controller->GetChatHistory();

	// UI 클리어
	ClearChat();

	// 각 메시지 추가
	for (const FChatMessage& Message : History)
	{
		AddMessageToListView(Message);
	}

	PRINTLOG_CHAT(TEXT("Loaded %d messages from history"), History.Num());

}

void UMVE_WC_Chat::ClearChat()
{
	MessageEntries.Empty();
	MessageEntryMap.Empty();

	// ListView 클리어
	if (MessageListView)
	{
		MessageListView->ClearListItems();
	}

	PRINTLOG_CHAT(TEXT("Chat cleared"));
}
void UMVE_WC_Chat::AddMessageToListView(const FChatMessage& Message)
{
	if (!MessageListView)
	{
		PRINTLOG_CHAT(TEXT("AddMessageToListView: MessageListView not bound"));
		return;
	}

	// 중복 체크: 이미 존재하는 메시지면 업데이트
	TObjectPtr<UMVE_ChatMessageEntry>* ExistingEntry = MessageEntryMap.Find(Message.MessageID);
	if (ExistingEntry && *ExistingEntry)
	{
		// 기존 Entry 업데이트 (Pending -> Confirmed)
		(*ExistingEntry)->UpdateMessage(Message);
		MessageListView->RequestRefresh();

		PRINTLOG_CHAT(TEXT("Message entry updated (ID: %s)"), *Message.MessageID.ToString());
		return;
	}

	// Entry 오브젝트 생성
	UMVE_ChatMessageEntry* Entry = UMVE_ChatMessageEntry::Create(this, Message);

	if (Entry)
	{
		// 리스트에 추가
		MessageEntries.Add(Entry);
		MessageEntryMap.Add(Message.MessageID, Entry);

		// ListView에 추가
		MessageListView->AddItem(Entry);

		// 맨 아래로 스크롤
		ScrollToBottom();

		PRINTLOG_CHAT(TEXT("Message entry added (Total: %d)"), MessageEntries.Num());
	}
}

void UMVE_WC_Chat::RemoveMessageFromListView(const FGuid& MessageID)
{
	if (!MessageListView)
	{
		return;
	}

	// Entry 찾기
	TObjectPtr<UMVE_ChatMessageEntry>* FoundEntry = MessageEntryMap.Find(MessageID);
	
	if (FoundEntry && *FoundEntry)
	{
		// ListView에서 제거
		MessageListView->RemoveItem(*FoundEntry);

		// 배열에서 제거
		MessageEntries.Remove(*FoundEntry);
		MessageEntryMap.Remove(MessageID);

		PRINTLOG_CHAT(TEXT("Message entry removed (Remaining: %d)"), MessageEntries.Num());
	}
}

void UMVE_WC_Chat::ScrollToBottom()
{
	if (MessageListView && MessageEntries.Num() > 0)
	{
		// 마지막 아이템으로 스크롤
		MessageListView->ScrollToBottom();
	}
}

void UMVE_WC_Chat::ShowError(const FString& ErrorMessage)
{
	// ErrorBorder와 ErrorText가 있으면 사용
	if (ErrorBorder && ErrorText)
	{
		ErrorText->SetText(FText::FromString(ErrorMessage));
		ErrorBorder->SetVisibility(ESlateVisibility::Visible);

		// 3초 후 숨김
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(
				ErrorTimerHandle,
				this,
				&UMVE_WC_Chat::HideError,
				3.0f,
				false
			);
		}
	}
	else
	{
		// 없으면 로그로만 출력
		PRINTLOG_CHAT(TEXT("Error: %s"), *ErrorMessage);
	}
}

//~ 내부 메서드 구현

void UMVE_WC_Chat::SetupInputBox()
{
	if (!MessageInputBox)
	{
		PRINTLOG_CHAT(TEXT("MessageInputBox not bound"));
		return;
	}

	// 텍스트 변경 이벤트 바인딩
	MessageInputBox->OnTextChanged.AddDynamic(this, &UMVE_WC_Chat::OnInputTextChanged);

	// Enter 키 이벤트 바인딩
	MessageInputBox->OnTextCommitted.AddDynamic(this, &UMVE_WC_Chat::OnInputTextCommitted);

	PRINTLOG_CHAT(TEXT("InputBox setup complete"));
}

void UMVE_WC_Chat::SetupSendButton()
{
	if (!SendButton)
	{
		PRINTLOG_CHAT(TEXT("SendButton not bound"));
		return;
	}

	// 클릭 이벤트 바인딩
	SendButton->OnClicked.AddDynamic(this, &UMVE_WC_Chat::HandleSendMessage);

	PRINTLOG_CHAT(TEXT("SendButton setup complete"));
}

void UMVE_WC_Chat::HandleSendMessage()
{
	if (!MessageInputBox)
	{
		return;
	}

	FString MessageContent = MessageInputBox->GetText().ToString();

	if (!MessageContent.IsEmpty())
	{
		SendMessage(MessageContent);
	}
}

void UMVE_WC_Chat::OnInputTextChanged(const FText& Text)
{
	// 텍스트 길이 제한 등 실시간 검증
	// 필요 시 추가 구현
}

void UMVE_WC_Chat::OnInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// Enter 키로 전송
	if (CommitMethod == ETextCommit::OnEnter)
	{
		HandleSendMessage();
	}
}

void UMVE_WC_Chat::HideError()
{
	if (ErrorBorder)
	{
		ErrorBorder->SetVisibility(ESlateVisibility::Collapsed);
	}
}