#include "AbstractMuseumEditorModule.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Runtime/AssetRegistry/Public/AssetRegistry/AssetRegistryModule.h"
#include "../Public/MuseumStorageManager.h"
#include "../Public/AbstractMuseumArt.h"
#include "../Public/AbstractMuseumItem.h"
#include "../Public/AbstractMuseumText.h"
#include "ArtCustomization.h"
#include "ItemCustomization.h"
#include "TextCustomization.h"
#include "AbstractMuseumActor.h"
#include "Engine/Selection.h"
#include "ToolMenus.h"

IMPLEMENT_MODULE(FAbstractMuseumEditorModule, AbstractMuseumEditor)

#define LOCTEXT_NAMESPACE "FAbstractMuseumEditorModule"

void FAbstractMuseumEditorModule::StartupModule()
{
    RegisterCustomizations();

    FAssetRegistryModule& ARM =
        FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

    ARM.Get().OnFilesLoaded().AddLambda([]()
        {
            FMuseumStorageManager::RebuildAMStorage();
        });
    FEditorDelegates::OnMapOpened.AddLambda(
        [](const FString&, bool)
        {
            FMuseumStorageManager::RebuildAMShow();
        }
    );
}

void FAbstractMuseumEditorModule::ShutdownModule()
{
    UnregisterCustomizations();
}

void FAbstractMuseumEditorModule::RegisterCustomizations()
{
    FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

    PropertyModule.RegisterCustomClassLayout(
        AAbstractMuseumArt::StaticClass()->GetFName(),
        FOnGetDetailCustomizationInstance::CreateStatic(&FMuseumArtCustomization::MakeInstance)
    );

    PropertyModule.RegisterCustomClassLayout(
        AAbstractMuseumItem::StaticClass()->GetFName(),
        FOnGetDetailCustomizationInstance::CreateStatic(&FItemCustomization::MakeInstance)
    );

    PropertyModule.RegisterCustomClassLayout(
        AAbstractMuseumText::StaticClass()->GetFName(),
        FOnGetDetailCustomizationInstance::CreateStatic(&FMuseumTextCustomization::MakeInstance)
    );
}

void FAbstractMuseumEditorModule::UnregisterCustomizations()
{
    FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

    PropertyModule.UnregisterCustomClassLayout(AAbstractMuseumArt::StaticClass()->GetFName());
    PropertyModule.UnregisterCustomClassLayout(AAbstractMuseumItem::StaticClass()->GetFName());
    PropertyModule.UnregisterCustomClassLayout(AAbstractMuseumText::StaticClass()->GetFName());
}

#undef LOCTEXT_NAMESPACE