// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.

#include "ConfigPresetUtility.h"

#include <ISettingsModule.h>
#include <ISettingsContainer.h>
#include <ISettingsCategory.h>
#include <ISettingsSection.h>

#include <PropertyHandle.h>



TSharedPtr<ISettingsSection> FConfigPresetUtility::GetConfigSection(FName CategoryName, FName SectionName)
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		TSharedPtr<ISettingsContainer> Container = SettingsModule->GetContainer("Project");
		if (Container)
		{
			TSharedPtr<ISettingsCategory> Category = Container->GetCategory(CategoryName);
			if (Category)
			{
				return Category->GetSection(SectionName);				
			}
		}
	}
	return nullptr;
}

TWeakObjectPtr<UObject> FConfigPresetUtility::GetConfigObject(FName CategoryName, FName SectionName)
{
	TSharedPtr<ISettingsSection> Section = GetConfigSection(CategoryName, SectionName);
	if (Section)
	{
		return Section->GetSettingsObject();
	}

	return nullptr;
}

TWeakObjectPtr<UObject> FConfigPresetUtility::GetConfigObject(TSharedPtr<IPropertyHandle> ConfigHandle)
{
	TWeakObjectPtr<UObject> ConfigObject;

	FName PropertyValue = NAME_None;
	if (ConfigHandle->GetValue(PropertyValue) == FPropertyAccess::Success && !PropertyValue.IsNone())
	{
		FString CategoryName;
		FString SectionName;
		if (PropertyValue.ToString().Split(TEXT("."), &CategoryName, &SectionName))
		{
			ConfigObject = FConfigPresetUtility::GetConfigObject(*CategoryName, *SectionName);
		}
	}

	return ConfigObject;
}
