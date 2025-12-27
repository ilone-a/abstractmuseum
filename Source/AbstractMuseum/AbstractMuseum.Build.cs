// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AbstractMuseum : ModuleRules
{
    public AbstractMuseum(ReadOnlyTargetRules Target) : base(Target)
    {
        //Type = ModuleType.Runtime;
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
            "SlateCore", "HTTP",
               "InputCore",
                "UMG",          // 
                "Slate",        //
                "SlateCore" ,
                "Projects"});
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd", });
        }
    }
}
