// Copyright 2025 REMOCAPP, PTY LTD. All Rights Reserved.

#include "RMCRecorder.h"

#include "Editor.h"
#include "TakeMetaData.h"
#include "TakeRecorderSources.h"
#include "Kismet/GameplayStatics.h"
#include "LevelSequence.h"
#include "ObjectTools.h"
#include "RemocappEditor.h"
#include "RMCCustomTimecodeProvider.h"
#include "Recorder/TakeRecorderBlueprintLibrary.h"
#include "Subsystems/UnrealEditorSubsystem.h"
#include "SequencerTools.h"
#include "Exporters/AnimSeqExportOption.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "TakesUtils.h"
#include "Exporters/FbxExportOption.h"
#include "Interfaces/IPluginManager.h"
#include "Sections/MovieSceneSkeletalAnimationSection.h"
#include "Editor/UnrealEd/Private/FbxExporter.h"
#include "TakeRecorderSources/Private/TakeRecorderActorSource.h"
#include "UObject/SavePackage.h"

namespace
{
	bool IsComponentOrDescendantAllowed(const UActorRecorderPropertyMap* PropertyMap, const TArray<UActorComponent*>& AllowedComponents)
	{
		if (!PropertyMap)
		{
			return false;
		}

		UActorComponent* CurrentComponent = Cast<UActorComponent>(PropertyMap->RecordedObject.Get());
		if (CurrentComponent && AllowedComponents.Contains(CurrentComponent))
		{
			return true;
		}

		for (const UActorRecorderPropertyMap* ChildMap : PropertyMap->Children)
		{
			if (IsComponentOrDescendantAllowed(ChildMap, AllowedComponents))
			{
				return true;
			}
		}

		return false;
	}

	void FilterComponentProperties(UActorRecorderPropertyMap* PropertyMap, const TArray<UActorComponent*>& AllowedComponents)
	{
		if (!PropertyMap || !PropertyMap->RecordedObject.IsValid())
		{
			return;
		}

		UActorComponent* CurrentComponent = Cast<UActorComponent>(PropertyMap->RecordedObject.Get());
		const bool bShouldRecordProperties = CurrentComponent && AllowedComponents.Contains(CurrentComponent);

		for (FActorRecordedProperty& Prop : PropertyMap->Properties)
		{
			Prop.bEnabled = bShouldRecordProperties;
		}

		for (int32 i = PropertyMap->Children.Num() - 1; i >= 0; --i)
		{
			UActorRecorderPropertyMap* ChildMap = PropertyMap->Children[i];
			if (!IsComponentOrDescendantAllowed(ChildMap, AllowedComponents))
			{
				PropertyMap->Children.RemoveAt(i);
			}
			else
			{
				FilterComponentProperties(ChildMap, AllowedComponents);
			}
		}
	}
}

ARMCRecorder::ARMCRecorder()
{
	Countdown = ERMCRecorderCountdown::Zero;
	RootTakeSaveDirectory.Path = FPaths::ProjectDir();
	SceneName = FString(TEXT("Scene_1"));
	TakeNumber = 1;
	FPS = 30;
	AnimationDirectoryPath.Path = FPaths::ProjectDir() / TEXT("ExportedAnimations");
	RecordingComponentTagName = TEXT("Record");
	TrackingActorTagName = TEXT("RMCActor");
	OriginalTimecodeProvider = nullptr;
	CustomTimecodeProvider = nullptr;
}

void ARMCRecorder::BeginPlay()
{
	Super::BeginPlay();

	if (RecordingSequence) RecordingSequence->Initialize();
	TakeMetadata = NewObject<UTakeMetaData>(this, NAME_None);
	TakeRecorderSources = NewObject<UTakeRecorderSources>(this, NAME_None);
	if (TakeRecorderSources)
	{
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
		if (PlayerPawn)
		{
			SetSourceActor();
		}
	}

	if (GEditor && GEditor->GetEditorWorldContext().World())
	{
		FEditorDelegates::EndPIE.AddUObject(this, &ARMCRecorder::HandleEndPIE);
	}
}

