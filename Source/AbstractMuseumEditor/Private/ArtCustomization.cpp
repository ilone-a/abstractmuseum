#include "ArtCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/Input/SFilePathPicker.h"
#include "../Public/AbstractMuseumArt.h"
#include "../Public/AbstractMuseumFileHelper.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "EditorStyleSet.h"
#include "Misc/Paths.h"
#include "CustomWidgetsHelper.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"

bool FMuseumArtCustomization::bShowDefaults = false;
TSharedRef<IDetailCustomization> FMuseumArtCustomization::MakeInstance()
{
	return MakeShareable(new FMuseumArtCustomization);
}

void FMuseumArtCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() == 0) return;
	TargetArt = Cast<AAbstractMuseumArt>(Objects[0].Get());
	if (!TargetArt) return;
	CachedDetailBuilder = &DetailBuilder;
	AlwaysVisible = { "Texture Loader", "View Options", "Projection", "Frame", "Info" };

	//----Hide all categories except our AlwaysVisible list---
	DetailBuilder.GetCategoryNames(AllCategories);
	if (!bShowDefaults)
	{

		for (const FName& Cat : AllCategories)
		{
			if (!AlwaysVisible.Contains(Cat))
			{
				DetailBuilder.HideCategory(Cat);
			}
		}
	}


	//----Texture Loader - PathPicker widget customization
	IDetailCategoryBuilder& LoaderCategory = DetailBuilder.EditCategory("Texture Loader");
	FOnPathPicked PathPickedDelegate;
	PathPickedDelegate.BindSP(SharedThis(this), &FMuseumArtCustomization::OnPathPicked);
	TSharedRef<IPropertyHandle> FilePathHandle =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(AAbstractMuseumActor, LocalFilePath),
			AAbstractMuseumActor::StaticClass());

	if (FilePathHandle->IsValidHandle())
	{
		FWidgetsHelper::AddLocalTexturePath(LoaderCategory, FilePathHandle, PathPickedDelegate);
	}


	//----View Options / Show Defaults Checkbox 
	IDetailCategoryBuilder& ViewCategory = DetailBuilder.EditCategory("View Options");
	FSimpleDelegate ToggleDefaultsDelegate =
		FSimpleDelegate::CreateSP(this, &FMuseumArtCustomization::OnShowDefaultsChanged, &DetailBuilder);
	FWidgetsHelper::AddShowDefaultsCheckbox(
		ViewCategory,
		ToggleDefaultsDelegate,
		TAttribute<ECheckBoxState>::Create([this]() {
			return bShowDefaults ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			})
	);




	
}

FString FMuseumArtCustomization::GetCurrentFrameMaterialPath() const
{
	
	return FString();
}

void FMuseumArtCustomization::OnFrameMaterialChanged(const FAssetData& AssetData)
{
	
}


void FMuseumArtCustomization::CustomizeLocalTexturePath(IDetailLayoutBuilder& DetailBuilder, AAbstractMuseumArt* InTargetArt)
{
	if (!InTargetArt) return;

	IDetailCategoryBuilder& LoaderCategory = DetailBuilder.EditCategory("Texture Loader");

	LoaderCategory.AddCustomRow(FText::FromString("Load file"))
		.NameContent()
		[
			SNew(STextBlock)
				.Text(FText::FromString("Load file"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		.MaxDesiredWidth(600)
		[
			SNew(SFilePathPicker)
				.BrowseButtonImage(FEditorStyle::GetBrush("PropertyWindow.Button_Ellipsis"))
				.BrowseButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
				.BrowseButtonToolTip(FText::FromString("Choose a file"))
				.FilePath(this, &FMuseumArtCustomization::GetSelectedFilePath)
				.FileTypeFilter(TEXT("Image files (*.jpg;*.png;*.bmp)|*.jpg;*.png;*.bmp|All files (*.*)|*.*"))
				.OnPathPicked(this, &FMuseumArtCustomization::OnPathPicked)
		];
}

void FMuseumArtCustomization::MoveChanged()
{

}

void FMuseumArtCustomization::OnShowDefaultsChanged(IDetailLayoutBuilder* DetailBuilder)
{
	bShowDefaults = !bShowDefaults;

	if (DetailBuilder)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}


void FMuseumArtCustomization::OnPathPicked(const FString& PickedPath)
{
#if WITH_EDITOR
	if (!TargetArt) {
		UE_LOG(LogTemp, Error, TEXT("Unable to change path: actor file missing"));
		return;
	}

	//source image path
	const FString NewImageSourcePath = FPaths::ConvertRelativePathToFull(PickedPath);
	FString CurrentHash = TargetArt->GetHash();

	//Create asset name
	const FString StableBaseName = FPackageName::GetShortName(TargetArt->GetName());
	const FString TextureAssetName = FString::Printf(TEXT("TEX_%s"), *StableBaseName);


	const FString FullPackagePath = NewImageSourcePath + TEXT("/") + TextureAssetName;
	const FString ObjectPath = FullPackagePath + TEXT(".") + TextureAssetName;

	//if assets exists, check nothing changed
	const bool bPathChanged = (TargetArt->LocalFilePath != NewImageSourcePath);
	const bool bFileChanged = FAbstractMuseumFileHelper::IsFileChanged(NewImageSourcePath, CurrentHash);

	if (!bPathChanged && !bFileChanged) return;

	{
		TargetArt->Modify(); // save changes
		//TODO setter
		TargetArt->LocalFilePath = NewImageSourcePath;
		TargetArt->MarkPackageDirty();

		UTexture2D* SavedTexture = nullptr;

		ChangeTexture(SavedTexture, NewImageSourcePath, TextureAssetName);

		// Update TargetArt data
		if (SavedTexture &&
			SavedTexture->GetSizeX() > 0 &&
			SavedTexture->GetSizeY() > 0)
		{
			TargetArt->ScaleMeshes();
		}

		TargetArt->SetHash(
			FAbstractMuseumFileHelper::CalculateFileHashFromPath(NewImageSourcePath)
		);
		TargetArt->MarkPackageDirty();
		TargetArt->GetOutermost()->MarkPackageDirty();

#endif
	}
}

FString FMuseumArtCustomization::GetSelectedFilePath() const
{
	FString FilePath;
	{
		FilePath = TargetArt->LocalFilePath;
	}
	return FilePath;
}

void FMuseumArtCustomization::ChangeTexture(UTexture2D* SavedTexture, FString NewImageSourcePath, FString TextureAssetName)
{

	SavedTexture =
		FAbstractMuseumFileHelper::ImportTextureAsAsset(
			NewImageSourcePath,
			TextureAssetName,
			TEXT("/Game/AbstractMuseum/GeneratedTextures")
		);

	if (!SavedTexture) 
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to import texture as asset"));
		return;
	}

	UMaterialInstanceConstant* MIC =
		FAbstractMuseumFileHelper::CreateOrGetMaterialInstance(
			TargetArt,
			TargetArt->BaseMaterial,
			TargetArt->Plane
		);

	if (!MIC)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to import material instance as asset"));
		return;
	}

	TargetArt->ArtMaterialAsset = MIC;

	FMaterialParameterInfo ParamInfo(TEXT("Art"));
	MIC->SetTextureParameterValueEditorOnly(ParamInfo, SavedTexture);

	MIC->PostEditChange();
	MIC->MarkPackageDirty();

	if (TargetArt->Plane)
	{
		TargetArt->Plane->SetMaterial(0, MIC);
	}
	SavedTexture->PostEditChange();
	SavedTexture->MarkPackageDirty();
}