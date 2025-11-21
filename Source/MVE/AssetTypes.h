// Source/MVE/GenAIAssetTypes.h
#pragma once

#include "CoreMinimal.h"
#include "AssetTypes.generated.h"

// 에셋 타입
UENUM(BlueprintType)
enum class EGenAIAssetType : uint8
{
    MESH        UMETA(DisplayName = "Mesh"),      // 3D 모델  (GLB)
    AUDIO       UMETA(DisplayName = "Audio"),     // 오디오 파일  (WAV)
    VIDEO       UMETA(DisplayName = "Video"),     // 비디오 파일  (MP4)
    IMAGE       UMETA(DisplayName = "Image"),     // 이미지 파일  (Png)
    GENERIC     UMETA(DisplayName = "Generic Data") // 기타 데이터
};

USTRUCT(BlueprintType)
struct FAssetMetadata
{
    GENERATED_BODY()

    // 고유 식별자 에셋 추적용
    UPROPERTY(BlueprintReadWrite, Category = "GenAI Asset")
    FGuid AssetID;

    //생성자 계정
    UPROPERTY(BlueprintReadWrite, Category = "GenAI Asset")
    FString UserEmail;

    //표시 이름 캐릭터 닉네임 등 
    UPROPERTY(BlueprintReadWrite, Category = "GenAI Asset")
    FString DisplayName;

    // 리소스 데이터     MESH = GLB,FBX IMAGE = PNG , AUDIO = WAV , VIDEO = MP4
    UPROPERTY(BlueprintReadWrite, Category = "GenAI Asset")
    EGenAIAssetType AssetType = EGenAIAssetType::GENERIC;

    // 다운로드 url
    UPROPERTY(BlueprintReadWrite, Category = "GenAI Asset")
    FString RemotePath;

    // 로컬 저장 경로
    UPROPERTY(BlueprintReadWrite, Category = "GenAI Asset")
    FString LocalPath;

    // 생성 날짜 등 시간
    UPROPERTY(BlueprintReadWrite, Category = "GenAI Asset")
    FDateTime Date;

    // 기본 생성자
    FAssetMetadata()
        : AssetType(EGenAIAssetType::GENERIC)
    {
        AssetID = FGuid::NewGuid();
        Date = FDateTime::Now();
    }
};
 
// 에셋 생성 완료 델리게이트 TODO OnAssetGenerated.AddDynamic(this, &AMyActor::OnAssetReceived); 처럼 사용하세요

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnAssetGenerated,
    UObject*, LoadedAsset,
    const FAssetMetadata&, Metadata
);

// 다운로드 진행 상황 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnDownloadProgress,
    FGuid, AssetID,
    int32, BytesReceived,
    int32, TotalBytes
);