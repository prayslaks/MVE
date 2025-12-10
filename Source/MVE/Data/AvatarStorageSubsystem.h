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
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// GLB 파일 선택 다이얼로그
	UFUNCTION(BlueprintCallable)
	FString OpenFileDialog();

	// GLB 파일 저장
	UFUNCTION(BlueprintCallable)
	bool SaveAvatarFile(const FString& SourceFilePath, FAvatarData& OutData);

	// 저장된 아바타 목록 가져오기
	UFUNCTION(BlueprintCallable)
	TArray<FAvatarData> GetSavedAvatars();

	// 특정 아바타 데이터 가져오기
	UFUNCTION(BlueprintCallable)
	bool GetAvatarData(const FString& UniqueID, FAvatarData& OutData);
	
	FString SavePath; // Saved/Avatars/
	TArray<FAvatarData> AvatarList;

	void LoadAvatarList();
	void SaveAvatarList();
	FString GenerateUniqueID(const FString& FileName);



	// 클래스 내부에 추가
public:
	// 프리뷰 액터 생성
	UFUNCTION(BlueprintCallable, Category = "Preview")
	AAvatarPreviewActor* CreatePreviewActor(UWorld* World);

	// 프리뷰 업데이트
	UFUNCTION(BlueprintCallable, Category = "Preview")
	void UpdatePreview(const FString& UniqueID);

private:
	UPROPERTY()
	TObjectPtr<AAvatarPreviewActor> PreviewActor;
};