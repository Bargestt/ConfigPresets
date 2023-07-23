// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IPropertyHandle;


class FConfigPropertyPresetCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShareable(new FConfigPropertyPresetCustomization);
	}

	//~ Begin IPropertyTypeCustomization Interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	//~ End IPropertyTypeCustomization Interface
private:
	FReply Reset();

	void GetPropertyNames(TArray<TSharedPtr<FString>>& Names, TArray<TSharedPtr<SToolTip>>& Tooltips, TArray<bool>& Enabled);
	void PropertyNameSelected(const FString& Name);

	TSharedPtr<IPropertyHandle> ConfigHandle;
	TSharedPtr<IPropertyHandle> PropertyNameHandle;
	TSharedPtr<IPropertyHandle> ValueHandle;
};
