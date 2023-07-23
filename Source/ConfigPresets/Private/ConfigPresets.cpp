// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.

#include "ConfigPresets.h"
#include "Modules/ModuleManager.h"
#include <PropertyEditorModule.h>

#include "ConfigPresetSettings.h"
#include "Customizations/ConfigPresetCustomization.h"
#include "Customizations/ConfigPropertyPresetCustomization.h"



#define LOCTEXT_NAMESPACE "FConfigPresetsModule"


class FConfigPresetsModule : public IConfigPresetsModule
{
public:
	virtual void StartupModule() override
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		{
			PropertyModule.RegisterCustomPropertyTypeLayout(FConfigPreset::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FConfigPresetCustomization::MakeInstance));
			PropertyModule.RegisterCustomPropertyTypeLayout(FConfigPropertyPreset::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FConfigPropertyPresetCustomization::MakeInstance));
		}
	}
	virtual void ShutdownModule() override
	{
		if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
		{
			FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

			PropertyModule.UnregisterCustomPropertyTypeLayout(FConfigPreset::StaticStruct()->GetFName());
			PropertyModule.UnregisterCustomPropertyTypeLayout(FConfigPropertyPreset::StaticStruct()->GetFName());
		}
	}
};

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FConfigPresetsModule, ConfigPresets)