#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SemanticNodeComponent.generated.h"
//

// data storage of the virtual museum art instance.


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ABSTRACTMUSEUM_API UAMInfoComponent : public UActorComponent {
	GENERATED_BODY()
public:
	UAMInfoComponent();

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	FName InventoryID;
	UPROPERTY(EditAnywhere, Category = "Default")
	FName Title;
	//enum ObjectType (enum: Painting, Sculpture, Object, etc.)
	UPROPERTY(EditAnywhere, Category = "Default")
	int Date;
	UPROPERTY(EditAnywhere, Category = "Default")
	FString Note;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ABSTRACTMUSEUM_API UAMShowComponent : public UActorComponent {
	GENERATED_BODY()
public:
	UAMShowComponent() {};

	UPROPERTY(BlueprintReadOnly, Category = "Show")
	FTransform ActorInGalleryTransform;
	//todo: create entity for the room in galery
	//not a real fstring, must be pointer
	UPROPERTY(EditAnywhere, Category = "Show")
	FString LinkToGallery;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ABSTRACTMUSEUM_API UAMStorageComponent : public UActorComponent {
	GENERATED_BODY()
public:
	UAMStorageComponent() {};

	UPROPERTY(BlueprintReadOnly, Category = "Show")
	FTransform ActorInGalleryTransform;
	//todo: create entity for the room in galery
	//not a real fstring, must be pointer
	UPROPERTY(EditAnywhere, Category = "Show")
	FString LinkToGallery;
};