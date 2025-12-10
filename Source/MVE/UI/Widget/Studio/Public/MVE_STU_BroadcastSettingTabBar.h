
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/BroadcastSettingTypes.h"
#include "MVE_STU_BroadcastSettingTabBar.generated.h"

class UMVE_STU_Button_TabButton;
class UHorizontalBox;
class UButton;

UCLASS(Config=Game)
class MVE_API UMVE_STU_BroadcastSettingTabBar : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	// 초기화
	UFUNCTION(BlueprintCallable)
	void InitializeTabs();

	// 특정 탭 선택 (시각적 상태만)
	UFUNCTION(BlueprintCallable)
	void SetActiveTab(EBroadcastSettingTab TabType);

	// 현재 활성 탭
	UFUNCTION(BlueprintCallable)
	EBroadcastSettingTab GetActiveTab() const { return ActiveTab; }

protected:
	// 탭 데이터테이블
	UPROPERTY(Config)
	TSoftObjectPtr<UDataTable> TabDataTableAsset;

	// 탭 버튼 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UMVE_STU_Button_TabButton> TabButtonClass;

	// 탭 색상 설정 (모든 탭 공통)
	UPROPERTY(EditDefaultsOnly, Category = "UI|Colors")
	FLinearColor SelectedColor = FLinearColor(0.0f, 0.7f, 1.0f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "UI|Colors")
	FLinearColor UnselectedColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

	// 위젯 바인딩
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> HBox_Tabs;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> HBox_Right;


private:
	// 탭 정보 맵
	TMap<EBroadcastSettingTab, FBroadcastSettingTabInfo> TabInfoMap;

	// 생성된 탭 버튼들
	UPROPERTY()
	TArray<TObjectPtr<UMVE_STU_Button_TabButton>> TabButtons;

	// 현재 활성 탭
	EBroadcastSettingTab ActiveTab = EBroadcastSettingTab::None;

	// 탭 클릭 핸들러
	UFUNCTION()
	void OnTabClicked(EBroadcastSettingTab TabType);

	// 탭 데이터 로드
	void LoadTabData();

	// 탭 버튼 생성
	void CreateTabButtons();

	// 화면 전환 (UIManagerSubsystem 사용)
	void SwitchToScreen(EBroadcastSettingTab TabType);
	
};
