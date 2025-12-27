// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbstractMuseum.h"
#include "Misc/Paths.h"
#include "Misc/ConfigCacheIni.h"
#include "Interfaces/IPluginManager.h"
#include "AbstractMuseumSettings.h"

#define LOCTEXT_NAMESPACE "FAbstractMuseumModule"

void FAbstractMuseumModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	const FString ConfigPath = FPaths::Combine(
		IPluginManager::Get().FindPlugin(TEXT("AbstractMuseum"))->GetBaseDir(),
		TEXT("Config/AbstractMuseumPaths.ini")
	);
	GConfig->LoadFile(ConfigPath);
	UAbstractMuseumSettings::LoadSettings();
}

void FAbstractMuseumModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAbstractMuseumModule, AbstractMuseum)