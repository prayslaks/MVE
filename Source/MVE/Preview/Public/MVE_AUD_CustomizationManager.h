
#pragma once
#include "CoreMinimal.h"
#include "MVE_API_ResponseData.h"
#include "Data/InputPromptData.h"
#include "Interfaces/IHttpRequest.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MVE_AUD_CustomizationManager.generated.h"


class UglTFRuntimeAsset;
class AMVE_AUD_PreviewCaptureActor;
class UMVE_AUD_WidgetClass_PreviewWidget;

USTRUCT(BlueprintType)
struct FCustomizationData
{
	GENERATED_BODY()

	UPROPERTY()
	FString SocketName;
	
	UPROPERTY()
	FVector RelativeLocation;

	UPROPERTY()
	FRotator RelativeRotation;

	UPROPERTY()
	float RelativeScale;

	UPROPERTY()
	FString ModelUrl;

	FCustomizationData()
		: SocketName(TEXT(""))
		, RelativeLocation(FVector::ZeroVector)
		, RelativeRotation(FRotator::ZeroRotator)
		, RelativeScale(1.0f)
		, ModelUrl(TEXT(""))
	{}
};

UCLASS()
class MVE_API UMVE_AUD_CustomizationManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 이미지 첨부 다이얼로그
	UFUNCTION(BlueprintCallable)
	FString OpenReferenceImageDialog();

	// 생성 요청
	UFUNCTION(BlueprintCallable)
	void RequestModelGeneration(const FString& PromptText, const FString& ImagePath, bool bIsThrowMesh = false);
	
	UFUNCTION(BlueprintCallable)
	FString GetReferenceImageDataAsBase64() const
	{
		return FBase64::Encode(ReferenceImageData);
	}

	// 참조 이미지 파일 경로 반환
	UFUNCTION(BlueprintCallable)
	FString GetReferenceImageFilePath() const
	{
		return ReferenceImageFilePath;
	}
	
private:
	// 현재 첨부된 참고 이미지
	TArray<uint8> ReferenceImageData;
	FString ReferenceImageFormat;
	FString ReferenceImageFileName;
	FString ReferenceImageFilePath;  // 이미지 파일 경로 저장

	// 이미지 파일 로드
	bool LoadReferenceImage(const FString& FilePath);

	// AI 모델 생성 응답 콜백
	void OnGenerateModelComplete(bool bSuccess, const FGenerateModelResponseData& ResponseData, const FString& ErrorCode);

	// 모델 생성 상태 폴링
	void CheckModelGenerationStatus();
	void OnGetModelStatusComplete(bool bSuccess, const FGetJobStatusResponseData& ResponseData, const FString& ErrorCode);

	// 모델 다운로드
	void OnModelDownloadComplete(bool bSuccess, const FGetModelDownloadUrlResponseData& Data, const FString& SavedPath);

	// 현재 진행 중인 JobId
	FString CurrentJobId;

	// 상태 확인 타이머
	FTimerHandle ModelStatusCheckTimer;

	// 상태 확인 주기 (초 단위)
	float StatusCheckInterval = 2.0f;

	// 현재 생성 중인 메시가 던지기 메시인지 여부
	bool bCurrentGenerationIsThrowMesh = false;


	/*
	 * 프리뷰 위젯
	 */

