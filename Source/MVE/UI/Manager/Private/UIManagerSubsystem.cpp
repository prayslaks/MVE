
#include "../Public/UIManagerSubsystem.h"
#include "MVE.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Data/ScreenTypes.h"
#include "UI/Widget/Dropdown/Public/MVE_WidgetClass_Dropdown.h"
#include "UI/Widget/Dropdown/Public/MVE_WidgetClass_DropdownOverlay.h"
#include "UI/Widget/Dropdown/Public/MVE_WidgetClass_Dropdown_UserSetting.h"
#include "UI/Widget/PopUp/Public/MVE_WidgetClass_ModalBackgroundWidget.h"

void UUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentScreen = EUIScreen::None;
	CurrentWidget = nullptr;

	InitScreenClasses();
    
	PRINTLOG(TEXT("UIManagerSubsystem Initialized"));
}

void UUIManagerSubsystem::Deinitialize()
{
	// 캐싱된 위젯들 정리
	for (auto& Pair : CachedWidgets)
	{
		if (Pair.Value && Pair.Value->IsInViewport())
		{
			Pair.Value->RemoveFromParent();
		}
	}
	CachedWidgets.Empty();
    
	Super::Deinitialize();
}

UUIManagerSubsystem* UUIManagerSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}
    
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}
    
	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}
    
	return GameInstance->GetSubsystem<UUIManagerSubsystem>();
}

void UUIManagerSubsystem::InitScreenClasses()
{
	// UI Screen Widget 클래스 설정
	if (UIClassesTableAsset.IsNull()) return;
	UDataTable* DT = UIClassesTableAsset.LoadSynchronous();

	for (auto& It : DT->GetRowMap())
	{
		const FScreenClassInfo* Row = DT->FindRow<FScreenClassInfo>(It.Key, TEXT("UUIManagerSubsystem::CacheUIScreen"), true);
		if (!Row) continue;

		if (Row->Screen == EUIScreen::None)
		{
			PRINTLOG(TEXT("UIScreen Enum 잘못 지정됨"));
			continue;
		}

		if (!Row->WidgetClass)
		{
			PRINTLOG(TEXT("Widget Class 지정 안함"));
			continue;
		}
		
		ScreenWidgetClasses.Add(Row->Screen, Row->WidgetClass);
	}

	// UI Popup Widget 클래스 설정
	if (PopUpClassesTableAsset.IsNull()) return;
	UDataTable* PDT = PopUpClassesTableAsset.LoadSynchronous();

	for (auto& It : PDT->GetRowMap())
	{
		const FPopupClassInfo* Row = PDT->FindRow<FPopupClassInfo>(It.Key, TEXT("UUIManagerSubsystem::CacheUIScreen"), true);
		if (!Row) continue;

		if (Row->PopupType == EUIPopup::None)
		{
			PRINTLOG(TEXT("Popup Type 잘못 지정됨"));
			continue;
		}

		if (!Row->WidgetClass)
		{
			PRINTLOG(TEXT("Widget Class 지정 안함"));
			continue;
		}

		if (Row->PopupType == EUIPopup::ModalBackground)
		{
			ModalBackgroundClass = Row->WidgetClass;
			continue;
		}
		
		PopupWidgetClasses.Add(Row->PopupType, Row->WidgetClass);
	}

	// Persistent Widget 설정
	if (!PersistentWidgetClassesTableAsset.IsNull())
	{
		UDataTable* PWDT = PersistentWidgetClassesTableAsset.LoadSynchronous();
		if (PWDT)
		{
			for (auto& It : PWDT->GetRowMap())
			{
				const FPersistentWidgetClassInfo* Row = PWDT->FindRow<FPersistentWidgetClassInfo>(
					It.Key, TEXT("UUIManagerSubsystem::InitScreenClasses"), true);
                
				if (!Row) continue;

				if (Row->WidgetType == EPersistentWidget::None)
				{
					PRINTLOG(TEXT("Persistent Widget Type 잘못 지정됨"));
					continue;
				}

				if (!Row->WidgetClass)
				{
					PRINTLOG(TEXT("Persistent Widget Class 지정 안함"));
					continue;
				}

				// 클래스 맵에 추가
				PersistentWidgetClasses.Add(Row->WidgetType, Row->WidgetClass);
                
				// 정보 맵에 추가 (ActiveScreens, ZOrder 포함)
				PersistentWidgetInfos.Add(Row->WidgetType, *Row);
			}
            
			PRINTLOG(TEXT("Loaded %d persistent widget classes"), PersistentWidgetClasses.Num());
		}
	}
}

