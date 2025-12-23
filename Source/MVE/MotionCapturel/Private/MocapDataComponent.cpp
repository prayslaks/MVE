#include "../Public/MocapDataComponent.h"
#include "MVE.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "ControlRigComponent.h"
#include "Misc/ScopeLock.h"

UMocapDataComponent::UMocapDataComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    ListenSocket = nullptr;
    SocketReceiver = nullptr;
}

UMocapDataComponent::~UMocapDataComponent()
{
    ShutdownSocket();
}

void UMocapDataComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("[MocapData] BeginPlay 시작"));
   
}

void UMocapDataComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    ShutdownSocket();
}

void UMocapDataComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ✅ 압축 모드 타임아웃 체크 (33ms)
    if (bUseCompression && bFrameStarted)
    {
        float CurrentTime = FPlatformTime::Seconds();
        if ((CurrentTime - FrameStartTime) > 0.033f)
        {
            ProcessCompressedFrameToQueue();  // ← 큐에 저장만
            CurrentFrameBones.Reset();
            bFrameStarted = false;
        }
    }

    // ✅ Transform 큐 처리 (Game Thread)
    FQueuedTransformData QueuedData;
    int32 ProcessedCount = 0;
    const int32 MaxProcessPerTick = 100;

    while (TransformQueue.Dequeue(QueuedData) && ProcessedCount < MaxProcessPerTick)
    {
        // Game Thread에서 안전하게 델리게이트 발동
        OnControlDataReceived.Broadcast(QueuedData.Index, QueuedData.Transform);
        ProcessedCount++;
    }

    // ✅ Compressed Frame 큐 처리 (Game Thread)
    FQueuedCompressedFrame QueuedFrame;
    while (CompressedFrameQueue.Dequeue(QueuedFrame))
    {
        // Game Thread에서 안전하게 델리게이트 발동
        OnCompressedFrameReceived.Broadcast(QueuedFrame.Frame);
    }

    if (ProcessedCount > 0)
    {
        static int32 LogCount = 0;
        if (LogCount++ % 30 == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("[MocapData] 🎮 Tick에서 처리: %d개 Transform"), ProcessedCount);
        }
    }
}

float UMocapDataComponent::CalculateAdaptiveFrameInterval() const
{
    if (!bUseAdaptiveFrameRate)
    {
        return BaseFrameIntervalMs;
    }

    // 클라이언트 수에 따라 선형 보간
    // 0명: BaseFrameIntervalMs, 10명 이상: MaxFrameIntervalMs
    const int32 MaxClientsForScaling = 10;
    float Alpha = FMath::Clamp((float)ConnectedClientCount / MaxClientsForScaling, 0.0f, 1.0f);

    return FMath::Lerp(BaseFrameIntervalMs, MaxFrameIntervalMs, Alpha);
}

bool UMocapDataComponent::IsKeyframe() const
{
    if (KeyframeInterval <= 0) return false;
    return (FrameCounter % KeyframeInterval) == 0;
}

void UMocapDataComponent::StartMocap()
{
    if (ListenSocket)
    {
        UE_LOG(LogTemp, Warning, TEXT("Mocap already started"));
        return;
    }
    InitializeSocket();
}

