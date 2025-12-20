#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_WC_StageLevel_AudInputHelp.generated.h"

class UTextBlock;
class UImage;
class UHorizontalBox;

UENUM(BlueprintType)
enum class EAudienceInputHelpState : uint8
{
	Unvisible = 0,
	Selectable = 1,
	Executable = 2,
	Aimable = 3
};

UCLASS()
class MVE_API UMVE_WC_StageLevel_AudInputHelp : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetInputHelpState(EAudienceInputHelpState NewState);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UHorizontalBox> FKeyHelpBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> FKeyHelpImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> FKeyHelpTextBlock;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UHorizontalBox> MouseRightButtonHelpBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> MouseRightButtonHelpImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> MouseRightButtonHelpTextBlock;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UHorizontalBox> MouseLeftButtonHelpBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> MouseLeftButtonHelpImage;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> MouseLeftButtonHelpTextBlock;

private:
	EAudienceInputHelpState CurrentState = EAudienceInputHelpState::Unvisible;
};
