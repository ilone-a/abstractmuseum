#pragma once

#include "AbstractMuseumActor.h"
#include "CoreMinimal.h"
#include "Components/DecalComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/DataAsset.h"
#include "AbstractMuseumArt.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UMaterialInstance;
class UMaterialInstanceConstant;

UCLASS()
class ABSTRACTMUSEUM_API AAbstractMuseumArt : public AAbstractMuseumActor
{
	GENERATED_BODY()

public:

	AAbstractMuseumArt();

	//---Material asset and base---
	UPROPERTY(EditAnywhere, Category = "Texture Loader")
	UMaterialInterface* BaseMaterial;
	UPROPERTY(EditAnywhere, Category = "Material")
	UMaterialInstanceConstant* ArtMaterialAsset;
	void ApplyTexture();
	virtual void ScaleMeshes() override;// Scale all meshes to art picture size

	//---Projection logic---
	virtual void UpdateLinetrace() override;
	bool bIsAttached = false;


	//--Camera--
	virtual void CalculateCameraPositionEditor() override;
	bool bCameraLocked = false;
	virtual void LockCameraToThing() override;
	virtual void UnlockCameraFromThing() override;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UStaticMeshComponent> Plane;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	//TObjectPtr<UDecalComponent> ProjectionDecal;
	APlayerController* PC = nullptr;
	APawn* PlayerPawn = nullptr;

	//--Camera parameters--
	AActor* OriginalViewTarget = nullptr;
	UPROPERTY(EditAnywhere, Category = "Camera")
	FVector SavedCameraLocation;
	UPROPERTY(EditAnywhere, Category = "Camera")
	FRotator SavedCameraRotation;

#if WITH_EDITOR
	virtual void PostEditMove(bool bFinished) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

};