void UMocapDataComponent::InitializeSocket()
{
    UE_LOG(LogTemp, Error, TEXT("🔥 InitializeSocket ENTERED"));
    UE_LOG(LogTemp, Error, TEXT("🔥 ReceivePort = %d"), ReceivePort);

    ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    if (!SocketSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ No SocketSubsystem"));
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("SocketSubsystem OK"));

    ListenSocket = SocketSubsystem->CreateSocket(
        NAME_DGram,
        TEXT("MocapUDPSocket"),
        false
    );

    if (!ListenSocket)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ CreateSocket FAILED"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("✅ Socket Created"));

    ListenSocket->SetReuseAddr(true);
    ListenSocket->SetNonBlocking(true);
    ListenSocket->SetRecvErr();

    TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
    Addr->SetAnyAddress();
    Addr->SetPort(ReceivePort);

    bool bBindOk = ListenSocket->Bind(*Addr);
    UE_LOG(LogTemp, Error, TEXT("🔥 Bind Result = %d"), bBindOk);

    if (!bBindOk)
    {
        int32 Err = SocketSubsystem->GetLastErrorCode();
        UE_LOG(LogTemp, Error, TEXT("❌ Bind FAILED Error=%d"), Err);
        return;
    }

    SocketReceiver = MakeUnique<FUdpSocketReceiver>(
        ListenSocket,
        FTimespan::FromMilliseconds(10),
        TEXT("MocapUDPReceiver")
    );

    SocketReceiver->OnDataReceived().BindUObject(
        this, &UMocapDataComponent::OnDataReceived);

    SocketReceiver->Start();

    UE_LOG(LogTemp, Error,
        TEXT("✅ UDP SOCKET LISTENING ON 0.0.0.0:%d"),
        ReceivePort);
}

void UMocapDataComponent::ShutdownSocket()
{
    if (SocketReceiver)
    {
        SocketReceiver->Stop();
        SocketReceiver.Reset();
        // SocketReceiver = nullptr;
    }

    if (ListenSocket)
    {
        ListenSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);
        ListenSocket = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("[MocapData] UDP 소켓 종료"));
    }
}

void UMocapDataComponent::OnDataReceived(const FArrayReaderPtr& Data, const FIPv4Endpoint& Endpoint)
{
    PRINTLOG(TEXT("데이터 리시브 받기 준비 완료 = %d"),Data->Num());
    
    if (!Data.IsValid() || Data->Num() == 0)
    { 
        return;
    }

    const uint8* RawData = Data->GetData();
    int32 DataSize = Data->Num();

    // OSC 번들 체크 ("#bundle")
    if (DataSize >= 8 && RawData[0] == '#' && RawData[1] == 'b')
    {
        TArray<FOSCPacket> Packets;
        if (ParseOSCBundle(RawData, DataSize, Packets))
        {
            for (const FOSCPacket& Packet : Packets)
            {
                if (Packet.Address.Contains(TEXT("Face")))
                {
                    ProcessFacePacket(Packet);
                }
                else
                {
                    ProcessControlPacket(Packet);
                }
            }
        }
    }
    // OSC 메시지 (주소가 '/'로 시작)
    else if (DataSize > 0 && RawData[0] == '/')
    {
        FOSCPacket Packet;
        if (ParseOSCMessage(RawData, DataSize, Packet))
        {
            UE_LOG(LogTemp, Warning, TEXT("[MocapData] 📨 OSC 메시지: %s (Float:%d, Int:%d)"),
                *Packet.Address, Packet.FloatArgs.Num(), Packet.IntArgs.Num());

            if (Packet.Address.Contains(TEXT("Face")))
            {
                ProcessFacePacket(Packet);
            }
            else
            {
                ProcessControlPacket(Packet);
            }
        }
    }
}

bool UMocapDataComponent::ParseOSCBundle(const uint8* Data, int32 Size, TArray<FOSCPacket>& OutPackets)
{
    // "#bundle" + TimeTag(8) + Messages...
    if (Size < 16) return false;
    
    int32 Offset = 16; // "#bundle\0" + TimeTag
    
    while (Offset < Size)
    {
        if (Offset + 4 > Size) break;
        
        int32 MessageSize = ReadInt32BigEndian(&Data[Offset]);
        Offset += 4;
        
        if (Offset + MessageSize > Size) break;
        
        FOSCPacket Packet;
        if (ParseOSCMessage(&Data[Offset], MessageSize, Packet))
        {
            OutPackets.Add(Packet);
        }
        
        Offset += MessageSize;
    }
    
    return OutPackets.Num() > 0;
}

