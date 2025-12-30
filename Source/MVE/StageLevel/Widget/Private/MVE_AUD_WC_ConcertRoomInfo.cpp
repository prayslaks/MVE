
#include "MVE_AUD_WC_ConcertRoomInfo.h"
#include "MVE.h"
#include "MVE_GIS_SessionManager.h"
#include "Components/TextBlock.h"
#include "StageLevel/Default/Public/MVE_GS_StageLevel.h"

void UMVE_AUD_WC_ConcertRoomInfo::NativeConstruct()
{
	Super::NativeConstruct();

	// UI 컴포넌트 유효성 검사
	if (!RoomTitleText || !ViewerCountText)
	{
		PRINTLOG(TEXT("Required UI components are not bound!"));
		return;
	}

	// 세션 매니저에서 방 제목 가져오기
	if (UMVE_GIS_SessionManager* SessionManager = GetGameInstance()->GetSubsystem<UMVE_GIS_SessionManager>())
	{
		FString ConcertName = SessionManager->GetCurrentSessionName();
		SetRoomTitle(ConcertName);
	}

	// GameState의 ViewerCount 업데이트 델리게이트 구독
	if (AMVE_GS_StageLevel* StageGameState = GetWorld()->GetGameState<AMVE_GS_StageLevel>())
	{
		StageGameState->OnViewerCountChanged.AddDynamic(this, &UMVE_AUD_WC_ConcertRoomInfo::OnViewerCountUpdated);

		// 현재 시청자 수로 초기화
		UpdateViewerCount(StageGameState->GetViewerCount());
	}
}

void UMVE_AUD_WC_ConcertRoomInfo::NativeDestruct()
{
	// GameState 델리게이트 언바인드
	if (AMVE_GS_StageLevel* StageGameState = GetWorld()->GetGameState<AMVE_GS_StageLevel>())
	{
		StageGameState->OnViewerCountChanged.RemoveDynamic(this, &UMVE_AUD_WC_ConcertRoomInfo::OnViewerCountUpdated);
	}

	Super::NativeDestruct();
}

void UMVE_AUD_WC_ConcertRoomInfo::SetRoomTitle(const FString& Title)
{
	if (!RoomTitleText)
	{
		PRINTLOG(TEXT("RoomTitleText is null!"));
		return;
	}

	RoomTitleText->SetText(FText::FromString(Title));
	PRINTLOG(TEXT("Room title set to: %s"), *Title);
}

void UMVE_AUD_WC_ConcertRoomInfo::UpdateViewerCount(int32 Count)
{
	if (!ViewerCountText)
	{
		PRINTLOG(TEXT("ViewerCountText is null!"));
		return;
	}

	FString ViewerText = FString::Printf(TEXT("%d"), Count);
	ViewerCountText->SetText(FText::FromString(ViewerText));
}

void UMVE_AUD_WC_ConcertRoomInfo::OnViewerCountUpdated(int32 NewCount)
{
	UpdateViewerCount(NewCount);
}
