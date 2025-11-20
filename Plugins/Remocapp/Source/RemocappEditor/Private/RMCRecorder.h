// Copyright 2025 REMOCAPP, PTY LTD. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityActor.h"
#include "Recorder/TakeRecorder.h"
#include "Recorder/TakeRecorderParameters.h"
#include "RMCRecorder.generated.h"

class UTakeRecorder;
class UTakeMetaData;
class UTakeRecorderSources;
class ULevelSequence;
class UTimecodeProvider;
class URMCCustomTimecodeProvider;

UENUM(BlueprintType)
enum class ERMCRecorderCountdown : uint8
{
	Zero = 0,
	Three = 3,
	Ten = 10
};

USTRUCT(BlueprintType)
struct FAnimationExportResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationExportResult")
	bool bExportedToAnimSequence { false };
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationExportResult")
	FString MeshName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationExportResult")
	FText FbxFileName;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAnimationsExported, const TArray<FAnimationExportResult>&);
DECLARE_MULTICAST_DELEGATE(FOnRecordingFailed);

/**
 * RMCRecorder
 *
 * An actor that provides functionality for recording the main player pawn animations while a PIE session is in progress.
 */
UCLASS(Blueprintable)
class REMOCAPPEDITOR_API ARMCRecorder : public AEditorUtilityActor
{
	GENERATED_BODY()

public:
	ARMCRecorder();
	
public:
	virtual void BeginPlay() override;

	// Sets up any parameters for recording and begins recording to the sequence
	UFUNCTION(BlueprintCallable, Category = "RemocappRecorder")
	void StartRecording();

	// Stops the recording if there is any in progress
	UFUNCTION(BlueprintCallable, Category = "RemocappRecorder")
	void StopRecording();

	// Returns whether a recording is in progress or not
	UFUNCTION(BlueprintPure, Category = "RemocappRecorder")
	static bool IsRecording();

	// Set seconds to count before the recording starts
	UFUNCTION(BlueprintCallable, Category = "RemocappRecorder")
	void SetCountdown(ERMCRecorderCountdown InCountdownSeconds) { Countdown = InCountdownSeconds; };

	// Returns countdown seconds
	UFUNCTION(BlueprintPure, Category = "RemocappRecorder")
	ERMCRecorderCountdown GetCountdown() const { return Countdown; };

	// Set frame rate of the recording
	UFUNCTION(BlueprintCallable, Category = "RemocappRecorder")
	void SetFPS(int32 InFPS) { FPS = InFPS; };

	// Returns recording frame rate
	UFUNCTION(BlueprintPure, Category = "RemocappRecorder")
	int32 GetFPS() const { return FPS; };

	// Set scene name that is mentioned in the exported filenames
	UFUNCTION(BlueprintCallable, Category = "RemocappRecorder")
	void SetSceneName(FString InSceneName) { SceneName = InSceneName; };

	// Returns scene name
	UFUNCTION(BlueprintPure, Category = "RemocappRecorder")
	FString GetSceneName() const { return SceneName; };

	FOnTakeRecordingStarted OnRecordingStarted;
	FOnTakeRecordingFinished OnRecordingFinished;
	FOnAnimationsExported OnAnimationsExported;
	FOnRecordingFailed OnRecordingFailed;

protected:
	// Main function to set up the recorder to get ready for recording
	UFUNCTION()
	void SetupRecorder();

	// Implemented in blueprint, sets actor to be recorded
	UFUNCTION(BlueprintImplementableEvent, Category = "RemocappRecorder")
	void SetSourceActor();

	// Sets up take recorder parameters
	UFUNCTION(BlueprintCallable, Category = "RemocappRecorder")
	void SetupTakeRecorderParameters();

	// Sets up take meta-data
	UFUNCTION(BlueprintCallable, Category = "RemocappRecorder")
	void SetupTakeMetadata();

	// Starts recording to the sequence asset
	UFUNCTION()
	void StartRecordingIntoSequence();

	// Stops recording to the sequence asset if one is active
	UFUNCTION()
	void StopRecordingIntoSequence();

	// Sets playback range according to the sections length in the sequence asset
	void SetPlaybackRange();

	// Tries to export recorded animations
	void ExportAnimations();
	
	// Exports a binding inside a level sequence into an animation sequence, returns true if the exporting was successful 
	bool ExportToAnimationSequence(USkeletalMeshComponent* InMeshComponent, uint32 InIndex);

	// Exports the animation sequence asset to a FBX file
	bool ExportAnimationToFile(USkeletalMeshComponent* InMeshComponent, uint32 InIndex, FText& ExportedFileName);

	// Cleans up the temporary created animation sequence asset from the project
	void Cleanup();
	
protected:
	// Seconds before the recording starts
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "RemocappRecorder")
	ERMCRecorderCountdown Countdown;

	// Root directory for saving takes
	UPROPERTY(BlueprintReadWrite, Category = "RemocappRecorder")
	FDirectoryPath RootTakeSaveDirectory;

	// Default scene name
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "RemocappRecorder")
	FString SceneName;

	// Initial take number
	UPROPERTY(BlueprintReadWrite, Category = "RemocappRecorder")
	int32 TakeNumber;

	// Recording frame rate
	UPROPERTY(EditAnywhere, Category = "RemocappRecorder")
	int32 FPS;

	// Directory to store FBX files
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "RemocappRecorder")
	FDirectoryPath AnimationDirectoryPath;
	
	// Main recorder object responsible for recording level sequences
	UPROPERTY(BlueprintReadWrite, Category = "RemocappRecorder")
	UTakeRecorder* TakeRecorder;

	// Meta data of the recording
	UPROPERTY(BlueprintReadWrite, Category = "RemocappRecorder")
	UTakeMetaData* TakeMetadata;

	// Recording parameters
	UPROPERTY(BlueprintReadWrite, Category = "RemocappRecorder")
	FTakeRecorderParameters TakeRecorderParameters;

	// Used to add player pawn into
	UPROPERTY(BlueprintReadWrite, Category = "RemocappRecorder")
	UTakeRecorderSources* TakeRecorderSources;
	
	// The level sequence used for recording
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "RemocappRecorder")
	ULevelSequence* RecordingSequence;

	FName TrackingActorTagName;

	FName RecordingComponentTagName;

	UPROPERTY()
	UAnimSequence* AnimSeq;

private:
	void HandleEndPIE(bool bIsSimulating);

private:
	UPROPERTY()
	TObjectPtr<UTimecodeProvider> OriginalTimecodeProvider;

	UPROPERTY()
	TObjectPtr<URMCCustomTimecodeProvider> CustomTimecodeProvider;
};