void UUIManagerSubsystem::ShowScreen(EUIScreen ScreenType)
{
	if (CurrentScreen == ScreenType && CurrentWidget)
	{
		return; // 이미 표시 중
	}
    
	// 이전 위젯 제거
	if (CurrentWidget)
	{
		CurrentWidget->RemoveFromParent();
	}
    
	// 새 위젯 가져오기
	UUserWidget* NewWidget = CreateOrGetWidget(ScreenType);
	if (!NewWidget)
	{
		PRINTLOG(TEXT("Failed to create widget for screen: %d"), (int32)ScreenType);
		return;
	}
    
	// 화면 전환
	NewWidget->AddToViewport();
    
	CurrentWidget = NewWidget;
	CurrentScreen = ScreenType;
	ScreenHistory.Add(ScreenType);

	UpdatePersistentWidgets(ScreenType);
    
	PRINTLOG(TEXT("Switched to screen: %d"), (int32)ScreenType);
}

UUserWidget* UUIManagerSubsystem::GetCurrentWidget()
{
	return CurrentWidget;
}

void UUIManagerSubsystem::GoBack()
{
	if (ScreenHistory.Num() > 1)
	{
		ScreenHistory.Pop();
		EUIScreen PreviousScreen = ScreenHistory.Last();
		ScreenHistory.Pop(); // ShowScreen에서 다시 추가됨
        
		ShowScreen(PreviousScreen);
	}
}

void UUIManagerSubsystem::ClearHistory()
{
	ScreenHistory.Empty();
}

void UUIManagerSubsystem::RegisterScreenWidget(EUIScreen ScreenType, TSubclassOf<UUserWidget> WidgetClass)
{
	if (WidgetClass)
	{
		ScreenWidgetClasses.Add(ScreenType, WidgetClass);
	}
}

UUserWidget* UUIManagerSubsystem::ShowPopup(EUIPopup PopupType, bool bShowModalBackground)
{
	// 팝업 클래스가 등록되어 있는지 확인
	if (!PopupWidgetClasses.Contains(PopupType))
	{
		return nullptr;
	}

	// 이미 같은 팝업이 열려있는지 확인
	for (UUserWidget* ExistingPopup : PopupStack)
	{
		if (ExistingPopup && ExistingPopup->GetClass() == PopupWidgetClasses[PopupType])
		{
			return ExistingPopup;
		}
	}

	if (bShowModalBackground && PopupStack.Num() == 0)
	{
		ShowModalBackground();
	}

	// 팝업 위젯 생성
	TSubclassOf<UUserWidget> PopupClass = PopupWidgetClasses[PopupType];
	UWorld* World = GetWorld();
    
	if (!World)
	{
		PRINTLOG(TEXT("World is nullptr!"));
		return nullptr;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController)
	{
		PRINTLOG(TEXT("PlayerController is nullptr!"));
		return nullptr;
	}

	UUserWidget* PopupWidget = CreateWidget<UUserWidget>(PlayerController, PopupClass);
	if (!PopupWidget)
	{
		return nullptr;
	}

	// 뷰포트에 추가
	int32 ZOrder = 100 + PopupStack.Num(); // 스택 위치에 따라 ZOrder 자동 증가
	PopupWidget->AddToViewport(ZOrder);


	PopupStack.Add(PopupWidget);
	
	return PopupWidget;
}

