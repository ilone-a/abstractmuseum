#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class AAbstractMuseumArt;

class FMuseumArtCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	FString GetCurrentFrameMaterialPath() const;
	static bool bShowDefaults;

protected:
	void OnFrameMaterialChanged(const FAssetData& AssetData);
	void CustomizeLocalTexturePath(IDetailLayoutBuilder& DetailBuilder, AAbstractMuseumArt* InTargetArt);
	void MoveChanged();
	void OnShowDefaultsChanged(IDetailLayoutBuilder* DetailBuilder);

	TArray<FName> AllCategories;
	TArray<FName> AlwaysVisible;
	IDetailLayoutBuilder* CachedDetailBuilder = nullptr; // to RefreshDetails

private:
	AAbstractMuseumArt* TargetArt = nullptr;
	void OnPathPicked(const FString& PickedPath);
	FString GetSelectedFilePath() const;
};