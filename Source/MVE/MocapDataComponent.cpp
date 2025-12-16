#include "MocapDataComponent.h"
#include "Receive/OscDispatcher.h"
#include "Common/OscFunctionLibrary.h"
#include "Misc/CompressedGrowableBuffer.h"

UMocapDataComponent::UMocapDataComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

UMocapDataComponent::~UMocapDataComponent()
{
}

void UMocapDataComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("[MocapData] BeginPlay 시작"));
 
	UOscDispatcher* Dispatcher = UOscDispatcher::Get();
	if (!Dispatcher)
	{
		UE_LOG(LogTemp, Error, TEXT("[MocapData] OSC Dispatcher 획득 실패"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[MocapData] OSC Dispatcher 획득 성공"));

	OscReceiver = MakeUnique<FMocapOscReceiver>(this);
	Dispatcher->RegisterReceiver(OscReceiver.Get());

	UE_LOG(LogTemp, Warning, TEXT("[MocapData] 등록 완료 - 포트:%d, 필터:%s, 압축:%s, 델타:%s, 페이스:%s"),
		ReceivePort,
		*AddressFilter.ToString(),
		bUseCompression ? TEXT("ON") : TEXT("OFF"),
		bUseDeltaCompression ? TEXT("ON") : TEXT("OFF"),
		bEnableFaceData ? TEXT("ON") : TEXT("OFF"));
}

void UMocapDataComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UOscDispatcher* Dispatcher = UOscDispatcher::Get();
	if (Dispatcher && OscReceiver)
	{
		Dispatcher->UnregisterReceiver(OscReceiver.Get());
		UE_LOG(LogTemp, Warning, TEXT("[MocapData] OSC Receiver 등록 해제"));
	}

	OscReceiver.Reset();
}

void UMocapDataComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 압축 모드에서 프레임 타임아웃 체크 (33ms = 30 FPS)
	if (bUseCompression && bFrameStarted)
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		if ((CurrentTime - FrameStartTime) > 0.033f)
		{
			// 프레임 완성
			ProcessCompressedFrame();
			CurrentFrameBones.Reset();
			bFrameStarted = false;
		}
	}
}

void UMocapDataComponent::OnOscMessageReceived(
	const FName& Address, 
	const TArray<FOscDataElemStruct>& Data, 
	const FString& SenderIp)
{
	FString AddressPath = Address.ToString();

	// 페이스 데이터
	if (AddressPath.Equals(TEXT("/Remocapp/UE/Face"), ESearchCase::IgnoreCase))
	{
		if (bEnableFaceData)
		{
			ProcessFaceMorphData(Data);
		}
		return;
	}

	// 컨트롤 데이터
	ProcessControlData(Data);
}

void UMocapDataComponent::ProcessControlData(const TArray<FOscDataElemStruct>& Data)
{
	if (Data.Num() < 8)
	{
		return;
	}

	int32 ControlIndex = UOscFunctionLibrary::AsInt(Data[0]);

	TArray<float> Values;
	Values.Reserve(7);

	for (int32 i = 1; i < Data.Num() && i <= 7; ++i)
	{
		if (Data[i].IsFloat())
		{
			Values.Add(UOscFunctionLibrary::AsFloat(Data[i]));
		}
	}

	if (Values.Num() < 7)
	{
		return;
	}

	FVector Location(Values[0], Values[1], Values[2]);
	FQuat Rotation(Values[3], Values[4], Values[5], Values[6]);

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
		OnControlDataReceived.Broadcast(ControlIndex, ControlTransform);

		static int32 LogCount = 0;
		if (LogCount++ < 10)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MocapData #%d] Index:%d, Loc:(%.2f,%.2f,%.2f)"),
				LogCount,
				ControlIndex,
				Location.X, Location.Y, Location.Z);
		}
	}
}

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
		if (BoneData.Index >= 256)
		{
			continue;
		}

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
			}
		}
		else
		{
			CurrentCompressedFrame.Bones.Add(CompressedBone);
		}
	}

	int32 CompressedSize = 6 + (CurrentCompressedFrame.Bones.Num() * 14);

	LastFrameBoneCount = CurrentCompressedFrame.Bones.Num();
	CompressionRatio = OriginalSize > 0 ? ((float)CompressedSize / (float)OriginalSize) : 1.0f;

	OnCompressedFrameReceived.Broadcast(CurrentCompressedFrame);

	static int32 FrameCount = 0;
	if (FrameCount++ % 30 == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MocapData 압축] 프레임 #%d | 본:%d개/%d개 | 원본:%d bytes → 압축:%d bytes (%.1f%%)"),
			FrameCount,
			CurrentCompressedFrame.Bones.Num(),
			CurrentFrameBones.Num(),
			OriginalSize,
			CompressedSize,
			CompressionRatio * 100.0f);
	}
}

bool UMocapDataComponent::ShouldSendBone(uint8 BoneIndex, const FCompressedBoneTransform& NewTransform)
{
	FCompressedBoneTransform* CachedBone = BoneCache.Find(BoneIndex);
	
	if (!CachedBone)
	{
		return true;
	}
	
	FVector OldLoc = CachedBone->Location.Decompress();
	FVector NewLoc = NewTransform.Location.Decompress();
	float LocDelta = FVector::DistSquared(OldLoc, NewLoc);
	
	if (LocDelta > DeltaThreshold * DeltaThreshold)
	{
		return true;
	}
	
	FQuat OldRot = CachedBone->Rotation.Decompress();
	FQuat NewRot = NewTransform.Rotation.Decompress();
	float RotDelta = FQuat::Error(OldRot, NewRot);
	
	if (RotDelta > DeltaThreshold)
	{
		return true;
	}
	
	return false;
}

void UMocapDataComponent::ProcessFaceMorphData(const TArray<FOscDataElemStruct>& Data)
{
	if (!bEnableFaceData)
	{
		return;
	}

	TArray<float> MorphTargets;
	MorphTargets.Reserve(52);

	for (const FOscDataElemStruct& Elem : Data)
	{
		if (Elem.IsFloat())
		{
			MorphTargets.Add(UOscFunctionLibrary::AsFloat(Elem));
		}
	}

	OnFaceMorphDataReceived.Broadcast(MorphTargets);
}
