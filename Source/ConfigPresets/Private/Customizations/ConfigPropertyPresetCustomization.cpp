// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.


#include "ConfigPropertyPresetCustomization.h"
#include <DetailWidgetRow.h>
#include <IDetailChildrenBuilder.h>
#include <PropertyCustomizationHelpers.h>

#include <ISettingsModule.h>
#include <ISettingsContainer.h>
#include <ISettingsCategory.h>
#include <ISettingsSection.h>

#include <Widgets/SBoxPanel.h>
#include <Widgets/Input/SButton.h>
#include <SAssetSearchBox.h>

#define LOCTEXT_NAMESPACE "ConfigPropertyPresetCustomization"




class SConfigObjectPicker : public SCompoundWidget
{

public:
	SLATE_BEGIN_ARGS(SConfigObjectPicker){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedRef<IPropertyHandle> InProperty)
	{
		Property = InProperty;

		FName PropertyValue = NAME_None;
		Property->GetValue(PropertyValue);

		FText ValueText = FText::FromName(PropertyValue);

		UpdateConfigObject();

		ChildSlot
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(SImage)
				.Image(this, &SConfigObjectPicker::GetMarkBrush)
				.ToolTipText(this, &SConfigObjectPicker::GetMarkText)
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(FMargin(5, 0, 5, 0))
			[
				SAssignNew(SearchBox, SAssetSearchBox)
				.InitialText(ValueText)
				.HintText(FText::FromString(TEXT("Value")))
				.OnTextCommitted(FOnTextCommitted::CreateSP(this, &SConfigObjectPicker::OnTextCommited))
				.OnAssetSearchBoxSuggestionFilter(FOnAssetSearchBoxSuggestionFilter::CreateSP(this, &SConfigObjectPicker::AssetSearchBoxSuggestionFilter))
				.PossibleSuggestions(PossibleSuggestions)
				.DelayChangeNotificationsWhileTyping(true)
				.MustMatchPossibleSuggestions(true)
			]
		];

		UpdateSuggestions();
	}

	void UpdateSuggestions()
	{
		PossibleSuggestions.Empty();

		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			TSharedPtr<ISettingsContainer> Container = SettingsModule->GetContainer("Project");

			TArray<TSharedPtr<ISettingsCategory>> Categories;
			Container->GetCategories(Categories);
			for (const TSharedPtr<ISettingsCategory>& Category : Categories)
			{
				TArray<TSharedPtr<ISettingsSection>> Sections;
				Category->GetSections(Sections);

				for (TSharedPtr<ISettingsSection>& Section : Sections)
				{
					TWeakObjectPtr<UObject> SettingsObject = Section->GetSettingsObject();
					if (SettingsObject.IsValid())
					{
						FAssetSearchBoxSuggestion Suggestion;
						Suggestion.CategoryName = Category->GetDisplayName();
						Suggestion.DisplayName = Section->GetDisplayName();
						Suggestion.SuggestionString = FString::Printf(TEXT("%s.%s"), *Category->GetName().ToString(), *Section->GetName().ToString());

						PossibleSuggestions.Add(Suggestion);
					}
				}
			}
		}
	}

	void UpdateConfigObject()
	{
		ConfigObject = FConfigPresetUtility::GetConfigObject(Property);
	}

	void OnTextCommited(const FText& Text, ETextCommit::Type Type)
	{
		FName NewValue;
		if (Type != ETextCommit::OnCleared)
		{
			NewValue = FName(Text.ToString());
		}
		Property->SetValue(NewValue);

		UpdateConfigObject();
	}

	void AssetSearchBoxSuggestionFilter(const FText& SearchText, TArray<FAssetSearchBoxSuggestion>& OutPossibleSuggestions, FText& SuggestionHighlightText)
	{
		OutPossibleSuggestions = PossibleSuggestions;
		SuggestionHighlightText = SearchText;

		FString SearchStr = SearchText.ToString();
		if (SearchStr.Len() >= 2 && !SearchStr.StartsWith(TEXT("None"), ESearchCase::IgnoreCase))
		{
			// Default implementation just filters against the current search text
			OutPossibleSuggestions.RemoveAll([SearchStr](const FAssetSearchBoxSuggestion& InSuggestion)
			{
				return !InSuggestion.SuggestionString.Contains(SearchStr, ESearchCase::IgnoreCase, ESearchDir::FromStart);
			});
		}
	}

	const FSlateBrush* GetMarkBrush() const
	{
		return ConfigObject.IsValid() ? FAppStyle::GetBrush("Symbols.Check") : FAppStyle::GetBrush("Icons.Error");
	}	

	FText GetMarkText() const
	{
		return ConfigObject.IsValid() ? LOCTEXT("Bound", "Bound to config") : LOCTEXT("NoConfig", "Config not found");
	}

