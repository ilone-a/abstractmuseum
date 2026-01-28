#include "../Public/MuseumStorageManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "AbstractMuseumActor.h"
#include "Misc/Paths.h"
#include "Runtime/AssetRegistry/Public/AssetRegistry/AssetRegistryModule.h"



#if WITH_EDITOR
// assets from Editor World
void FMuseumStorageManager::RebuildAMShow()
{
	if (!GEditor) return;
	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
		if (!EditorWorld) return;

		//TODO remove hardcode
		UDataTable* Table = GetOrCreateDataTable("/Game/Data", "Show");
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

	UDataTable* Table = GetOrCreateDataTable("/Game/Data", "Storage");
	if (!Table) return;

	Table->Modify();
	Table->EmptyTable();

	FAssetRegistryModule& ARM =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& Registry = ARM.Get();

	// 1. Collect all child classes of AbstractMuseumActor
	TArray<FTopLevelAssetPath> BaseClasses;
	BaseClasses.Add(AAbstractMuseumActor::StaticClass()->GetClassPathName());

	TSet<FTopLevelAssetPath> DerivedClasses;
	Registry.GetDerivedClassNames(BaseClasses, {}, DerivedClasses);

	// 2. Get all BP
	TArray<FAssetData> Assets;
	Registry.GetAssetsByClass(
		UBlueprint::StaticClass()->GetClassPathName(),
		Assets,
		true
	);

	for (const FAssetData& Asset : Assets)
	{
		const FString GeneratedClassPath =
			Asset.GetTagValueRef<FString>("GeneratedClass");

		if (GeneratedClassPath.IsEmpty())
			continue;

		const FString CleanClassPath =
			FPackageName::ExportTextPathToObjectPath(GeneratedClassPath);

		const FTopLevelAssetPath ClassPath(CleanClassPath);
		// 3. Check hierarchy
		if (!DerivedClasses.Contains(ClassPath))
			continue;
		UClass* BPClass = LoadObject<UClass>(nullptr, *CleanClassPath);
		if (!BPClass)
			continue;

		UClass* It = BPClass;
		UClass* FoundCppParent = nullptr;

		while (It)
		{
			// first C++ native parent class
			if (It->ClassGeneratedBy == nullptr)
			{
				FoundCppParent = It;
				break;
			}

			It = It->GetSuperClass();
		}

		if (!FoundCppParent)
			continue;

		if (!FoundCppParent->IsChildOf(AAbstractMuseumActor::StaticClass()))
			continue;

		FAMDataTableRowEntry Row;
		Row.DisplayName = Asset.AssetName;
		Row.ActorClass = FoundCppParent;
		//Row.ActorClass = nullptr; 
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
	else return CreateDataTable(FolderPath, AssetName);
}

#endif