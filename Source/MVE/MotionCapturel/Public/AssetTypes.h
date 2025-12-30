// Source/MVE/AssetTypes.h
#pragma once

#include "CoreMinimal.h"
#include "AssetTypes.generated.h"

/*
 * ============================================================================
 *                          GenAI 에셋 타입 정의
 * ============================================================================
 * 
 * AI 서버: http://172.16.20.234:8001
 * 엔드포인트: /generate_mesh
 * 
 * [AI 서버 응답 형식]
 * {
 *     "UserEmail": "user@example.com",     // 생성자 계정 ID
 *     "DisplayName": "캐릭터이름",          // 위젯 표시 이름
 *     "Path": "http://.../xxx.glb",        // 파일 다운로드 경로
 *     "Date": "2024-01-15 14:30:00"        // 생성 날짜
 * }
 * ============================================================================
 */

// ============================================================================
//                              에셋 타입 열거형
// ============================================================================

UENUM(BlueprintType)
enum class EAssetType : uint8
{
    MESH      UMETA(DisplayName = "Mesh"),     // 3D 모델 (GLB/GLTF)
    IMAGE     UMETA(DisplayName = "Image"),    // 이미지 (PNG/JPG)
    AUDIO     UMETA(DisplayName = "Audio"),    // 오디오 (WAV)
    VIDEO     UMETA(DisplayName = "Video"),    // 비디오 (MP4)
    GENERIC   UMETA(DisplayName = "Generic")   // 기타
};

// ============================================================================
//                              메타데이터 구조체
// ============================================================================

/**
 * FAssetMetadata
 * 
 * AI 서버 응답 및 로컬 에셋 정보를 담는 구조체
 * Widget에서 에셋 정보 표시에 사용
 */
USTRUCT(BlueprintType)
struct FAssetMetadata
{
    GENERATED_BODY()

    // ---------------------- 클라이언트 생성 필드 ----------------------
    
    /** 에셋 고유 ID (클라이언트에서 생성) */
    UPROPERTY(BlueprintReadWrite, Category = "Metadata")
    FGuid AssetID;

    // ---------------------- AI 서버 응답 필드 ----------------------
    
    /** 생성자 계정 ID (AI 응답: UserEmail) */
    UPROPERTY(BlueprintReadWrite, Category = "Metadata")
    FString UserEmail;

    /** 위젯 표시 이름 (AI 응답: DisplayName) */
    UPROPERTY(BlueprintReadWrite, Category = "Metadata")
    FString DisplayName;

    /** 파일 서버 다운로드 URL (AI 응답: Path) */
    UPROPERTY(BlueprintReadWrite, Category = "Metadata")
    FString RemotePath;

    /** 생성 날짜 (AI 응답: Date → 파싱) */
    UPROPERTY(BlueprintReadWrite, Category = "Metadata")
    FDateTime Date;

    // ---------------------- 클라이언트 관리 필드 ----------------------
    
    /** 에셋 타입 */
    UPROPERTY(BlueprintReadWrite, Category = "Metadata")
    EAssetType AssetType = EAssetType::GENERIC;

    /** 로컬 저장 경로 (다운로드 후 설정) */
    UPROPERTY(BlueprintReadWrite, Category = "Metadata")
    FString LocalPath;

    // ---------------------- 생성자 ----------------------
    
    FAssetMetadata()
        : AssetType(EAssetType::GENERIC)
    {
        AssetID = FGuid::NewGuid();
        Date = FDateTime::Now();
    }
};

// ============================================================================
//                              델리게이트 선언
// ============================================================================

/**
 * FOnAssetLoaded
 * 
 * 에셋 다운로드 + UE 오브젝트 변환 완료 시 발동
 * Widget에서 결과 표시에 사용
 * 
 * @param LoadedAsset - 로드된 UE 오브젝트 (UTexture2D, USkeletalMesh 등)
 * @param Metadata    - 에셋 메타데이터
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnAssetLoaded,
    UObject*, LoadedAsset,
    const FAssetMetadata&, Metadata
);

/**
 * FOnGenerationResponse
 * 
 * AI 서버 응답 수신 시 발동 (다운로드 전)
 * 
 * @param bSuccess     - 성공 여부
 * @param Metadata     - 파싱된 메타데이터
 * @param ErrorMessage - 실패 시 에러 메시지
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnGenerationResponse,
    bool, bSuccess,
    const FAssetMetadata&, Metadata,
    const FString&, ErrorMessage
);

/**
 * FOnDownloadProgress
 *
 * 다운로드 진행률 업데이트 시 발동
 *
 * @param AssetID       - 다운로드 중인 에셋 ID
 * @param BytesReceived - 현재까지 수신한 바이트
 * @param TotalBytes    - 전체 바이트 (0이면 알 수 없음)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnDownloadProgress,
    FGuid, AssetID,
    int32, BytesReceived,
    int32, TotalBytes
);

/**
 * FOnMusicAnalysisComplete
 *
 * 음악 분석 완료 시 발동
 * AI 서버가 음악 메타데이터 분석 후 타임라인별 이펙트 데이터 반환
 *
 * @param bSuccess     - 성공 여부
 * @param SequenceData - 분석된 이펙트 시퀀스 데이터 배열
 * @param ErrorMessage - 실패 시 에러 메시지
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnMusicAnalysisComplete,
    bool, bSuccess,
    const TArray<struct FEffectSequenceData>&, SequenceData,
    const FString&, ErrorMessage
);