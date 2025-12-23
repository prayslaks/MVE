#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MoCapCompressionLib.generated.h"

// 압축된 개별 본 데이터 구조체
USTRUCT(BlueprintType)
struct FCompressedBoneData
{
	GENERATED_BODY()

	// 본 ID (0~255 범위지만 BP 호환성을 위해 int32 사용)
	UPROPERTY(BlueprintReadWrite)
	int32 BoneID; 

	// 위치: 소수점 1자리까지 정밀도 보장 (언리얼 내장 NetQuantize 사용 - 약 6~12바이트 소모)
	UPROPERTY(BlueprintReadWrite)
	FVector_NetQuantize10 Location;

	// 회전: 0~65535 값을 담지만 BP 호환성을 위해 int32 그릇에 담음
	UPROPERTY(BlueprintReadWrite)
	int32 Pitch;

	UPROPERTY(BlueprintReadWrite)
	int32 Yaw;

	UPROPERTY(BlueprintReadWrite)
	int32 Roll;
};

UCLASS()
class UMoCapCompressionLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 변환된 Transform을 압축하는 함수
	UFUNCTION(BlueprintCallable, Category = "MoCap Optimization")
	static FCompressedBoneData CompressBoneData(int32 InBoneID, FTransform InTransform);

	// 압축된 데이터를 다시 Transform으로 복원하는 함수
	UFUNCTION(BlueprintPure, Category = "MoCap Optimization")
	static FTransform DecompressBoneData(const FCompressedBoneData& InData, int32& OutBoneID);
    
	// 여러 개의 데이터를 한 번에 압축 (배열 처리)
	UFUNCTION(BlueprintCallable, Category = "MoCap Optimization")
	static TArray<FCompressedBoneData> CompressMoCapBatch(const TMap<int32, FTransform>& InBoneMap);
};