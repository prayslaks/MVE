#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/AvatarData.h"
#include "PresetButtonWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;

DECLARE_DELEGATE_OneParam(FOnPresetClicked, const FString&);

UCLASS()
class MVE_API UPresetButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void SetAvatarData(const FAvatarData& InData);

	FOnPresetClicked OnButtonClicked;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MainButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ThumbnailImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NameText;

private:
	FAvatarData AvatarData;

	UFUNCTION()
	void OnClicked();
};