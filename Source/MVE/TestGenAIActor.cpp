// Source/MVE/TestGenAIActor.cpp

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

    // 서브시스템 가져오기
    USenderReceiver* SR = GetGameInstance()->GetSubsystem<USenderReceiver>();

    if (!SR)
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] ✗ USenderReceiver 서브시스템을 찾을 수 없습니다"));
        return;
    }

    // 델리게이트 바인딩 [수정] 새로운 델리게이트명 사용
    SR->OnAssetGenerated.AddDynamic(this, &ATestGenAIActor::HandleAssetLoaded);
    SR->OnDownloadProgress.AddDynamic(this, &ATestGenAIActor::HandleDownloadProgress);
    SR->OnGenerationResponse.AddDynamic(this, &ATestGenAIActor::HandleGenerationResponse);

    // 서버 URL 설정
    if (!TestServerURL.IsEmpty())
    {
        SR->ServerURL = TestServerURL;
        UE_LOG(LogMVE, Log, TEXT("[Test] 서버 URL: %s"), *TestServerURL);
    }

    UE_LOG(LogMVE, Log, TEXT("[Test] ✓ GenAI 시스템 준비 완료"));

    // 자동 테스트 실행
    if (bAutoTest)
    {
        UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));
        UE_LOG(LogMVE, Warning, TEXT("[Test] 자동 테스트 시작"));
        UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));

        // 1초 후 이미지 테스트
        FTimerHandle Timer1;
        GetWorld()->GetTimerManager().SetTimer(Timer1,
            [this]() { TestLoadLocalImage(); }, 1.0f, false);

        // 3초 후 메시 테스트
        FTimerHandle Timer2;
        GetWorld()->GetTimerManager().SetTimer(Timer2,
            [this]() { TestLoadLocalMesh(); }, 3.0f, false);
    }
}

//=============================================================================
// 테스트 1: 로컬 이미지 로드
//=============================================================================

void ATestGenAIActor::TestLoadLocalImage()
{
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] 테스트 1: 로컬 이미지 로드"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));

    USenderReceiver* SR = GetGameInstance()->GetSubsystem<USenderReceiver>();
    if (!SR)
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] ✗ 서브시스템 없음"));
        return;
    }

    if (TestImagePath.IsEmpty())
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] ✗ TestImagePath가 비어있습니다"));
        UE_LOG(LogMVE, Error, TEXT("[Test] → Details 패널에서 설정하세요"));
        return;
    }

    UE_LOG(LogMVE, Log, TEXT("[Test] 경로: %s"), *TestImagePath);

    if (!FPaths::FileExists(TestImagePath))
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] ✗ 파일이 존재하지 않습니다"));
        return;
    }

    UE_LOG(LogMVE, Log, TEXT("[Test] ✓ 파일 존재 확인"));
    
}

//=============================================================================
// 테스트 2: 로컬 메시 로드
//=============================================================================

void ATestGenAIActor::TestLoadLocalMesh()
{
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] 테스트 2: 로컬 메시 로드"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));

    USenderReceiver* SR = GetGameInstance()->GetSubsystem<USenderReceiver>();
    if (!SR)
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] ✗ 서브시스템 없음"));
        return;
    }

    if (TestMeshPath.IsEmpty())
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] ✗ TestMeshPath가 비어있습니다"));
        UE_LOG(LogMVE, Error, TEXT("[Test] → Details 패널에서 설정하세요"));
        return;
    }

    UE_LOG(LogMVE, Log, TEXT("[Test] 경로: %s"), *TestMeshPath);

    if (!FPaths::FileExists(TestMeshPath))
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] ✗ 파일이 존재하지 않습니다"));

        FString Extension = FPaths::GetExtension(TestMeshPath).ToLower();
        if (Extension != TEXT("glb") && Extension != TEXT("gltf"))
        {
            UE_LOG(LogMVE, Warning, TEXT("[Test] ⚠ 확장자가 .glb/.gltf가 아닙니다 (현재: .%s)"), *Extension);
        }
        return;
    }

    UE_LOG(LogMVE, Log, TEXT("[Test] ✓ 파일 존재 확인"));

    int64 FileSize = IFileManager::Get().FileSize(*TestMeshPath);
    UE_LOG(LogMVE, Log, TEXT("[Test] 파일 크기: %.2f MB"), FileSize / 1024.0f / 1024.0f);
    
}

//=============================================================================
// 테스트 3: 생성 요청 전송
//=============================================================================

void ATestGenAIActor::TestSendRequest()
{
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] 테스트 3: 생성 요청 전송"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));

    USenderReceiver* SR = GetGameInstance()->GetSubsystem<USenderReceiver>();
    if (!SR)
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] ✗ 서브시스템 없음"));
        return;
    }

    // [수정] 새로운 API 호출
    SR->RequestGeneration(
        TEXT("A futuristic robot warrior"),
        TEXT("test@example.com"),
        TEXT("")  // 이미지 없음
    );

    UE_LOG(LogMVE, Log, TEXT("[Test] → 서버 응답 대기중..."));
}

//=============================================================================
// 테스트 4: 이미지 포함 생성 요청
//=============================================================================

void ATestGenAIActor::TestSendWithImage()
{
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] 테스트 4: 이미지 포함 생성 요청"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));

    USenderReceiver* SR = GetGameInstance()->GetSubsystem<USenderReceiver>();
    if (!SR)
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] ✗ 서브시스템 없음"));
        return;
    }

    if (TestImagePath.IsEmpty())
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] ✗ TestImagePath가 설정되지 않았습니다"));
        return;
    }

    // [수정] 새로운 API 호출
    SR->RequestGeneration(
        TEXT("Transform this character into sci-fi style"),
        TEXT("test@example.com"),
        TestImagePath
    );

    UE_LOG(LogMVE, Log, TEXT("[Test] → 이미지 포함 요청 전송됨"));
}

