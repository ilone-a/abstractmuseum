#pragma once
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbstractMuseumActor.h"
#include "AbstractMuseumItem.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UBoxComponent;

UCLASS()
class ABSTRACTMUSEUM_API AAbstractMuseumItem : public AAbstractMuseumActor
{
	GENERATED_BODY()

public:
	AAbstractMuseumItem();

	UPROPERTY(Transient)
	FString MeshAssetPath;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(EditAnywhere, Category = "Default")
	TSoftObjectPtr<UStaticMesh> MeshAsset;
protected:

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UBoxComponent> BoundingBox;
	void UpdateBoundingBox();

	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
#if WITH_EDITOR
	virtual void PostEditMove(bool bFinished) override;

#endif

};
