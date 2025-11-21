// Source/MVE/TestGenAIActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AssetTypes.h"
#include "TestGenAIActor.generated.h"

class UGenAISenderReceiver;

/**
 * GenAI 시스템 테스트 액터
 * 서버 없이 로컬 파일로 테스트
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

	// 테스트 설정

	/** 테스트할 이미지 파일 경로 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test|Files")
	FString TestImagePath = TEXT("C:/Users/user/Downloads/image.png");

	/** 테스트할 메시 파일 경로 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test|Files")
	FString TestMeshPath = TEXT("C:/Users/user/Downloads/3d_character_boy_with_rigging.glb");

	/** 서버 URL (Mock 서버 사용 시) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test|Server")
	FString TestServerURL = TEXT("https://www.postman.com/woals13752580-7105422/test/request/o1cd05u/test?action=share&creator=50234590");


	// 테스트 함수들

    
	/** 1. 로컬 이미지 로드 테스트 */
	UFUNCTION(BlueprintCallable, Category = "Test")
	void TestLoadLocalImage();

	/** 2. 로컬 메시 로드 테스트 */
	UFUNCTION(BlueprintCallable, Category = "Test")
	void TestLoadLocalMesh();

	/** 3. Mock 서버로 생성 요청 테스트 */
	UFUNCTION(BlueprintCallable, Category = "Test")
	void TestRequestGeneration();

	/** 4. Mock 서버에서 다운로드 테스트 */
	UFUNCTION(BlueprintCallable, Category = "Test")
	void TestDownloadAsset();



private:
	/** 에셋 수신 핸들러 */
	UFUNCTION()
	void OnAssetReceived(UObject* Asset, const FAssetMetadata& Metadata);

	/** 다운로드 진행 핸들러 */
	UFUNCTION()
	void OnDownloadProgress(FGuid AssetID, int32 BytesReceived, int32 TotalBytes);
};