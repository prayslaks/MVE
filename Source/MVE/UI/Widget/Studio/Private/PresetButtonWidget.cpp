#include "PresetButtonWidget.h"
#include "MVE.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/AvatarDataObject.h"

void UPresetButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (PresetButton)
	{
		PresetButton->OnClicked.AddDynamic(this, &UPresetButtonWidget::OnClicked);
	}
}

void UPresetButtonWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	UAvatarDataObject* DataObject = Cast<UAvatarDataObject>(ListItemObject);
	if (DataObject)
	{
		SetAvatarData(DataObject->AvatarData);
	}
}


void UPresetButtonWidget::SetAvatarData(const FAvatarData& InData)
{
	AvatarData = InData;

	if (ThumbnailImage && InData.ThumbnailTexture)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(InData.ThumbnailTexture);
		Brush.ImageSize = FVector2D(35,35);
		Brush.DrawAs = ESlateBrushDrawType::Image;

		ThumbnailImage->SetBrush(Brush);
	}
	else if (ThumbnailImage)
	{
		PRINTLOG(TEXT("디버깅 하러 가쇼 없오"))
	}
	if (FileNameText)
	{
		FileNameText->SetText(FText::FromString(InData.FileName));
	}
}

void UPresetButtonWidget::OnClicked()
{
	
	if (OnButtonClicked.IsBound())OnButtonClicked.ExecuteIfBound(AvatarData.UniqueID);
}