bool UMocapDataComponent::ParseOSCMessage(const uint8* Data, int32 Size, FOSCPacket& OutPacket)
{
    if (Size < 4) return false;
    
    int32 Offset = 0;
    
    // Address 읽기
    int32 BytesRead = 0;
    OutPacket.Address = ReadOSCString(Data, BytesRead);
    Offset += BytesRead;
    
    if (Offset >= Size) return false;
    
    // Type Tags 읽기 (",iifffff" 같은 형식)
    FString TypeTags = ReadOSCString(&Data[Offset], BytesRead);
    Offset += BytesRead;
    
    if (!TypeTags.StartsWith(TEXT(",")))
    {
        return false;
    }
    
    // 각 타입에 따라 파싱
    for (int32 i = 1; i < TypeTags.Len(); ++i)
    {
        if (Offset + 4 > Size) break;
        
        TCHAR TypeChar = TypeTags[i];
        
        if (TypeChar == 'i') // Int32
        {
            int32 Value = ReadInt32BigEndian(&Data[Offset]);
            OutPacket.IntArgs.Add(Value);
            Offset += 4;
        }
        else if (TypeChar == 'f') // Float
        {
            float Value = ReadFloatBigEndian(&Data[Offset]);
            OutPacket.FloatArgs.Add(Value);
            Offset += 4;
        }
        else if (TypeChar == 's') // String
        {
            ReadOSCString(&Data[Offset], BytesRead);
            Offset += BytesRead;
        }
    }
    
    return true;
}

void UMocapDataComponent::ProcessControlPacket(const FOSCPacket& Packet)
{
    if (Packet.IntArgs.Num() < 1 || Packet.FloatArgs.Num() < 7)
    {
        return;
    }
    
    int32 ControlIndex = Packet.IntArgs[0];
    
    FVector Location(Packet.FloatArgs[0], Packet.FloatArgs[1], Packet.FloatArgs[2]);
    FQuat Rotation(Packet.FloatArgs[3], Packet.FloatArgs[4], Packet.FloatArgs[5], Packet.FloatArgs[6]);
    
    if (bUseCompression)
    {
        // ✅ 압축 모드: 데이터 누적만 (델리게이트 발동 X)
        if (!bFrameStarted)
        {
            bFrameStarted = true;
            FrameStartTime = FPlatformTime::Seconds();
            CurrentFrameBones.Reset();
        }

        FBoneData BoneData;
        BoneData.Index = ControlIndex;
        BoneData.Location = Location;
        BoneData.Rotation = Rotation;
        CurrentFrameBones.Add(BoneData);

        // ✅ 50개 모이면 압축 처리 (델리게이트는 X, 큐에 저장만)
        if (CurrentFrameBones.Num() >= 50)
        {
            ProcessCompressedFrameToQueue();  // ← 새 함수
            CurrentFrameBones.Reset();
            bFrameStarted = false;
        }
    }
    else
    {
        // ✅ 비압축 모드: 큐에 저장
        FQueuedTransformData QueuedData;
        QueuedData.Index = ControlIndex;
        QueuedData.Transform = FTransform(Rotation, Location, FVector::OneVector);
        QueuedData.Timestamp = FPlatformTime::Seconds();
        
        TransformQueue.Enqueue(QueuedData);
    }
}

void UMocapDataComponent::ProcessFacePacket(const FOSCPacket& Packet)
{
    if (!bEnableFaceData || Packet.FloatArgs.Num() == 0)
    {
        return;
    }
    
    OnFaceMorphDataReceived.Broadcast(Packet.FloatArgs);
    
    static int32 LogCount = 0;
    if (LogCount++ < 5)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MocapData Face #%d] 모프 타겟: %d개"), 
            LogCount, Packet.FloatArgs.Num());
    }
}

