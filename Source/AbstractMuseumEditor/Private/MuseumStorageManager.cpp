#include "../Public/MuseumStorageManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "AbstractMuseumArt.h"
#include "AbstractMuseumText.h"
#include "AbstractMuseumItem.h"
#include "Misc/Paths.h"
#include "Runtime/AssetRegistry/Public/AssetRegistry/AssetRegistryModule.h"
#include "FileHelpers.h"


#if WITH_EDITOR
// assets from Editor World
void FMuseumStorageManager::RebuildAMShow()
{
	if (!GEditor) return;
	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
		if (!EditorWorld) return;

		//TODO remove hardcode
		UDataTable* Table = GetOrCreateDataTable("/Game/AbstractMuseum/GeneratedStorage", "Show");
		if (!Table) return;

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
			Row.AssetType = ResolveMuseumAssetType(AM->GetClass());
			Row.AssetPath = AM->GetPathName();
			
			Table->AddRow(Row.DisplayName, Row);
		}
//Save data table
		Table->MarkPackageDirty();
		UPackage* Package = Table->GetOutermost();
		FEditorFileUtils::PromptForCheckoutAndSave({ Package }, true, false);
}
// assets from AssetRegistry
void FMuseumStorageManager::RebuildAMStorage()
{
	UDataTable* Table = GetOrCreateDataTable("/Game/AbstractMuseum/GeneratedStorage", "Storage");
	if (!Table)
		return;

	//Table->Modify();
	Table->EmptyTable();

	FAssetRegistryModule& ARM =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& Registry = ARM.Get();

	const TSet<FTopLevelAssetPath> MuseumClasses =
		GetAllMuseumDerivedClasses();

	TArray<FAssetData> Assets;
	Registry.GetAssetsByClass(
		UBlueprint::StaticClass()->GetClassPathName(),
		Assets,
		true
	);

	for (const FAssetData& Asset : Assets)
	{
		if (!IsMuseumAsset(Asset, MuseumClasses))
			continue;

		UClass* RootCppClass = GetRootCppClassFromBP(Asset);
		if (!RootCppClass)
			continue;

		FAMDataTableRowEntry Row;
		Row.DisplayName = Asset.AssetName;
		Row.AssetType = ResolveMuseumAssetType(RootCppClass);
		Row.AssetPath = Asset.ObjectPath.ToString();

		Table->AddRow(Row.DisplayName, Row);
	}
}
 bool FMuseumStorageManager::IsMuseumAsset(const FAssetData& Asset, const TSet<FTopLevelAssetPath>& MuseumDerivedClasses)
{
	 const FString GeneratedClassPath =
		 Asset.GetTagValueRef<FString>("GeneratedClass");

	 if (GeneratedClassPath.IsEmpty())
		 return false;

	 const FString CleanPath =
		 FPackageName::ExportTextPathToObjectPath(GeneratedClassPath);

	 return MuseumDerivedClasses.Contains(
		 FTopLevelAssetPath(CleanPath));
}

 TSet<FTopLevelAssetPath> FMuseumStorageManager::GetAllMuseumDerivedClasses()
 {
	 FAssetRegistryModule& ARM =
		 FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	 TArray<FTopLevelAssetPath> Base;
	 Base.Add(AAbstractMuseumActor::StaticClass()->GetClassPathName());

	 TSet<FTopLevelAssetPath> Derived;
	 TSet<FTopLevelAssetPath> Excluded;

	 ARM.Get().GetDerivedClassNames(Base, Excluded, Derived);

	 return Derived;
 }



EAMAssetType FMuseumStorageManager::ResolveMuseumAssetType(UClass* RootCppClass)
{
	check(RootCppClass);

	if (RootCppClass->IsChildOf(AAbstractMuseumArt::StaticClass()))
		return EAMAssetType::Art;

	if (RootCppClass->IsChildOf(AAbstractMuseumText::StaticClass()))
		return EAMAssetType::Text;

	if (RootCppClass->IsChildOf(AAbstractMuseumItem::StaticClass()))
		return EAMAssetType::Item;

	return EAMAssetType::Error;
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
	// Try to get created data table
	if (UDataTable* Existing = GetDataTable(FolderPath, AssetName))
	{
		return Existing;
	}

	// Or create new one
	UDataTable* NewTable = CreateDataTable(FolderPath, AssetName);
	if (!NewTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to create DataTable %s/%s"), *FolderPath, *AssetName);
		return nullptr;
	}

	FAssetRegistryModule::AssetCreated(NewTable);

	return NewTable;
}

UClass* FMuseumStorageManager::GetRootCppClassFromBP(const FAssetData& Asset)
{
	const FString GeneratedClassPath =
		Asset.GetTagValueRef<FString>("GeneratedClass");

	if (GeneratedClassPath.IsEmpty())
		return nullptr;

	const FString CleanPath =
		FPackageName::ExportTextPathToObjectPath(GeneratedClassPath);

	UClass* BPClass = LoadObject<UClass>(nullptr, *CleanPath);
	if (!BPClass)
		return nullptr;

	for (UClass* It = BPClass; It; It = It->GetSuperClass())
	{
		if (It->ClassGeneratedBy == nullptr)
			return It;
	}

	return nullptr;
}

#endif