
#include "../Public/MVE_STU_BroadcastSettingTabBar.h"

#include "MVE.h"
#include "MVE_STU_Button_TabButton.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Data/BroadcastSettingTypes.h"

void UMVE_STU_BroadcastSettingTabBar::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeTabs();

	if (UserSettingButton)
		UserSettingButton.Get()->OnClicked.AddDynamic(this, &UMVE_STU_BroadcastSettingTabBar::OnUserSettingButtonClicked);
}

void UMVE_STU_BroadcastSettingTabBar::InitializeTabs()
{
	LoadTabData();
	CreateTabButtons();

	// 기본 탭 선택
	SetActiveTab(EBroadcastSettingTab::CharacterSettings);
}

void UMVE_STU_BroadcastSettingTabBar::SetActiveTab(EBroadcastSettingTab TabType)
{
	if (ActiveTab == TabType)
	{
		return;
	}

	if (!TabInfoMap.Contains(TabType))
	{
		PRINTLOG(TEXT("Tab type not found: %d"), (int32)TabType);
		return;
	}

	// 시각적 상태만 업데이트
	for (UMVE_STU_Button_TabButton* TabButton : TabButtons)
	{
		if (TabButton)
		{
			TabButton->SetSelected(TabButton->GetTabType() == TabType);
		}
	}

	ActiveTab = TabType;
	PRINTLOG(TEXT("Active tab changed to: %d"), (int32)ActiveTab);
}

void UMVE_STU_BroadcastSettingTabBar::OnTabClicked(EBroadcastSettingTab TabType)
{
	// 화면 전환
	SwitchToScreen(TabType);
    
	// 시각적 상태 업데이트
	SetActiveTab(TabType);
}

void UMVE_STU_BroadcastSettingTabBar::LoadTabData()
{
	if (TabDataTableAsset.IsNull())
	{
		PRINTLOG(TEXT("TabDataTableAsset is not set!"));
		return;
	}

	UDataTable* DT = TabDataTableAsset.LoadSynchronous();
	if (!DT)
	{
		PRINTLOG(TEXT("Failed to load TabDataTable!"));
		return;
	}

	TabInfoMap.Empty();

	for (auto& It : DT->GetRowMap())
	{
		const FBroadcastSettingTabInfo* Row = DT->FindRow<FBroadcastSettingTabInfo>(
			It.Key, TEXT("UMVE_BroadcastSettingTabBar::LoadTabData"), true);
        
		if (!Row || Row->TabType == EBroadcastSettingTab::None)
		{
			continue;
		}

		TabInfoMap.Add(Row->TabType, *Row);
	}

	PRINTLOG(TEXT("Loaded %d tab infos"), TabInfoMap.Num());
}

void UMVE_STU_BroadcastSettingTabBar::CreateTabButtons()
{
	if (!HBox_Tabs || !TabButtonClass)
	{
		PRINTLOG(TEXT("HBox_Tabs or TabButtonClass is not set!"));
		return;
	}

	TabButtons.Empty();
	HBox_Tabs->ClearChildren();

	TArray<EBroadcastSettingTab> TabOrder = {
		EBroadcastSettingTab::CharacterSettings,
		EBroadcastSettingTab::MusicStageSettings
	};

	for (EBroadcastSettingTab TabType : TabOrder)
	{
		if (!TabInfoMap.Contains(TabType))
		{
			continue;
		}

		const FBroadcastSettingTabInfo& TabInfo = TabInfoMap[TabType];

		// 공통 색상으로 TabInfo 구성
		FBroadcastSettingTabInfo TabInfoWithColors = TabInfo;

		
		if (TabType != EBroadcastSettingTab::CheckSettings)
		{
			// 탭 버튼 생성
			UMVE_STU_Button_TabButton* TabButton = CreateWidget<UMVE_STU_Button_TabButton>(this, TabButtonClass);
			if (!TabButton)
			{
				continue;
			}

			TabButton->SetupTab(TabInfoWithColors);
			TabButton->OnTabButtonClicked.AddDynamic(this, &UMVE_STU_BroadcastSettingTabBar::OnTabClicked);
		
			HBox_Tabs->AddChildToHorizontalBox(TabButton);
			TabButtons.Add(TabButton);
		}
		else
		{
			// 탭 버튼 생성
			UMVE_STU_Button_TabButton* TabButton = CreateWidget<UMVE_STU_Button_TabButton>(this, ConfirmButtonClass);
			if (!TabButton)
			{
				continue;
			}

			TabButton->SetupTab(TabInfoWithColors);
			TabButton->OnTabButtonClicked.AddDynamic(this, &UMVE_STU_BroadcastSettingTabBar::OnTabClicked);
		
			HBox_Right->AddChildToHorizontalBox(TabButton);
			TabButtons.Add(TabButton);
		}
	}

	PRINTLOG(TEXT("Created %d tab buttons"), TabButtons.Num());
}

void UMVE_STU_BroadcastSettingTabBar::SwitchToScreen(EBroadcastSettingTab TabType)
{
	if (!TabInfoMap.Contains(TabType))
	{
		return;
	}

	const FBroadcastSettingTabInfo& TabInfo = TabInfoMap[TabType];
    
	if (TabInfo.TargetScreen == EUIScreen::None)
	{
		PRINTLOG(TEXT("TargetScreen not set for tab: %d"), (int32)TabType);
		return;
	}

	// UIManagerSubsystem을 통한 화면 전환
	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UIManager->ShowScreen(TabInfo.TargetScreen);
	}
}

void UMVE_STU_BroadcastSettingTabBar::OnUserSettingButtonClicked()
{
	if (!UserSettingButton) return;
    
	FGeometry ButtonGeometry = UserSettingButton->GetCachedGeometry();
	FVector2D ButtonAbsPos = ButtonGeometry.GetAbsolutePosition();
	FVector2D ButtonSize = ButtonGeometry.GetLocalSize();
    
	// 버튼의 오른쪽 상단 좌표 (또는 원하는 기준점)
	FVector2D ButtonTopRight = ButtonAbsPos + FVector2D(ButtonSize.X, 0);
    
	if (UUIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UUIManagerSubsystem>())
	{
		FDropdownContext DropdownContext;
		DropdownContext.AnchorPosition = EDropdownAnchorPosition::TopRight;
		DropdownContext.ButtonSize = ButtonSize;
		DropdownContext.StringData = TEXT("UserEmail");
		DropdownContext.ButtonPosition = ButtonTopRight;
		
		// 우상단 앵커로 드롭다운 표시
		UIManager->ShowDropdown(EDropdownType::UserSetting, DropdownContext);
	}
}

FVector2D UMVE_STU_BroadcastSettingTabBar::GetProfileButtonScreenPosition() const
{
	if (!UserSettingButton)
	{
		return FVector2D::ZeroVector;
	}

	// 버튼의 Geometry 가져오기
	FGeometry ButtonGeometry = UserSettingButton->GetCachedGeometry();
    
	// 버튼의 화면 절대 위치 (왼쪽 상단)
	FVector2D AbsolutePosition = ButtonGeometry.GetAbsolutePosition();
    
	// 버튼의 크기
	FVector2D ButtonSize = ButtonGeometry.GetLocalSize();
    
	// 버튼의 왼쪽 하단 좌표 = 왼쪽 상단 + (0, Height)
	FVector2D BottomLeftPosition = AbsolutePosition + FVector2D(0, ButtonSize.Y);
    
	return BottomLeftPosition;
}

