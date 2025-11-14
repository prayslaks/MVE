// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MVE : ModuleRules
{
	public MVE(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"glTFRuntime"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"MVE",
			"MVE/Variant_Platforming",
			"MVE/Variant_Platforming/Animation",
			"MVE/Variant_Combat",
			"MVE/Variant_Combat/AI",
			"MVE/Variant_Combat/Animation",
			"MVE/Variant_Combat/Gameplay",
			"MVE/Variant_Combat/Interfaces",
			"MVE/Variant_Combat/UI",
			"MVE/Variant_SideScrolling",
			"MVE/Variant_SideScrolling/AI",
			"MVE/Variant_SideScrolling/Gameplay",
			"MVE/Variant_SideScrolling/Interfaces",
			"MVE/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
