#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ArtInteractInterface.generated.h"

UINTERFACE(MinimalAPI)
class UArtInteractInterface : public UInterface
{
	GENERATED_BODY()
};


class IArtInteractInterface
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Art)
	void ArtOnInteract();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Art)
	void ArtOnFocus();
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