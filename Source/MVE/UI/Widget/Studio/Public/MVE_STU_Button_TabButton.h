
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/BroadcastSettingTypes.h"
#include "MVE_STU_Button_TabButton.generated.h"

class UBorder;
class UButton;
class UTextBlock;
struct FBroadcastSettingTabInfo;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabButtonClicked, EBroadcastSettingTab, TabType);

UCLASS()
class MVE_API UMVE_STU_Button_TabButton : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	// 탭 정보 설정
	UFUNCTION(BlueprintCallable)
	void SetupTab(const FBroadcastSettingTabInfo& TabInfo);

	// 선택 상태 설정
	UFUNCTION(BlueprintCallable)
	void SetSelected(bool bInSelected);

	// 선택 상태 확인
	UFUNCTION(BlueprintCallable)
	bool IsSelected() const { return bIsSelected; }

	// 탭 타입 가져오기
	UFUNCTION(BlueprintCallable)
	EBroadcastSettingTab GetTabType() const { return TabType; }

	// 탭 클릭 이벤트
	UPROPERTY(BlueprintAssignable)
	FOnTabButtonClicked OnTabButtonClicked;

protected:
	// 위젯 바인딩
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> BackgroundBorder;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Tab;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_TabName;

	UPROPERTY(EditDefaultsOnly, Category = "TabColor")
	FLinearColor SelectedColor;

	UPROPERTY(EditDefaultsOnly, Category = "TabColor")
	FLinearColor UnselectedColor;

	UFUNCTION()
	void OnTabClicked();

private:
	EBroadcastSettingTab TabType = EBroadcastSettingTab::None;
    
	bool bIsSelected = false;

	void UpdateVisuals();
};
