#include "PresetButtonWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UPresetButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (MainButton)
	{
		MainButton->OnClicked.AddDynamic(this, &UPresetButtonWidget::OnClicked);
	}
}

void UPresetButtonWidget::SetAvatarData(const FAvatarData& InData)
{
	AvatarData = InData;

	if (NameText)
	{
		NameText->SetText(FText::FromString(AvatarData.FileName));
	}

	// TODO: 썸네일 이미지 설정
}

void UPresetButtonWidget::OnClicked()
{
	OnButtonClicked.ExecuteIfBound(AvatarData.UniqueID);
}