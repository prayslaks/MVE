#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/AvatarData.h"
#include "PresetButtonWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;

DECLARE_DELEGATE_OneParam(FOnPresetButtonClicked, const FString&);

UCLASS()
class MVE_API UPresetButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	FOnPresetButtonClicked OnButtonClicked;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PresetButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>  ThumbnailImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> FileNameText;

	UFUNCTION(BlueprintCallable)
	void SetAvatarData(FAvatarData& InData);

	FAvatarData AvatarData;

	UFUNCTION()
	void OnClicked();
};