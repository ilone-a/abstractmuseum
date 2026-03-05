#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InfoComponent.generated.h"

// Data storage of the virtual museum actor instance


USTRUCT(BlueprintType)
struct FAMInfoComponent {
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FName InventoryID;
	UPROPERTY(EditAnywhere, Category = "Default")
	FName Title;

		// object description
	UPROPERTY(EditAnywhere, Category = "Additional")
	FString Description;
	UPROPERTY(EditAnywhere, Category = "Additional")
	bool bShowDescription = true;
	UPROPERTY(EditAnywhere, Category = "Additional")
	int Date;
	// where from
	UPROPERTY(EditAnywhere, Category = "Additional")
	FString Source;

};
