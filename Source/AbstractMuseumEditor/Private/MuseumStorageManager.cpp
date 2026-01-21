#include "../Public/MuseumStorageManager.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/AssetRegistry/Public/AssetRegistry/AssetRegistryModule.h"



#if WITH_EDITOR
// assets from Editor World
void FMuseumStorageManager::RebuildAMShow()
{
	if (!GEditor) return;
	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
		if (!EditorWorld) return;

}
// assets from AssetRegistry
void FMuseumStorageManager::RebuildAMStorage()
{
}

UDataTable* FMuseumStorageManager::GetDataTable(const FString& FolderPath, const FString& AssetName)
{
	const FString AssetPath = FolderPath / AssetName;
	if (AssetPath.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("DataTable path is empty"));
		return nullptr;
	}
	//todo check if file exists and log loading
	return Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(),
		nullptr,
		*AssetPath));
}

UDataTable* FMuseumStorageManager::CreateDataTable(const FString& FolderPath, const FString& AssetName)
{
	if (FolderPath.IsEmpty() || AssetName.IsEmpty()) return nullptr;

	const FString PackagePath = FolderPath / AssetName;
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package) return nullptr;

	Package->FullyLoad();
	UDataTable* NewTable = NewObject<UDataTable>(
		Package, UDataTable::StaticClass(), *AssetName,
		RF_Public | RF_Standalone | RF_Transactional
	);

	if (!NewTable) return nullptr;
	else
	{
		NewTable->RowStruct = FAMDataTableRowEntry::StaticStruct();
		NewTable->MarkPackageDirty();
		FAssetRegistryModule::AssetCreated(NewTable);
		return NewTable;
	}
}

UDataTable* FMuseumStorageManager::GetOrCreateDataTable(const FString& FolderPath, const FString& AssetName)
{
	
	if (UDataTable* Existing = GetDataTable(FolderPath, AssetName)) return Existing;
	else return nullptr;
}

#endif