#pragma once
#include "Engine/DataTable.h"
#include "CoreMinimal.h"
#include "AbstractMuseumDataTable.generated.h"

class AAbstractMuseumActor;

USTRUCT(BlueprintType)
struct FAMDataTableRowEntry : public FTableRowBase
{
	GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadOnly)
FName DisplayName;

UPROPERTY(EditAnywhere, BlueprintReadOnly)
TSubclassOf<AAbstractMuseumActor> ActorClass;

UPROPERTY(EditAnywhere, BlueprintReadOnly)
FString AssetPath;
};