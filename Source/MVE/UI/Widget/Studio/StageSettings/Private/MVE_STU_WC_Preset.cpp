
#include "../Public/MVE_STU_WC_Preset.h"

#include "MVE.h"
#include "MVE_STU_WC_PresetCategory.h"
#include "Components/TileView.h"
#include "Data/MVE_STU_PresetTypes.h"

void UMVE_STU_WC_Preset::NativeConstruct()
{
	Super::NativeConstruct();

	CurrentCategory = EPresetCategory::Lighting;
	bIsLoading = false;

	// 카테고리 위젯 델리게이트 바인딩
	if (CategoryWidget)
	{
		CategoryWidget->OnCategorySelected.AddDynamic(this, &UMVE_STU_WC_Preset::OnCategoryChanged);
	}
	else
	{
		PRINTLOG(TEXT("CategoryWidget is nullptr"));
	}

	// 프리셋 데이터 로딩 시작
	LoadPresetData();
}

void UMVE_STU_WC_Preset::NativeDestruct()
{
	Super::NativeDestruct();

	// 델리게이트 해제
	if (CategoryWidget)
	{
		CategoryWidget->OnCategorySelected.RemoveDynamic(this, &UMVE_STU_WC_Preset::OnCategoryChanged);
	}
}

void UMVE_STU_WC_Preset::LoadPresetData()
{
	bIsLoading = true;
	PRINTLOG(TEXT("Loading preset data..."));

	UDataTable* DT = PresetDataTable.LoadSynchronous();

	for (auto& It : DT->GetRowMap())
	{
		const FPresetData* Row = DT->FindRow<FPresetData>(
			It.Key, TEXT("UMVE_STU_WC_Preset::LoadPresetData"), true);

		if (!Row) continue;

		UMVEPresetItemData* ItemObject = NewObject<UMVEPresetItemData>(this);
		ItemObject->PresetData = *Row;

		FPresetDataArray& Arr = AllPresetItems.FindOrAdd(Row->Category);
		Arr.Values.Add(ItemObject);
	}
}

void UMVE_STU_WC_Preset::OnCategoryChanged(EPresetCategory Category)
{
	PRINTLOG(TEXT("Category changed to: %d"), static_cast<int32>(Category));
    
	CurrentCategory = Category;
	RefreshPresetList(Category);
}

void UMVE_STU_WC_Preset::RefreshPresetList(EPresetCategory Category)
{
	if (!PresetTileView)
	{
		PRINTLOG(TEXT("PresetTileView is nullptr"));
		return;
	}

	const FPresetDataArray* ArrayPtr = AllPresetItems.Find(Category);
	if (!ArrayPtr) return;

	// TileView 업데이트
	PresetTileView->SetListItems(ArrayPtr->Values);
}
