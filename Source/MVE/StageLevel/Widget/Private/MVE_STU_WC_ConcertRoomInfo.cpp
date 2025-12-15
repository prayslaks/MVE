
#include "StageLevel/Widget/Public/MVE_STU_WC_ConcertRoomInfo.h"
#include "MVE.h"
#include "MVE_API_Helper.h"
#include "MVE_GIS_SessionManager.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "GameFramework/GameModeBase.h"

void UMVE_STU_WC_ConcertRoomInfo::NativeConstruct()
{
	Super::NativeConstruct();


	// UI 컴포넌트 유효성 검사
	if (!RoomTitleTextBox || !ViewerCountText || !ConcertToggleButton || !ConcertToggleButtonText)
	{
		PRINTLOG(TEXT("Required UI components are not bound!"));
		return;
	}

	// 이벤트 바인딩
	RoomTitleTextBox->OnTextCommitted.AddDynamic(this, &ThisClass::OnRoomTitleCommitted);
	ConcertToggleButton->OnClicked.AddDynamic(this, &ThisClass::OnConcertToggleClicked);

	// 초기 상태 설정
	bIsConcertOpen = false;
	UpdateButtonAppearance();
	UpdateViewerCount(0); // 초기 시청자 수 0명

	// 콘서트 방 제목 설정
	if (UMVE_GIS_SessionManager* SessionManager = GetGameInstance()->GetSubsystem<UMVE_GIS_SessionManager>())
	{
		FString ConcertName = SessionManager->GetCurrentSessionName();
		SetRoomTitle(ConcertName);
	}

	// 10초마다 시청자 수 업데이트
	GetWorld()->GetTimerManager().SetTimer(
		ViewerCountUpdateTimerHandle,
		this,
		&UMVE_STU_WC_ConcertRoomInfo::GetCurrentPlayer,
		10.f,
		true
		);
}

void UMVE_STU_WC_ConcertRoomInfo::NativeDestruct()
{

	// 이벤트 언바인딩
	if (RoomTitleTextBox)
	{
		RoomTitleTextBox->OnTextCommitted.RemoveDynamic(this, &ThisClass::OnRoomTitleCommitted);
	}

	if (ConcertToggleButton)
	{
		ConcertToggleButton->OnClicked.RemoveDynamic(this, &ThisClass::OnConcertToggleClicked);
	}

	// Delegate 클리어
	OnRoomTitleChanged.Clear();
	OnConcertToggled.Clear();

	// 타이머 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ViewerCountUpdateTimerHandle);
	}

	Super::NativeDestruct();
}

void UMVE_STU_WC_ConcertRoomInfo::SetRoomTitle(const FString& Title)
{
	if (!RoomTitleTextBox)
	{
		PRINTLOG(TEXT("RoomTitleTextBox is null!"));
		return;
	}

	RoomTitleTextBox->SetText(FText::FromString(Title));
	PRINTLOG(TEXT("Room title set to: %s"), *Title);
}

void UMVE_STU_WC_ConcertRoomInfo::UpdateViewerCount(int32 Count)
{
	if (!ViewerCountText)
	{
		PRINTLOG(TEXT("ViewerCountText is null!"));
		return;
	}

	PRINTLOG(TEXT("ViewerCount : %d"), Count);
	FString ViewerText = FString::Printf(TEXT("시청자: %d명"), Count);
	ViewerCountText->SetText(FText::FromString(ViewerText));
	
}

void UMVE_STU_WC_ConcertRoomInfo::GetCurrentPlayer()
{
	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
	if (!GameMode)
	{
		return; // 클라이언트에서 호출 시
	}
	
	UpdateViewerCount(GameMode->GetNumPlayers()-1);
}

void UMVE_STU_WC_ConcertRoomInfo::SetConcertState(bool bIsOpen)
{
	bIsConcertOpen = bIsOpen;
	UpdateButtonAppearance();
	PRINTLOG(TEXT("Concert state set to: %s"), bIsConcertOpen ? TEXT("Open") : TEXT("Closed"));
}

void UMVE_STU_WC_ConcertRoomInfo::OnRoomTitleCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// 엔터 키로 입력 완료했을 때만 처리
	if (CommitMethod != ETextCommit::OnEnter)
	{
		return;
	}

	FString NewTitle = Text.ToString();

	// 빈 제목 방지
	if (NewTitle.IsEmpty())
	{
		PRINTLOG(TEXT("Room title cannot be empty!"));
		// 이전 제목으로 되돌리기 (옵션)
		return;
	}

	PRINTLOG(TEXT("Room title changed to: %s"), *NewTitle);

	// Delegate로 컨트롤러에게 알림
	OnRoomTitleChanged.Broadcast(NewTitle);
}

void UMVE_STU_WC_ConcertRoomInfo::OnConcertToggleClicked()
{
	// 상태 토글
	bIsConcertOpen = !bIsConcertOpen;

	// UI 업데이트
	UpdateButtonAppearance();

	PRINTLOG(TEXT("Concert toggled: %s"), bIsConcertOpen ? TEXT("Open") : TEXT("Closed"));

	
	// 세션 매니저
	if (UMVE_GIS_SessionManager* SessionManager = GetGameInstance()->GetSubsystem<UMVE_GIS_SessionManager>())
	{
		if (bIsConcertOpen) SessionManager->StartBroadcast();
		else SessionManager->StopBroadcast();
	}
	
}

void UMVE_STU_WC_ConcertRoomInfo::UpdateButtonAppearance()
{
	if (!ConcertToggleButtonText)
	{
		PRINTLOG(TEXT("ConcertToggleButtonText is null!"));
		return;
	}

	if (bIsConcertOpen)
	{
		ConcertToggleButtonText->SetText(FText::FromString(TEXT("콘서트 닫기")));
		// TODO: 버튼 색상 변경 (빨간색 계열로)
		// ConcertToggleButton->SetBackgroundColor(FLinearColor::Red);
	}
	else
	{
		ConcertToggleButtonText->SetText(FText::FromString(TEXT("콘서트 열기")));
		// TODO: 버튼 색상 변경 (초록색 계열로)
		// ConcertToggleButton->SetBackgroundColor(FLinearColor::Green);
	}
}

void UMVE_STU_WC_ConcertRoomInfo::HandleViewerCountUpdate(bool bSuccess, const FGetConcertInfoResponseData& Data,
	const FString& ErrorMsg)
{
	PRINTLOG(TEXT("HandleViewerCountUpdate 호출됨"));
	
	if (bSuccess)
	{
		int32 ViewerCount = Data.Concert.CurrentAudience;
		PRINTLOG(TEXT("HandleViewerCountUpdate success"));
		UpdateViewerCount(ViewerCount);
	}
	else
	{
		PRINTLOG(TEXT("HandleViewerCountUpdate failed"));
	}
}