void ARMCRecorder::SetupTakeRecorderParameters()
{
	FTakeRecorderUserParameters UserParameters;
	UserParameters.CountdownSeconds = (uint8)Countdown;
	UserParameters.bSaveRecordedAssets = false;
	UserParameters.ReduceKeysTolerance = .0001;
	UserParameters.bAutoLock = true;

	FTakeRecorderProjectParameters ProjectParameters;
	ProjectParameters.RootTakeSaveDir.Path = IPluginManager::Get().FindPlugin("Remocapp")->GetContentDir() /
		TEXT("Animations");
	ProjectParameters.TakeSaveDir = FString(TEXT("{slate}_")) + FDateTime::Now().ToString().Replace(
		TEXT("."), TEXT("_"));
	ProjectParameters.DefaultSlate = SceneName;
	ProjectParameters.bRecordSourcesIntoSubSequences = false;
	ProjectParameters.bRecordToPossessable = true;
	ProjectParameters.bShowNotifications = false;
	ProjectParameters.bStartAtCurrentTimecode = false;
	ProjectParameters.RecordingClockSource = EUpdateClockSource::RelativeTimecode;

	TakeRecorderParameters.User = UserParameters;
	TakeRecorderParameters.Project = ProjectParameters;
	TakeRecorderParameters.TakeRecorderMode = ETakeRecorderMode::RecordIntoSequence;
}

void ARMCRecorder::SetupTakeMetadata()
{
	if (TakeMetadata == nullptr)
	{
		UE_LOG(LogRemocapp, Error, TEXT("Take meta data not valid"))
		OnRecordingFailed.Broadcast();
		return;
	}
	TakeMetadata->SetFrameRate(FFrameRate(FPS, 1));
	TakeMetadata->SetLevelOrigin(GetLevel());
	TakeMetadata->SetTakeNumber(TakeNumber);
	TakeMetadata->SetSlate(SceneName);
}

void ARMCRecorder::SetupRecorder()
{
	if (RecordingSequence == nullptr)
	{
		UE_LOG(LogRemocapp, Error, TEXT("No valid level sequence asset found"))
		OnRecordingFailed.Broadcast();
		return;
	}
	SetupTakeRecorderParameters();
	SetupTakeMetadata();
	if (RecordingSequence) RecordingSequence->Initialize();
	UMovieScene* MovieScene = RecordingSequence->GetMovieScene();
	if (MovieScene)
	{
		MovieScene->Modify();
		MovieScene->SetDisplayRate(FFrameRate(FPS, 1));
		MovieScene->SetTickResolutionDirectly(FFrameRate(24000, 1));
	}
}

