// Source/MVE/TestGenAIActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AssetTypes.h"
#include "TestGenAIActor.generated.h"

/**
 * GenAI 송수신 시스템 테스트 액터
 * 
 * [테스트 시나리오]
 * 1. TestLoadLocalImage  - 로컬 PNG 파일 → UTexture2D 변환 테스트
 * 2. TestLoadLocalMesh   - 로컬 GLB 파일 → USkeletalMesh 변환 테스트
 * 3. TestSendRequest     - ComfyUI 서버에 생성 요청 전송 테스트
 * 4. TestSendWithImage   - 이미지 포함 생성 요청 테스트
 * 5. TestDownload        - 파일 서버에서 다운로드 테스트
 * 
 * [사용법]
 * 1. 레벨에 배치
 * 2. Details 패널에서 TestImagePath, TestMeshPath 설정
 * 3. bAutoTest = true 설정하거나 블루프린트에서 함수 호출
 */
UCLASS()
class MVE_API ATestGenAIActor : public AActor
{
    GENERATED_BODY()

public:
    ATestGenAIActor();

protected:
    virtual void BeginPlay() override;

public:
    //=========================================================================
    // 설정
    //=========================================================================

    /** BeginPlay에서 자동 테스트 실행 여부 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenAI Test|Settings")
    bool bAutoTest = false;

    /** 테스트용 이미지 파일 경로 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenAI Test|Paths")
    FString TestImagePath;

    /** 테스트용 GLB 메시 파일 경로 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenAI Test|Paths")
    FString TestMeshPath;

    /** 테스트용 서버 URL */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenAI Test|Settings")
    FString TestServerURL = TEXT("http://localhost:8000");

    //=========================================================================
    // 테스트 함수들
    //=========================================================================

    /** 1. 로컬 이미지 → UTexture2D 변환 테스트 */
    UFUNCTION(BlueprintCallable, Category = "GenAI Test")
    void TestLoadLocalImage();

    /** 2. 로컬 GLB → USkeletalMesh 변환 테스트 */
    UFUNCTION(BlueprintCallable, Category = "GenAI Test")
    void TestLoadLocalMesh();

    /** 3. 서버에 생성 요청 전송 테스트 (텍스트만) */
    UFUNCTION(BlueprintCallable, Category = "GenAI Test")
    void TestSendRequest();  // [수정] 이름 변경: TestRequestGeneration → TestSendRequest

    /** 4. 이미지 포함 생성 요청 테스트 */
    UFUNCTION(BlueprintCallable, Category = "GenAI Test")
    void TestSendWithImage();  // [수정] 이름 변경: TestRequestWithImage → TestSendWithImage

    /** 5. 파일 서버에서 다운로드 테스트 */
    UFUNCTION(BlueprintCallable, Category = "GenAI Test")
    void TestDownload();  // [수정] 이름 변경: TestDownloadAsset → TestDownload

    //=========================================================================
    // 콜백 핸들러
    //=========================================================================

    /** [수정] 에셋 로드 완료 콜백 (이름 변경) */
    UFUNCTION()
    void HandleAssetLoaded(UObject* Asset, const FAssetMetadata& Metadata);

    /** 다운로드 진행률 콜백 */
    UFUNCTION()
    void HandleDownloadProgress(FGuid AssetID, int32 BytesReceived, int32 TotalBytes);

    /** [추가] 생성 요청 응답 콜백 */
    UFUNCTION()
    void HandleGenerationResponse(bool bSuccess, const FAssetMetadata& Metadata, const FString& ErrorMessage);

    /** Blueprint 확장용 이벤트 */
    UFUNCTION(BlueprintImplementableEvent, Category = "GenAI Test")
    void OnAssetLoadedBP(UObject* Asset, const FAssetMetadata& Metadata);
};