/*
void UMocapDataComponent::ProcessCompressedFrame()
{
    if (CurrentFrameBones.Num() == 0)
    {
        return;
    }

    CurrentCompressedFrame.Bones.Reset();
    CurrentCompressedFrame.Timestamp = GetWorld()->GetTimeSeconds();
    CurrentCompressedFrame.bDeltaCompressed = bUseDeltaCompression;

    int32 OriginalSize = CurrentFrameBones.Num() * 32;

    for (const FBoneData& BoneData : CurrentFrameBones)
    {
        if (BoneData.Index >= 256) continue;

        FCompressedBoneTransform CompressedBone(
            static_cast<uint8>(BoneData.Index),
            BoneData.Location,
            BoneData.Rotation
        );

        if (bUseDeltaCompression)
        {
            if (ShouldSendBone(CompressedBone.BoneIndex, CompressedBone))
            {
                CurrentCompressedFrame.Bones.Add(CompressedBone);
                BoneCache.Add(CompressedBone.BoneIndex, CompressedBone);
                
                // ✅ 큐에 저장
                FQueuedTransformData QueuedData;
                QueuedData.Index = CompressedBone.BoneIndex;
                QueuedData.Transform = CompressedBone.Decompress();
                QueuedData.Timestamp = FPlatformTime::Seconds();
                TransformQueue.Enqueue(QueuedData);
            }
        }
        else
        {
            CurrentCompressedFrame.Bones.Add(CompressedBone);
            
            // ✅ 큐에 저장
            FQueuedTransformData QueuedData;
            QueuedData.Index = CompressedBone.BoneIndex;
            QueuedData.Transform = CompressedBone.Decompress();
            QueuedData.Timestamp = FPlatformTime::Seconds();
            TransformQueue.Enqueue(QueuedData);
        }
    }

    int32 CompressedSize = 6 + (CurrentCompressedFrame.Bones.Num() * 14);
    LastFrameBoneCount = CurrentCompressedFrame.Bones.Num();
    CompressionRatio = OriginalSize > 0 ? ((float)CompressedSize / (float)OriginalSize) : 1.0f;

    // Compressed Frame 델리게이트는 Game Thread에서 발동 (TickComponent에서 호출되므로)
    OnCompressedFrameReceived.Broadcast(CurrentCompressedFrame);

    static int32 FrameCount = 0;
    if (FrameCount++ % 30 == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MocapData 압축] 프레임 #%d | 본:%d개/%d개 | 압축률:%.1f%%"),
            FrameCount,
            CurrentCompressedFrame.Bones.Num(),
            CurrentFrameBones.Num(),
            CompressionRatio * 100.0f);
    }
}
*/
void UMocapDataComponent::ProcessCompressedFrameToQueue()
{
    if (CurrentFrameBones.Num() == 0)
    {
        return;
    }

    FCompressedMotionFrame CompressedFrame;
    CompressedFrame.Timestamp = FPlatformTime::Seconds();
    CompressedFrame.bDeltaCompressed = bUseDeltaCompression;

    int32 OriginalSize = CurrentFrameBones.Num() * 32;

    for (const FBoneData& BoneData : CurrentFrameBones)
    {
        if (BoneData.Index >= 256) continue;

        FCompressedBoneTransform CompressedBone(
            static_cast<uint8>(BoneData.Index),
            BoneData.Location,
            BoneData.Rotation
        );

        if (bUseDeltaCompression)
        {
            if (ShouldSendBone(CompressedBone.BoneIndex, CompressedBone))
            {
                CompressedFrame.Bones.Add(CompressedBone);
                BoneCache.Add(CompressedBone.BoneIndex, CompressedBone);
                
                // ✅ 개별 Transform도 큐에 저장
                FQueuedTransformData QueuedData;
                QueuedData.Index = CompressedBone.BoneIndex;
                QueuedData.Transform = CompressedBone.Decompress();
                QueuedData.Timestamp = FPlatformTime::Seconds();
                TransformQueue.Enqueue(QueuedData);
            }
        }
        else
        {
            CompressedFrame.Bones.Add(CompressedBone);
            
            // ✅ 개별 Transform도 큐에 저장
            FQueuedTransformData QueuedData;
            QueuedData.Index = CompressedBone.BoneIndex;
            QueuedData.Transform = CompressedBone.Decompress();
            QueuedData.Timestamp = FPlatformTime::Seconds();
            TransformQueue.Enqueue(QueuedData);
        }
    }

    // ✅ CompressedFrame도 큐에 저장 (델리게이트 발동 X)
    FQueuedCompressedFrame QueuedFrame;
    QueuedFrame.Frame = CompressedFrame;
    QueuedFrame.Timestamp = FPlatformTime::Seconds();
    CompressedFrameQueue.Enqueue(QueuedFrame);

    int32 CompressedSize = 6 + (CompressedFrame.Bones.Num() * 14);
    LastFrameBoneCount = CompressedFrame.Bones.Num();
    CompressionRatio = OriginalSize > 0 ? ((float)CompressedSize / (float)OriginalSize) : 1.0f;

    static int32 FrameCount = 0;
    if (FrameCount++ % 30 == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MocapData 압축] 프레임 #%d | 본:%d개/%d개 | 압축률:%.1f%% (큐 저장)"),
            FrameCount,
            CompressedFrame.Bones.Num(),
            CurrentFrameBones.Num(),
            CompressionRatio * 100.0f);
    }
}


