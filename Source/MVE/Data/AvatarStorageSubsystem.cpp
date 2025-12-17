// AvatarStorageSubsystem.cpp
#include "AvatarStorageSubsystem.h"
#include "MVE.h"
#include "MVE/UI/Widget/Studio/Public/AvatarPreviewActor.h"
#include "IDesktopPlatform.h"
#include "DesktopPlatformModule.h"
#include "Framework/Application/SlateApplication.h"

void UAvatarStorageSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTemp, Warning, TEXT(" AvatarStorageSubsystem 초기화됨"));
}

void UAvatarStorageSubsystem::Deinitialize()
{
	AvatarDataMap.Empty();
	
	UE_LOG(LogTemp, Warning, TEXT("AvatarStorageSubsystem 종료됨"));
	
	Super::Deinitialize();
}

FString UAvatarStorageSubsystem::OpenFileDialog()
{
	TArray<FString> OutFiles;
	
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		UE_LOG(LogTemp, Error, TEXT(" DesktopPlatform을 가져올 수 없음"));
		return FString();
	}

	void* ParentWindowHandle = nullptr;
	if (FSlateApplication::IsInitialized())
	{
		TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
		if (ParentWindow.IsValid())
		{
			ParentWindowHandle = ParentWindow->GetNativeWindow()->GetOSWindowHandle();
		}
	}

	const FString FileTypes = TEXT("GLB Files (*.glb)|*.glb|All Files (*.*)|*.*");
	const FString DefaultPath = FPaths::ProjectContentDir();
	
	bool bFileSelected = DesktopPlatform->OpenFileDialog(
		ParentWindowHandle,
		TEXT("GLB 파일 선택"),
		DefaultPath,
		TEXT(""),
		FileTypes,
		EFileDialogFlags::None,
		OutFiles
	);

	if (bFileSelected && OutFiles.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT(" 파일 선택됨: %s"), *OutFiles[0]);
		return OutFiles[0];
	}

	return FString();
}

bool UAvatarStorageSubsystem::SaveAvatarFile(const FString& FilePath, FAvatarData& OutData)
{
	UE_LOG(LogTemp, Warning, TEXT("🔍 [Storage-1] SaveAvatarFile 시작: %s"), *FilePath);
	
	if (FilePath.IsEmpty() || !FPaths::FileExists(FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT(" [Storage-1] 잘못된 파일 경로: %s"), *FilePath);
		return false;
	}

	// UniqueID 생성
	FString UniqueID = FGuid::NewGuid().ToString();
	
	// FAvatarData 생성
	OutData.UniqueID = UniqueID;
	OutData.FileName = FPaths::GetCleanFilename(FilePath);
	OutData.FilePath = FilePath;
	OutData.SavedDate = FDateTime::Now();
	OutData.ThumbnailTexture = nullptr;  // 나중에 UpdateAvatarThumbnail로 설정됨
	
	// 메모리에 저장
	AvatarDataMap.Add(UniqueID, OutData);
	
	UE_LOG(LogTemp, Warning, TEXT(" [Storage-1] Avatar 저장됨: %s, ID: %s"), *OutData.FileName, *UniqueID);
	UE_LOG(LogTemp, Warning, TEXT("   - 현재 저장된 Avatar 개수: %d"), AvatarDataMap.Num());
	
	return true;
}

//  매개변수 이름 수정
bool UAvatarStorageSubsystem::UpdateAvatarThumbnail(const FString& UniqueID, UTexture2D* Thumbnail)
{
	UE_LOG(LogTemp, Warning, TEXT("🔍 [Storage-2] UpdateAvatarThumbnail 시작: %s"), *UniqueID);
	
	if (UniqueID.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT(" [Storage-2] UniqueID가 비어있음"));
		return false;
	}
	
	if (!Thumbnail)
	{
		UE_LOG(LogTemp, Error, TEXT(" [Storage-2] Thumbnail이 null"));
		return false;
	}
	
	// 메모리에서 데이터 찾기
	FAvatarData* Data = AvatarDataMap.Find(UniqueID);
	
	if (Data)
	{
		UE_LOG(LogTemp, Warning, TEXT(" [Storage-2] Avatar 찾음: %s"), *Data->FileName);
		
		// 썸네일 설정
		Data->ThumbnailTexture = Thumbnail;
		
		UE_LOG(LogTemp, Warning, TEXT(" [Storage-2] 썸네일 업데이트 성공!"));
		UE_LOG(LogTemp, Warning, TEXT("   - Thumbnail Size: %dx%d"), 
			Thumbnail->GetSizeX(), 
			Thumbnail->GetSizeY());
		
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT(" [Storage-2] Avatar를 찾을 수 없음: %s"), *UniqueID);
		UE_LOG(LogTemp, Error, TEXT("   - 현재 저장된 Avatar 개수: %d"), AvatarDataMap.Num());
		
		// 디버깅: 저장된 모든 ID 출력
		for (const auto& Pair : AvatarDataMap)
		{
			UE_LOG(LogTemp, Error, TEXT("   - 저장된 ID: %s"), *Pair.Key);
		}
		
		return false;
	}
}

TArray<FAvatarData> UAvatarStorageSubsystem::GetSavedAvatars() const
{
	UE_LOG(LogTemp, Warning, TEXT("🔍 [Storage-3] GetSavedAvatars 호출"));
	
	TArray<FAvatarData> Result;
	AvatarDataMap.GenerateValueArray(Result);
	
	UE_LOG(LogTemp, Warning, TEXT(" [Storage-3] 반환할 Avatar 개수: %d"), Result.Num());
	
	// 각 Avatar의 썸네일 상태 출력
	for (int32 i = 0; i < Result.Num(); i++)
	{
		UE_LOG(LogTemp, Warning, TEXT("   - [%d] %s: Thumbnail %s"), 
			i, 
			*Result[i].FileName,
			Result[i].ThumbnailTexture ? TEXT("있음") : TEXT("없음"));
	}
	
	// 시간순 정렬 (최신순)
	Result.Sort([](const FAvatarData& A, const FAvatarData& B) {
		return A.SavedDate > B.SavedDate;
	});
	
	return Result;
}

bool UAvatarStorageSubsystem::GetAvatarData(const FString& UniqueID, FAvatarData& OutData) const
{
	UE_LOG(LogTemp, Warning, TEXT("🔍 [Storage-4] GetAvatarData: %s"), *UniqueID);
	
	const FAvatarData* Data = AvatarDataMap.Find(UniqueID);
	
	if (Data)
	{
		OutData = *Data;
		
		UE_LOG(LogTemp, Warning, TEXT(" [Storage-4] Avatar 찾음: %s"), *Data->FileName);
		UE_LOG(LogTemp, Warning, TEXT("   - Thumbnail: %s"), 
			Data->ThumbnailTexture ? TEXT("있음") : TEXT("없음"));
		
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT(" [Storage-4] Avatar를 찾을 수 없음: %s"), *UniqueID);
		return false;
	}
}

AAvatarPreviewActor* UAvatarStorageSubsystem::CreatePreviewActor(UWorld* World)
{
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT(" World is null"));
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	AAvatarPreviewActor* PreviewActor = World->SpawnActor<AAvatarPreviewActor>(
		AAvatarPreviewActor::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (PreviewActor)
	{
		UE_LOG(LogTemp, Warning, TEXT(" PreviewActor 생성됨"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT(" PreviewActor 생성 실패"));
	}

	return PreviewActor;
}