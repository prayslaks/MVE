// TestGenAIActor_Improved.cpp
// 자동으로 모든 테스트를 실행하는 개선된 버전

#include "TestGenAIActor.h"
#include "SenderReceiver.h"
#include "Engine/Texture2D.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "MVE.h"

ATestGenAIActor::ATestGenAIActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ATestGenAIActor::BeginPlay()
{
    Super::BeginPlay();

    // Subsystem 가져오기
    USenderReceiver* GenAI = 
        GetGameInstance()->GetSubsystem<USenderReceiver>();

    if (GenAI)
    {
        // 델리게이트 바인딩
        GenAI->OnAssetGenerated.AddDynamic(this, &ATestGenAIActor::OnAssetReceived);
        GenAI->OnDownloadProgress.AddDynamic(this, &ATestGenAIActor::OnDownloadProgress);
        
        // 테스트 서버 URL 설정
        if (!TestServerURL.IsEmpty())
        {
            GenAI->ServerURL = TestServerURL;
            UE_LOG(LogMVE, Log, TEXT("[Test] 서버 URL 설정: %s"), *TestServerURL);
        }
        
        UE_LOG(LogMVE, Log, TEXT("[Test] GenAI 시스템 준비 완료"));
        
        // 자동 테스트 실행 (bAutoTest가 true인 경우)
        if (bAutoTest)
        {
            UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));
            UE_LOG(LogMVE, Warning, TEXT("[Test] 자동 테스트 시작"));
            UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));
            
            // 1초 후 이미지 테스트
            FTimerHandle TimerHandle1;
            GetWorld()->GetTimerManager().SetTimer(
                TimerHandle1,
                [this]() { TestLoadLocalImage(); },
                1.0f,
                false
            );
            
            // 3초 후 메시 테스트
            FTimerHandle TimerHandle2;
            GetWorld()->GetTimerManager().SetTimer(
                TimerHandle2,
                [this]() { TestLoadLocalMesh(); },
                3.0f,
                false
            );
        }
    }
    else
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] GenAISenderReceiver Subsystem을 찾을 수 없습니다"));
    }
}

// 1. 로컬 이미지 로드 테스트
void ATestGenAIActor::TestLoadLocalImage()
{
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] 테스트 1: 로컬 이미지 로드"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));

    USenderReceiver* GenAI = 
        GetGameInstance()->GetSubsystem<USenderReceiver>();

    if (!GenAI)
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] ❌ Subsystem 없음"));
        return;
    }

    if (TestImagePath.IsEmpty())
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] ❌ TestImagePath가 비어있습니다!"));
        UE_LOG(LogMVE, Error, TEXT("[Test] Editor에서 TestGenAIActor의 TestImagePath를 설정하세요"));
        UE_LOG(LogMVE, Error, TEXT("[Test] 예: C:\\Users\\user\\Downloads\\test_image.png"));
        return;
    }
    
    UE_LOG(LogMVE, Warning, TEXT("[Test] 이미지 경로: %s"), *TestImagePath);

    // 파일 존재 확인
    if (!FPaths::FileExists(TestImagePath))
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] ❌ 파일이 존재하지 않습니다!"));
        UE_LOG(LogMVE, Error, TEXT("[Test] 경로를 다시 확인하세요: %s"), *TestImagePath);
        return;
    }
    
    UE_LOG(LogMVE, Warning, TEXT("[Test] ✓ 파일 존재 확인"));

    // 메타데이터 구성
    FAssetMetadata Metadata;
    Metadata.AssetType = EAssetType::IMAGE;
    Metadata.DisplayName = TEXT("로컬 테스트 이미지");
    Metadata.RemotePath = TestImagePath;

    GenAI->LoadLocalAsset(Metadata);
}

