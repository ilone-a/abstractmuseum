#pragma once
#include "CoreMinimal.h" 
#include "UObject/NoExportTypes.h" 
#include "AbstractMuseumSettings.generated.h"

UCLASS(BlueprintType)
class ABSTRACTMUSEUM_API UAbstractMuseumSettings : public UObject {
	GENERATED_BODY()
public:
	// 
	static float LensTimerDelay;
	static float FrameOffset;
	static float CameraDistanceOffset;

	static float ProjectionOffset;
	static float OffsetFromWall;

	static float TextWorldScale;
	static float TextPlaneScaleOffset;

	static int	TextMaxLen;
	static int	StringMaxLen;

	// Load from ini
	static void LoadSettings();
	static void SaveSettings();

	static void ReloadSettings() {LoadSettings();}
};