#include "AvatarStorageSubsystem.h"
#include "MVE.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "Framework/Application/SlateApplication.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Mve/UI/Widget/Studio/Public/AvatarPreviewActor.h"

void UAvatarStorageSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Saved/Avatars/ 경로 설정
	SavePath = FPaths::ProjectSavedDir() / TEXT("Avatars/");

	// 디렉토리 생성
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*SavePath))
	{
		PlatformFile.CreateDirectory(*SavePath);
		PRINTLOG(TEXT("Created avatar directory: %s"), *SavePath);
	}

	// 저장된 목록 로드
	LoadAvatarList();

	PRINTLOG(TEXT("AvatarStorageSubsystem Initialized"));
}

FString UAvatarStorageSubsystem::OpenFileDialog()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		return FString();
	}

	TArray<FString> OutFiles;
	const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
	const FString FileTypes = TEXT("GLB Files (*.glb)|*.glb");

	bool bOpened = DesktopPlatform->OpenFileDialog(
		ParentWindowHandle,
		TEXT("GLB 파일 선택"),
		FPaths::ProjectDir(),
		TEXT(""),
		FileTypes,
		EFileDialogFlags::None,
		OutFiles
	);

	if (bOpened && OutFiles.Num() > 0)
	{
		return OutFiles[0];
	}

	return FString();
}

FString UAvatarStorageSubsystem::GenerateUniqueID(const FString& FileName)
{
	FDateTime Now = FDateTime::Now();
	return FString::Printf(TEXT("%s_%04d%02d%02d_%02d%02d%02d"),
		*FPaths::GetBaseFilename(FileName),
		Now.GetYear(), Now.GetMonth(), Now.GetDay(),
		Now.GetHour(), Now.GetMinute(), Now.GetSecond());
}

bool UAvatarStorageSubsystem::SaveAvatarFile(const FString& SourceFilePath, FAvatarData& OutData)
{
	// 파일 존재 확인
	if (!FPaths::FileExists(SourceFilePath))
	{
		PRINTLOG(TEXT("Source file not found: %s"), *SourceFilePath);
		return false;
	}

	// 확장자 확인
	if (FPaths::GetExtension(SourceFilePath).ToLower() != TEXT("glb"))
	{
		PRINTLOG(TEXT("Invalid file extension"));
		return false;
	}

	// 파일 정보 생성
	FString FileName = FPaths::GetCleanFilename(SourceFilePath);
	FString UniqueID = GenerateUniqueID(FileName);
	FString DestPath = SavePath / (UniqueID + TEXT(".glb"));

	// 파일 복사
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.CopyFile(*DestPath, *SourceFilePath))
	{
		PRINTLOG(TEXT("Failed to copy file"));
		return false;
	}

	// 데이터 생성
	OutData.FileName = FileName;
	OutData.UniqueID = UniqueID;
	OutData.FilePath = DestPath;
	OutData.SavedDate = FDateTime::Now();

	// 목록에 추가
	AvatarList.Add(OutData);
	SaveAvatarList();

	PRINTLOG(TEXT("Avatar saved: %s"), *UniqueID);
	return true;
}

TArray<FAvatarData> UAvatarStorageSubsystem::GetSavedAvatars()
{
	return AvatarList;
}

bool UAvatarStorageSubsystem::GetAvatarData(const FString& UniqueID, FAvatarData& OutData)
{
	for (const FAvatarData& Data : AvatarList)
	{
		if (Data.UniqueID == UniqueID)
		{
			OutData = Data;
			return true;
		}
	}
	return false;
}

void UAvatarStorageSubsystem::LoadAvatarList()
{
	FString JsonPath = SavePath / TEXT("AvatarList.json");
	FString JsonString;

	if (FFileHelper::LoadFileToString(JsonString, *JsonPath))
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			const TArray<TSharedPtr<FJsonValue>>* JsonArray;
			if (JsonObject->TryGetArrayField(TEXT("Avatars"), JsonArray))
			{
				for (const TSharedPtr<FJsonValue>& JsonValue : *JsonArray)
				{
					TSharedPtr<FJsonObject> AvatarObj = JsonValue->AsObject();
					
					FAvatarData Data;
					Data.FileName = AvatarObj->GetStringField(TEXT("FileName"));
					Data.UniqueID = AvatarObj->GetStringField(TEXT("UniqueID"));
					Data.FilePath = AvatarObj->GetStringField(TEXT("FilePath"));
					FDateTime::Parse(AvatarObj->GetStringField(TEXT("SavedDate")), Data.SavedDate);

					AvatarList.Add(Data);
				}
			}
		}
	}

	PRINTLOG(TEXT("Loaded %d avatars"), AvatarList.Num());
}

void UAvatarStorageSubsystem::SaveAvatarList()
{
	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> JsonArray;

	for (const FAvatarData& Data : AvatarList)
	{
		TSharedPtr<FJsonObject> AvatarObj = MakeShareable(new FJsonObject);
		AvatarObj->SetStringField(TEXT("FileName"), Data.FileName);
		AvatarObj->SetStringField(TEXT("UniqueID"), Data.UniqueID);
		AvatarObj->SetStringField(TEXT("FilePath"), Data.FilePath);
		AvatarObj->SetStringField(TEXT("SavedDate"), Data.SavedDate.ToString());

		JsonArray.Add(MakeShareable(new FJsonValueObject(AvatarObj)));
	}

	RootObject->SetArrayField(TEXT("Avatars"), JsonArray);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	FString JsonPath = SavePath / TEXT("AvatarList.json");
	FFileHelper::SaveStringToFile(OutputString, *JsonPath);
}

AAvatarPreviewActor* UAvatarStorageSubsystem::CreatePreviewActor(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	// 기존 액터가 있으면 제거
	if (PreviewActor)
	{
		PreviewActor->Destroy();
	}

	// 새 액터 스폰 (화면 밖에 배치)
	FVector SpawnLocation = FVector(10000.0f, 0.0f, 0.0f);
	FRotator SpawnRotation = FRotator::ZeroRotator;

	PreviewActor = World->SpawnActor<AAvatarPreviewActor>(
		AAvatarPreviewActor::StaticClass(),
		SpawnLocation,
		SpawnRotation
	);

	if (PreviewActor)
	{
		PRINTLOG(TEXT("Preview actor created"));
	}

	return PreviewActor;
}

void UAvatarStorageSubsystem::UpdatePreview(const FString& UniqueID)
{
	if (!PreviewActor)
	{
		PRINTLOG(TEXT("PreviewActor is null"));
		return;
	}

	FAvatarData Data;
	if (GetAvatarData(UniqueID, Data))
	{
		PreviewActor->LoadAvatarMesh(Data.FilePath);
	}
}