void ARMCRecorder::StartRecording()
{
	if (RecordingSequence == nullptr)
	{
		UE_LOG(LogRemocapp, Error, TEXT("No valid level sequence asset found"))
		OnRecordingFailed.Broadcast();
		return;
	}

	if (GEngine)
	{
		OriginalTimecodeProvider = GEngine->GetTimecodeProvider();
		CustomTimecodeProvider = NewObject<URMCCustomTimecodeProvider>(GetTransientPackage());
		CustomTimecodeProvider->FrameRate = FFrameRate(FPS, 1);
		GEngine->SetTimecodeProvider(CustomTimecodeProvider);
	}
	
	SetupRecorder();
	
	TArray<AActor*> ActorsToFind;
	UGameplayStatics::GetAllActorsWithTag(this, TrackingActorTagName, ActorsToFind);
	if (ActorsToFind.Num() == 0 || TakeRecorderSources->GetSources().Num() == 0)
	{
		UE_LOG(LogRemocapp, Error, TEXT("No actors to record"))
		OnRecordingFailed.Broadcast();
		return;
	}
	TArray<UActorComponent*> RecordingComponents = ActorsToFind[0]->GetComponentsByTag(
		USkeletalMeshComponent::StaticClass(), RecordingComponentTagName);
	if (RecordingComponents.Num() == 0)
	{
		UE_LOG(LogRemocapp, Error, TEXT("No skeletal meshes to record"))
		OnRecordingFailed.Broadcast();
		return;
	}
	for (const UActorComponent* RecordingComponent : RecordingComponents)
	{
		const USkeletalMeshComponent* MeshComponent = Cast<USkeletalMeshComponent>(RecordingComponent);
		if (MeshComponent && !MeshComponent->GetSkeletalMeshAsset())
		{
			UE_LOG(LogRemocapp, Error, TEXT("A skeletal mesh component is missing skeletal mesh"))
			OnRecordingFailed.Broadcast();
			return;
		}
	}

	for (UTakeRecorderSource* Source : TakeRecorderSources->GetSources())
	{
		if (UTakeRecorderActorSource* ActorSource = Cast<UTakeRecorderActorSource>(Source))
		{
			AActor* ActorToRecord = ActorSource->Target.Get();
			if (ActorToRecord)
			{
				FilterComponentProperties(ActorSource->RecordedProperties, RecordingComponents);
			}
		}
	}

	RecordingSequence->MovieScene->SetPlaybackRange(
		TRange<FFrameNumber>(RecordingSequence->MovieScene->GetPlaybackRange().GetLowerBoundValue(),
		                     TNumericLimits<int32>::Max() - 1), false);
	StartRecordingIntoSequence();
}

void ARMCRecorder::StopRecording()
{
	if (!IsRecording()) return;

	StopRecordingIntoSequence();
	
	ExportAnimations();

	if (GEngine)
	{
		GEngine->SetTimecodeProvider(OriginalTimecodeProvider);
		CustomTimecodeProvider = nullptr;
		OriginalTimecodeProvider = nullptr;
	}
	
	if (GetWorld() && GetWorld()->GetFirstPlayerController())
	{
		if (GetWorld()->GetFirstPlayerController()->GetPawn())
		{
			GetWorld()->GetFirstPlayerController()->SetViewTarget(
				GetWorld()->GetFirstPlayerController()->GetPawn());
		}
	}
	Cleanup();
}

bool ARMCRecorder::IsRecording()
{
	return UTakeRecorderBlueprintLibrary::IsRecording();
}

void ARMCRecorder::StartRecordingIntoSequence()
{
	TakeRecorder = UTakeRecorderBlueprintLibrary::StartRecording(RecordingSequence, TakeRecorderSources,
																TakeMetadata, TakeRecorderParameters);

	if (IsValid(TakeRecorder))
	{
		OnRecordingStarted.Broadcast(TakeRecorder);
		return;
	}
	OnRecordingFailed.Broadcast();
	UE_LOG(LogRemocapp, Error, TEXT("Can't start recording"))
}

void ARMCRecorder::SetPlaybackRange()
{
	UMovieScene* MovieScene = RecordingSequence->GetMovieScene();
	if (MovieScene)
	{
		TRange<FFrameNumber> NewPlayRange;
		NewPlayRange = MovieScene->GetPlaybackRange();
		TArray<UMovieSceneSection*> MovieSceneSections = MovieScene->GetAllSections();
		if (MovieSceneSections[1])
		{
			TRange<FFrameNumber> SectionRange = MovieSceneSections[1]->GetRange();
			NewPlayRange.SetUpperBoundValue(SectionRange.GetUpperBoundValue());
		}
		MovieScene->SetPlaybackRange(NewPlayRange);
	}
}

void ARMCRecorder::StopRecordingIntoSequence()
{
	UTakeRecorderBlueprintLibrary::StopRecording();
	SetPlaybackRange();
	OnRecordingFinished.Broadcast(TakeRecorder);
}

