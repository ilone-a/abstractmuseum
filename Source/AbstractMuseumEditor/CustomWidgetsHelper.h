#pragma once
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "DetailCategoryBuilder.h"



DECLARE_DELEGATE_OneParam(FOnPathPicked, const FString&)
class FWidgetsHelper {
public:
	static void AddShowDefaultsCheckbox(
		IDetailCategoryBuilder& CategoryBuilder,
		FSimpleDelegate OnChanged,
		TAttribute<ECheckBoxState> IsCheckedAttr);

	static void AddLocalTexturePath(IDetailCategoryBuilder& CategoryBuilder, TSharedRef<IPropertyHandle> FilePathHandle,
		FOnPathPicked OnPathPickedDelegate);
	static void AddLocalTextFilePath(IDetailCategoryBuilder& CategoryBuilder, TSharedRef<IPropertyHandle> FilePathHandle,
		FOnPathPicked OnPathPickedDelegate);

	static FString GetFilePath(TSharedRef<IPropertyHandle> FilePathHandle);
	static void SetFilePath(TSharedRef<IPropertyHandle> FilePathHandle, const FString& PickedPath);
};