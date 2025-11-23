
#include "../Public/UIManagerSubsystem.h"
#include "MVE.h"
#include "Blueprint/UserWidget.h"
#include "Data/ScreenTypes.h"

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

		if (Row->PopupName == FName(""))
		{
			PRINTLOG(TEXT("Popup Name 잘못 지정됨"));
			continue;
		}

		if (!Row->WidgetClass)
		{
			PRINTLOG(TEXT("Widget Class 지정 안함"));
			continue;
		}
		
		PopupWidgetClasses.Add(Row->PopupName, Row->WidgetClass);
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
    
	PRINTLOG(TEXT("Switched to screen: %d"), (int32)ScreenType);
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

UUserWidget* UUIManagerSubsystem::ShowPopup(FName PopupName, bool bAddToStack)
{
	// 팝업 클래스가 등록되어 있는지 확인
	if (!PopupWidgetClasses.Contains(PopupName))
	{
		PRINTLOG(TEXT("Popup '%s' is not registered!"), *PopupName.ToString());
		return nullptr;
	}

	// 이미 같은 팝업이 열려있는지 확인
	for (UUserWidget* ExistingPopup : PopupStack)
	{
		if (ExistingPopup && ExistingPopup->GetClass() == PopupWidgetClasses[PopupName])
		{
			PRINTLOG(TEXT("Popup '%s' is already open!"), *PopupName.ToString());
			return ExistingPopup;
		}
	}

	// 팝업 위젯 생성
	TSubclassOf<UUserWidget> PopupClass = PopupWidgetClasses[PopupName];
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
		PRINTLOG(TEXT("Failed to create popup widget '%s'!"), *PopupName.ToString());
		return nullptr;
	}

	// 뷰포트에 추가
	int32 ZOrder = 100 + PopupStack.Num(); // 스택 위치에 따라 ZOrder 자동 증가
	PopupWidget->AddToViewport(ZOrder);
    
	PRINTLOG(TEXT("Popup '%s' created with ZOrder: %d"), *PopupName.ToString(), ZOrder);

	// 스택에 추가 (옵션)
	if (bAddToStack)
	{
		PopupStack.Add(PopupWidget);
		PRINTLOG(TEXT("Popup '%s' added to stack. Current stack size: %d"), 
				 *PopupName.ToString(), PopupStack.Num());
	}

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