bool UMocapDataComponent::ShouldSendBone(uint8 BoneIndex, const FCompressedBoneTransform& NewTransform)
{
    FCompressedBoneTransform* CachedBone = BoneCache.Find(BoneIndex);

    // 캐시에 없으면 무조건 전송
    if (!CachedBone) return true;

    // LOD 기반 threshold 결정
    float CurrentThreshold = bUseLODSystem
        ? LODConfig.GetThresholdForBone(BoneIndex)
        : DeltaThreshold;

    // 위치 변화 검사
    FVector OldLoc = CachedBone->Location.Decompress();
    FVector NewLoc = NewTransform.Location.Decompress();
    float LocDelta = FVector::DistSquared(OldLoc, NewLoc);

    if (LocDelta > CurrentThreshold * CurrentThreshold) return true;

    // 회전 변화 검사
    FQuat OldRot = CachedBone->Rotation.Decompress();
    FQuat NewRot = NewTransform.Rotation.Decompress();
    float RotDelta = FQuat::Error(OldRot, NewRot);

    return RotDelta > CurrentThreshold;
}

// 유틸리티 함수들
int32 UMocapDataComponent::ReadInt32BigEndian(const uint8* Data)
{
    return (Data[0] << 24) | (Data[1] << 16) | (Data[2] << 8) | Data[3];
}

float UMocapDataComponent::ReadFloatBigEndian(const uint8* Data)
{
    uint32 IntValue = (Data[0] << 24) | (Data[1] << 16) | (Data[2] << 8) | Data[3];
    return *reinterpret_cast<float*>(&IntValue);
}

FString UMocapDataComponent::ReadOSCString(const uint8* Data, int32& OutBytesRead)
{
    int32 Len = 0;
    while (Data[Len] != 0) Len++;
    
    FString Result = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(Data)));
    
    // OSC는 4바이트 정렬
    OutBytesRead = ((Len + 1) + 3) & ~3;
    
    return Result;
}

TArray<FName> UMocapDataComponent::ControlNames;