void UUIManagerSubsystem::CloseTopPopup()
{
	if (PopupStack.Num() == 0)
	{
		PRINTLOG(TEXT("No popup to close. Stack is empty."));
		return;
	}

	// 최상위 팝업 가져오기
	UUserWidget* TopPopup = PopupStack.Last();
    
	if (TopPopup)
	{
		PRINTLOG(TEXT("Closing top popup: %s"), *TopPopup->GetName());
        
		// 뷰포트에서 제거
		TopPopup->RemoveFromParent();
        
		// 스택에서 제거
		PopupStack.RemoveAt(PopupStack.Num() - 1);
        
		PRINTLOG(TEXT("Popup closed. Remaining stack size: %d"), PopupStack.Num());
	}
	else
	{
		PRINTLOG(TEXT("Top popup is nullptr!"));
		PopupStack.RemoveAt(PopupStack.Num() - 1);
	}

	// 모든 팝업이 닫혔으면 모달 배경도 제거
	if (PopupStack.Num() == 0)
	{
		HideModalBackground();
	}
}

void UUIManagerSubsystem::ClosePopup(UUserWidget* PopupWidget)
{
	if (!PopupWidget)
	{
		PRINTLOG(TEXT("PopupWidget is nullptr!"));
		return;
	}

	// 스택에서 해당 팝업 찾기
	int32 FoundIndex = PopupStack.Find(PopupWidget);
    
	if (FoundIndex == INDEX_NONE)
	{
		PRINTLOG(TEXT("Popup '%s' not found in stack!"), *PopupWidget->GetName());
        
		// 스택에 없어도 뷰포트에서는 제거 시도
		PopupWidget->RemoveFromParent();
		return;
	}

	PRINTLOG(TEXT("Closing popup: %s (Index: %d)"), *PopupWidget->GetName(), FoundIndex);
    
	// 뷰포트에서 제거
	PopupWidget->RemoveFromParent();
    
	// 스택에서 제거
	PopupStack.RemoveAt(FoundIndex);

	// 모달 배경 제거
	if (PopupStack.Num() == 0)
	{
		HideModalBackground();
	}
    
	PRINTLOG(TEXT("Popup closed. Remaining stack size: %d"), PopupStack.Num());
	
}

void UUIManagerSubsystem::CloseAllPopups()
{
	if (PopupStack.Num() == 0)
	{
		PRINTLOG(TEXT("No popups to close."));
		return;
	}

	PRINTLOG(TEXT("Closing all popups. Current count: %d"), PopupStack.Num());
    
	// 역순으로 순회하면서 모든 팝업 제거
	for (int32 i = PopupStack.Num() - 1; i >= 0; --i)
	{
		UUserWidget* PopupWidget = PopupStack[i];
        
		if (PopupWidget)
		{
			PRINTLOG(TEXT("Closing popup [%d]: %s"), i, *PopupWidget->GetName());
			PopupWidget->RemoveFromParent();
		}
		else
		{
			PRINTLOG(TEXT("Popup at index [%d] is nullptr!"), i);
		}
	}
    
	// 스택 비우기
	PopupStack.Empty();
    
	PRINTLOG(TEXT("All popups closed. Stack cleared."));
}

