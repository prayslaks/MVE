#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/AvatarData.h"
#include "MVE_STD_WidgetClass_StudioCharacterCustomize.generated.h"

class UButton;
class UTextBlock;
class UHorizontalBox;
class UImage;
class UAvatarStorageSubsystem;
class UPresetButtonWidget;
class AAvatarPreviewActor;

UCLASS()
class MVE_API UMVE_STD_WidgetClass_StudioCharacterCustomize : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 마우스 이벤트

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

protected:
	// 바인딩된 위젯들
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PreviewImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> PresetContainer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> FileNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> AttachFileButton;

	
	UPROPERTY(EditDefaultsOnly, Category = "Avatar")
	TSubclassOf<UUserWidget> PresetButtonClass;
	
	// 카메라 설정
	UPROPERTY(EditAnywhere, Category = "Preview")
	float RotationSpeed = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Preview")
	float ZoomSpeed = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Preview")
	float MinDistance = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Preview")
	float MaxDistance = 6000.0f;

	UPROPERTY(EditAnywhere, Category = "Preview")
	float DefaultDistance = 300.0f;

private:
	UPROPERTY()
	TObjectPtr<UAvatarStorageSubsystem> StorageSubsystem;

	UPROPERTY()
	TObjectPtr<AAvatarPreviewActor> PreviewActor;

	UPROPERTY()
	FString CurrentSelectedID;

	UPROPERTY()
	TArray<TObjectPtr<UUserWidget>> PresetButtons;

	// 카메라 컨트롤 변수
	bool bIsDragging = false;
	FVector2D LastMousePosition;
	float CurrentYaw = 180.0f;
	float CurrentPitch = 0.0f;
	float CurrentDistance = 300.0f;

	UFUNCTION()
	void OnAttachFileClicked();

	UFUNCTION()
	void OnNextClicked();

	void RefreshPresetList();
	void CreatePresetButton(FAvatarData& Data);
	void OnPresetClicked(const FString& UniqueID);
	void UpdatePreview(const FAvatarData& Data);
	void InitializePreview();
	UTexture2D* CapturePreviewThumbnail();
	void UpdateCameraPosition();
	
	void SetRenderTarget(UTextureRenderTarget2D* InRenderTarget);
};