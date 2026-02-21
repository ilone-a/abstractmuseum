#include "AbstractMuseumFileHelper.h"
#include "Misc/FileHelper.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/Package.h"
#include "Factories/TextureFactory.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
#include "Editor.h"

UTexture2D* FAbstractMuseumFileHelper::LoadTextureFromDisk(const FString& FilePath, FString& OutFileHash)
{
	
	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Error reading file: %s"), *FilePath);
		return nullptr;
	}

	//Calculate hash
	FSHAHash Hash;
	FSHA1::HashBuffer(FileData.GetData(), FileData.Num(), Hash.Hash);
	OutFileHash = Hash.ToString();


	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

	//Detect image file format
	EImageFormat Format = EImageFormat::PNG;
	if (FilePath.EndsWith(".jpg") || FilePath.EndsWith(".jpeg"))
	{
		Format = EImageFormat::JPEG;
	}
	else if (FilePath.EndsWith(".bmp"))
	{
		Format = EImageFormat::BMP;
	}

	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(Format);
	//TODO check valid
	if (ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num()))
	{
		 TArray<uint8> RawData;
		if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData))
		{
			int32 Width = ImageWrapper->GetWidth();
			int32 Height = ImageWrapper->GetHeight();
			UTexture2D*  LoadedTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);

			if (!LoadedTexture)
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to create transient texture for: %s"), *FilePath);
				OutFileHash = TEXT("");
				return nullptr;
			}

			LoadedTexture->AddToRoot();//specially for GC
			
			// 5.3
			// void* TextureData = LoadedTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			// FMemory::Memcpy(TextureData, RawData.GetData(), RawData.Num());
			// LoadedTexture->PlatformData->Mips[0].BulkData.Unlock();
			// LoadedTexture->UpdateResource();

			// 5.5 update: PlatformData is protected now
			FTexturePlatformData* PlatformData = LoadedTexture->GetPlatformData();
			if (PlatformData && PlatformData->Mips.Num() > 0)
			{
				// lock first mip
				void* TextureData = PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
				// copy raw data to texture
				FMemory::Memcpy(TextureData, RawData.GetData(), RawData.Num());
				// unlock
				PlatformData->Mips[0].BulkData.Unlock();
				// update resource to move data to GPU
				LoadedTexture->UpdateResource();
			}
			return LoadedTexture;
		}
	}
	
	OutFileHash = TEXT("");
	return nullptr;
	
}