void ARMCRecorder::ExportAnimations()
{
	TArray<AActor*> ActorsToFind;
	UGameplayStatics::GetAllActorsWithTag(this, TrackingActorTagName, ActorsToFind);
	TArray<UActorComponent*> RecordingComponents = ActorsToFind[0]->GetComponentsByTag(
		USkeletalMeshComponent::StaticClass(), RecordingComponentTagName);
	TArray<FAnimationExportResult> AnimationExportResults;
	uint32 Index = 0;
	for (int32 i = 0; i < RecordingComponents.Num(); i++)
	{
		USkeletalMeshComponent* MeshComponent = Cast<USkeletalMeshComponent>(RecordingComponents[i]);
		FAnimationExportResult AnimationExportResult;
		if (ExportToAnimationSequence(MeshComponent, Index))
		{
			AnimationExportResult.MeshName = MeshComponent->GetSkeletalMeshAsset()->GetName();
			AnimationExportResult.bExportedToAnimSequence = true;
			ExportAnimationToFile(MeshComponent, Index, AnimationExportResult.FbxFileName);
			Index++;
		}
		else
		{
			AnimationExportResult.MeshName = FString();
			AnimationExportResult.bExportedToAnimSequence = false;
		}
		AnimationExportResults.Add(AnimationExportResult);
	}

	OnAnimationsExported.Broadcast(AnimationExportResults);
}

bool ARMCRecorder::ExportToAnimationSequence(USkeletalMeshComponent* InMeshComponent, uint32 InIndex)
{
	if (InMeshComponent == nullptr || InMeshComponent->GetSkeletalMeshAsset() == nullptr) return false;
	
	if (!(IsValid(GEditor) && IsValid(RecordingSequence))) return false;

	UUnrealEditorSubsystem* EditorSubsystem = GEditor->GetEditorSubsystem<UUnrealEditorSubsystem>();
	if (!IsValid(EditorSubsystem)) return false;

	UWorld* World = EditorSubsystem->GetGameWorld();
	if (!IsValid(World)) return false;

	UMovieScene* MovieScene = RecordingSequence->GetMovieScene();
	if (!IsValid(MovieScene)) return false;

	UAnimSeqExportOption* ExportOption = NewObject<UAnimSeqExportOption>(
		GetTransientPackage(), UAnimSeqExportOption::StaticClass());
	if (ExportOption == nullptr) return false;

	ExportOption->bExportTransforms = true;
	ExportOption->bExportMorphTargets = true;
	ExportOption->bExportMaterialCurves = true;
	ExportOption->bRecordInWorldSpace = false;
	ExportOption->DelayBeforeStart = 1;
	ExportOption->bEvaluateAllSkeletalMeshComponents = false;
	
	FString AnimDir = FPackageName::GetLongPackagePath(MovieScene->GetPackage()->GetPathName());
	FString AnimSeqName = SceneName + TEXT("-") + FString::FromInt(InIndex) + TEXT("-") +
		InMeshComponent->GetSkeletalMeshAsset()->GetName() + TEXT("-") +
		FDateTime::Now().ToString().Replace(TEXT("."), TEXT("_"));
	if (!FPaths::ValidatePath(AnimSeqName)) return false;

	AnimSeq = TakesUtils::MakeNewAsset<UAnimSequence>(AnimDir, AnimSeqName);
	if (!IsValid(AnimSeq)) return false;

	AnimSeq->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(AnimSeq);
	AnimSeq->SetSkeleton(InMeshComponent->GetSkeletalMeshAsset()->GetSkeleton());

	for (const FMovieSceneBinding& MovieSceneBinding : MovieScene->GetBindings())
	{
		if (InMeshComponent->GetName() == MovieSceneBinding.GetName())
		{
			FMovieSceneBindingProxy Binding(MovieSceneBinding.GetObjectGuid(), RecordingSequence);
			bool bExportedToAnimSeq = USequencerToolsFunctionLibrary::ExportAnimSequence(
				World, RecordingSequence, AnimSeq, ExportOption, Binding, false);
			if (!bExportedToAnimSeq) return false;

			FSavePackageArgs SaveArgs;
			SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
			SaveArgs.SaveFlags = SAVE_NoError;
			FString PackageFileName = FPackageName::LongPackageNameToFilename(
				AnimSeq->GetPackage()->GetName(), FPackageName::GetAssetPackageExtension());
			if (GEditor->SavePackage(AnimSeq->GetPackage(), AnimSeq, *PackageFileName, SaveArgs))
			{
				TArray<FAssetData> AssetsToSave;
				AssetsToSave.Add(AnimSeq);
				FAssetRegistryModule::AssetsSaved(TArray(AssetsToSave));
			}
			break;
		}
	}

	return true;
}

