#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SemanticNodeComponent.generated.h"
//

// Semantic Node class: data storage of the virtual museum art instance.
// Stores unique id, year, tags and ids connected nodes, 
// calculates an array of connected nodes.

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ABSTRACTMUSEUM_API USemanticNodeComponent : public UActorComponent {
	GENERATED_BODY()
public:
	USemanticNodeComponent();
	UPROPERTY(EditAnywhere, Category = "Semantic")
	FName NodeID;

	UPROPERTY(EditAnywhere, Category = "Semantic")
	int Year;

	UPROPERTY(EditAnywhere, Category = "Semantic")
	TArray<FName>SemanticTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Semantic")
	TArray<FName>ConnectedNodeIDs;
	// runtime-data
	TArray<TWeakObjectPtr<USemanticNodeComponent>> ConnectedNodes;

};