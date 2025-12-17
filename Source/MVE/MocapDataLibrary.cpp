#include "MocapDataLibrary.h"

void UMocapDataLibrary::DecompressFrame(
	const FCompressedMotionFrame& CompressedFrame, 
	TArray<int32>& OutIndices, 
	TArray<FTransform>& OutTransforms)
{
	OutIndices.Reset();
	OutTransforms.Reset();

	OutIndices.Reserve(CompressedFrame.Bones.Num());
	OutTransforms.Reserve(CompressedFrame.Bones.Num());

	for (const FCompressedBoneTransform& CompressedBone : CompressedFrame.Bones)
	{
		OutIndices.Add(CompressedBone.BoneIndex);
		OutTransforms.Add(CompressedBone.Decompress());
	}
}

void UMocapDataLibrary::UpdateTransformArray(
	const FCompressedMotionFrame& CompressedFrame, 
	TArray<FTransform>& TransformArray)
{
	// 배열 크기 확보 (50개)
	if (TransformArray.Num() < 50)
	{
		TransformArray.SetNum(50);
	}

	// 압축 해제 후 배열 업데이트
	for (const FCompressedBoneTransform& CompressedBone : CompressedFrame.Bones)
	{
		if (TransformArray.IsValidIndex(CompressedBone.BoneIndex))
		{
			TransformArray[CompressedBone.BoneIndex] = CompressedBone.Decompress();
		}
	}
}

int32 UMocapDataLibrary::GetCompressedFrameBoneCount(const FCompressedMotionFrame& CompressedFrame)
{
	return CompressedFrame.Bones.Num();
}

float UMocapDataLibrary::GetCompressedFrameTimestamp(const FCompressedMotionFrame& CompressedFrame)
{
	return CompressedFrame.Timestamp;
}