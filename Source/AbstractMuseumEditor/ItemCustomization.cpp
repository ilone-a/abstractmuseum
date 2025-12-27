#include "ItemCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/Input/SFilePathPicker.h"
#include "../Public/AbstractMuseumItem.h"

#include "IDetailChildrenBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "EditorStyleSet.h"
#include "Misc/Paths.h"
#include "Engine/StaticMesh.h"
#include "AssetToolsModule.h"
#include "Factories/FbxFactory.h"
#include "EditorAssetLibrary.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"

TSharedRef<IDetailCustomization> FItemCustomization::MakeInstance()
{
	return MakeShareable(new FItemCustomization);
}

void FItemCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() == 0) return;
	TargetItem = Cast<AAbstractMuseumItem>(Objects[0].Get());
	if (!TargetItem) return;

	IDetailCategoryBuilder& Cat = DetailBuilder.EditCategory("Default");

	//----Get property handle
	TSharedPtr<IPropertyHandle> MeshProp =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(AAbstractMuseumItem, MeshAsset));
	Cat.AddProperty(MeshProp);
}


FString FItemCustomization::GetSelectedFilePath() const
{
	FString FilePath;
	if (TargetItem)
	{
		FilePath = TargetItem->MeshAssetPath;
	}
	return FilePath;
}

