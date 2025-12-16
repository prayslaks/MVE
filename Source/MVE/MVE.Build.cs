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
			"UMG",							// 위젯
			"Slate",						// 위젯
			"SlateCore",					// 위젯
			"glTFRuntime",					// 3D 모델 런타임 로드
			"glTFRuntimeAudio",				// 음원 런타임 로드
			"IKRig",
			"HTTP",							// FHttpModule, IHttpRequest, IHttpResponse
			"Json",							// FJsonObject, FJsonValue
			"JsonUtilities",				// FJsonObjectConverter, FJsonSerializer
			"ImageWrapper",					// IImageWrapper (이미지 로딩) 
			"Niagara",						// 각종 이펙트
			"OnlineSubsystem",
			"OnlineSubsystemSteam",
			"RenderCore",					// 
			"Niagara",						// 나이아가라 이펙트
			"Sockets",						// 리슨 서버 IP와 포트 관리
			"WebSockets",
			"Networking",					// 리슨 서버 IP와 포트 관리
			"AudioCaptureCore",				// STT 오디오 캡쳐 
			"GameplayTags"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"DesktopPlatform", "MediaAssets", "UE_OSC"
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
