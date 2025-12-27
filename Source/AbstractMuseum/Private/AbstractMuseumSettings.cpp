#include "AbstractMuseumSettings.h" 
#include "Interfaces/IPluginManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"

float UAbstractMuseumSettings::LensTimerDelay = 0.1f;
float UAbstractMuseumSettings::FrameOffset = 1.5f;
float UAbstractMuseumSettings::CameraDistanceOffset = 55.f;
float UAbstractMuseumSettings::ProjectionOffset = 1000.f;
float UAbstractMuseumSettings::OffsetFromWall = 2.f;

float UAbstractMuseumSettings::TextWorldScale = 10.f;
float UAbstractMuseumSettings::TextPlaneScaleOffset = 5.0f;

int UAbstractMuseumSettings::TextMaxLen = 280;
int UAbstractMuseumSettings::StringMaxLen = 60;


void UAbstractMuseumSettings::LoadSettings() {

	const FString ConfigPath = FPaths::Combine(
		IPluginManager::Get().FindPlugin(TEXT("AbstractMuseum"))->GetBaseDir(),
		TEXT("Config/AbstractMuseumVariables.ini")
	);

	if (!GConfig)
		return;

	GConfig->LoadFile(ConfigPath);
	GConfig->GetFloat(TEXT("Default Variables"), TEXT("LensTimerDelay"), LensTimerDelay, ConfigPath);
	GConfig->GetFloat(TEXT("Default Variables"), TEXT("FrameOffset"), FrameOffset, ConfigPath);
	GConfig->GetFloat(TEXT("Default Variables"), TEXT("CameraDistanceOffset"), CameraDistanceOffset, ConfigPath);
	GConfig->GetFloat(TEXT("Default Variables"), TEXT("ProjectionOffset"), ProjectionOffset, ConfigPath);
	GConfig->GetFloat(TEXT("Default Variables"), TEXT("OffsetFromWall"), OffsetFromWall, ConfigPath);
	GConfig->GetFloat(TEXT("Default Variables"), TEXT("TextWorldScale"), TextWorldScale, ConfigPath);
	GConfig->GetFloat(TEXT("Default Variables"), TEXT("TextPlaneScaleOffset"), TextPlaneScaleOffset, ConfigPath);
	GConfig->GetInt(TEXT("Default Variables"), TEXT("TextMaxLen"), TextMaxLen, ConfigPath);
	GConfig->GetInt(TEXT("Default Variables"), TEXT("StringMaxLen"), StringMaxLen, ConfigPath);

}

void UAbstractMuseumSettings::SaveSettings() {

	const FString ConfigPath = FPaths::Combine(
		IPluginManager::Get().FindPlugin(TEXT("AbstractMuseum"))->GetBaseDir(),
		TEXT("Config/AbstractMuseumVariables.ini")
	);

	if (!GConfig)
		return;

	GConfig->LoadFile(ConfigPath);
	GConfig->SetFloat(TEXT("Default Variables"), TEXT("LensTimerDelay"), LensTimerDelay, ConfigPath);
	GConfig->SetFloat(TEXT("Default Variables"), TEXT("FrameOffset"), FrameOffset, ConfigPath);
	GConfig->SetFloat(TEXT("Default Variables"), TEXT("CameraDistanceOffset"), CameraDistanceOffset, ConfigPath);
	GConfig->SetFloat(TEXT("Default Variables"), TEXT("ProjectionOffset"), ProjectionOffset, ConfigPath);
	GConfig->SetFloat(TEXT("Default Variables"), TEXT("OffsetFromWall"), OffsetFromWall, ConfigPath);
	GConfig->SetFloat(TEXT("Default Variables"), TEXT("TextWorldScale"), TextWorldScale, ConfigPath);
	GConfig->SetFloat(TEXT("Default Variables"), TEXT("TextPlaneScaleOffset"), TextPlaneScaleOffset, ConfigPath);
	GConfig->SetInt(TEXT("Default Variables"), TEXT("TextMaxLen"), TextMaxLen, ConfigPath);
	GConfig->SetInt(TEXT("Default Variables"), TEXT("StringMaxLen"), StringMaxLen, ConfigPath);

}