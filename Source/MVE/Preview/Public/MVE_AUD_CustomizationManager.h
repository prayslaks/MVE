
#pragma once
#include "CoreMinimal.h"
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
	void RequestModelGeneration(const FString& PromptText);
	
	UFUNCTION(BlueprintCallable)
	FString GetReferenceImageDataAsBase64() const 
	{ 
		return FBase64::Encode(ReferenceImageData); 
	}
	
private:
	// 현재 첨부된 참고 이미지
	TArray<uint8> ReferenceImageData;
	FString ReferenceImageFormat;
	FString ReferenceImageFileName;

	// 이미지 파일 로드
	bool LoadReferenceImage(const FString& FilePath);

	// HTTP 요청으로 서버에 전송
	void SendToExternalServer(const FInputPromptData& Request);

	// HTTP 응답 콜백
	void OnModelGenerationResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);

	// 생성 완료 처리
	void OnModelGenerationComplete(const FString& ModelID, const FString& GLBFileURL);


	
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

	// 프리뷰 캐릭터 가져오기 (GameMode에서)
	UFUNCTION(BlueprintCallable, Category = "Customization")
	AActor* GetPreviewCharacter() const;

	// 저장된 커스터마이징 데이터 가져오기
	UFUNCTION(BlueprintCallable, Category = "Customization")
	FCustomizationData GetSavedCustomization() const { return SavedCustomization; }

	// Transform 저장
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void SaveAccessoryTransform(AActor* Accessory, const FTransform& NewTransform);
	
private:
	// 프리뷰용 캐릭터 생성
	AActor* SpawnPreviewCharacter();
	
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
    
	// GLB 로딩 완료 콜백
	void OnMeshLoaded(AActor* LoadedActor);
    
	// 바운딩 박스 기반 자동 카메라 거리 조정
	void AutoAdjustCameraDistance();
	
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

	// 현재 로드된 GLB 파일 경로
	FString CurrentGLBFilePath;

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
	
	// JSON 직렬화/역직렬화
	UFUNCTION(BlueprintCallable, Category = "Customization")
	FString SerializeCustomizationData(const FCustomizationData& Data) const;
	
	UFUNCTION(BlueprintCallable, Category = "Customization")
	FCustomizationData DeserializeCustomizationData(const FString& JsonString) const;
	
	// 중계서버에 프리셋 저장
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void SavePresetToServer(const FCustomizationData& Data);

private:
	// HTTP 응답 콜백
	void OnSavePresetResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);
	void OnLoadPresetResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);

public:
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void SetRemoteModelUrl(const FString& RemoteUrl);
    
private:
	FString CurrentRemoteURL;        // PresignedURL (네트워크 동기화용)
	
	
};
