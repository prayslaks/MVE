// Copyright 2025 REMOCAPP, PTY LTD. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TimecodeProvider.h"
#include "RMCCustomTimecodeProvider.generated.h"

/**
 * RMCCustomTimecodeProvider
 * 
 * A custom Timecode Provider that allows to specify a custom frame rate.
 */
UCLASS()
class URMCCustomTimecodeProvider : public UTimecodeProvider
{
	GENERATED_BODY()

public:
	virtual FQualifiedFrameTime GetQualifiedFrameTime() const override;
	virtual ETimecodeProviderSynchronizationState GetSynchronizationState() const override;
	virtual bool Initialize(class UEngine* InEngine) override { return true; };
	virtual void Shutdown(class UEngine* InEngine) override {};

public:
	FFrameRate FrameRate;
};
