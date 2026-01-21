#include "../Public/MuseumStorageManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "AbstractMuseumActor.h"

#include "Runtime/AssetRegistry/Public/AssetRegistry/AssetRegistryModule.h"



#if WITH_EDITOR
// assets from Editor World
void FMuseumStorageManager::RebuildAMShow()
{
	if (!GEditor) return;
	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
		if (!EditorWorld) return;

		//TODO remove hardcode
		FString ShowTablePath = "/AbstractMuseum/Data/";
		UDataTable* Table = GetOrCreateDataTable(ShowTablePath, "Show");
		if (!Table) return;

		Table->Modify();
		Table->EmptyTable();

		TArray<AActor*> Found;
		UGameplayStatics::GetAllActorsOfClass(
			EditorWorld,
			AAbstractMuseumActor::StaticClass(),
			Found
		);

		for (AActor* A : Found)
		{
			AAbstractMuseumActor* AM = Cast<AAbstractMuseumActor>(A);
			if (!AM) continue;

			FAMDataTableRowEntry Row;
			Row.DisplayName = AM->GetFName();
			Row.ActorClass = AM->GetClass();
			Row.AssetPath = AM->GetPathName();

			Table->AddRow(Row.DisplayName, Row);
		}

}
// assets from AssetRegistry
void FMuseumStorageManager::RebuildAMStorage()
{
	//TODO remove hardcode
	FString ShowTablePath = "/AbstractMuseum/Data/";
	UDataTable* Table = GetOrCreateDataTable(ShowTablePath, "Storage");
	if (!Table) return;

	Table->Modify();
	Table->EmptyTable();

	FAssetRegistryModule& ARM =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> Assets;
	const FTopLevelAssetPath BlueprintClassPath =
		UBlueprint::StaticClass()->GetClassPathName();

	ARM.Get().GetAssetsByClass(
		BlueprintClassPath,
		Assets,
		true
	);

	for (const FAssetData& Asset : Assets)
	{
		const FString ParentClass =
			Asset.GetTagValueRef<FString>("ParentClass");

		if (!ParentClass.Contains("AbstractMuseumActor"))
			continue;

		FAMDataTableRowEntry Row;
		Row.DisplayName = Asset.AssetName;
		Row.ActorClass = nullptr; // todo resolve
		Row.AssetPath = Asset.ObjectPath.ToString();

		Table->AddRow(Row.DisplayName, Row);
	}
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