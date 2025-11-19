// Copyright 2025 REMOCAPP, PTY LTD. All Rights Reserved.


#include "RMCCustomTimecodeProvider.h"

FQualifiedFrameTime URMCCustomTimecodeProvider::GetQualifiedFrameTime() const
{
	const double TotalSeconds = FPlatformTime::Seconds();
	const int32 FrameNumber = FMath::FloorToInt(TotalSeconds * FrameRate.AsDecimal());
	const FFrameTime FrameTime(FrameNumber);
	return FQualifiedFrameTime(FrameTime, FrameRate);
}

ETimecodeProviderSynchronizationState URMCCustomTimecodeProvider::GetSynchronizationState() const
{
	return ETimecodeProviderSynchronizationState::Synchronized;
}
