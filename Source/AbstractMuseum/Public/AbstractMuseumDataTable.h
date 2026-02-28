#pragma once
#include "Engine/DataTable.h"
#include "CoreMinimal.h"
#include "AbstractMuseumDataTable.generated.h"

class AAbstractMuseumActor;

UENUM(BlueprintType)
enum class EAMAssetType : uint8
{
	Art  UMETA(DisplayName = "Art"),
	Text UMETA(DisplayName = "Text"),
	Item UMETA(DisplayName = "Item"),
	Error UMETA(DisplayName = "Error", Hidden)
};

USTRUCT(BlueprintType)
struct FAMDataTableRowEntry : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTable")
	FName DisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable")
	EAMAssetType AssetType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTable")
	FString AssetPath;
};