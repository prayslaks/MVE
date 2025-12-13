
#include "../Public/MVE_WC_Dropdown_ListItem.h"

#include "UIManagerSubsystem.h"
#include "Components/Button.h"
#include "UI/Widget/Studio/FilterList/Public/MVE_STU_WC_FilterableListItem.h"

void UMVE_WC_Dropdown_ListItem::NativeConstruct()
{
	Super::NativeConstruct();

	if (EditButton)
		EditButton.Get()->OnClicked.AddDynamic(this, &UMVE_WC_Dropdown_ListItem::OnEditButtonClicked);

	if (DeleteButton)
		DeleteButton.Get()->OnClicked.AddDynamic(this, &UMVE_WC_Dropdown_ListItem::OnDeleteButtonClicked);
}

void UMVE_WC_Dropdown_ListItem::SetOwnerItem(UMVE_STU_WC_FilterableListItem* InOwnerItem)
{
	OwnerItem = InOwnerItem;
}

void UMVE_WC_Dropdown_ListItem::OnEditButtonClicked()
{
	if (OwnerItem)
	{
		OwnerItem->RequestEdit();
	}

	// 드롭다운 닫기
	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UIManager->CloseDropdown();
	}
}

void UMVE_WC_Dropdown_ListItem::OnDeleteButtonClicked()
{
	if (OwnerItem)
	{
		OwnerItem->RequestDelete();
	}

	// 드롭다운 닫기
	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UIManager->CloseDropdown();
	}
}
