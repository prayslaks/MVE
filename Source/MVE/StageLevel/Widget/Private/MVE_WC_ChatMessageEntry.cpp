
#include "MVE_WC_ChatMessageEntry.h"

#include "MVE.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "StageLevel/Data/MVE_ChatMessageEntry.h"

void UMVE_WC_ChatMessageEntry::NativeConstruct()
{
	Super::NativeConstruct();

	// 위젯이 아직 생성되지 않았으면 생성
	if (!bWidgetsCreated)
	{
		CreateWidgets();
	}

	PRINTLOG(TEXT("ChatMessageEntry widget constructed"));
}

void UMVE_WC_ChatMessageEntry::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 위젯 생성
	CreateWidgets();
}

void UMVE_WC_ChatMessageEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	// IUserObjectListEntry 인터페이스 구현
	UMVE_ChatMessageEntry* Entry = Cast<UMVE_ChatMessageEntry>(ListItemObject);
	
	if (!Entry)
	{
		PRINTLOG(TEXT("NativeOnListItemObjectSet: Invalid entry object"));
		return;
	}

	CurrentEntry = Entry;

	// UI 업데이트
	UpdateUI(Entry);

	PRINTLOG(TEXT("List item object set: %s"), *Entry->GetSenderName());
}

void UMVE_WC_ChatMessageEntry::CreateWidgets()
{
	if (bWidgetsCreated)
	{
		return;
	}
	
	if (!RootHorizontalBox)
	{
		PRINTLOG(TEXT("Failed to create RootHorizontalBox"));
		return;
	}

	// 발신자 이름 TextBlock
	if (SenderNameText)
	{
		// 폰트 설정
		//FSlateFontInfo FontInfo = SenderNameText->GetFont();
		//SenderNameText->SetFont(FontInfo);
		
		// 색상 설정
		SenderNameText->SetColorAndOpacity(FLinearColor::Yellow);
		
		// HorizontalBox에 추가
		UHorizontalBoxSlot* NameSlot = RootHorizontalBox->AddChildToHorizontalBox(SenderNameText);
		if (NameSlot)
		{
			NameSlot->SetPadding(FMargin(5.0f, 2.0f));
			NameSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
			NameSlot->SetVerticalAlignment(VAlign_Center);
		}
	}

	// 메시지 내용 TextBlock 생성
	if (MessageContentText)
	{
		// 폰트 설정
		//FSlateFontInfo FontInfo = MessageContentText->GetFont();
		//FontInfo.Size = 13;
		//MessageContentText->SetFont(FontInfo);
		
		// 색상 설정
		MessageContentText->SetColorAndOpacity(FLinearColor::Black);
		
		// 자동 줄바꿈 설정
		MessageContentText->SetAutoWrapText(true);
		MessageContentText->SetWrapTextAt(200.f);
		
		// HorizontalBox에 추가
		UHorizontalBoxSlot* ContentSlot = RootHorizontalBox->AddChildToHorizontalBox(MessageContentText);
		if (ContentSlot)
		{
			ContentSlot->SetPadding(FMargin(5.0f, 2.0f));
			ContentSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			ContentSlot->SetVerticalAlignment(VAlign_Center);
		}
	}

	// 타임스탬프 TextBlock 생성
	if (TimestampText)
	{
		// 폰트 설정
		//FSlateFontInfo FontInfo = TimestampText->GetFont();
		//FontInfo.Size = 10;
		//TimestampText->SetFont(FontInfo);
		
		// 색상 설정 (회색)
		TimestampText->SetColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 1.0f));
		
		// HorizontalBox에 추가
		UHorizontalBoxSlot* TimeSlot = RootHorizontalBox->AddChildToHorizontalBox(TimestampText);
		if (TimeSlot)
		{
			TimeSlot->SetPadding(FMargin(5.0f, 2.0f));
			TimeSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
			TimeSlot->SetHorizontalAlignment(HAlign_Right);
			TimeSlot->SetVerticalAlignment(VAlign_Center);
		}
	}

	bWidgetsCreated = true;
	
	PRINTLOG(TEXT("Chat message entry widgets created"));
}

void UMVE_WC_ChatMessageEntry::UpdateUI(UMVE_ChatMessageEntry* Entry)
{
	if (!Entry)
	{
		return;
	}

	// 위젯이 생성되지 않았으면 생성
	if (!bWidgetsCreated)
	{
		CreateWidgets();
	}

	// 발신자 이름 설정
	if (SenderNameText)
	{
		FString DisplayName = FString::Printf(TEXT("[%s]"), *Entry->GetSenderName());
		SenderNameText->SetText(FText::FromString(DisplayName));

		// 시스템 메시지면 빨간색
		if (Entry->IsSystemMessage())
		{
			SenderNameText->SetColorAndOpacity(FLinearColor::Red);
		}
		else
		{
			SenderNameText->SetColorAndOpacity(FLinearColor::Blue);
		}
	}

	// 메시지 내용 설정
	if (MessageContentText)
	{
		MessageContentText->SetText(FText::FromString(Entry->GetMessageContent()));
	}

	// 타임스탬프 설정
	if (TimestampText)
	{
		TimestampText->SetText(FText::FromString(Entry->GetTimestampString()));
	}

	PRINTLOG(TEXT("Entry UI updated: %s"), *Entry->GetMessageContent());
}
