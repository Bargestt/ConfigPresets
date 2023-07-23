// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ConfigPresetSettings.generated.h"

/**  */
USTRUCT()
struct FConfigPropertyPreset
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FName Config;

	UPROPERTY(EditAnywhere)
	FName Property;
	
	UPROPERTY(EditAnywhere)
	FString Value;

};

/**  */
USTRUCT()
struct FConfigPreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FString Name;
	
	UPROPERTY(EditAnywhere)
	TArray<FConfigPropertyPreset> PropertyPresets;
};



/**
 * 
 */
UCLASS(Config = EditorPerProjectUserSettings, defaultconfig, meta = (DisplayName = "Config Presets"))
class UConfigPresetSettings : public UDeveloperSettings
{
	GENERATED_BODY()	
	
	UPROPERTY(config, EditAnywhere)
	TArray<FConfigPreset> Presets;

public:
	UConfigPresetSettings();
	virtual FName GetContainerName() const override { return TEXT("Project"); }
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
};
