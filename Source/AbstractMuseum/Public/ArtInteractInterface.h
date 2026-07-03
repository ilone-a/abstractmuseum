#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ArtInteractInterface.generated.h"

UINTERFACE(MinimalAPI)
class UAMInteractInterface : public UInterface
{
	GENERATED_BODY()
};

class IAMInteractInterface
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Art)
	void AMOnInteract();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Art)
	void AMOnFocus();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Art)
	void AMBeginInspect();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Art)
	void AMEndInspect();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Art)
	void AMInspectLook(const FVector2D& Delta);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Art)
	bool AMIsInspecting();
};


UINTERFACE(MinimalAPI)
class UEnvInteractInterface : public UInterface
{
	GENERATED_BODY()
};

class IEnvInteractInterface
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Environment)
	void EnvOnInteract();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Environment)
	void EnvOnFocus();
};