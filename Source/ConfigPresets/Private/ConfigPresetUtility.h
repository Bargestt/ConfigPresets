// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class IPropertyHandle;
class ISettingsSection;

struct FConfigPresetUtility
{
	static TSharedPtr<ISettingsSection> GetConfigSection(FName CategoryName, FName SectionName);
	static TWeakObjectPtr<UObject> GetConfigObject(FName CategoryName, FName SectionName);

	static TWeakObjectPtr<UObject> GetConfigObject(TSharedPtr<IPropertyHandle> ConfigHandle);
};
