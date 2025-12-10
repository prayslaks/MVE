// Fill out your copyright notice in the Description page of Project Settings.


#include "MVE_API_PCTest.h"

#include "MVE_API_PCTest_AudComponent.h"
#include "MVE_API_PCTest_StdComponent.h"

AMVE_API_PCTest::AMVE_API_PCTest()
{
	AudComponent = CreateDefaultSubobject<UMVE_API_PCTest_AudComponent>(TEXT("AudComponent"));
	StdComponent = CreateDefaultSubobject<UMVE_API_PCTest_StdComponent>(TEXT("StdComponent"));

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	// PlayerControllers do not have a root component to attach to,
	// so the AudioComponent will simply exist as a subobject.
	// You might want to attach it to a specific Actor if a spatial sound is needed,
	// but for playing general sounds, it can remain a standalone component.
}
