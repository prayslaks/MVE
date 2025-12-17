#include "MocapDataComponent.h"
#include "MVE.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
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

    if (bUseCompression && bFrameStarted)
    {
        // 적응형 프레임 간격 계산
        CurrentFrameIntervalMs = CalculateAdaptiveFrameInterval();
        float FrameIntervalSeconds = CurrentFrameIntervalMs / 1000.0f;

        float CurrentTime = GetWorld()->GetTimeSeconds();
        if ((CurrentTime - FrameStartTime) > FrameIntervalSeconds)
        {
            ProcessCompressedFrame();
            CurrentFrameBones.Reset();
            bFrameStarted = false;
            FrameCounter++;
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
    // 기대 형식: Int(Index) + Float*7 (Loc XYZ, Quat XYZW)
    if (Packet.IntArgs.Num() < 1 || Packet.FloatArgs.Num() < 7)
    {
        return;
    }
    
    int32 ControlIndex = Packet.IntArgs[0];
    
    FVector Location(Packet.FloatArgs[0], Packet.FloatArgs[1], Packet.FloatArgs[2]);
    FQuat Rotation(Packet.FloatArgs[3], Packet.FloatArgs[4], Packet.FloatArgs[5], Packet.FloatArgs[6]);
    
    if (bUseCompression)
    {
        if (!bFrameStarted)
        {
            bFrameStarted = true;
            FrameStartTime = GetWorld()->GetTimeSeconds();
            CurrentFrameBones.Reset();
        }

        FBoneData BoneData;
        BoneData.Index = ControlIndex;
        BoneData.Location = Location;
        BoneData.Rotation = Rotation;
        CurrentFrameBones.Add(BoneData);

        if (CurrentFrameBones.Num() >= 50)
        {
            ProcessCompressedFrame();
            CurrentFrameBones.Reset();
            bFrameStarted = false;
        }
    }
    else
    {
        FTransform ControlTransform(Rotation, Location, FVector::OneVector);
        
        static int32 LogCount = 0;
        if (LogCount++ < 10)
        {
            UE_LOG(LogTemp, Warning, TEXT("[MocapData] 🔔 델리게이트 발동: Index=%d"), ControlIndex);
        }
        
        OnControlDataReceived.Broadcast(ControlIndex, ControlTransform);
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

void UMocapDataComponent::ProcessCompressedFrame()
{
    if (CurrentFrameBones.Num() == 0)
    {
        return;
    }

    const bool bIsKeyframeNow = IsKeyframe();

    CurrentCompressedFrame.Bones.Reset();
    CurrentCompressedFrame.Timestamp = GetWorld()->GetTimeSeconds();
    CurrentCompressedFrame.bDeltaCompressed = bUseDeltaCompression && !bIsKeyframeNow;

    int32 OriginalSize = CurrentFrameBones.Num() * 32;

    for (const FBoneData& BoneData : CurrentFrameBones)
    {
        if (BoneData.Index >= 256) continue;

        FCompressedBoneTransform CompressedBone(
            static_cast<uint8>(BoneData.Index),
            BoneData.Location,
            BoneData.Rotation
        );

        // Keyframe이면 모든 본 전송
        if (bIsKeyframeNow)
        {
            CurrentCompressedFrame.Bones.Add(CompressedBone);
            BoneCache.Add(CompressedBone.BoneIndex, CompressedBone);

            FTransform Transform = CompressedBone.Decompress();
            OnControlDataReceived.Broadcast(CompressedBone.BoneIndex, Transform);
        }
        else if (bUseDeltaCompression)
        {
            // Delta 압축: 변경된 본만 전송 (LOD 기반 threshold 사용)
            if (ShouldSendBone(CompressedBone.BoneIndex, CompressedBone))
            {
                CurrentCompressedFrame.Bones.Add(CompressedBone);
                BoneCache.Add(CompressedBone.BoneIndex, CompressedBone);

                FTransform Transform = CompressedBone.Decompress();
                OnControlDataReceived.Broadcast(CompressedBone.BoneIndex, Transform);
            }
        }
        else
        {
            CurrentCompressedFrame.Bones.Add(CompressedBone);

            FTransform Transform = CompressedBone.Decompress();
            OnControlDataReceived.Broadcast(CompressedBone.BoneIndex, Transform);
        }
    }

    int32 CompressedSize = 6 + (CurrentCompressedFrame.Bones.Num() * 14);
    LastFrameBoneCount = CurrentCompressedFrame.Bones.Num();
    CompressionRatio = OriginalSize > 0 ? ((float)CompressedSize / (float)OriginalSize) : 1.0f;

    // 기존 델리게이트
    OnCompressedFrameReceived.Broadcast(CurrentCompressedFrame);

    // 네트워크 최적화된 패킹 프레임 생성 및 브로드캐스트
    FPackedMotionFrame PackedFrame;
    PackedFrame.PackFromCompressedFrame(CurrentCompressedFrame, bIsKeyframeNow, static_cast<uint8>(FrameCounter % 256));
    PackedFrame.InterpolationDuration = CurrentFrameIntervalMs / 1000.0f;
    LastPackedDataSize = PackedFrame.GetPackedSize();

    OnPackedFrameReceived.Broadcast(PackedFrame);

    // 디버그 로그 (30프레임마다)
    if (FrameCounter % 30 == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MocapData] 프레임 #%d | %s | 본:%d/%d | 압축률:%.1f%% | 패킹:%d bytes | 간격:%.1fms"),
            FrameCounter,
            bIsKeyframeNow ? TEXT("KEYFRAME") : TEXT("delta"),
            CurrentCompressedFrame.Bones.Num(),
            CurrentFrameBones.Num(),
            CompressionRatio * 100.0f,
            LastPackedDataSize,
            CurrentFrameIntervalMs);
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