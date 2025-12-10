
#pragma once

#include "CoreMinimal.h"
#include "UIManagerSubsystem.h"
#include "Engine/DataTable.h"
#include "BroadcastSettingTypes.generated.h"

UENUM(BlueprintType)
enum class EBroadcastSettingTab : uint8
{
	None,
	CharacterSettings,
	StageSettings,
	ChatFilter,
	CheckSettings
};

USTRUCT(BlueprintType)
struct FBroadcastSettingTabInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBroadcastSettingTab TabType = EBroadcastSettingTab::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TabText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EUIScreen TargetScreen = EUIScreen::None;
};