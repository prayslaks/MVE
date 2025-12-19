#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Networking.h"
#include "CompressedMotionData.h"
#include "MocapDataComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnControlDataReceived, int32, ControlIndex, FTransform, ControlTransform);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCompressedFrameReceived, FCompressedMotionFrame, CompressedFrame);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPackedFrameReceived, FPackedMotionFrame, PackedFrame);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFaceMorphDataReceived, const TArray<float>&, MorphTargets);

class UControlRigComponent;

UCLASS(ClassGroup=(Mocap), meta=(BlueprintSpawnableComponent))
class MVE_API UMocapDataComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMocapDataComponent();
    virtual ~UMocapDataComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Mocap")
    FOnControlDataReceived OnControlDataReceived;

    UPROPERTY(BlueprintAssignable, Category = "Mocap")
    FOnCompressedFrameReceived OnCompressedFrameReceived;

    // 네트워크 최적화된 패킹 프레임 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Mocap")
    FOnPackedFrameReceived OnPackedFrameReceived;

    UPROPERTY(BlueprintAssignable, Category = "Mocap")
    FOnFaceMorphDataReceived OnFaceMorphDataReceived;

    // UDP 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mocap")
    int32 ReceivePort = 39570;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mocap")
    FString ListenAddress = TEXT("0.0.0.0");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mocap")
    bool bEnableFaceData = false;

    // 압축 옵션
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mocap|Compression")
    bool bUseCompression = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mocap|Compression")
    bool bUseDeltaCompression = true;

    // 기본 Delta Threshold (LOD 미사용 시)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mocap|Compression")
    float DeltaThreshold = 0.02f;

    // LOD 시스템 사용 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mocap|Compression")
    bool bUseLODSystem = true;

    // LOD 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mocap|Compression")
    FBoneLODConfig LODConfig;

    // Keyframe 간격 (프레임 단위, 0이면 keyframe 없음)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mocap|Compression", meta = (ClampMin = "0", ClampMax = "120"))
    int32 KeyframeInterval = 30;

    // 적응형 프레임 레이트 사용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mocap|Compression")
    bool bUseAdaptiveFrameRate = true;

    // 기본 프레임 간격 (ms)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mocap|Compression", meta = (ClampMin = "16", ClampMax = "100"))
    float BaseFrameIntervalMs = 33.0f;

    // 클라이언트 수에 따른 최대 프레임 간격 (ms)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mocap|Compression", meta = (ClampMin = "33", ClampMax = "200"))
    float MaxFrameIntervalMs = 66.0f;

    // 현재 연결된 클라이언트 수 (외부에서 설정)
    UPROPERTY(BlueprintReadWrite, Category = "Mocap|Compression")
    int32 ConnectedClientCount = 0;

    // 디버그
    UFUNCTION(BlueprintPure, Category = "Mocap|Debug")
    int32 GetLastFrameBoneCount() const { return LastFrameBoneCount; }

    UFUNCTION(BlueprintPure, Category = "Mocap|Debug")
    float GetCompressionRatio() const { return CompressionRatio; }

    UFUNCTION(BlueprintPure, Category = "Mocap|Debug")
    int32 GetCurrentFrameIndex() const { return FrameCounter; }

    UFUNCTION(BlueprintPure, Category = "Mocap|Debug")
    float GetCurrentFrameIntervalMs() const { return CurrentFrameIntervalMs; }

    UFUNCTION(BlueprintPure, Category = "Mocap|Debug")
    int32 GetLastPackedDataSize() const { return LastPackedDataSize; }

    UFUNCTION(BlueprintCallable, Category="Mocap")
    void StartMocap();

    // UDP 소켓
    FSocket* ListenSocket;
    
    TUniquePtr<FUdpSocketReceiver> SocketReceiver;

    UFUNCTION(BlueprintCallable)
    void InitializeSocket();
    
    void ShutdownSocket();
    void OnDataReceived(const FArrayReaderPtr& Data, const FIPv4Endpoint& Endpoint);
    
    // OSC 파싱
    struct FOSCPacket
    {
        FString Address;
        TArray<float> FloatArgs;
        TArray<int32> IntArgs;
    };
    
    bool ParseOSCBundle(const uint8* Data, int32 Size, TArray<FOSCPacket>& OutPackets);
    bool ParseOSCMessage(const uint8* Data, int32 Size, FOSCPacket& OutPacket);
    
    void ProcessControlPacket(const FOSCPacket& Packet);
    void ProcessFacePacket(const FOSCPacket& Packet);

    // 압축 관련
    struct FBoneData
    {
        int32 Index;
        FVector Location;
        FQuat Rotation;
    };

    TArray<FBoneData> CurrentFrameBones;
    float FrameStartTime = 0.0f;
    bool bFrameStarted = false;

    FCompressedMotionFrame CurrentCompressedFrame;
    TMap<uint8, FCompressedBoneTransform> BoneCache;

   // void ProcessCompressedFrame();
    void ProcessCompressedFrameToQueue();
    bool ShouldSendBone(uint8 BoneIndex, const FCompressedBoneTransform& NewTransform);

    // Keyframe 판단
    bool IsKeyframe() const;

    // 적응형 프레임 간격 계산
    float CalculateAdaptiveFrameInterval() const;

    int32 LastFrameBoneCount = 0;
    float CompressionRatio = 0.0f;
    int32 FrameCounter = 0;
    float CurrentFrameIntervalMs = 33.0f;
    int32 LastPackedDataSize = 0;
    
    // 유틸리티
    int32 ReadInt32BigEndian(const uint8* Data);
    float ReadFloatBigEndian(const uint8* Data);
    FString ReadOSCString(const uint8* Data, int32& OutBytesRead);

    // Control Rig 헬퍼 함수
    UFUNCTION(BlueprintPure, Category = "Mocap|ControlRig", meta = (DisplayName = "Get Control Name By Index"))
    static FName GetControlNameByIndex(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Mocap|ControlRig", meta = (DisplayName = "Apply Transforms To Control Rig"))
    static void ApplyTransformsToControlRig(
        UControlRigComponent* ControlRig,
        const TArray<FTransform>& Transforms
    );

    // 클래스 private 섹션 맨 끝에 추가
    // Control 이름 캐시
    static TArray<FName> ControlNames;
    static void InitializeControlNames();

private:
    struct FQueuedTransformData
    {
        int32 Index;
        FTransform Transform;
        float Timestamp;
    };
    struct FQueuedCompressedFrame
    {
        FCompressedMotionFrame Frame;
        float Timestamp;
    };


    TQueue<FQueuedTransformData, EQueueMode::Mpsc> TransformQueue;  // Multi-Producer Single-Consumer
    FCriticalSection QueueLock;
    TQueue<FQueuedCompressedFrame, EQueueMode::Mpsc> CompressedFrameQueue; 
};