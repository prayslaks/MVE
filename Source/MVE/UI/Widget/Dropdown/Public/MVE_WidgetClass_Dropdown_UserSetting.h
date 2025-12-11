
#pragma once

#include "CoreMinimal.h"
#include "MVE_WidgetClass_Dropdown.h"
#include "MVE_WidgetClass_Dropdown_UserSetting.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class MVE_API UMVE_WidgetClass_Dropdown_UserSetting : public UMVE_WidgetClass_Dropdown
{
	GENERATED_BODY()

public:
	// 사용자 이름 설정
	void SetUserName(const FString& InUserName);

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// 드롭다운 항목들
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_UserName;

	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Logout;

	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_MainMenu;


private:
	UFUNCTION()
	void OnLogoutClicked();

	UFUNCTION()
	void OnMainMenuClicked();



	
};