UUserWidget* UUIManagerSubsystem::CreateOrGetWidget(EUIScreen ScreenType)
{
	// 캐싱 사용 시 캐시에서 찾기
	if (bCacheWidgets && CachedWidgets.Contains(ScreenType))
	{
		return CachedWidgets[ScreenType];
	}
    
	// Widget 클래스 찾기
	TSubclassOf<UUserWidget>* WidgetClass = ScreenWidgetClasses.Find(ScreenType);
	if (!WidgetClass || !*WidgetClass)
	{
		PRINTLOG(TEXT("Widget class not registered for screen: %d"), (int32)ScreenType);
		return nullptr;
	}
    
	// PlayerController 가져오기
	APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		PRINTLOG(TEXT("PlayerController not found"));
		return nullptr;
	}
    
	// 위젯 생성
	UUserWidget* NewWidget = CreateWidget<UUserWidget>(PC, *WidgetClass);
	if (NewWidget && bCacheWidgets)
	{
		CachedWidgets.Add(ScreenType, NewWidget);
	}
    
	return NewWidget;
}

APlayerController* UUIManagerSubsystem::GetPlayerController()
{
	if (!CachedPlayerController)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			CachedPlayerController = World->GetFirstPlayerController();
		}
	}
	return CachedPlayerController;
}

