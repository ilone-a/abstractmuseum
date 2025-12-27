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


UCLASS(BlueprintType)
class ABSTRACTMUSEUM_API UArtMaterialStruct : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	UMaterialInterface* BaseMaterial;
};

UCLASS()
class ABSTRACTMUSEUM_API AAbstractMuseumArt : public AAbstractMuseumActor
{
	GENERATED_BODY()

public:

	AAbstractMuseumArt();
	//---Material---
	UPROPERTY(EditAnywhere, Category = "Texture Loader")
	UArtMaterialStruct* ArtMaterialStruct;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* ArtMaterial;

	UPROPERTY()
	UTexture2D* LoadedTexture;
	void CreateDynamicMaterial();
	void ApplyTexture();
	virtual void ScaleMeshes() override;// Scale all meshes to art picture size

	//---Projection logic---
	virtual void UpdateLinetrace() override;
	bool bIsAttached = false;

	//---Frame---
	void UpdateFrame();
	UPROPERTY(EditAnywhere, Category = "Frame")
	bool bIsFrameVisible;// = true;
	TObjectPtr<UStaticMeshComponent> Frame;
	float FrameDepth = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Frame")
	float FrameBorder = 0.05f;
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* FrameMaterial;
	UPROPERTY(EditAnywhere, Category = "Frame")
	UMaterialInterface* FrameBaseMaterial;
	void CreateDynamicFrameMaterial();
	void SetFrameVisible(bool bVisible);

	//--Camera--
	virtual void CalculateCameraPositionEditor() override;
	bool bCameraLocked = false;
	virtual void LockCameraToThing() override;
	virtual void UnlockCameraFromThing() override;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UStaticMeshComponent> Plane;

	TObjectPtr<UDecalComponent> ProjectionDecal;
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