private:
	TSharedPtr<IPropertyHandle> Property;
	TWeakObjectPtr<UObject> ConfigObject;


	TSharedPtr<SAssetSearchBox> SearchBox;
	TArray<FAssetSearchBoxSuggestion> PossibleSuggestions;
};

void FConfigPropertyPresetCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	ConfigHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FConfigPropertyPreset, Config));
	PropertyNameHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FConfigPropertyPreset, Property));
	ValueHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FConfigPropertyPreset, Value));

	FPropertyComboBoxArgs NameComboBoxArs;
	NameComboBoxArs.PropertyHandle = PropertyNameHandle;
	NameComboBoxArs.OnGetStrings.BindSP(this, &FConfigPropertyPresetCustomization::GetPropertyNames);
	NameComboBoxArs.OnValueSelected.BindSP(this, &FConfigPropertyPresetCustomization::PropertyNameSelected);

	HeaderRow
	.NameContent()
	.HAlign(HAlign_Fill)
	[
		SNew(SConfigObjectPicker, ConfigHandle.ToSharedRef())
	]
	.ValueContent()
	.MinDesiredWidth(300)
	.MaxDesiredWidth(4096)
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(5, 0, 3, 0)
		[
			PropertyNameHandle->CreatePropertyNameWidget()
		]
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SNew(SBox).MinDesiredWidth(200)
			[
				PropertyCustomizationHelpers::MakePropertyComboBox(NameComboBoxArs)
			]
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(5, 0, 3, 0)
		[
			ValueHandle->CreatePropertyNameWidget()
		]
		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[
			ValueHandle->CreatePropertyValueWidget(true)
		]
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SNew(SButton)
			.OnClicked(this, &FConfigPropertyPresetCustomization::Reset)
			.ButtonStyle(FAppStyle::Get(), "ToggleButton")
			.TextStyle(FAppStyle::Get(), "SmallText.Subdued")
			.ForegroundColor(FLinearColor::White)
			.HAlign(HAlign_Center)
			.Text(LOCTEXT("Reset", "Reset"))
		]
	];
}

void FConfigPropertyPresetCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// No children
}

FReply FConfigPropertyPresetCustomization::Reset()
{
	FString NewValue;

	if (UObject* ConfigObject = FConfigPresetUtility::GetConfigObject(ConfigHandle).Get())
	{
		FName PropertyName;
		if (PropertyNameHandle->GetValue(PropertyName) == FPropertyAccess::Success)
		{
			FProperty* Property = ConfigObject->GetClass()->FindPropertyByName(PropertyName);
			if (Property)
			{
				void* Data = Property->ContainerPtrToValuePtr<void>(ConfigObject);
				Property->ExportTextItem_Direct(NewValue, Data, nullptr, nullptr, PPF_None);
			}
		}
	}	
	ValueHandle->SetValue(NewValue);

	return FReply::Handled();
}


void FConfigPropertyPresetCustomization::GetPropertyNames(TArray<TSharedPtr<FString>>& Names, TArray<TSharedPtr<SToolTip>>& Tooltips, TArray<bool>& Enabled)
{
	TArray<FString> AllNames;

	if (UObject* ConfigObject = FConfigPresetUtility::GetConfigObject(ConfigHandle).Get())
	{
		for (TFieldIterator<FProperty> PropIt(ConfigObject->GetClass()); PropIt; ++PropIt)
		{
			FProperty* Prop = *PropIt;
			if (Prop && Prop->HasAllPropertyFlags(CPF_Edit))
			{
				AllNames.Add(Prop->GetName());
			}
		}
	}

	AllNames.StableSort();
	AllNames.Insert(TEXT("None"), 0);
	for (FString Str : AllNames)
	{
		Names.Add(MakeShared<FString>(Str));
	}
}

void FConfigPropertyPresetCustomization::PropertyNameSelected(const FString& Name)
{
	PropertyNameHandle->SetValue(Name);
	Reset();
}

#undef LOCTEXT_NAMESPACE