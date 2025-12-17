
#include "MVE_STU_WC_PresetEntry.h"

#include "MVE.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/MVE_STU_PresetTypes.h"

void UMVE_STU_WC_PresetEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	PresetItemData = Cast<UMVEPresetItemData>(ListItemObject);
	if (!PresetItemData)
	{
		PRINTLOG(TEXT("Invalid PresetItemData"));
		return;
	}

	// 프리셋 이름 설정
	if (PresetNameText)
	{
		PresetNameText->SetText(PresetItemData->PresetData.PresetName);
	}

	// 썸네일 이미지 설정
	if (ThumbnailImage)
	{
		if (UTexture2D* Thumbnail = PresetItemData->PresetData.ThumbnailImage.Get())
		{
			ThumbnailImage->SetBrushFromTexture(Thumbnail);
		}
		else
		{
			// 기본 이미지 또는 플레이스홀더 설정
			PRINTLOG(TEXT("Thumbnail not loaded for: %s"), *PresetItemData->PresetData.PresetName.ToString());
		}
	}
}
