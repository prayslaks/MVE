// Copyright 2025 REMOCAPP, PTY LTD. All Rights Reserved.

using UnrealBuildTool;

public class RemocappEditor : ModuleRules
{
	public RemocappEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "TakeRecorder", "Blutility", "UMG", "Projects", "OSC" });

		PrivateDependencyModuleNames.AddRange(
			new string[] { 
				"Core", "CoreUObject", "Engine", "Slate", "SlateCore", "UnrealEd",
				"TakesCore", "TakeRecorderSources", "SequencerScripting", "MovieSceneTracks",
				"UnrealEd", "LevelSequence", "MovieScene", "SequencerScriptingEditor", "FBX" });
	}
}
