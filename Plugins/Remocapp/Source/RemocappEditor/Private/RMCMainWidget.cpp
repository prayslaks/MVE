// Copyright 2025 REMOCAPP, PTY LTD. All Rights Reserved.


#include "RMCMainWidget.h"

#include "Components/EditableText.h"

void URMCMainWidget::SetRecorder(ARMCRecorder* InRecorder)
{
	Recorder = InRecorder;
}

void URMCMainWidget::InitializeRecordingOptions()
{
	if (Recorder == nullptr) return;
	SetCountdown(Recorder->GetCountdown());
	SceneNameEditableText->SetText(FText::FromString(Recorder->GetSceneName()));
	Recorder->OnRecordingStarted.AddUObject(this, &URMCMainWidget::OnRecordingStarted);
	Recorder->OnRecordingFinished.AddUObject(this, &URMCMainWidget::OnRecordingFinished);
	Recorder->OnRecordingFailed.AddUObject(this, &URMCMainWidget::OnRecordingFailed);
	Recorder->OnAnimationsExported.AddUObject(this, &URMCMainWidget::OnAnimationsExported);
}
