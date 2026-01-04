
#include "../Public/MVE_WidgetClass_Dropdown_UserSetting.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UMVE_WidgetClass_Dropdown_UserSetting::NativeConstruct()
{
	Super::NativeConstruct();
}

FReply UMVE_WidgetClass_Dropdown_UserSetting::NativeOnMouseButtonDown(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	// 드롭다운 외부 클릭시 닫기 (이벤트 전파는 계속)
	return FReply::Handled();
}

void UMVE_WidgetClass_Dropdown_UserSetting::SetUserName(const FString& InUserName)
{
	if (Text_UserName)
	{
		Text_UserName->SetText(FText::FromString(InUserName));
	}
}

