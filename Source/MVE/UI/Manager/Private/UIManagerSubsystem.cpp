
#include "../Public/UIManagerSubsystem.h"
#include "MVE.h"
#include "Blueprint/UserWidget.h"
#include "UI/Manager/ScreenTypes.h"

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
		
		//ScreenWidgetClasses[Row->Screen] = Row->WidgetClass;
		ScreenWidgetClasses.Add(Row->Screen, Row->WidgetClass);
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
