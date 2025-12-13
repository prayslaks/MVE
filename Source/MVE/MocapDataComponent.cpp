#include "MocapDataComponent.h"
#include "Receive/OscDispatcher.h"
#include "Common/OscFunctionLibrary.h"

UMocapDataComponent::UMocapDataComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UMocapDataComponent::~UMocapDataComponent()
{
}

void UMocapDataComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("[MocapData] BeginPlay 시작"));

	// OSC Dispatcher 획득
	UOscDispatcher* Dispatcher = UOscDispatcher::Get();
	if (!Dispatcher)
	{
		UE_LOG(LogTemp, Error, TEXT("[MocapData] OSC Dispatcher 획득 실패!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[MocapData] OSC Dispatcher 획득 성공"));

	// Receiver 등록
	OscReceiver = MakeUnique<FMocapOscReceiver>(this);
	Dispatcher->RegisterReceiver(OscReceiver.Get());

	UE_LOG(LogTemp, Warning, TEXT("[MocapData] OSC Receiver 등록 완료 - 포트:%d, 필터:%s"),
		ReceivePort,
		*AddressFilter.ToString());
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

void UMocapDataComponent::OnOscMessageReceived(
	const FName& Address, 
	const TArray<FOscDataElemStruct>& Data, 
	const FString& SenderIp)
{
	FString AddressPath = Address.ToString();

	// 바디 컨트롤 데이터
	ProcessControlData(Data);

	// 페이스 데이터 처리 (활성화된 경우만)
	if (AddressPath.Equals(TEXT("/Remocapp/UE/Face"), ESearchCase::IgnoreCase))
	{
		if (bEnableFaceData)
		{
			ProcessFaceMorphData(Data);
		}
		return;
	}
	
}

void UMocapDataComponent::ProcessControlData(const TArray<FOscDataElemStruct>& Data)
{
	if (Data.Num() < 8)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MocapData] 데이터 부족: %d개 (최소 8개 필요)"), Data.Num());
		return;
	}

	// Index 추출
	int32 ControlIndex = UOscFunctionLibrary::AsInt(Data[0]);

	// Float 배열 추출
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
		UE_LOG(LogTemp, Warning, TEXT("[MocapData] Float 값 부족: %d개 (7개 필요)"), Values.Num());
		return;
	}

	// Transform 구성
	FVector Location(Values[0], Values[1], Values[2]);
	FQuat Rotation(Values[3], Values[4], Values[5], Values[6]);
	FTransform ControlTransform(Rotation, Location, FVector::OneVector);

	// 델리게이트 브로드캐스트 
	OnControlDataReceived.Broadcast(ControlIndex, ControlTransform);

	// 디버그 로그
	static int32 LogCount = 0;
	if (LogCount++ < 10)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MocapData #%d] Index:%d, Loc:(%.2f,%.2f,%.2f)"),
			LogCount,
			ControlIndex,
			Location.X, Location.Y, Location.Z);
	}
}

void UMocapDataComponent::ProcessFaceMorphData(const TArray<FOscDataElemStruct>& Data)
{
	// 현재는 안받을꺼니까 들어오면 무시 TODO 페이셜 까지 고치면 이 부분 없앨껏
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

	// 델리게이트 브로드캐스트
	OnFaceMorphDataReceived.Broadcast(MorphTargets);

	static int32 LogCount = 0;
	if (LogCount++ < 5)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MocapData Face #%d] 모프 타겟: %d개"), LogCount, MorphTargets.Num());
	}
}
