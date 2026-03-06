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
#include "AbstractMuseumFileHelper.h"
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
	AlwaysVisible = { "Texture Loader", "View Options", "Projection", "Frame", "Info"};

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

	//----Frame Customization
	IDetailCategoryBuilder& FrameCategory = DetailBuilder.EditCategory("Frame");

	TSharedPtr<IPropertyHandle> FrameVisibleHandle = DetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(AAbstractMuseumArt, bIsFrameVisible),
		AAbstractMuseumArt::StaticClass()
	);

	FrameCategory.AddProperty(FrameVisibleHandle)
		.CustomWidget()
		.NameContent()
		[
			FrameVisibleHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			SNew(SCheckBox)
				.IsChecked_Lambda([this, FrameVisibleHandle]()
					{

						bool bVal = TargetArt->bIsFrameVisible;
						FrameVisibleHandle->GetValue(bVal);
						return bVal ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
				.OnCheckStateChanged_Lambda([this, FrameVisibleHandle](ECheckBoxState NewState)
					{
						const bool bNewVal = (NewState == ECheckBoxState::Checked);
						TargetArt->SetFrameVisible(bNewVal);
						FrameVisibleHandle->SetValue(bNewVal);

						if (TargetArt && TargetArt->Frame)
						{
							TargetArt->Modify();
							TargetArt->Frame->SetVisibility(bNewVal, true);
							TargetArt->PostEditChange();
							TargetArt->MarkPackageDirty();
						}
					})
		];

	//----Frame Border

	TSharedPtr<IPropertyHandle> FrameBorderHandle = DetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(AAbstractMuseumArt, FrameBorder),
		AAbstractMuseumArt::StaticClass()
	);


	FrameCategory.AddProperty(FrameBorderHandle)
		.CustomWidget()
		.NameContent()
		[
			FrameBorderHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(200.f)
		[
			SNew(SNumericEntryBox<float>)
				.MinValue(0.001f)
				.MaxValue(0.5f)
				.MinSliderValue(0.001f)
				.MaxSliderValue(0.5f)
				.Value(TargetArt->FrameBorder)
				.OnValueCommitted_Lambda([this, FrameBorderHandle](float NewValue, ETextCommit::Type)
					{
						const float Clamped = FMath::Clamp(NewValue, 0.001f, 0.5f);
						FrameBorderHandle->SetValue(Clamped);

						if (TargetArt)
						{
							TargetArt->FrameBorder = Clamped;
							TargetArt->UpdateFrame();
						}
					})
		];
}

FString FMuseumArtCustomization::GetCurrentFrameMaterialPath() const
{
	if (TargetArt && TargetArt->Frame)
	{
		if (UMaterialInterface* Mat = TargetArt->Frame->GetMaterial(0))
		{
			return Mat->GetPathName();
		}
	}
	return FString();
}

void FMuseumArtCustomization::OnFrameMaterialChanged(const FAssetData& AssetData)
{
	if (TargetArt && TargetArt->Frame)
	{
		if (UMaterialInterface* Mat = Cast<UMaterialInterface>(AssetData.GetAsset()))
		{
			TargetArt->Frame->SetMaterial(0, Mat);
		}
	}
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
	if (!TargetArt)
		return;

	const FString FullPath = FPaths::ConvertRelativePathToFull(PickedPath);

	FString Hash = TargetArt->GetHash();
	//check if nothing changed
	const bool bPathChanged = (TargetArt->LocalFilePath != FullPath);
	const bool bFileChanged = FAbstractMuseumFileHelper::IsFileChanged(FullPath, Hash);
	if (!bPathChanged && !bFileChanged) return;

	//if changed
	//if ((TargetArt->LocalFilePath != PickedPath) || (FAbstractMuseumFileHelper::IsFileChanged(PickedPath, Hash)))

	{

		TargetArt->Modify(); // save changes
		TargetArt->LocalFilePath = FullPath;
		TargetArt->MarkPackageDirty();

		//Create asset name
		const FString StableBaseName = FPackageName::GetShortName(TargetArt->GetName());
		const FString TextureAssetName = FString::Printf(TEXT("TEX_%s"), *StableBaseName);

		//Import file to save to texture
		//UTexture2D* Texture = nullptr;
		TargetArt->Modify();
		TargetArt->LocalFilePath = FullPath;



		UTexture2D* SavedTexture = nullptr;

		// ----------------------------
		// CASE 1 — NEW PATH
		// ----------------------------
		if (bPathChanged)
		{
			SavedTexture =
				FAbstractMuseumFileHelper::ImportTextureAsAsset(
					FullPath,
					TextureAssetName,
					TEXT("/Game/AbstractMuseum/GeneratedTextures")
				);

			if (!SavedTexture)
				return;

			UMaterialInstanceConstant* MIC =
				FAbstractMuseumFileHelper::CreateOrGetMaterialInstance(
					TargetArt,
					TargetArt->BaseMaterial,
					TargetArt->Plane
				);

			if (!MIC)
				return;

			TargetArt->ArtMaterialAsset = MIC;

			FMaterialParameterInfo ParamInfo(TEXT("Art"));
			MIC->SetTextureParameterValueEditorOnly(ParamInfo, SavedTexture);

			MIC->PostEditChange();
			MIC->MarkPackageDirty();

			if (TargetArt->Plane)
			{
				TargetArt->Plane->SetMaterial(0, MIC);
			}
		}

		// ----------------------------
		// CASE 2 — CHANGED FILE
		// ----------------------------
		else if (bFileChanged)
		{
			SavedTexture =
				FAbstractMuseumFileHelper::ImportTextureAsAsset(
					FullPath,
					TextureAssetName,
					TEXT("/Game/AbstractMuseum/GeneratedTextures")
				);

			if (!SavedTexture)
				return;

			SavedTexture->PostEditChange();
			SavedTexture->MarkPackageDirty();
		}

		// ----------------------------
		// COMMON PART
		// ----------------------------

		if (SavedTexture &&
			SavedTexture->GetSizeX() > 0 &&
			SavedTexture->GetSizeY() > 0)
		{
			TargetArt->ScaleMeshes();
		}

		TargetArt->SetHash(
			FAbstractMuseumFileHelper::CalculateFileHashFromPath(FullPath)
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