
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVE_WidgetClass_MainLevel_ModeSelect.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class MVE_API UMVE_WidgetClass_MainLevel_ModeSelect : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	// 유저 이름
	UPROPERTY(meta=(BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> UserNameTextBlock1;
	UPROPERTY(meta=(BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> UserNameTextBlock2;
	UPROPERTY(meta=(BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> UserNameTextBlock3;

	// 버튼
	UPROPERTY(meta=(BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> MoveStudioButton;
	UPROPERTY(meta=(BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> MoveAudienceButton;
	UPROPERTY(meta=(BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> MoveMainButton;
};