public:
	
	// 캐릭터에 메시 부착
	UFUNCTION(BlueprintCallable)
	void AttachMeshToSocket(const FName& SocketName);

	// 캐릭터에 부착된 메시 제거
	UFUNCTION(BlueprintCallable)
	void RemoveMesh();
	
	// 메시 프리뷰 시작
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void StartMeshPreview(const FString& GLBFilePath, UMVE_AUD_WidgetClass_PreviewWidget* InPreviewWidget);

	// 프리뷰 종료
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void StopMeshPreview();

	// 생성된 메시 액터 가져오기
	UFUNCTION(BlueprintCallable, Category = "Customization")
	AActor* GetPreviewedMesh() const { return PreviewedMesh; }

	// ========== 던지기 메시 프리뷰 ==========
	// 던지기 메시 프리뷰 시작
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void StartThrowMeshPreview(const FString& GLBFilePath, UMVE_AUD_WidgetClass_PreviewWidget* InPreviewWidget);

	// 던지기 프리뷰 종료
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void StopThrowMeshPreview();

	// 던지기 메시 가져오기
	UFUNCTION(BlueprintCallable, Category = "Customization")
	UStaticMesh* GetThrowMesh() const { return ThrowMesh; }

	// 던지기 메시 직접 설정 (멀티플레이 동기화용)
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void SetThrowMeshDirect(UStaticMesh* NewMesh) { ThrowMesh = NewMesh; }

	// 프리뷰 캐릭터 가져오기 (GameMode에서)
	UFUNCTION(BlueprintCallable, Category = "Customization")
	AActor* GetPreviewCharacter() const;

	// 저장된 커스터마이징 데이터 가져오기
	UFUNCTION(BlueprintCallable, Category = "Customization")
	FCustomizationData GetSavedCustomization() const { return SavedCustomization; }

	// 저장된 던지기 메시 데이터 가져오기
	UFUNCTION(BlueprintCallable, Category = "Customization")
	FCustomizationData GetSavedThrowMeshData() const { return SavedThrowMeshData; }

	// Transform 저장
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void SaveAccessoryTransform(AActor* Accessory, const FTransform& NewTransform);

	// 테스트용: 로컬 GLB 파일로 프리뷰 + 가짜 RemoteURL 설정
	UFUNCTION(BlueprintCallable, Category = "Customization|Test")
	void TestLoadLocalGLBWithFakeURL(const FString& LocalGLBPath, const FString& FakeRemoteURL, UMVE_AUD_WidgetClass_PreviewWidget* InPreviewWidget);

	// 현재 로드된 GLB 파일 경로
	FString CurrentGLBFilePath;
	
private:
	// 프리뷰용 캐릭터 생성
	AActor* SpawnPreviewCharacter();
	
	// ========== 액세서리용 프리뷰 (기존) ==========
	// Render Target (에디터에서 만든 RT)
	UPROPERTY()
	UTextureRenderTarget2D* MeshRenderTarget;

	// Scene Capture Actor
	UPROPERTY()
	AMVE_AUD_PreviewCaptureActor* MeshCaptureActor;

	// 프리뷰 중인 액세서리 액터
	UPROPERTY()
	AActor* PreviewedMesh;

	// 프리뷰 위젯 참조
	UPROPERTY()
	UMVE_AUD_WidgetClass_PreviewWidget* MeshPreviewWidget;

	// ========== 던지기용 프리뷰 (새로 추가) ==========
	// 던지기 메시용 Render Target
	UPROPERTY()
	UTextureRenderTarget2D* ThrowMeshRenderTarget;

	// 던지기 메시용 Scene Capture Actor
	UPROPERTY()
	AMVE_AUD_PreviewCaptureActor* ThrowMeshCaptureActor;

	// 던지기 메시 프리뷰 중인 액터
	UPROPERTY()
	AActor* ThrowPreviewedMesh;

	// 던지기 메시 프리뷰 위젯 참조
	UPROPERTY()
	UMVE_AUD_WidgetClass_PreviewWidget* ThrowMeshPreviewWidget;

	// 던지기용 메시 (캐싱)
	UPROPERTY()
	UStaticMesh* ThrowMesh;
    
	// GLB 로딩 완료 콜백
	void OnMeshLoaded(AActor* LoadedActor);

	// 던지기 메시 GLB 로딩 완료 콜백
	void OnThrowMeshLoaded(AActor* LoadedActor);

	// 바운딩 박스 기반 자동 카메라 거리 조정
	void AutoAdjustCameraDistance();

	// 던지기 메시용 카메라 거리 조정
	void AutoAdjustThrowCameraDistance();
	
	// GLB 파일 로딩 (비동기)
	void LoadMeshFromGLB(const FString& GLBFilePath, TFunction<void(AActor*)> OnLoadComplete);
    
	// glTFRuntime 에셋 로딩 완료 콜백
	UFUNCTION()
	void OnGLTFAssetLoaded(UglTFRuntimeAsset* Asset);
    
	// 로딩 완료 시 호출할 콜백 저장
	TFunction<void(AActor*)> LoadCompleteCallback;
	
	UPROPERTY()
	TObjectPtr<AActor> AttachedMesh;  // 캐릭터에 부착된 액세서리

	// 저장된 커스터마이징 데이터 (StageLevel에서 적용)
	UPROPERTY()
	FCustomizationData SavedCustomization;

	// 던지기 메시 데이터 (별도 저장)
	UPROPERTY()
	FCustomizationData SavedThrowMeshData;

	// 서버에 저장된 프리셋 ID (-1이면 아직 저장 안 함)
	UPROPERTY()
	int32 CachedPresetId = -1;

