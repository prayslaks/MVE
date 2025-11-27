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
			"glTFRuntime",
			"IKRig",
			"HTTP",              // FHttpModule, IHttpRequest, IHttpResponse
			"Json",              // FJsonObject, FJsonValue
			"JsonUtilities",     // FJsonObjectConverter, FJsonSerializer
			"Niagara",
			"ImageWrapper", // IImageWrapper (이미지 로딩) ,
			"OnlineSubsystem",
			"OnlineSubsystemSteam"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"DesktopPlatform"
		});

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

		// 모든 하위 Public 폴더를 자동으로 추가
		string ModulePath = ModuleDirectory;
		string[] PublicFolders = System.IO.Directory.GetDirectories(ModulePath, "Public", System.IO.SearchOption.AllDirectories);
        
		foreach (string Folder in PublicFolders)
		{
			PublicIncludePaths.Add(Folder);
		}
		
	}
}
