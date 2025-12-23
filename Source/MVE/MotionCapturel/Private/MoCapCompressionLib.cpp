#include "../Public/MoCapCompressionLib.h"

FCompressedBoneData UMoCapCompressionLib::CompressBoneData(int32 InBoneID, FTransform InTransform)
{
	FCompressedBoneData Data;
	Data.BoneID = InBoneID; 

	Data.Location = InTransform.GetLocation();
	FRotator Rot = InTransform.GetRotation().Rotator();

	auto CompressAxis = [](double Angle) -> int32 {
		Angle = FRotator::NormalizeAxis(Angle);

		return (int32)((Angle + 180.0) / 360.0 * 65535.0);
	};

	Data.Pitch = CompressAxis(Rot.Pitch);
	Data.Yaw = CompressAxis(Rot.Yaw);
	Data.Roll = CompressAxis(Rot.Roll);

	return Data;
}

FTransform UMoCapCompressionLib::DecompressBoneData(const FCompressedBoneData& InData, int32& OutBoneID)
{
	OutBoneID = InData.BoneID;
	
	FVector Loc = InData.Location;
	
	auto DecompressAxis = [](int32 Value) -> double {
		return ((double)Value / 65535.0 * 360.0) - 180.0;
	};

	FRotator Rot;
	Rot.Pitch = DecompressAxis(InData.Pitch);
	Rot.Yaw = DecompressAxis(InData.Yaw);
	Rot.Roll = DecompressAxis(InData.Roll);

	return FTransform(Rot, Loc, FVector::OneVector);
}

TArray<FCompressedBoneData> UMoCapCompressionLib::CompressMoCapBatch(const TMap<int32, FTransform>& InBoneMap)
{
	TArray<FCompressedBoneData> ResultArray;
	ResultArray.Reserve(InBoneMap.Num());

	for (const TPair<int32, FTransform>& Pair : InBoneMap)
	{
		ResultArray.Add(CompressBoneData(Pair.Key, Pair.Value));
	}
	return ResultArray;
}