private:
	// 캐릭터 대비 메시의 최대 크기 비율 (예: 0.3 = 캐릭터의 30%)
	UPROPERTY(EditAnywhere, Category = "Preview")
	float MaxMeshSizeRatio = 0.1f;
    
	// 메시를 캐릭터 크기에 맞게 조정
	void ScaleMeshToCharacter();
    
	// 캐릭터 크기 가져오기
	FVector GetCharacterSize() const;

	// 저장된 Transform (나중에 "적용" 버튼 눌렀을 때 사용)
	UPROPERTY()
	TMap<AActor*, FTransform> SavedTransforms;

	/**
	 * 직렬화
	 */

public:
	// 프리셋 로드 완료 델리게이트
	DECLARE_DELEGATE_OneParam(FOnPresetLoaded, const FCustomizationData&);
	FOnPresetLoaded OnPresetLoadedDelegate;

	// 모델 생성 완료 델리게이트 (UI 업데이트용 - 로딩 애니메이션 중지 등)
	// Params: bSuccess, RemoteURL (PresignedURL)
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnModelGenerationComplete, bool /* bSuccess */, const FString& /* RemoteURL */);
	FOnModelGenerationComplete OnModelGenerationComplete;
	
	// JSON 직렬화/역직렬화
	UFUNCTION(BlueprintCallable, Category = "Customization")
	FString SerializeCustomizationData(const FCustomizationData& Data) const;
	
	UFUNCTION(BlueprintCallable, Category = "Customization")
	FCustomizationData DeserializeCustomizationData(const FString& JsonString) const;
	
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void SaveAccessoryPresetToServer(const FString& PresetName = TEXT("MyAccessory"));

	void HandleSavePresetComplete(bool bSuccess, const FSavePresetResponseData& Data, const FString& ErrorCode);

	// 서버에서 액세서리 프리셋 로드
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void LoadAccessoryPresetFromServer();

	void HandleLoadPresetComplete(bool bSuccess, const FGetPresetListResponseData& Data, const FString& ErrorCode);

	// ========== 던지기 메시 저장/로드 ==========
	// 던지기 메시 프리셋 서버에 저장
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void SaveThrowMeshPreset(const FString& ModelUrl);

	// 서버에서 던지기 메시 프리셋 로드
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void LoadThrowMeshPreset();

	// 던지기 메시 URL에서 GLB 다운로드 및 로드
	void LoadThrowMeshFromURL(const FString& ModelUrl);

private:
	// HTTP 응답 콜백 (Deprecated - HandleLoadPresetComplete 사용)
	void OnLoadPresetResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);

public:
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void SetRemoteModelUrl(const FString& RemoteUrl);
    
private:
	FString CurrentRemoteURL;        // PresignedURL (네트워크 동기화용)
	
	
};
