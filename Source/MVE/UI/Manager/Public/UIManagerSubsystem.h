
#pragma once

#include "CoreMinimal.h"
#include "Data/ScreenTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UIManagerSubsystem.generated.h"

class UMVE_WidgetClass_DropdownOverlay;
class UMVE_WidgetClass_Dropdown_UserSetting;
class UMVE_WidgetClass_ModalBackgroundWidget;
class UMVE_WidgetClass_Dropdown;

UCLASS(Config=Game)
class MVE_API UUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable)
	static UUIManagerSubsystem* Get(const UObject* WorldContextObject);

	UPROPERTY(Config, EditDefaultsOnly, Category = "DataTable")
	TSoftObjectPtr<UDataTable> UIClassesTableAsset;

	UPROPERTY(Config, EditDefaultsOnly, Category = "DataTable")
	TSoftObjectPtr<UDataTable> PopUpClassesTableAsset;

	UPROPERTY(Config, EditDefaultsOnly, Category = "DataTable")
	TSoftObjectPtr<UDataTable> PersistentWidgetClassesTableAsset;

	UPROPERTY(Config, EditDefaultsOnly, Category = "DataTable")
	TSoftObjectPtr<UDataTable> DropdownClassesTableAsset;
	
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

	UFUNCTION(BlueprintCallable)
	UUserWidget* GetCurrentWidget();
    
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
	UUserWidget* ShowPopup(EUIPopup PopupType, bool bShowModalBackground = true);
    
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
	TMap<EUIPopup, TSubclassOf<UUserWidget>> PopupWidgetClasses;

	// 모달 배경 표시
	void ShowModalBackground();

	// 모달 배경 숨기기
	void HideModalBackground();

	// 모달 배경 클릭 처리
	void OnModalBackgroundClicked();

	/*
	 * --------------- Persistent Widget ------------------- 
	 */
public:
	
	// Persistent Widget 표시
	UFUNCTION(BlueprintCallable)
	void ShowPersistentWidget(EPersistentWidget WidgetType);

	// Persistent Widget 숨기기
	UFUNCTION(BlueprintCallable)
	void HidePersistentWidget(EPersistentWidget WidgetType);

	// 특정 Persistent Widget이 표시 중인지 확인
	UFUNCTION(BlueprintCallable)
	bool IsPersistentWidgetVisible(EPersistentWidget WidgetType) const;

	
private:
	// Screen 전환 시 Persistent Widget 자동 관리
	void UpdatePersistentWidgets(EUIScreen NewScreen);
	
	// Persistent Widget 클래스 맵
	UPROPERTY()
	TMap<EPersistentWidget, TSubclassOf<UUserWidget>> PersistentWidgetClasses;

	// Persistent Widget 정보 맵
	UPROPERTY()
	TMap<EPersistentWidget, FPersistentWidgetClassInfo> PersistentWidgetInfos;

	// 현재 활성화된 Persistent Widget들
	UPROPERTY()
	TMap<EPersistentWidget, TObjectPtr<UUserWidget>> ActivePersistentWidgets;
	
	/**
	 * --------------- Dropdown Widget ------------------- 
	 */

public:
	UFUNCTION(BlueprintCallable, Category = "Dropdown")
	void ShowDropdown(EDropdownType DropdownType, const FDropdownContext& Context);
	
	// 드롭다운 닫기
	UFUNCTION(BlueprintCallable)
	void CloseDropdown();

	

protected:
	UPROPERTY()
	TMap<EDropdownType, TSubclassOf<UMVE_WidgetClass_Dropdown>> DropdownClassMap;
	
	UPROPERTY(Config)
	TSubclassOf<UMVE_WidgetClass_Dropdown_UserSetting> UserDropdownWidgetClass;

	UPROPERTY(Config)
	TSubclassOf<UMVE_WidgetClass_DropdownOverlay> DropdownOverlayClass;

	// 드롭다운 타입별 초기화
	void InitializeDropdown(EDropdownType Type, UMVE_WidgetClass_Dropdown* Dropdown, const FDropdownContext& Context);
	
private:
	// 현재 열려있는 드롭다운
	UPROPERTY()
	TObjectPtr<UMVE_WidgetClass_Dropdown> CurrentDropdown;

	// 드롭다운 오버레이
	UPROPERTY()
	TObjectPtr<UMVE_WidgetClass_DropdownOverlay> DropdownOverlay;

	UFUNCTION()
	void OnOverlayClicked();
};
