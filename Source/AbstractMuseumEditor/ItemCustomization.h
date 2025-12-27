#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class AAbstractMuseumItem;

class FItemCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

protected:
	void MoveChanged();

private:
	AAbstractMuseumItem* TargetItem = nullptr;
	void OnPathPicked(const FString& PickedPath);
	FString GetSelectedFilePath() const;
};