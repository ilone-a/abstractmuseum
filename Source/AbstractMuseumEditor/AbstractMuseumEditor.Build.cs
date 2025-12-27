// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AbstractMuseumEditor : ModuleRules
{
    public AbstractMuseumEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        // Type = ModuleType.Editor;

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] {
    "Core",
    "CoreUObject",
    "Engine",
    "InputCore",
    "Slate",
    "SlateCore",
    "UnrealEd",
    "EditorWidgets",
    "DesktopWidgets",
    "Kismet",
    "GraphEditor"
}
    );
        PrivateDependencyModuleNames.AddRange(new string[] {

            "Engine",
            "UnrealEd",
            "Slate",
            "SlateCore",
            "PropertyEditor",
            "InputCore",
            "EditorStyle",
            "AssetTools",
            "EditorScriptingUtilities",
            "ContentBrowser",
            "MainFrame",
            "Projects",
           "GraphEditor",
            "AbstractMuseum",
            "WorkspaceMenuStructure",

        });

    }
}
