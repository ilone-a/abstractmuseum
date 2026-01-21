#pragma once
#include "Engine/DataTable.h"
#include "CoreMinimal.h"
#include "AbstractMuseumDataTable.generated.h"

class AAbstractMuseumActor;

USTRUCT(BlueprintType)
struct FAMDataTableRowEntry : public FTableRowBase
{
	GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTable")
FName DisplayName;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTable")
TSubclassOf<AAbstractMuseumActor> ActorClass;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTable")
FString AssetPath;
};