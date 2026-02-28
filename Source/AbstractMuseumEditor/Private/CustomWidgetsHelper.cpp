#include "CustomWidgetsHelper.h"

#include "Widgets/Input/SFilePathPicker.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/SlateTypes.h"
#include "EditorStyleSet.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "AbstractMuseumActor.h"
#include "Engine/Texture2D.h"

void FWidgetsHelper::AddShowDefaultsCheckbox(
	IDetailCategoryBuilder& CategoryBuilder,
	FSimpleDelegate OnChanged,
	TAttribute<ECheckBoxState> IsCheckedAttr)
{
	CategoryBuilder.AddCustomRow(FText::FromString("ShowDefaults"))
		.NameContent()
		[
			SNew(STextBlock)
				.Text(FText::FromString("Show default actor categories"))
		]
		.ValueContent()
		[
			SNew(SCheckBox)
				.IsChecked(IsCheckedAttr)
				.OnCheckStateChanged_Lambda([OnChanged](ECheckBoxState)
					{
						if (OnChanged.IsBound())
						{
							OnChanged.Execute();
						}
					})
		];
}


void FWidgetsHelper::AddLocalTexturePath(IDetailCategoryBuilder& CategoryBuilder, TSharedRef<IPropertyHandle> FilePathHandle, FOnPathPicked OnPathPickedDelegate)
{

	CategoryBuilder.AddCustomRow(FText::FromString("Load file"))
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

				.FileTypeFilter(TEXT("Image files (*.jpg;*.png;*.bmp)|*.jpg;*.png;*.bmp|All files (*.*)|*.*"))
				// pure static
				.FilePath_Lambda([FilePathHandle]() -> FString
					{
						FString Path;
						FilePathHandle->GetValue(Path);
						return Path;
					})

				// Delegate to set path
				.OnPathPicked_Lambda([FilePathHandle, OnPathPickedDelegate](const FString& PickedPath)
					{
						AsyncTask(ENamedThreads::GameThread, [FilePathHandle, OnPathPickedDelegate, PickedPath]()
							{
								if (FilePathHandle->IsValidHandle())
								{
									FilePathHandle->SetValue(PickedPath);
								}
								OnPathPickedDelegate.ExecuteIfBound(PickedPath);
							});
					})
		];
}

void FWidgetsHelper::AddLocalTextFilePath(IDetailCategoryBuilder& CategoryBuilder, TSharedRef<IPropertyHandle> FilePathHandle, FOnPathPicked OnPathPickedDelegate)
{

	CategoryBuilder.AddCustomRow(FText::FromString("Load file"))
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

				.FileTypeFilter(TEXT("Text files (*.txt;)|All files (*.*)|*.*"))
				// pure static
				.FilePath_Lambda([FilePathHandle]() -> FString
					{
						FString Path;
						FilePathHandle->GetValue(Path);
						return Path;
					})

				// Delegate to set path
				.OnPathPicked_Lambda([FilePathHandle, OnPathPickedDelegate](const FString& PickedPath)
					{
						AsyncTask(ENamedThreads::GameThread, [FilePathHandle, OnPathPickedDelegate, PickedPath]()
							{
								if (FilePathHandle->IsValidHandle())
								{
									FilePathHandle->SetValue(PickedPath);
								}
								OnPathPickedDelegate.ExecuteIfBound(PickedPath);
							});
					})
		];
}

FString FWidgetsHelper::GetFilePath(TSharedRef<IPropertyHandle> FilePathHandle)
{
	FString Path;
	FilePathHandle->GetValue(Path);
	return Path;
}
void FWidgetsHelper::SetFilePath(TSharedRef<IPropertyHandle> FilePathHandle, const FString& PickedPath)
{
	FilePathHandle->SetValue(PickedPath);
}
;
