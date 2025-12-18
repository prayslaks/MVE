
#include "../Public/MVE_AUD_WidgetClass_JoinRoomConfirmPopup.h"
#include "MVE.h"
#include "MVE_AUD_CustomizationManager.h"
#include "MVE_AUD_WidgetClass_GenerateMesh.h"
#include "MVE_GIS_SessionManager.h"
#include "UIManagerSubsystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UMVE_AUD_WidgetClass_JoinRoomConfirmPopup::NativeConstruct()
{
	Super::NativeConstruct();

	if (ConfirmButton)
	{
		ConfirmButton.Get()->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_JoinRoomConfirmPopup::OnConfirmButtonClicked);
	}

	if (CancelButton)
	{
		CancelButton.Get()->OnClicked.AddDynamic(this, &UMVE_AUD_WidgetClass_JoinRoomConfirmPopup::OnCancelButtonClicked);
	}
}

void UMVE_AUD_WidgetClass_JoinRoomConfirmPopup::SetRoomInfo(const FConcertInfo& RoomInfo)
{
	// 방 정보 저장
	CurrentRoomInfo = RoomInfo;

	// UI 업데이트
	UpdateUI();
}

void UMVE_AUD_WidgetClass_JoinRoomConfirmPopup::OnConfirmButtonClicked()
{
	// 델리게이트 브로드캐스트
	OnConfirmed.Broadcast(CurrentRoomInfo);
	
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMVE_GIS_SessionManager* SessionManager = GI->GetSubsystem<UMVE_GIS_SessionManager>())
		{
			// CustomizationManager 가져오기
			UMVE_AUD_CustomizationManager* CustomizationManager = GetGameInstance()->GetSubsystem<UMVE_AUD_CustomizationManager>();
			if (!CustomizationManager) return;
				
			// 메모리에서 커스터마이징 데이터 가져오기
			FCustomizationData SavedData = CustomizationManager->GetSavedCustomization();
	
			if (SavedData.ModelUrl.IsEmpty())
			{
				PRINTLOG(TEXT("⚠️ No customization data. User will join without accessory."));
			}
			else
			{
				//서버에 저장 (비동기)
				PRINTLOG(TEXT("Saving preset to server before joining..."));
				CustomizationManager->SavePresetToServer(SavedData);
			}
			
			// 방 참가
			PRINTLOG(TEXT("Joining session: %s"), *CurrentRoomInfo.ConcertName);
			SessionManager->JoinSessionByRoomId(CurrentRoomInfo.RoomId);
		}
	}

	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UIManager->CloseTopPopup();
	}
}

void UMVE_AUD_WidgetClass_JoinRoomConfirmPopup::OnCancelButtonClicked()
{
	OnCancelled.Broadcast();

	// 팝업 닫기
	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UIManager->ClosePopup(this);
	}
}

void UMVE_AUD_WidgetClass_JoinRoomConfirmPopup::UpdateUI()
{
	// 방 제목
	if (RoomTitleText)
	{
		RoomTitleText->SetText(FText::FromString(
			FString::Printf(TEXT("%s"), *CurrentRoomInfo.ConcertName)));
	}
}