void UUIManagerSubsystem::ShowModalBackground()
{
	if (ModalBackground && ModalBackground->IsInViewport())
	{
		PRINTLOG(TEXT("Modal background already visible."));
		return;
	}

	if (!ModalBackgroundClass)
	{
		PRINTLOG(TEXT("ModalBackgroundClass is not set!"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		PRINTLOG(TEXT("World is nullptr!"));
		return;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController)
	{
		PRINTLOG(TEXT("PlayerController is nullptr!"));
		return;
	}

	// 모달 배경 생성
	ModalBackground = CreateWidget<UMVE_WidgetClass_ModalBackgroundWidget>(
		PlayerController, ModalBackgroundClass);

	if (!ModalBackground)
	{
		PRINTLOG(TEXT("Failed to create modal background!"));
		return;
	}

	// 배경 클릭 이벤트 바인딩
	ModalBackground->OnBackgroundClicked.AddUObject(
		this, &UUIManagerSubsystem::OnModalBackgroundClicked);

	// ZOrder: 50 (화면 위, 팝업 아래)
	ModalBackground->AddToViewport(50);

	PRINTLOG(TEXT("Modal background shown."));
}

void UUIManagerSubsystem::HideModalBackground()
{
	if (ModalBackground && ModalBackground->IsInViewport())
	{
		ModalBackground->RemoveFromParent();
		PRINTLOG(TEXT("Modal background hidden."));
	}

	ModalBackground = nullptr;
}

void UUIManagerSubsystem::OnModalBackgroundClicked()
{
	CloseTopPopup();
}

void UUIManagerSubsystem::ShowPersistentWidget(EPersistentWidget WidgetType)
{
	// 이미 표시 중이면 무시
	if (ActivePersistentWidgets.Contains(WidgetType))
	{
		return;
	}
    
	// Widget 클래스 찾기
	TSubclassOf<UUserWidget>* WidgetClass = PersistentWidgetClasses.Find(WidgetType);
	if (!WidgetClass || !*WidgetClass)
	{
		return;
	}
    
	// Widget 생성
	UUserWidget* Widget = CreateWidget<UUserWidget>(GetPlayerController(), *WidgetClass);
	if (!Widget)
	{
		return;
	}
    
	// ZOrder 가져오기
	int32 ZOrder = 10; // 기본값
	if (FPersistentWidgetClassInfo* Info = PersistentWidgetInfos.Find(WidgetType))
	{
		ZOrder = Info->ZOrder;
	}
    
	// Viewport에 추가
	Widget->AddToViewport(ZOrder);
    
	// 활성 위젯으로 등록
	ActivePersistentWidgets.Add(WidgetType, Widget);
}

void UUIManagerSubsystem::HidePersistentWidget(EPersistentWidget WidgetType)
{
	UUserWidget* Widget = ActivePersistentWidgets.FindRef(WidgetType);
	if (Widget && Widget->IsInViewport())
	{
		Widget->RemoveFromParent();
	}
    
	ActivePersistentWidgets.Remove(WidgetType);
}

bool UUIManagerSubsystem::IsPersistentWidgetVisible(EPersistentWidget WidgetType) const
{
	return ActivePersistentWidgets[WidgetType]->GetVisibility() == ESlateVisibility::Visible;
}

void UUIManagerSubsystem::UpdatePersistentWidgets(EUIScreen NewScreen)
{
	// 모든 등록된 Persistent Widget을 순회
	for (auto& Pair : PersistentWidgetInfos)
	{
		EPersistentWidget WidgetType = Pair.Key;
		const FPersistentWidgetClassInfo& Info = Pair.Value;
        
		// 현재 Screen이 ActiveScreens에 포함되어 있는지 확인
		bool bShouldShow = Info.ActiveScreens.Contains(NewScreen);
		bool bCurrentlyShown = ActivePersistentWidgets.Contains(WidgetType);
        
		if (bShouldShow && !bCurrentlyShown)
		{
			// 표시해야 하는데 안 보이면 → 표시
			ShowPersistentWidget(WidgetType);
		}
		else if (!bShouldShow && bCurrentlyShown)
		{
			// 숨겨야 하는데 보이면 → 숨김
			HidePersistentWidget(WidgetType);
		}
	}
}

void UUIManagerSubsystem::ShowUserDropdown(const FVector2D& ButtonPosition, const FString& UserName)
{
	// 기존 드롭다운이 있으면 닫기
	CloseDropdown();

	PRINTLOG(TEXT("ShowUserDropdown - Position: %s, UserName: %s"), *ButtonPosition.ToString(), *UserName);

	
	// 1. 먼저 투명 오버레이 생성 (ZOrder 99)
	if (DropdownOverlayClass)
	{
		DropdownOverlay = CreateWidget<UMVE_WidgetClass_DropdownOverlay>(CachedPlayerController, DropdownOverlayClass);
		if (DropdownOverlay)
		{
			DropdownOverlay->AddToViewport(99);
            
			// 오버레이 클릭 이벤트 바인딩
			DropdownOverlay->OnOverlayClickedEvent.AddDynamic(this, &UUIManagerSubsystem::OnOverlayClicked);
		}
	}
	
	// 드롭다운 위젯 생성
	if (UserDropdownWidgetClass)
	{
		
		UMVE_WidgetClass_Dropdown_UserSetting* UserSettingDropdown = CreateWidget<UMVE_WidgetClass_Dropdown_UserSetting>(CachedPlayerController, UserDropdownWidgetClass);
	if (UserSettingDropdown)
		{
			PRINTLOG(TEXT("UserSettingDropdown widget created successfully"));
			CurrentDropdown = UserSettingDropdown;

			UserSettingDropdown->SetUserName(UserName);
			UserSettingDropdown->AddToViewport(300); // 높은 ZOrder
			PRINTLOG(TEXT("UserSettingDropdown Added to Viewport with ZOrder 300"));

			UserSettingDropdown->SetDropdownPosition(ButtonPosition);
		}
		else
		{
			PRINTLOG(TEXT("Failed to create UserSettingDropdown widget!"));
		}
	}
	else
	{
		PRINTLOG(TEXT("UserDropdownWidgetClass is not set!"));
	}
}

void UUIManagerSubsystem::CloseDropdown()
{
	// 드롭다운 제거
	if (CurrentDropdown)
	{
		CurrentDropdown->RemoveFromParent();
		CurrentDropdown = nullptr;
	}

	// 오버레이 제거
	if (DropdownOverlay)
	{
		DropdownOverlay->OnOverlayClickedEvent.RemoveDynamic(this, &UUIManagerSubsystem::OnOverlayClicked);
		DropdownOverlay->RemoveFromParent();
		DropdownOverlay = nullptr;
	}
}

void UUIManagerSubsystem::OnOverlayClicked()
{
	// 오버레이를 클릭하면 드롭다운 닫기
	CloseDropdown();
}
