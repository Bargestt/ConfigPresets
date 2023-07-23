// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.


#include "ConfigPresetCustomization.h"
#include <DetailWidgetRow.h>
#include <IDetailChildrenBuilder.h>
#include <PropertyCustomizationHelpers.h>

#include <Widgets/Input/SButton.h>
#include <Widgets/SWindow.h>
#include <Widgets/Layout/SScrollBox.h>
#include <Misc/MessageDialog.h>
#include <Editor/EditorEngine.h>


#define LOCTEXT_NAMESPACE "ConfigPresetCustomization"


void FConfigPresetCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{	
	PresetHandle = PropertyHandle;

	HeaderRow
	.NameContent()
	[
		PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FConfigPreset, Name))->CreatePropertyValueWidget(true)
	]
	.ValueContent()
	.MinDesiredWidth(100)
	.MaxDesiredWidth(4096)
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Left)
	[
		SNew(SButton)
		.OnClicked(this, &FConfigPresetCustomization::Apply)
		.ButtonStyle(FAppStyle::Get(), "FlatButton.Success")
		.TextStyle(FAppStyle::Get(), "NormalText")
		.ForegroundColor(FLinearColor::White)
		.ContentPadding(FMargin(6, 2))
		.HAlign(HAlign_Center)
		.Text(LOCTEXT("Apply", "Apply"))
	];
}

void FConfigPresetCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	ChildBuilder.AddProperty(PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FConfigPreset, PropertyPresets)).ToSharedRef());
}

FReply FConfigPresetCustomization::Apply()
{
	FConfigPreset Preset;
	{
		void* PropertyValuePtr;
		if (PresetHandle->GetValueData(PropertyValuePtr) == FPropertyAccess::Success)
		{
			Preset = *(FConfigPreset*)PropertyValuePtr;
		}
	}

	TSharedRef<SScrollBox> Messages = SNew(SScrollBox);

	auto AddMessage = [&Messages](bool bSuccess, TArray<TTuple<FText, int32>> Cols = {})
	{
		TSharedRef<SHorizontalBox> Box = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SImage)
				.Image(bSuccess ? FAppStyle::GetBrush("Symbols.Check") : FAppStyle::GetBrush("Symbols.X"))
				.ColorAndOpacity(bSuccess ? FLinearColor::Green : FLinearColor::Red)
			];

		for (int32 Index = 0; Index < Cols.Num(); Index++)
		{
			auto& Slot = Box->AddSlot()
			[
				SNew(SBox).MinDesiredWidth(Cols[Index].Get<1>())
				[
					SNew(STextBlock).Text(Cols[Index].Get<0>())
				]
			];

			if (Index < Cols.Num() - 1)
			{
				Slot.AutoWidth();
			}
			else
			{
				Slot.FillWidth(1.0f);
			}
		}		

		Messages->AddSlot()[ Box ];
	};


	FScopedTransaction Transaction(*FString::Printf(TEXT("ConfigPreset_Apply_%s"), *Preset.Name), LOCTEXT("ApplyPreset", "Applied config preset"), nullptr);

	TSet<UObject*> ModifiedConfigs;
	TSet<TSharedPtr<ISettingsSection>> ModifiedSections;
	
	for (const FConfigPropertyPreset& PropertyPreset : Preset.PropertyPresets)
	{
		if (PropertyPreset.Config.IsNone() || PropertyPreset.Property.IsNone())
		{
			continue;
		}

		FString CategoryName;
		FString SectionName;
		if (!PropertyPreset.Config.ToString().Split(TEXT("."), &CategoryName, &SectionName))
		{
			AddMessage(false, { MakeTuple(FText::FormatOrdered(LOCTEXT("PresetError_BadConfig", "Error: Invalid config {0}"), FText::FromName(PropertyPreset.Config)), 100 ) });
			continue;
		}

		TSharedPtr<ISettingsSection> Section = FConfigPresetUtility::GetConfigSection(*CategoryName, *SectionName);

		UObject* ConfigObject = Section ? Section->GetSettingsObject().Get() : nullptr; 
		if (!ConfigObject)
		{
			AddMessage(false, { MakeTuple(FText::FormatOrdered(LOCTEXT("PresetError_NoConfig", "Error: Config {0} does not exists"), FText::FromName(PropertyPreset.Config)), 100 ) });
			continue;
		}

		FProperty* Property = ConfigObject->GetClass()->FindPropertyByName(PropertyPreset.Property);
		if (!Property)
		{
			AddMessage(false, { MakeTuple(FText::FormatOrdered(LOCTEXT("PresetError_NoProperty", "Error: Config {0} has no property {1}"), FText::FromName(PropertyPreset.Config), FText::FromName(PropertyPreset.Property)), 100 ) });
			continue;
		}


		void* Data = Property->ContainerPtrToValuePtr<void>(ConfigObject);

		FString OldValue;
		Property->ExportTextItem_Direct(OldValue, Data, nullptr, nullptr, PPF_None);
		{		
			ModifiedSections.Add(Section);

			if (!ModifiedConfigs.Contains(ConfigObject))
			{
				ModifiedConfigs.Add(ConfigObject);

				ConfigObject->SetFlags(RF_Transactional);
				ConfigObject->Modify();
			}			

			ConfigObject->PreEditChange(Property);
			
			Property->ImportText_Direct(*PropertyPreset.Value, Data, nullptr, PPF_None);

			FPropertyChangedEvent Event(Property, EPropertyChangeType::ValueSet);
			ConfigObject->PostEditChangeProperty(Event);
		}
		FString NewValue;
		Property->ExportTextItem_Direct(NewValue, Data, nullptr, nullptr, PPF_None);

		AddMessage(true, 
		{
			MakeTuple(LOCTEXT("PresetError_Applied", "Applied"), 75),
			MakeTuple(FText::FromName(PropertyPreset.Config), 200),
			MakeTuple(FText::FromName(PropertyPreset.Property), 200),
			MakeTuple(FText::FormatOrdered(LOCTEXT("PresetError_Change", "{0} -> {1}"), FText::FromString(OldValue),FText::FromString(NewValue)), 300) 
		});
	}

	for (auto& Section : ModifiedSections)
	{
		Section->Save();
	}


	const TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(FText::FormatOrdered(LOCTEXT("PresetApplied_Title", "Preset {0} Applied"), FText::FromString(Preset.Name)))
		.ClientSize(FVector2D(900, 700))
		.SupportsMinimize(false)
		.SupportsMaximize(false)
		[			
			Messages
		];
	GEditor->EditorAddModalWindow(Window);

	return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE