
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/MVE_STU_PresetTypes.h"
#include "MVE_STU_WC_Preset.generated.h"

class UMVE_STU_WC_PresetCategory;
class UTileView;
class UDataTable;

UCLASS(Config=Game)
class MVE_API UMVE_STU_WC_Preset : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	// 바인딩된 위젯들
	UPROPERTY(meta = (BindWidget))
	UMVE_STU_WC_PresetCategory* CategoryWidget;

	UPROPERTY(meta = (BindWidget))
	UTileView* PresetTileView;

	// 프리셋 DataTable
	UPROPERTY(Config)
	TSoftObjectPtr<UDataTable> PresetDataTable;

	// TileView Entry 위젯 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
	TSubclassOf<UUserWidget> PresetEntryClass;

private:
	// 전체 프리셋 데이터
	UPROPERTY()
	TMap<EPresetCategory, FPresetDataArray> AllPresetItems;

	// 현재 선택된 카테고리
	EPresetCategory CurrentCategory;

	// 로딩 중 플래그
	bool bIsLoading;

	// 데이터 로딩
	void LoadPresetData();

	// 카테고리 변경 처리
	UFUNCTION()
	void OnCategoryChanged(EPresetCategory Category);

	// 프리셋 필터링 및 TileView 업데이트
	void RefreshPresetList(EPresetCategory Category);
};
