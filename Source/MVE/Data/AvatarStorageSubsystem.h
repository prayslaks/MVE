// AvatarStorageSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/AvatarData.h" 
#include "AvatarStorageSubsystem.generated.h"

class AAvatarPreviewActor;

UCLASS()
class MVE_API UAvatarStorageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 초기화
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//  파일 다이얼로그 (GLB 선택)
	UFUNCTION(BlueprintCallable, Category = "Avatar Storage")
	FString OpenFileDialog();

	//  Avatar 파일 저장
	UFUNCTION(BlueprintCallable, Category = "Avatar Storage")
	bool SaveAvatarFile(const FString& FilePath, FAvatarData& OutData);

	//  썸네일 업데이트 함수 선언 (추가!) 
	UFUNCTION(BlueprintCallable, Category = "Avatar Storage")
	bool UpdateAvatarThumbnail(const FString& UniqueID, UTexture2D* Thumbnail);

	//  저장된 Avatar 목록 가져오기
	UFUNCTION(BlueprintCallable, Category = "Avatar Storage")
	TArray<FAvatarData> GetSavedAvatars() const;

	//  특정 Avatar 데이터 가져오기
	UFUNCTION(BlueprintCallable, Category = "Avatar Storage")
	bool GetAvatarData(const FString& UniqueID, FAvatarData& OutData) const;

	//  PreviewActor 생성
	UFUNCTION(BlueprintCallable, Category = "Avatar Storage")
	AAvatarPreviewActor* CreatePreviewActor(UWorld* World);
	
	//  메모리에 Avatar 데이터 저장 (UniqueID -> FAvatarData)
	TMap<FString, FAvatarData> AvatarDataMap;
};