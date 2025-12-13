#pragma once

#include "CoreMinimal.h"
#include "CompressedMotionData.generated.h"

// 16비트 압축 벡터 (6 bytes)
USTRUCT()
struct FCompressedVector
{
	GENERATED_BODY()

	// -327.68 ~ +327.67 범위, 0.01 정밀도
	int16 X = 0;
	int16 Y = 0;
	int16 Z = 0;

	FCompressedVector() = default;

	// 압축
	explicit FCompressedVector(const FVector& Vec)
	{
		X = FMath::Clamp(FMath::RoundToInt(Vec.X * 100.0f), -32768, 32767);
		Y = FMath::Clamp(FMath::RoundToInt(Vec.Y * 100.0f), -32768, 32767);
		Z = FMath::Clamp(FMath::RoundToInt(Vec.Z * 100.0f), -32768, 32767);
	}

	// 압축 해제
	FVector Decompress() const
	{
		return FVector(
			X / 100.0f,
			Y / 100.0f,
			Z / 100.0f
		);
	}

	// 커스텀 직렬화 (6 bytes)
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << X;
		Ar << Y;
		Ar << Z;
		bOutSuccess = true;
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FCompressedVector> : public TStructOpsTypeTraitsBase2<FCompressedVector>
{
	enum
	{
		WithNetSerializer = true
	};
};

// 압축된 Quaternion (7 bytes) - Smallest Three
USTRUCT()
struct FCompressedQuat
{
	GENERATED_BODY()

	// 가장 큰 컴포넌트 인덱스 (2 bits)
	uint8 LargestIndex : 2;
	
	// 부호 비트 (1 bit)
	uint8 SignBit : 1;
	
	// 예약 (5 bits)
	uint8 Reserved : 5;

	// 3개의 작은 컴포넌트 (-1 ~ +1 범위, 16비트)
	int16 A = 0;
	int16 B = 0;
	int16 C = 0;

	FCompressedQuat() : LargestIndex(0), SignBit(0), Reserved(0) {}

	// 압축 (Smallest Three 알고리즘)
	explicit FCompressedQuat(const FQuat& Quat)
	{
		FQuat Normalized = Quat;
		Normalized.Normalize();

		// FQuat 컴포넌트를 float로 명시적 변환
		const float X_f = static_cast<float>(Normalized.X);
		const float Y_f = static_cast<float>(Normalized.Y);
		const float Z_f = static_cast<float>(Normalized.Z);
		const float W_f = static_cast<float>(Normalized.W);

		// 절댓값 계산
		const float AbsX = FMath::Abs(X_f);
		const float AbsY = FMath::Abs(Y_f);
		const float AbsZ = FMath::Abs(Z_f);
		const float AbsW = FMath::Abs(W_f);

		// 배열로 정리
		const float Values[4] = { X_f, Y_f, Z_f, W_f };
		const float AbsValues[4] = { AbsX, AbsY, AbsZ, AbsW };

		// 가장 큰 컴포넌트 찾기
		LargestIndex = 0;
		float MaxAbs = AbsValues[0];
		for (int32 i = 1; i < 4; ++i)
		{
			if (AbsValues[i] > MaxAbs)
			{
				MaxAbs = AbsValues[i];
				LargestIndex = i;
			}
		}

		// 부호 저장
		SignBit = (Values[LargestIndex] < 0.0f) ? 1 : 0;

		// 나머지 3개 컴포넌트 압축
		constexpr float Scale = 32767.0f; // int16 max
		int32 Idx = 0;
		int16* CompressedValues[3] = { &A, &B, &C };

		for (int32 i = 0; i < 4; ++i)
		{
			if (i == LargestIndex) continue;
			
			float Value = Values[i];
			*CompressedValues[Idx++] = FMath::Clamp(
				FMath::RoundToInt(Value * Scale),
				-32767, 32767
			);
		}
	}