// 2. 로컬 메시 로드 테스트
void ATestGenAIActor::TestLoadLocalMesh()
{
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] 테스트 2: 로컬 메시 로드"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));

    USenderReceiver* GenAI = 
        GetGameInstance()->GetSubsystem<USenderReceiver>();
    
    if (!GenAI)
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] ❌ Subsystem 없음"));
        return;
    }

    if (TestMeshPath.IsEmpty())
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] ❌ TestMeshPath가 비어있습니다!"));
        UE_LOG(LogMVE, Error, TEXT("[Test] Editor에서 TestGenAIActor의 TestMeshPath를 설정하세요"));
        UE_LOG(LogMVE, Error, TEXT("[Test] 예: D:\\PROJECT\\MVE\\Saved\\TEST\\character.glb"));
        return;
    }
    
    UE_LOG(LogMVE, Warning, TEXT("[Test] 메시 경로: %s"), *TestMeshPath);
    
    // 파일 존재 확인
    if (!FPaths::FileExists(TestMeshPath))
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] ❌ 파일이 존재하지 않습니다!"));
        UE_LOG(LogMVE, Error, TEXT("[Test] 경로를 다시 확인하세요: %s"), *TestMeshPath);
        
        // 확장자 확인
        FString Extension = FPaths::GetExtension(TestMeshPath).ToLower();
        if (Extension != TEXT("glb") && Extension != TEXT("gltf"))
        {
            UE_LOG(LogMVE, Error, TEXT("[Test] ⚠️ 확장자가 .glb 또는 .gltf가 아닙니다 (현재: .%s)"), *Extension);
        }
        
        return;
    }
    
    UE_LOG(LogMVE, Warning, TEXT("[Test] ✓ 파일 존재 확인"));
    
    // 파일 크기 확인
    int64 FileSize = IFileManager::Get().FileSize(*TestMeshPath);
    UE_LOG(LogMVE, Warning, TEXT("[Test] 파일 크기: %lld bytes (%.2f MB)"), 
        FileSize, FileSize / 1024.0f / 1024.0f);

    // 메타데이터 구성
    FAssetMetadata Metadata;
    Metadata.AssetType = EAssetType::MESH;
    Metadata.DisplayName = TEXT("로컬 테스트 메시");
    Metadata.RemotePath = TestMeshPath;
    
    UE_LOG(LogMVE, Warning, TEXT("[Test] LoadLocalAsset() 호출..."));
    GenAI->LoadLocalAsset(Metadata);
}

// 3. Mock 서버로 생성 요청 테스트
void ATestGenAIActor::TestRequestGeneration()
{
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] 테스트 3: 생성 요청"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));

    USenderReceiver* GenAI = 
        GetGameInstance()->GetSubsystem<USenderReceiver>();

    if (!GenAI)
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] ❌ Subsystem 없음"));
        return;
    }

    // 텍스트만으로 생성 요청
    GenAI->RequestGeneration(
        TEXT("A futuristic robot warrior"),  // Prompt
        TEXT("test@example.com"),             // UserEmail
        TEXT("")                               // 이미지 없음
    );

    UE_LOG(LogMVE, Log, TEXT("[Test] → Mock 서버 응답 대기 중..."));
}

// 4. 이미지 포함 생성 요청 테스트
void ATestGenAIActor::TestRequestWithImage()
{
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] 테스트 4: 이미지 포함 생성 요청"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));

    USenderReceiver* GenAI = 
        GetGameInstance()->GetSubsystem<USenderReceiver>();

    if (!GenAI)
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] ❌ Subsystem 없음"));
        return;
    }

    if (TestImagePath.IsEmpty())
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] ❌ TestImagePath가 설정되지 않았습니다"));
        return;
    }

    // 이미지와 함께 생성 요청
    GenAI->RequestGeneration(
        TEXT("Transform this character into a sci-fi style"),
        TEXT("test@example.com"),
        TestImagePath
    );

    UE_LOG(LogMVE, Log, TEXT("[Test] → 이미지 포함 요청 전송됨"));
}

