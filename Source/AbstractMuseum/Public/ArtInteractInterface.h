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
	void ArtOnInteract() ;
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Art)
	 void ArtOnFocus();
};