	// 압축 해제
	FQuat Decompress() const
	{
		constexpr float InvScale = 1.0f / 32767.0f;

		// 명시적 float 변환
		const float A_f = static_cast<float>(A) * InvScale;
		const float B_f = static_cast<float>(B) * InvScale;
		const float C_f = static_cast<float>(C) * InvScale;

		// 4번째 컴포넌트 계산 (가장 큰 값)
		const float SumSquares = A_f * A_f + B_f * B_f + C_f * C_f;
		float Largest = FMath::Sqrt(FMath::Max(1.0f - SumSquares, 0.0f));
		
		if (SignBit)
		{
			Largest = -Largest;
		}

		// Quaternion 재구성
		FQuat Result = FQuat::Identity;
		
		// 압축된 3개 값을 원래 위치에 재배치
		const float SmallValues[3] = { A_f, B_f, C_f };
		int32 SmallIdx = 0;

		for (int32 i = 0; i < 4; ++i)
		{
			double Value = (i == LargestIndex) ? Largest : SmallValues[SmallIdx++];

			switch (i)
			{
			case 0: Result.X = Value; break;
			case 1: Result.Y = Value; break;
			case 2: Result.Z = Value; break;
			case 3: Result.W = Value; break;
			}
		}

		Result.Normalize();
		return Result;
	}

	// 커스텀 직렬화 (7 bytes: 1 byte header + 6 bytes data)
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		uint8 Header = (LargestIndex << 6) | (SignBit << 5) | Reserved;
		Ar << Header;
		
		if (Ar.IsLoading())
		{
			LargestIndex = (Header >> 6) & 0x03;
			SignBit = (Header >> 5) & 0x01;
			Reserved = Header & 0x1F;
		}

		Ar << A;
		Ar << B;
		Ar << C;

		bOutSuccess = true;
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FCompressedQuat> : public TStructOpsTypeTraitsBase2<FCompressedQuat>
{
	enum
	{
		WithNetSerializer = true
	};
};

// 압축된 본 트랜스폼 (14 bytes)
USTRUCT()
struct FCompressedBoneTransform
{
	GENERATED_BODY()

	UPROPERTY()
	uint8 BoneIndex = 0; // 1 byte (255개 본까지 지원)

	UPROPERTY()
	FCompressedVector Location; // 6 bytes

	UPROPERTY()
	FCompressedQuat Rotation; // 7 bytes

	// 총 14 bytes per bone

	FCompressedBoneTransform() = default;

	FCompressedBoneTransform(uint8 InBoneIndex, const FVector& Loc, const FQuat& Rot)
		: BoneIndex(InBoneIndex)
		, Location(Loc)
		, Rotation(Rot)
	{}

	FTransform Decompress() const
	{
		return FTransform(
			Rotation.Decompress(),
			Location.Decompress(),
			FVector::OneVector
		);
	}

	// 커스텀 직렬화
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << BoneIndex;
		Location.NetSerialize(Ar, Map, bOutSuccess);
		Rotation.NetSerialize(Ar, Map, bOutSuccess);
		bOutSuccess = true;
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FCompressedBoneTransform> : public TStructOpsTypeTraitsBase2<FCompressedBoneTransform>
{
	enum
	{
		WithNetSerializer = true
	};
};

// 프레임 단위 압축 데이터 (배열)
USTRUCT()
struct FCompressedMotionFrame
{
	GENERATED_BODY()

	// 타임스탬프 (4 bytes)
	UPROPERTY()
	float Timestamp = 0.0f;

	// 압축된 본 배열 (14 bytes × N)
	UPROPERTY()
	TArray<FCompressedBoneTransform> Bones;

	// 델타 전송: 이전 프레임과 달라진 본만 전송
	UPROPERTY()
	bool bDeltaCompressed = false;

	// 커스텀 직렬화
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << Timestamp;
		Ar << bDeltaCompressed;

		uint8 BoneCount = Bones.Num();
		Ar << BoneCount;

		if (Ar.IsLoading())
		{
			Bones.SetNum(BoneCount);
		}

		for (FCompressedBoneTransform& Bone : Bones)
		{
			Bone.NetSerialize(Ar, Map, bOutSuccess);
		}

		bOutSuccess = true;
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FCompressedMotionFrame> : public TStructOpsTypeTraitsBase2<FCompressedMotionFrame>
{
	enum
	{
		WithNetSerializer = true
	};
};