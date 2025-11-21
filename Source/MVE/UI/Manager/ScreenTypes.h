#pragma once

#include "CoreMinimal.h"
#include "UIManagerSubsystem.h"
#include "ScreenTypes.generated.h"

USTRUCT(BlueprintType)
struct FScreenClassInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EUIScreen Screen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> WidgetClass;
};