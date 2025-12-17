#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CompressedMotionData.h"
#include "MocapDataLibrary.generated.h"

UCLASS()
class MVE_API UMocapDataLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 압축 프레임을 Index + Transform 배열로 변환
	UFUNCTION(BlueprintCallable, Category = "Mocap")
	static void DecompressFrame(const FCompressedMotionFrame& CompressedFrame, TArray<int32>& OutIndices, TArray<FTransform>& OutTransforms);

	// 압축 프레임으로 Transform 배열 직접 업데이트
	UFUNCTION(BlueprintCallable, Category = "Mocap")
	static void UpdateTransformArray(const FCompressedMotionFrame& CompressedFrame, UPARAM(ref) TArray<FTransform>& TransformArray);

	// 압축 프레임의 본 개수 반환
	UFUNCTION(BlueprintPure, Category = "Mocap")
	static int32 GetCompressedFrameBoneCount(const FCompressedMotionFrame& CompressedFrame);

	// 압축 프레임의 타임스탬프 반환
	UFUNCTION(BlueprintPure, Category = "Mocap")
	static float GetCompressedFrameTimestamp(const FCompressedMotionFrame& CompressedFrame);
};