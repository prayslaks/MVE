
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UIManagerSubsystem.generated.h"

UENUM(BlueprintType)
enum class EUIScreen : uint8
{
	None,
	Main,
	Login,
	Register,
	ModeSelect,
	Credit,
	StudioCharacterCustomize,
	StudioBroadcastSetting,
	StudioBroadcast,
	AudienceMenu,
	AudienceView
};

UCLASS(Config=Game)
class MVE_API UUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
    
	static UUIManagerSubsystem* Get(const UObject* WorldContextObject);

	UPROPERTY(Config, EditDefaultsOnly, Category = "DataTable")
	TSoftObjectPtr<UDataTable> UIClassesTableAsset;

	void InitScreenClasses();
    
	// 화면 전환 - 이게 전부
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowScreen(EUIScreen ScreenType);
    
	UFUNCTION(BlueprintCallable, Category = "UI")
	void GoBack();
    
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ClearHistory();
    
	UFUNCTION(BlueprintCallable, Category = "UI")
	EUIScreen GetCurrentScreen() const { return CurrentScreen; }
    
	UFUNCTION(BlueprintCallable, Category = "UI")
	void RegisterScreenWidget(EUIScreen ScreenType, TSubclassOf<UUserWidget> WidgetClass);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TMap<EUIScreen, TSubclassOf<UUserWidget>> ScreenWidgetClasses;
    
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	bool bCacheWidgets = true;
    
	UPROPERTY()
	TMap<EUIScreen, UUserWidget*> CachedWidgets;
    
	UPROPERTY()
	UUserWidget* CurrentWidget;
    
	EUIScreen CurrentScreen;
	TArray<EUIScreen> ScreenHistory;
    
	UPROPERTY()
	APlayerController* CachedPlayerController;

private:
	UUserWidget* CreateOrGetWidget(EUIScreen ScreenType);
	APlayerController* GetPlayerController();
};
