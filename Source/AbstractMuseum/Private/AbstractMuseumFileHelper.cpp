#include "AbstractMuseumFileHelper.h"
#include "Misc/FileHelper.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"

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
