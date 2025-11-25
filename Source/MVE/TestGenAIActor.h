// TestGenAIActor_Improved.h
// 자동 테스트 기능이 추가된 개선된 버전

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AssetTypes.h"
#include "TestGenAIActor.generated.h"

UCLASS()
class MVE_API ATestGenAIActor : public AActor
{
    GENERATED_BODY()
    
public:    
    ATestGenAIActor();

protected:
    virtual void BeginPlay() override;

public:
    // ==========================================
    // 설정
    // ==========================================
    

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenAI Test|Settings")
    bool bAutoTest = false;
    
    /** 테스트용 이미지 파일 경로 (예: C:\Users\user\Downloads\test.png) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenAI Test|Paths")
    FString TestImagePath;
    
    /** 테스트용 메시 파일 경로 (예: D:\PROJECT\MVE\Saved\TEST\character.glb) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenAI Test|Paths")
    FString TestMeshPath;
    
    /** 테스트용 서버 URL */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GenAI Test|Settings")
    FString TestServerURL = TEXT("http://localhost:8000");

    // ==========================================
    // 테스트 함수들
    // ==========================================
    
    /** 1. 로컬 이미지 파일을 직접 로드하는 테스트 */
    UFUNCTION(BlueprintCallable, Category = "GenAI Test")
    void TestLoadLocalImage();
    
    /** 2. 로컬 GLB 메시 파일을 직접 로드하는 테스트 */
    UFUNCTION(BlueprintCallable, Category = "GenAI Test")
    void TestLoadLocalMesh();
    
    /** 3. Mock 서버로 생성 요청 전송 테스트 */
    UFUNCTION(BlueprintCallable, Category = "GenAI Test")
    void TestRequestGeneration();
    
    /** 4. 이미지를 포함한 생성 요청 테스트 */
    UFUNCTION(BlueprintCallable, Category = "GenAI Test")
    void TestRequestWithImage();
    
    /** 5. Mock 서버에서 에셋 다운로드 테스트 */
    UFUNCTION(BlueprintCallable, Category = "GenAI Test")
    void TestDownloadAsset();

    // ==========================================
    // 콜백 핸들러
    // ==========================================
    
    /** 에셋 생성/다운로드 완료 시 호출 */
    UFUNCTION()
    void OnAssetReceived(UObject* Asset, const FAssetMetadata& Metadata);
    
    /** 다운로드 진행률 업데이트 */
    UFUNCTION()
    void OnDownloadProgress(FGuid AssetID, int32 BytesReceived, int32 TotalBytes);
    
    /** Blueprint에서 추가 처리를 위한 이벤트 */
    UFUNCTION(BlueprintImplementableEvent, Category = "GenAI Test")
    void OnAssetReceivedBP(UObject* Asset, const FAssetMetadata& Metadata);
};