void UMocapDataComponent::InitializeControlNames()
{
    if (ControlNames.Num() > 0)
    {
        return;
    }

    ControlNames = {
        "pelvis_ctrl",                  // 0
        "spine_01_ctrl",                // 1
        "spine_02_ctrl",                // 2
        "spine_03_ctrl",                // 3
        "spine_04_ctrl",                // 4
        "spine_05_ctrl",                // 5
        "neck_01_ctrl",                 // 6
        "head_ctrl",                    // 7
        "clavicle_l_ctrl",              // 8
        "upperarm_l_ctrl",              // 9
        "lowerarm_l_ctrl",              // 10
        "hand_l_ctrl",                  // 11
        "thumb_01_l_ctrl",              // 12
        "thumb_02_l_ctrl",              // 13
        "thumb_03_l_ctrl",              // 14
        "index_metacarpal_l_ctrl",      // 15
        "index_01_l_ctrl",              // 16
        "index_02_l_ctrl",              // 17
        "index_03_l_ctrl",              // 18
        "middle_metacarpal_l_ctrl",     // 19
        "middle_01_l_ctrl",             // 20
        "middle_02_l_ctrl",             // 21
        "middle_03_l_ctrl",             // 22
        "ring_metacarpal_l_ctrl",       // 23
        "ring_01_l_ctrl",               // 24
        "ring_02_l_ctrl",               // 25
        "ring_03_l_ctrl",               // 26
        "pinky_metacarpal_l_ctrl",      // 27
        "pinky_01_l_ctrl",              // 28
        "pinky_02_l_ctrl",              // 29
        "pinky_03_l_ctrl",              // 30
        "clavicle_r_ctrl",              // 31
        "upperarm_r_ctrl",              // 32
        "lowerarm_r_ctrl",              // 33
        "hand_r_ctrl",                  // 34
        "thumb_01_r_ctrl",              // 35
        "thumb_02_r_ctrl",              // 36
        "thumb_03_r_ctrl",              // 37
        "index_metacarpal_r_ctrl",      // 38
        "index_01_r_ctrl",              // 39
        "index_02_r_ctrl",              // 40
        "index_03_r_ctrl",              // 41
        "middle_metacarpal_r_ctrl",     // 42
        "middle_01_r_ctrl",             // 43
        "middle_02_r_ctrl",             // 44
        "middle_03_r_ctrl",             // 45
        "ring_metacarpal_r_ctrl",       // 46
        "ring_01_r_ctrl",               // 47
        "ring_02_r_ctrl",               // 48
        "ring_03_r_ctrl",               // 49
        "pinky_metacarpal_r_ctrl",      // 50
        "pinky_01_r_ctrl",              // 51
        "pinky_02_r_ctrl",              // 52
        "pinky_03_r_ctrl"               // 53
    };

    UE_LOG(LogTemp, Warning, TEXT("[MocapData] ✅ Control 이름 초기화: %d개"), ControlNames.Num());
}

FName UMocapDataComponent::GetControlNameByIndex(int32 Index)
{
    InitializeControlNames();
    
    if (Index >= 0 && Index < ControlNames.Num())
    {
        return ControlNames[Index];
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[MocapData] ⚠️ 유효하지 않은 Index: %d"), Index);
    return NAME_None;
}

void UMocapDataComponent::ApplyTransformsToControlRig(
    UControlRigComponent* ControlRig,
    const TArray<FTransform>& Transforms)
{
    if (!ControlRig)
    {
        UE_LOG(LogTemp, Error, TEXT("[MocapData] ❌ ControlRig가 nullptr"));
        return;
    }

    if (Transforms.Num() == 0)
    {
        return;
    }

    InitializeControlNames();

    int32 AppliedCount = 0;
    int32 MaxIndex = FMath::Min(Transforms.Num(), ControlNames.Num());

    for (int32 i = 0; i < MaxIndex; ++i)
    {
       
        ControlRig->SetControlTransform(
            ControlNames[i],
            Transforms[i],
            EControlRigComponentSpace::WorldSpace
        );
        
        AppliedCount++;
    }

    // Control Rig 업데이트
    ControlRig->Update(0.0f);

    static int32 LogCount = 0;
    if (LogCount++ % 30 == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MocapData] 🎮 Control Rig 업데이트: %d개 본"), AppliedCount);
    }
}