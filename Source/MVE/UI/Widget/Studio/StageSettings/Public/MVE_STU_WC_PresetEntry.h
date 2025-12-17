
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "MVE_STU_WC_PresetEntry.generated.h"


class UImage;
class UTextBlock;

/**
 * 프리셋 TileView Entry 위젯
 */

UCLASS()
class MVE_API UMVE_STU_WC_PresetEntry : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	// IUserObjectListEntry 인터페이스 구현
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* ThumbnailImage;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PresetNameText;

private:
	UPROPERTY()
	class UMVEPresetItemData* PresetItemData;
};
