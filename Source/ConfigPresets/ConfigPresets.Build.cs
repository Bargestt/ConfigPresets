// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.

using UnrealBuildTool;

public class ConfigPresets : ModuleRules
{
	public ConfigPresets(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
		);			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",
                "CoreUObject",
                "DeveloperSettings",
                "Slate",
                "SlateCore",
                "EditorStyle",
                "PropertyEditor",
				"EditorWidgets",
				"UnrealEd"
			}
		);
	}
}
