#pragma once
#include "CoreMinimal.h"
#include "Engine/Texture2D.h"


class ABSTRACTMUSEUM_API FAbstractMuseumFileHelper 
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
#endif
	//----Hash---
	static FString CalculateFileHash(const TArray<uint8>& Data);
	static bool IsFileChanged(const FString& FilePath, const FString& OldHash);
};

