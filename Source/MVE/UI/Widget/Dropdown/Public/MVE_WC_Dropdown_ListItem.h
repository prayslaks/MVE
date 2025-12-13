
#pragma once

#include "CoreMinimal.h"
#include "MVE_WidgetClass_Dropdown.h"
#include "MVE_WC_Dropdown_ListItem.generated.h"


class UMVE_STU_WC_FilterableListItem;
class UButton;

UCLASS()
class MVE_API UMVE_WC_Dropdown_ListItem : public UMVE_WidgetClass_Dropdown
{
	GENERATED_BODY()

public:
	void SetOwnerItem(UMVE_STU_WC_FilterableListItem* InOwnerItem);


protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> EditButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> DeleteButton;

	UPROPERTY()
	TObjectPtr<UMVE_STU_WC_FilterableListItem> OwnerItem;
	
private:
	UFUNCTION()
	void OnEditButtonClicked();

	UFUNCTION()
	void OnDeleteButtonClicked();
};
