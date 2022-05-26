// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Krazy_Karts : ModuleRules
{
	public Krazy_Karts(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "PhysXVehicles", "HeadMountedDisplay" });

		PublicDefinitions.Add("HMD_MODULE_INCLUDED=1");

		PublicIncludePaths.AddRange(new string[]
		{
			"Krazy_Karts",
			"Krazy_Karts/Public"
		});
	}
}
