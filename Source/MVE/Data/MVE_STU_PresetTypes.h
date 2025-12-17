#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "MVE_STU_PresetTypes.generated.h"

/**
 * 프리셋 카테고리
 */
UENUM(BlueprintType)
enum class EPresetCategory : uint8
{
	Effect		 	 UMETA(DisplayName = "이펙트"),	
	Lighting         UMETA(DisplayName = "조명"),
	CameraEffect     UMETA(DisplayName = "카메라 효과"),
	StageBackground  UMETA(DisplayName = "무대 배경")
};

/**
 * 프리셋 데이터 구조체
 */
USTRUCT(BlueprintType)
struct FPresetData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
	FText PresetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
	EPresetCategory Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
	TSoftObjectPtr<UTexture2D> ThumbnailImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
	TSoftObjectPtr<UStaticMesh> Asset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
	FGameplayTag ID_tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
	TArray<FString> Tags;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
	FText Description;
};

USTRUCT(BlueprintType)
struct FPresetDataArray
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<UObject*> Values;
};

/**
 * 프리셋 아이템 (ListView/TileView용)
 */
UCLASS(BlueprintType)
class MVE_API UMVEPresetItemData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Preset")
	FPresetData PresetData;
	
};

/**
 * 카테고리 선택 델리게이트
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCategorySelected, EPresetCategory, SelectedCategory);