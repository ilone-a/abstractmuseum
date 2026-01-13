#include "ArtCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/Input/SFilePathPicker.h"
#include "../Public/AbstractMuseumArt.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "EditorStyleSet.h"
#include "Misc/Paths.h"
#include "AbstractMuseumFileHelper.h"
#include "CustomWidgetsHelper.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SNumericEntryBox.h"

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
	AlwaysVisible = { "Texture Loader", "View Options", "Projection", "Frame" };

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
	if (!TargetArt)
		return;

	const FString FullPath = FPaths::ConvertRelativePathToFull(PickedPath);

	TargetArt->Modify(); // save changes
	TargetArt->LocalFilePath = FullPath;
	TargetArt->MarkPackageDirty();

	FString Hash = TargetArt->GetHash();
	if (FAbstractMuseumFileHelper::IsFileChanged(FullPath, Hash))
	{
		TargetArt->LoadedTexture =
			FAbstractMuseumFileHelper::LoadTextureFromDisk(FullPath, TargetArt->GetHash());

		if (TargetArt->LoadedTexture)
		{
			if (!TargetArt->ArtMaterial)
			{
				TargetArt->CreateDynamicMaterial();
			}

			TargetArt->ApplyTexture();

			if (TargetArt->LoadedTexture->GetSizeX() > 0 &&
				TargetArt->LoadedTexture->GetSizeY() > 0)
			{
				TargetArt->ScaleMeshes();
			}
		}
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