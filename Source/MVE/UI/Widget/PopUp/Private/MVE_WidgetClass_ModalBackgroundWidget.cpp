
#include "../Public/MVE_WidgetClass_ModalBackgroundWidget.h"

#include "UIManagerSubsystem.h"
#include "Components/Button.h"

void UMVE_WidgetClass_ModalBackgroundWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 기본 블러 설정
	if (BackgroundBlur)
	{
		SetBlurStrength(10.0f);
		SetBackgroundDarkness(0.5f);
	}

	// 배경 클릭 시 이벤트
	if (DismissButton)
	{
		DismissButton.Get()->OnClicked.AddDynamic(this, &UMVE_WidgetClass_ModalBackgroundWidget::OnDismissButtonClicked);
	}
}

void UMVE_WidgetClass_ModalBackgroundWidget::SetBlurStrength(float Strength)
{
}

void UMVE_WidgetClass_ModalBackgroundWidget::SetBackgroundDarkness(float Darkness)
{
}

void UMVE_WidgetClass_ModalBackgroundWidget::OnDismissButtonClicked()
{
	if (UUIManagerSubsystem* US = UUIManagerSubsystem::Get(this))
	{
		US->CloseTopPopup();
	}
}