FString FAbstractMuseumFileHelper::LoadTextFileFromDisk(const FString& FilePath, FString& OutFileHash)
{
	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Error reading file: %s"), *FilePath);
		OutFileHash = TEXT("");
		return FString();
	}
	//Calculate hash
	FSHAHash Hash;
	FSHA1::HashBuffer(FileData.GetData(), FileData.Num(), Hash.Hash);
	OutFileHash = Hash.ToString();

	// Convert buffer to FString (handles UTF-8 / ANSI properly)
	FString FileContents;
	FFileHelper::BufferToString(FileContents, FileData.GetData(), FileData.Num());

	return FileContents;
}
UTexture2D* FAbstractMuseumFileHelper::ImportTextureAsAsset(const FString& SourceFilePath, const FString& AssetName, const FString& PackagePath)
{
	if (!FPaths::FileExists(SourceFilePath))
		return nullptr;

	const FString FullPackagePath = PackagePath + TEXT("/") + AssetName;

	// если уже существует Ч возвращаем
	if (UTexture2D* Existing = LoadObject<UTexture2D>(nullptr, *FullPackagePath))
		return Existing;

	UPackage* Package = CreatePackage(*FullPackagePath);
	if (!Package)
		return nullptr;

	Package->FullyLoad();

	UTextureFactory* Factory = NewObject<UTextureFactory>();
	Factory->AddToRoot(); // защита от GC
	Factory->SuppressImportOverwriteDialog();

	Factory->bCreateMaterial = false;
	//Factory->bCreateMaterialInstance = false;
	Factory->bEditorImport = true;

	const uint8* Buffer = nullptr;
	TArray<uint8> FileData;

	if (!FFileHelper::LoadFileToArray(FileData, *SourceFilePath))
		return nullptr;

	Buffer = FileData.GetData();

	UTexture2D* ImportedTexture = Cast<UTexture2D>(
		Factory->FactoryCreateBinary(
			UTexture2D::StaticClass(),
			Package,
			*AssetName,
			RF_Public | RF_Standalone,
			nullptr,
			*FPaths::GetExtension(SourceFilePath),
			Buffer,
			Buffer + FileData.Num(),
			GWarn
		)
	);

	if (!ImportedTexture)
		return nullptr;

	// регистраци€
	FAssetRegistryModule::AssetCreated(ImportedTexture);
	ImportedTexture->MarkPackageDirty();
	Package->MarkPackageDirty();

	const FString PackageFileName =
		FPackageName::LongPackageNameToFilename(
			FullPackagePath,
			FPackageName::GetAssetPackageExtension()
		);

	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;

	UPackage::SavePackage(
		Package,
		ImportedTexture,
		*PackageFileName,
		SaveArgs
	);

	Factory->RemoveFromRoot();

	return ImportedTexture;
}
UTexture2D* FAbstractMuseumFileHelper::SaveTextureAsAsset(UTexture2D* SourceTexture, const FString& AssetName, const FString& PackagePath)
{
	if (!SourceTexture)
		return nullptr;

	const FString FullPackagePath = PackagePath + TEXT("/") + AssetName;

	// если уже существует Ч возвращаем
	if (UTexture2D* Existing = LoadObject<UTexture2D>(nullptr, *FullPackagePath))
		return Existing;

	UPackage* Package = CreatePackage(*FullPackagePath);
	if (!Package)
		return nullptr;

	Package->FullyLoad();

	// создаЄм новую текстуру как ассет
	UTexture2D* NewTexture = NewObject<UTexture2D>(
		Package,
		*AssetName,
		RF_Public | RF_Standalone
	);

	// создаЄм платформенные данные
	FTexturePlatformData* SrcData = SourceTexture->GetPlatformData();
	if (!SrcData || SrcData->Mips.Num() == 0)
		return nullptr;

	const int32 Width = SrcData->SizeX;
	const int32 Height = SrcData->SizeY;
	const EPixelFormat Format = SrcData->PixelFormat;

	NewTexture->SetPlatformData(new FTexturePlatformData());
	FTexturePlatformData* DstData = NewTexture->GetPlatformData();

	DstData->SizeX = Width;
	DstData->SizeY = Height;
	DstData->PixelFormat = Format;

	// создаЄм mip
	FTexture2DMipMap* Mip = new FTexture2DMipMap();
	DstData->Mips.Add(Mip);

	Mip->SizeX = Width;
	Mip->SizeY = Height;

	const int64 DataSize = SrcData->Mips[0].BulkData.GetBulkDataSize();

	void* SrcPtr = SrcData->Mips[0].BulkData.Lock(LOCK_READ_ONLY);

	Mip->BulkData.Lock(LOCK_READ_WRITE);
	void* DstPtr = Mip->BulkData.Realloc(DataSize);

	FMemory::Memcpy(DstPtr, SrcPtr, DataSize);

	Mip->BulkData.Unlock();
	SrcData->Mips[0].BulkData.Unlock();

	// базовые настройки
	NewTexture->SRGB = true;
	NewTexture->CompressionSettings = TextureCompressionSettings::TC_Default;
	NewTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;

	NewTexture->UpdateResource();

	// регистрируем
	FAssetRegistryModule::AssetCreated(NewTexture);
	NewTexture->MarkPackageDirty();

	const FString PackageFileName =
		FPackageName::LongPackageNameToFilename(
			FullPackagePath,
			FPackageName::GetAssetPackageExtension()
		);

	UPackage::SavePackage(
		Package,
		NewTexture,
		RF_Public | RF_Standalone,
		*PackageFileName
	);

	return NewTexture;
}
//--Item is loaded from assets----

FString FAbstractMuseumFileHelper::CalculateFileHash(const TArray<uint8>& Data)
{
	FSHAHash Hash;
	FSHA1::HashBuffer(Data.GetData(), Data.Num(), Hash.Hash);
	return Hash.ToString();
}

bool FAbstractMuseumFileHelper::IsFileChanged(const FString& FilePath, const FString& OldHash)
{
	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("IsFileChanged: Cannot read file %s"), *FilePath);
		return true; // if no file - hash changed
	}
	FString NewHash = CalculateFileHash(FileData);
	return !(NewHash == OldHash);
}
