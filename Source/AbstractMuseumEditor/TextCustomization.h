#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class AAbstractMuseumText;

class FMuseumTextCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	bool IsTextTooLong(const FString& Text);

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	static bool bShowDefaults;
protected:
	TSharedPtr<STextBlock> WarningTextWidget;
	void CustomizeLocalTextFilePath(IDetailLayoutBuilder& DetailBuilder, AAbstractMuseumText* InTargetText);
	void MoveChanged();
	TArray<FName> AllCategories;
	TArray<FName> AlwaysVisible;
	IDetailLayoutBuilder* CachedDetailBuilder = nullptr; // to RefreshDetails
	void OnShowDefaultsChanged(IDetailLayoutBuilder* DetailBuilder);
	void OnTextPropertiesChanged();
	FString SplitLongLines(const FString& Input, int32 MaxLen);
private:
	AAbstractMuseumText* TargetText = nullptr;

	void OnPathPicked(const FString& PickedPath);
	FString GetSelectedFilePath() const;
};