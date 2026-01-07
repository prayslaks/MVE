
#pragma once

#include "CoreMinimal.h"
#include "AssetTypes.h"
#include "Blueprint/UserWidget.h"
#include "Components/Overlay.h"
#include "Data/RoomInfo.h"
#include "MVE_AUD_WidgetClass_GenerateMesh.generated.h"

class UMVE_AUD_WidgetClass_CharacterPreviewWidget;
class UTextBlock;
class UButton;
class UMultiLineEditableTextBox;
class UImage;

UCLASS()
class MVE_API UMVE_AUD_WidgetClass_GenerateMesh : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool bTestMode = false;

	// 테스트 모드용 Model ID (중계 서버에서 presigned URL 받아오기)
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int32 TestModelId = 18;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
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

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> SaveButton;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> TestButton;
	
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

	UFUNCTION()
	void OnSaveButtonClicked();

	UFUNCTION()
	void OnTestButtonClicked();
	
	// 현재 표시 중인 방 정보
	FRoomInfo CurrentRoomInfo;

	// 다운로드 진행률 콜백
	UFUNCTION()
	void HandleDownloadProgress(FGuid AssetID, int32 BytesReceived, int32 TotalBytes);

	// 생성 요청 응답 콜백
	UFUNCTION()
	void HandleGenerationResponse(bool bSuccess, const FAssetMetadata& Metadata, const FString& ErrorMessage);
	void Download();

	// 테스트 모드: Model Download URL 콜백
	UFUNCTION()
	void HandleGetModelDownloadUrl(bool bSuccess, const FGetModelDownloadUrlResponseData& Data, const FString& ErrorCode);

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

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> StatusTextBlock;

	// 마지막으로 수신한 메타데이터 (부착 시 사용)
	UPROPERTY()
	FAssetMetadata LastReceivedMetadata;

	// 마지막으로 수신한 메시 (SkeletalMesh 또는 StaticMesh)
	UPROPERTY()
	TObjectPtr<UObject> LastReceivedMesh;

	// ========== 로딩 애니메이션 ==========

	/** 로딩 애니메이션 오버레이 이미지 */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> LoadingOverlayImage;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> LoadingBackgroundImage;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> LoadingOverlay;

	/** 로딩 애니메이션 프레임 (GIF를 여러 PNG로 나눠서 배열로 저장) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loading Animation")
	TArray<TObjectPtr<UTexture2D>> LoadingFrames;

	/** 로딩 애니메이션 프레임 전환 속도 (초 단위) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loading Animation")
	float LoadingFrameRate = 0.1f;

	/** 로딩 시작 시 재생할 사운드 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loading Animation")
	TObjectPtr<USoundBase> LoadingStartSound;

	/** 로딩 중 반복 재생할 배경 음악 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loading Animation")
	TObjectPtr<USoundBase> LoadingLoopSound;

	/**
	 * 로딩 애니메이션 시작 (AI 서버 응답 대기 중 표시)
	 */
	UFUNCTION(BlueprintCallable, Category = "Loading Animation")
	void StartLoadingAnimation();

	/**
	 * 로딩 애니메이션 중지
	 */
	UFUNCTION(BlueprintCallable, Category = "Loading Animation")
	void StopLoadingAnimation();

private:
	/** 로딩 애니메이션 타이머 핸들 (프레임 전환용) */
	FTimerHandle LoadingAnimationTimerHandle;

	/** 현재 로딩 프레임 인덱스 */
	int32 CurrentLoadingFrameIndex = 0;

	/** 로딩 애니메이션 활성 여부 */
	bool bIsLoadingAnimationActive = false;

	/** 로딩 애니메이션 프레임 업데이트 */
	void UpdateLoadingFrame();

	/** 로딩 배경음악 오디오 컴포넌트 */
	UPROPERTY()
	TObjectPtr<class UAudioComponent> LoadingAudioComponent;
};

