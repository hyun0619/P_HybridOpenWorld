// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HybridOpenWorld : ModuleRules
{
	public HybridOpenWorld(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
	        "Core", "CoreUObject", "Engine", "InputCore", "NavigationSystem", "AIModule", "Niagara", "EnhancedInput"
        });
    }
}