bool ARMCRecorder::ExportAnimationToFile(USkeletalMeshComponent* InMeshComponent, uint32 InIndex, FText& ExportedFileName)
{
	if (InMeshComponent == nullptr ||  InMeshComponent->GetSkeletalMeshAsset() == nullptr)
	{
		ExportedFileName = FText();
		return false;
	}
	UnFbx::FFbxExporter* Exporter = UnFbx::FFbxExporter::GetInstance();
	UFbxExportOption* ExportOptions = NewObject<UFbxExportOption>();
	ExportOptions->Collision = 1;
	ExportOptions->LevelOfDetail = 0;
	ExportOptions->bExportPreviewMesh = false;
	ExportOptions->bExportMorphTargets = true;
	ExportOptions->FbxExportCompatibility = EFbxExportCompatibility::FBX_2016;
	Exporter->SetExportOptionsOverride(ExportOptions);

	FString ExportingDirectory = AnimationDirectoryPath.Path;
	FString ExportingFilePath = ExportingDirectory / SceneName + TEXT("-") + FString::FromInt(InIndex) + TEXT("-") +
		InMeshComponent->GetSkeletalMeshAsset()->GetName() + TEXT("-") +
		FDateTime::Now().ToString().Replace(TEXT("."), TEXT("_")) + TEXT(".fbx");
	if (!FPaths::ValidatePath(ExportingFilePath))
	{
		UE_LOG(LogRemocapp, Error, TEXT("Invalid filename"))
		ExportedFileName = FText();
		return false;
	}
	Exporter->CreateDocument();
	USkeletalMesh* SkelMesh = InMeshComponent->GetSkeletalMeshAsset();
	Exporter->ExportAnimSequence(AnimSeq, SkelMesh, Exporter->GetExportOptions()->bExportPreviewMesh);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*ExportingDirectory))
	{
		PlatformFile.CreateDirectory(*ExportingDirectory);
	}
	Exporter->WriteToFile(*ExportingFilePath);
	AnimSeq = nullptr;
	ExportedFileName = FText::FromString(ExportingFilePath);
	return true;
}

void ARMCRecorder::Cleanup()
{
	if (RecordingSequence == nullptr) return;
	UMovieScene* MovieScene = RecordingSequence->GetMovieScene();
	TArray<UMovieSceneSection*> MovieSceneSections = MovieScene->GetAllSections();
	TArray<UObject*> AssetsToDelete;
	for (auto MovieSceneSection : MovieSceneSections)
	{
		UMovieSceneSkeletalAnimationSection* SkelAnimSection = Cast<UMovieSceneSkeletalAnimationSection>(
			MovieSceneSection);
		if (SkelAnimSection && SkelAnimSection->Params.Animation)
		{
			AssetsToDelete.Add(SkelAnimSection->Params.Animation);
		}
	}
	ObjectTools::ForceDeleteObjects(AssetsToDelete, false);
}

void ARMCRecorder::HandleEndPIE(bool bIsSimulating)
{
	if (TakeRecorder && RecordingSequence && IsRecording())
	{
		StopRecording();
	}

	if (GEngine)
	{
		if (OriginalTimecodeProvider)
		{
			GEngine->SetTimecodeProvider(OriginalTimecodeProvider);
			OriginalTimecodeProvider = nullptr;
		}
		CustomTimecodeProvider = nullptr;
	}
	
	Cleanup();
	FEditorDelegates::EndPIE.RemoveAll(this);
}
