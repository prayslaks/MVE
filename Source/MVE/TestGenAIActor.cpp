// Source/MVE/TestGenAIActor.cpp

#include "TestGenAIActor.h"
#include "SenderReceiver.h"
#include "Engine/Texture2D.h"
#include "Components/StaticMeshComponent.h"
#include "MVE.h"

ATestGenAIActor::ATestGenAIActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ATestGenAIActor::BeginPlay()
{
    Super::BeginPlay();

    // Subsystem 가져오기
    UGenAISenderReceiver* GenAI = 
        GetGameInstance()->GetSubsystem<UGenAISenderReceiver>();

    if (GenAI)
    {
        // 델리게이트 바인딩
        GenAI->OnAssetGenerated.AddDynamic(this, &ATestGenAIActor::OnAssetReceived);
        GenAI->OnDownloadProgress.AddDynamic(this, &ATestGenAIActor::OnDownloadProgress);
        
        // Mock 서버 URL 설정
        GenAI->ServerURL = TestServerURL;
        
        UE_LOG(LogMVE, Log, TEXT("[Test] GenAI 시스템 준비 완료"));
    }
    else
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] GenAISenderReceiver Subsystem을 찾을 수 없습니다"));
    }
}


// 1. 로컬 이미지 로드 테스트


void ATestGenAIActor::TestLoadLocalImage()
{
    UE_LOG(LogMVE, Log, TEXT("[Test] ===== 로컬 이미지 로드 테스트 ====="));

    UGenAISenderReceiver* GenAI = 
        GetGameInstance()->GetSubsystem<UGenAISenderReceiver>();

    if (!GenAI)
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] Subsystem 없음"));
        return;
    }

    // 메타데이터 구성
    FAssetMetadata Metadata;
    Metadata.AssetType = EGenAIAssetType::IMAGE;
    Metadata.DisplayName = TEXT("로컬 테스트 이미지");
    Metadata.RemotePath = TestImagePath;

    //  DownloadAsset 대신 LoadLocalAsset 사용!
    GenAI->LoadLocalAsset(Metadata);
}

// 2. 로컬 메시 로드 테스트


void ATestGenAIActor::TestLoadLocalMesh()
{
    UE_LOG(LogMVE, Log, TEXT("[Test] ===== 로컬 메시 로드 테스트 ====="));

    UGenAISenderReceiver* GenAI = 
        GetGameInstance()->GetSubsystem<UGenAISenderReceiver>();
    if (!GenAI)
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] Subsystem 없음"));
        return;
    }

    // 메타데이터 구성
    FAssetMetadata Metadata;
    Metadata.AssetType = EGenAIAssetType::MESH;
    Metadata.DisplayName = TEXT("로컬 테스트 메시");
    Metadata.RemotePath = TestMeshPath;

    //  DownloadAsset 대신 LoadLocalAsset 사용!
    GenAI->LoadLocalAsset(Metadata);
}

// 3. Mock 서버로 생성 요청 테스트


void ATestGenAIActor::TestRequestGeneration()
{
    UE_LOG(LogMVE, Log, TEXT("[Test] ===== 생성 요청 테스트 ====="));

    UGenAISenderReceiver* GenAI = 
        GetGameInstance()->GetSubsystem<UGenAISenderReceiver>();

    if (!GenAI)
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] Subsystem 없음"));
        return;
    }

    // 텍스트만으로 생성 요청
    GenAI->RequestGeneration(
        TEXT("A futuristic robot warrior"),  // Prompt
        TEXT("test@example.com"),             // UserEmail
        TEXT("")                               // 이미지 없음
    );

    UE_LOG(LogMVE, Log, TEXT("[Test] → Mock 서버가 실행 중이어야 합니다"));
}

// 4. Mock 서버에서 다운로드 테스트


void ATestGenAIActor::TestDownloadAsset()
{
    UE_LOG(LogMVE, Log, TEXT("[Test] ===== 다운로드 테스트 ====="));

    UGenAISenderReceiver* GenAI = 
        GetGameInstance()->GetSubsystem<UGenAISenderReceiver>();

    if (!GenAI)
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] Subsystem 없음"));
        return;
    }

    // 메타데이터 구성
    FAssetMetadata Metadata;
    Metadata.AssetType = EGenAIAssetType::IMAGE;
    Metadata.DisplayName = TEXT("Mock 서버 이미지");
    Metadata.RemotePath = TestServerURL + TEXT("/api/download/image");

    // 다운로드
    GenAI->DownloadAsset(Metadata);

    UE_LOG(LogMVE, Log, TEXT("[Test] → Mock 서버가 실행 중이어야 합니다"));
}


// 콜백 핸들러
void ATestGenAIActor::OnAssetReceived(UObject* Asset, const FAssetMetadata& Metadata)
{
    UE_LOG(LogMVE, Log, TEXT("[Test] ===== 에셋 수신 성공! ====="));
    UE_LOG(LogMVE, Log, TEXT("[Test] 이름: %s"), *Metadata.DisplayName);
    UE_LOG(LogMVE, Log, TEXT("[Test] 타입: %d"), (int32)Metadata.AssetType);

    // 이미지 처리
    if (UTexture2D* Texture = Cast<UTexture2D>(Asset))
    {
        UE_LOG(LogMVE, Log, TEXT("[Test] ✓ Texture2D 생성됨"));
        UE_LOG(LogMVE, Log, TEXT("[Test]   - 크기: %dx%d"), 
            Texture->GetSizeX(), Texture->GetSizeY());
    }
    
    // 메시 처리
    else if (USkeletalMesh* Mesh = Cast<USkeletalMesh>(Asset))
    {
        UE_LOG(LogMVE, Log, TEXT("[Test] ✓ SkeletalMesh 생성됨"));
        UE_LOG(LogMVE, Log, TEXT("[Test]   - Bone 개수: %d"), 
            Mesh->GetRefSkeleton().GetNum());
    }
}

void ATestGenAIActor::OnDownloadProgress(FGuid AssetID, int32 BytesReceived, int32 TotalBytes)
{
    if (TotalBytes > 0)
    {
        float Progress = (float)BytesReceived / (float)TotalBytes * 100.0f;
        UE_LOG(LogMVE, Verbose, TEXT("[Test] 다운로드 진행: %.1f%%"), Progress);
    }
}
