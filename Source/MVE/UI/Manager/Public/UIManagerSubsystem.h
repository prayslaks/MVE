
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UIManagerSubsystem.generated.h"

class UMVE_WidgetClass_ModalBackgroundWidget;

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
	AudienceStation,
	AudienceConcertRoom
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

	UPROPERTY(Config, EditDefaultsOnly, Category = "DataTable")
	TSoftObjectPtr<UDataTable> PopUpClassesTableAsset;

	void InitScreenClasses();

	// 모달 배경 위젯
	UPROPERTY()
	TObjectPtr<UMVE_WidgetClass_ModalBackgroundWidget> ModalBackground;

	// 모달 배경 위젯 클래스
	UPROPERTY()
	TSubclassOf<UMVE_WidgetClass_ModalBackgroundWidget> ModalBackgroundClass;
    
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

	// 팝업 표시 (동적 생성)
	UFUNCTION(BlueprintCallable)
	UUserWidget* ShowPopup(FName PopupName, bool bShowModalBackground = true);
    
	// 최상위 팝업 닫기
	UFUNCTION(BlueprintCallable)
	void CloseTopPopup();
    
	// 특정 팝업 닫기
	UFUNCTION(BlueprintCallable)
	void ClosePopup(UUserWidget* PopupWidget);
    
	// 모든 팝업 닫기
	UFUNCTION(BlueprintCallable)
	void CloseAllPopups();
	
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

	// 팝업 스택 관리
	UPROPERTY()
	TArray<TObjectPtr<UUserWidget>> PopupStack;

	// 팝업 위젯 클래스 맵
	UPROPERTY()
	TMap<FName, TSubclassOf<UUserWidget>> PopupWidgetClasses;

	// 모달 배경 표시
	void ShowModalBackground();

	// 모달 배경 숨기기
	void HideModalBackground();

	// 모달 배경 클릭 처리
	void OnModalBackgroundClicked();
};
