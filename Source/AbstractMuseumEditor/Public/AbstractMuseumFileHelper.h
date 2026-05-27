#pragma once
#include "CoreMinimal.h"
#include "Engine/Texture2D.h"


class ABSTRACTMUSEUMEDITOR_API FAbstractMuseumFileHelper
{
	//C++ static lib 
private:
	FAbstractMuseumFileHelper() = delete;
	~FAbstractMuseumFileHelper() = delete;
public:
	//----Load---
	static UTexture2D* LoadTextureFromDisk(const FString& FilePath, FString & OutFileHash);
	static FString LoadTextFileFromDisk(const FString& FilePath, FString& OutFileHash);
#if WITH_EDITOR
	static UTexture2D* ImportTextureAsAsset(
		const FString& SourceFilePath,
		const FString& AssetName,
		const FString& PackagePath   // "/Game/AbstractMuseum/GeneratedTextures"
	);
	static UMaterialInstanceConstant* CreateOrGetMaterialInstance(
		UObject* Owner,
		UMaterialInterface* BaseMaterial,
		UMeshComponent* TargetMesh
	);
	static UTexture2D* CreateOrGetTextureAsset(const FString& PackagePath, const FString& AssetName);
#endif
	//----Hash---
	//static FString CalculateFileHash(const TArray<uint8>& Data);
	static bool IsFileChanged(const FString& FilePath, const FString& OldHash);
	static FString CalculateFileHashFromPath(const FString& FilePath);
};

