
#pragma once

#include "CoreMinimal.h"
#include "MVE_STU_WC_FilterableListItem.h"
#include "MVE_STU_WC_BannedWordItem.generated.h"

UCLASS()
class MVE_API UMVE_STU_WC_BannedWordItem : public UMVE_STU_WC_FilterableListItem
{
	GENERATED_BODY()

public:
	virtual void SetItemData(const FString& Data) override;

protected:
	virtual void OnItemHovered() override;
	virtual void OnItemUnhovered() override;
	virtual void OnItemClicked() override;
};
