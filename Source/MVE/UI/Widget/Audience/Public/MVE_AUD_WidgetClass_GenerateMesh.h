
#pragma once

#include "CoreMinimal.h"
#include "AssetTypes.h"
#include "Blueprint/UserWidget.h"
#include "Data/RoomInfo.h"
#include "MVE_AUD_WidgetClass_GenerateMesh.generated.h"

class UMVE_AUD_WidgetClass_CharacterPreviewWidget;
class UTextBlock;
class UButton;
class UMultiLineEditableTextBox;

UCLASS()
class MVE_API UMVE_AUD_WidgetClass_GenerateMesh : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool bTestMode = false;

protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMultiLineEditableTextBox> PromptEditableBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> SendPromptButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UUserWidget> MeshPreviewWidget;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UMVE_AUD_WidgetClass_CharacterPreviewWidget> CharacterPreviewWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> InputImageButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ImportedImageNameTextBlock;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> HeadButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> LeftHandButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> RightHandButton;

	
private:
	UFUNCTION()
	void OnSendPromptButtonClicked();

	UFUNCTION()
	void OnInputImageButtonClicked();

	UFUNCTION()
	void OnCloseButtonClicked();

	UFUNCTION()
	void OnRightHandButtonClicked();

	UFUNCTION()
	void OnLeftHandButtonClicked();

	UFUNCTION()
	void OnHeadButtonClicked();
	
	// 현재 표시 중인 방 정보
	FRoomInfo CurrentRoomInfo;

	// 다운로드 진행률 콜백
	UFUNCTION()
	void HandleDownloadProgress(FGuid AssetID, int32 BytesReceived, int32 TotalBytes);

	// 생성 요청 응답 콜백
	UFUNCTION()
	void HandleGenerationResponse(bool bSuccess, const FAssetMetadata& Metadata, const FString& ErrorMessage);
	void Download();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenAI Test|Settings")
	FString TestServerURL = TEXT("http://localhost:8000");

	// 에셋 로드 완료 콜백
	UFUNCTION()
	void HandleAssetLoaded(UObject* Asset, const FAssetMetadata& Metadata);

	// 상태 텍스트 업데이트
	void SetStatus(const FString& Message);
    
	// 버튼 활성화/비활성화
	void SetButtonsEnabled(bool bEnabled);

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> StatusTextBlock; 

	// 마지막으로 수신한 메타데이터 (부착 시 사용)
	UPROPERTY()
	FAssetMetadata LastReceivedMetadata;
    
	// 마지막으로 수신한 메시 (SkeletalMesh 또는 StaticMesh)
	UPROPERTY()
	TObjectPtr<UObject> LastReceivedMesh;
	
};

