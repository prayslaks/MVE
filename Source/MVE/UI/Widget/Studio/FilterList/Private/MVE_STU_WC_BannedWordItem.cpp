
#include "../Public/MVE_STU_WC_BannedWordItem.h"

#include "MVE.h"
#include "UIManagerSubsystem.h"
#include "Components/Button.h"
#include "Data/ScreenTypes.h"

void UMVE_STU_WC_BannedWordItem::SetItemData(const FString& Data)
{
	Super::SetItemData(Data);
}

void UMVE_STU_WC_BannedWordItem::OnItemHovered()
{
	Super::OnItemHovered();
}

void UMVE_STU_WC_BannedWordItem::OnItemUnhovered()
{
	Super::OnItemUnhovered();
}

void UMVE_STU_WC_BannedWordItem::HandleItemClicked()
{
	Super::HandleItemClicked();

	if (!InteractionButton) return;
	
	PRINTLOG(TEXT("HandleItemClicked Called"));
    
	FGeometry ButtonGeometry = InteractionButton->GetCachedGeometry();
	FVector2D ButtonAbsPos = ButtonGeometry.GetAbsolutePosition();
	FVector2D ButtonSize = ButtonGeometry.GetLocalSize();
	
	FVector2D ButtonMiddleLeft = ButtonAbsPos + FVector2D(0, ButtonSize.Y / 2.0f);
    
	if (UUIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UUIManagerSubsystem>())
	{
		FDropdownContext DropdownContext;
		DropdownContext.AnchorPosition = EDropdownAnchorPosition::MiddleLeft;
		DropdownContext.ButtonSize = ButtonSize;
		DropdownContext.ButtonPosition = ButtonMiddleLeft;
		DropdownContext.ObjectData = this;
		
		// 우상단 앵커로 드롭다운 표시
		UIManager->ShowDropdown(EDropdownType::BannedWordItem, DropdownContext);
	}
	
}
