#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Data/AvatarData.h"
#include "PresetButtonWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;

DECLARE_DELEGATE_OneParam(FOnPresetButtonClicked, const FString&);

UCLASS()
class MVE_API UPresetButtonWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// IUserObjectListEntry 인터페이스 구현
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	FOnPresetButtonClicked OnButtonClicked;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PresetButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>  ThumbnailImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> FileNameText;

	// 내부용 - TileView에서 자동 호출됨
	void SetAvatarData(const FAvatarData& InData);

	FAvatarData AvatarData;

	UFUNCTION()
	void OnClicked();
};