//=============================================================================
// 테스트 5: 파일 다운로드
//=============================================================================

void ATestGenAIActor::TestDownload()
{
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] 테스트 5: 파일 다운로드"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));

    USenderReceiver* SR = GetGameInstance()->GetSubsystem<USenderReceiver>();
    if (!SR)
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] ✗ 서브시스템 없음"));
        return;
    }

    // Mock 메타데이터 구성
    FAssetMetadata Metadata;
    Metadata.AssetType = EAssetType::IMAGE;
    Metadata.DisplayName = TEXT("Mock 서버 이미지");
    Metadata.RemotePath = TestServerURL + TEXT("/api/download/image");

    // [수정] 새로운 API 호출
    SR->DownloadFileServer(Metadata);

    UE_LOG(LogMVE, Log, TEXT("[Test] → 다운로드 요청 전송됨"));
}

//=============================================================================
// 콜백 핸들러
//=============================================================================

void ATestGenAIActor::HandleAssetLoaded(UObject* Asset, const FAssetMetadata& Metadata)
{
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] ✅ 에셋 로드 성공!"));
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));
    UE_LOG(LogMVE, Log, TEXT("  - ID: %s"), *Metadata.AssetID.ToString());
    UE_LOG(LogMVE, Log, TEXT("  - 이름: %s"), *Metadata.DisplayName);
    UE_LOG(LogMVE, Log, TEXT("  - 타입: %d"), (int32)Metadata.AssetType);
    UE_LOG(LogMVE, Log, TEXT("  - 경로: %s"), *Metadata.LocalPath);

    // 이미지 처리
    if (UTexture2D* Texture = Cast<UTexture2D>(Asset))
    {
        UE_LOG(LogMVE, Warning, TEXT("[Test] 타입: Texture2D"));
        UE_LOG(LogMVE, Log, TEXT("  크기: %dx%d"), Texture->GetSizeX(), Texture->GetSizeY());
        UE_LOG(LogMVE, Log, TEXT("  포맷: %s"),
            *StaticEnum<EPixelFormat>()->GetNameStringByValue(Texture->GetPixelFormat()));
    }
    // 메시 처리
    else if (USkeletalMesh* Mesh = Cast<USkeletalMesh>(Asset))
    {
        UE_LOG(LogMVE, Warning, TEXT("[Test] 타입: SkeletalMesh"));
        UE_LOG(LogMVE, Log, TEXT("  Bone: %d"), Mesh->GetRefSkeleton().GetNum());
        UE_LOG(LogMVE, Log, TEXT("  MorphTarget: %d"), Mesh->GetMorphTargets().Num());
        UE_LOG(LogMVE, Log, TEXT("  Material: %d"), Mesh->GetMaterials().Num());

        // 첫 5개 Bone 이름 출력
        const FReferenceSkeleton& RefSkel = Mesh->GetRefSkeleton();
        int32 BoneCount = FMath::Min(5, RefSkel.GetNum());
        for (int32 i = 0; i < BoneCount; i++)
        {
            UE_LOG(LogMVE, Log, TEXT("    [%d] %s"), i, *RefSkel.GetBoneName(i).ToString());
        }
    }
    // StaticMesh 처리
    else if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(Asset))
    {
        UE_LOG(LogMVE, Warning, TEXT("[Test] 타입: StaticMesh"));
        UE_LOG(LogMVE, Log, TEXT("  Material: %d"), StaticMesh->GetStaticMaterials().Num());
    }

    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));

    // Blueprint 이벤트 호출
    OnAssetLoadedBP(Asset, Metadata);
}

void ATestGenAIActor::HandleDownloadProgress(FGuid AssetID, int32 BytesReceived, int32 TotalBytes)
{
    if (TotalBytes > 0)
    {
        float Progress = (float)BytesReceived / (float)TotalBytes * 100.0f;
        UE_LOG(LogMVE, Verbose, TEXT("[Test] 다운로드: %.1f%% (%d/%d)"),
            Progress, BytesReceived, TotalBytes);
    }
}

void ATestGenAIActor::HandleGenerationResponse(bool bSuccess, const FAssetMetadata& Metadata, const FString& ErrorMessage)
{
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));
    
    if (bSuccess)
    {
        UE_LOG(LogMVE, Warning, TEXT("[Test] ✅ 생성 요청 성공!"));
        UE_LOG(LogMVE, Log, TEXT("  - AssetID: %s"), *Metadata.AssetID.ToString());
        UE_LOG(LogMVE, Log, TEXT("  - RemotePath: %s"), *Metadata.RemotePath);
        
        // 자동으로 다운로드 시작
        if (!Metadata.RemotePath.IsEmpty())
        {
            UE_LOG(LogMVE, Log, TEXT("[Test] → 자동 다운로드 시작..."));
            USenderReceiver* SR = GetGameInstance()->GetSubsystem<USenderReceiver>();
            if (SR)
            {
                SR->DownloadFileServer(Metadata);
            }
        }
    }
    else
    {
        UE_LOG(LogMVE, Error, TEXT("[Test] ✗ 생성 요청 실패"));
        UE_LOG(LogMVE, Error, TEXT("  - 에러: %s"), *ErrorMessage);
    }
    
    UE_LOG(LogMVE, Warning, TEXT("[Test] ========================================"));
}