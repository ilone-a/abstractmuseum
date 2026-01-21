#pragma once
#include "CoreMinimal.h"
#include "../Public/AbstractMuseumDataTable.h"


class FMuseumStorageManager {
public:
#if WITH_EDITOR
	static void RebuildAMShow() ;
	static void RebuildAMStorage() ;
	static UDataTable* GetDataTable(const FString& FolderPath, const FString& AssetName) ;
	static UDataTable* CreateDataTable(const FString& FolderPath, const FString& AssetName) ;
	static UDataTable* GetOrCreateDataTable(const FString& FolderPath,const FString& AssetName);
#endif
};
