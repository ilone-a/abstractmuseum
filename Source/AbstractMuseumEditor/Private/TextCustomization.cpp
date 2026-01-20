#include "TextCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/Input/SFilePathPicker.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "../Public/AbstractMuseumText.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "EditorStyleSet.h"
#include "Misc/Paths.h"
#include "AbstractMuseumFileHelper.h"
#include "CustomWidgetsHelper.h"

bool FMuseumTextCustomization::bShowDefaults = false;

TSharedRef<IDetailCustomization> FMuseumTextCustomization::MakeInstance()
{
	return MakeShareable(new FMuseumTextCustomization);
}
bool FMuseumTextCustomization::IsTextTooLong(const FString& Text)
{
	if (TargetText) {
		return Text.Len() > TargetText->GetTextMaxLen();
	}
	else return false;
}
void FMuseumTextCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() == 0) return;
	TargetText = Cast<AAbstractMuseumText>(Objects[0].Get());
	if (!TargetText) return;
	CachedDetailBuilder = &DetailBuilder;
	AlwaysVisible = { "Text file Loader", "View Options", "Projection", "Text Settings", "Plane", "Warning" };
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
	IDetailCategoryBuilder& LoaderCategory = DetailBuilder.EditCategory("Text file Loader");
	FOnPathPicked PathPickedDelegate;
	PathPickedDelegate.BindSP(SharedThis(this), &FMuseumTextCustomization::OnPathPicked);
	TSharedRef<IPropertyHandle> FilePathHandle =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(AAbstractMuseumActor, LocalFilePath),
			AAbstractMuseumActor::StaticClass());

	if (FilePathHandle->IsValidHandle())
	{
		FWidgetsHelper::AddLocalTextFilePath(LoaderCategory, FilePathHandle, PathPickedDelegate);
	}
	// Multi-line editable text box
	LoaderCategory.AddCustomRow(FText::FromString("Warning"))
		[
			SAssignNew(WarningTextWidget, STextBlock)
				.Text(FText::FromString("Text is too long (> 280 symbols)"))
				.ColorAndOpacity(FLinearColor::Yellow)
				.Visibility(EVisibility::Collapsed)
				.AutoWrapText(true)
		];
	LoaderCategory.AddCustomRow(FText::FromString("Text Content"))
		.NameContent()
		[
			SNew(STextBlock)
				.Text(FText::FromString("Text Content_M"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		.MaxDesiredWidth(600.f)

		[
			SNew(SMultiLineEditableTextBox)
				.Text_Lambda([this]() { return FText::FromString(TargetText->TextContent); })
				.OnTextChanged_Lambda([this](const FText& NewText)
					{
						if (TargetText)
						{
							if (IsTextTooLong(NewText.ToString())) WarningTextWidget->SetVisibility(EVisibility::Visible);
							else WarningTextWidget->SetVisibility(EVisibility::Collapsed);

							TargetText->TextContent = NewText.ToString();
							TargetText->UpdateText();
						}
					})
				.OnTextCommitted_Lambda([this](const FText& NewText, ETextCommit::Type)
					{
						if (TargetText)
						{
							if (IsTextTooLong(NewText.ToString())) WarningTextWidget->SetVisibility(EVisibility::Visible);
							else WarningTextWidget->SetVisibility(EVisibility::Collapsed);

							FString Fixed = SplitLongLines(NewText.ToString(), TargetText->GetStringMaxLen());
							TargetText->TextContent = Fixed;
							TargetText->UpdateText();
						}
					})
		];



	// View Options / Show Defaults Checkbox 
	IDetailCategoryBuilder& ViewCategory = DetailBuilder.EditCategory("View Options");
	FSimpleDelegate ToggleDefaultsDelegate =
		FSimpleDelegate::CreateSP(this, &FMuseumTextCustomization::OnShowDefaultsChanged, &DetailBuilder);
	FWidgetsHelper::AddShowDefaultsCheckbox(
		ViewCategory,
		ToggleDefaultsDelegate,
		TAttribute<ECheckBoxState>::Create([this]() {
			return bShowDefaults ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			})
	);
	// ====== Text Settings ======
	IDetailCategoryBuilder& TextCategory = DetailBuilder.EditCategory("Text Settings");

	TSharedRef<IPropertyHandle> ColorHandle =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(AAbstractMuseumText, TextColor),
			AAbstractMuseumText::StaticClass());

	TSharedRef<IPropertyHandle> ScaleHandle =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(AAbstractMuseumText, TextScale),
			AAbstractMuseumText::StaticClass());

	TSharedRef<IPropertyHandle> FontHandle =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(AAbstractMuseumText, Font),
			AAbstractMuseumText::StaticClass());



	// Horizontal alignment
	TSharedRef<IPropertyHandle> HorizHandle =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(AAbstractMuseumText, HorizontalAlignment));
	TextCategory.AddProperty(HorizHandle);

	// Vertical alignment
	TSharedRef<IPropertyHandle> VertHandle =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(AAbstractMuseumText, VerticalAlignment));
	TextCategory.AddProperty(VertHandle);

	// Delegate
	ColorHandle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateSP(this, &FMuseumTextCustomization::OnTextPropertiesChanged));

	ScaleHandle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateSP(this, &FMuseumTextCustomization::OnTextPropertiesChanged));

	FontHandle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateSP(this, &FMuseumTextCustomization::OnTextPropertiesChanged));
	//---Plane---

	// Plane Customization
	IDetailCategoryBuilder& PlaneCategory = DetailBuilder.EditCategory("Plane");

	TSharedPtr<IPropertyHandle> PlaneVisibleHandle = DetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(AAbstractMuseumText, bIsPlaneVisible),
		AAbstractMuseumText::StaticClass()
	);

	PlaneCategory.AddProperty(PlaneVisibleHandle)
		.CustomWidget()
		.NameContent()
		[
			PlaneVisibleHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			SNew(SCheckBox)
				.IsChecked_Lambda([PlaneVisibleHandle]()
					{
						bool bVal = false;
						PlaneVisibleHandle->GetValue(bVal);
						return bVal ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
				.OnCheckStateChanged_Lambda([this, PlaneVisibleHandle](ECheckBoxState NewState)
					{
						const bool bNewVal = (NewState == ECheckBoxState::Checked);
						PlaneVisibleHandle->SetValue(bNewVal);

						if (TargetText && TargetText->TextPlane)
						{
							TargetText->TextPlane->SetVisibility(bNewVal, true);
						}
					})
		];

}

void FMuseumTextCustomization::OnTextPropertiesChanged()
{
	if (TargetText)
	{
		TargetText->UpdateText();
		TargetText->UpdateTextPlane();
	}
}
FString FMuseumTextCustomization::SplitLongLines(const FString& Input, int32 MaxLen)
{
	TArray<FString> Paragraphs;
	TArray<FString> Delimiters;

	FString Current;
	for (int32 i = 0; i < Input.Len(); i++)
	{
		TCHAR C = Input[i];

		// found \r\n
		if (C == '\r' && i + 1 < Input.Len() && Input[i + 1] == '\n')
		{
			Paragraphs.Add(Current);
			Delimiters.Add(TEXT("\r\n"));
			Current.Empty();
			i++; // skip \n
		}
		// found \n
		else if (C == '\n')
		{
			Paragraphs.Add(Current);
			Delimiters.Add(TEXT("\n"));
			Current.Empty();
		}
		// found \r
		else if (C == '\r')
		{
			Paragraphs.Add(Current);
			Delimiters.Add(TEXT("\r"));
			Current.Empty();
		}
		else
		{
			Current.AppendChar(C);
		}
	}

	// last paragraph
	Paragraphs.Add(Current);

	// every paragraph check strings
	FString Output;

	for (int32 p = 0; p < Paragraphs.Num(); p++)
	{
		const FString& Para = Paragraphs[p];

		// empty
		if (Para.IsEmpty())
		{
			Output += Para;
		}
		else
		{
			// split to words
			TArray<FString> Words;
			Para.ParseIntoArray(Words, TEXT(" "), true);

			int32 CurrentCount = 0;
			FString Line;

			for (const FString& Word : Words)
			{
				if (CurrentCount + Word.Len() > MaxLen)
				{
					Output += Line + TEXT("\n");
					Line.Empty();
					CurrentCount = 0;
				}

				Line += Word + TEXT(" ");
				CurrentCount += Word.Len() + 1;
			}

			if (!Line.IsEmpty())
			{
				Output += Line;
			}
		}

		// return splitters
		if (p < Delimiters.Num())
		{
			Output += Delimiters[p];
		}
	}

	return Output;
}
void FMuseumTextCustomization::OnShowDefaultsChanged(IDetailLayoutBuilder* DetailBuilder)
{
	bShowDefaults = !bShowDefaults;

	if (DetailBuilder)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}


void FMuseumTextCustomization::CustomizeLocalTextFilePath(IDetailLayoutBuilder& DetailBuilder, AAbstractMuseumText* InTargetText)
{
	if (!InTargetText) return;

}

void FMuseumTextCustomization::MoveChanged()
{

}


void FMuseumTextCustomization::OnPathPicked(const FString& PickedPath)
{
	if (TargetText)
	{
		FString hash = TargetText->GetHash();
		if (FAbstractMuseumFileHelper::IsFileChanged(PickedPath, hash))
		{
			TargetText->TextContent = FAbstractMuseumFileHelper::LoadTextFileFromDisk(PickedPath, TargetText->GetHash());

			if (!TargetText->TextContent.IsEmpty())
			{
				TargetText->UpdateText();
			}
		}
	}

}

FString FMuseumTextCustomization::GetSelectedFilePath() const
{
	FString FilePath;
	{
		FilePath = TargetText->LocalFilePath;
	}
	return FilePath;
}