// 5. Mock 서버에서 다운로드 테스트
void ATestGenAIActor::TestDownloadAsset()
{
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] 테스트 5: 다운로드"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));

    USenderReceiver* GenAI = 
        GetGameInstance()->GetSubsystem<USenderReceiver>();

    if (!GenAI)
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] ❌ Subsystem 없음"));
        return;
    }

    // 메타데이터 구성
    FAssetMetadata Metadata;
    Metadata.AssetType = EAssetType::IMAGE;
    Metadata.DisplayName = TEXT("Mock 서버 이미지");
    Metadata.RemotePath = TestServerURL + TEXT("/api/download/image");

    // 다운로드
    GenAI->DownloadAsset(Metadata);

    UE_LOG(LogMVE, Log, TEXT("[Test] → 다운로드 요청 전송됨"));
}

// 콜백 핸들러
void ATestGenAIActor::OnAssetReceived(UObject* Asset, const FAssetMetadata& Metadata)
{
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] ✅ 에셋 수신 성공!"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));
    UE_LOG(LogMVE, Log, TEXT("  - ID: %s"), *Metadata.AssetID.ToString());
    UE_LOG(LogMVE, Log, TEXT("  - 이름: %s"), *Metadata.DisplayName);
    UE_LOG(LogMVE, Log, TEXT("  - 타입: %d"), (int32)Metadata.AssetType);
    UE_LOG(LogMVE, Log, TEXT("  - 로컬 경로: %s"), *Metadata.LocalPath);

    // 이미지 처리
    if (UTexture2D* Texture = Cast<UTexture2D>(Asset))
    {
        UE_LOG(LogMVE, Warning, TEXT("[Test] 타입: Texture2D"));
        UE_LOG(LogMVE, Log, TEXT("     크기: %dx%d"), 
            Texture->GetSizeX(), Texture->GetSizeY());
        UE_LOG(LogMVE, Log, TEXT("     포맷: %s"), 
            *StaticEnum<EPixelFormat>()->GetNameStringByValue(Texture->GetPixelFormat()));
    }
    
    // 메시 처리
    else if (USkeletalMesh* Mesh = Cast<USkeletalMesh>(Asset))
    {
        UE_LOG(LogMVE, Warning, TEXT("[Test] 타입: SkeletalMesh"));
        UE_LOG(LogMVE, Log, TEXT("     Bone 개수: %d"), 
            Mesh->GetRefSkeleton().GetNum());
        UE_LOG(LogMVE, Log, TEXT("     MorphTarget 개수: %d"), 
            Mesh->GetMorphTargets().Num());
        UE_LOG(LogMVE, Log, TEXT("     머티리얼 슬롯: %d"), 
            Mesh->GetMaterials().Num());
            
        // Bone 이름 출력 (처음 5개)
        const FReferenceSkeleton& RefSkel = Mesh->GetRefSkeleton();
        int32 BoneCount = FMath::Min(5, RefSkel.GetNum());
        if (BoneCount > 0)
        {
            UE_LOG(LogMVE, Log, TEXT("     첫 %d개 Bone:"), BoneCount);
            for (int32 i = 0; i < BoneCount; i++)
            {
                UE_LOG(LogMVE, Log, TEXT("       [%d] %s"), 
                    i, *RefSkel.GetBoneName(i).ToString());
            }
        }
    }
    
    // StaticMesh 처리
    else if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(Asset))
    {
        UE_LOG(LogMVE, Warning, TEXT("[Test] 타입: StaticMesh"));
        UE_LOG(LogMVE, Log, TEXT("     머티리얼 슬롯: %d"), 
            StaticMesh->GetStaticMaterials().Num());
    }

    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));
    
    // 추가 처리 (블루프린트에서 구현)
    OnAssetReceivedBP(Asset, Metadata);
}

void ATestGenAIActor::OnDownloadProgress(FGuid AssetID, int32 BytesReceived, int32 TotalBytes)
{
    if (TotalBytes > 0)
    {
        float Progress = (float)BytesReceived / (float)TotalBytes * 100.0f;
        UE_LOG(LogMVE, Verbose, TEXT("[Test] 다운로드 진행: %.1f%% (%d/%d bytes)"), 
            Progress, BytesReceived, TotalBytes);
    }
}