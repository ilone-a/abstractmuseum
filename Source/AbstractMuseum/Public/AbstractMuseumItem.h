#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbstractMuseumActor.h"
#include "InputActionValue.h"
#include "AbstractMuseumItem.generated.h"

class USceneComponent;
class UStaticMeshComponent;


class UInputAction;
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


	//--Camera--

	bool bCameraLocked = false;


	void AMInspectLook_Implementation(const FVector2D& Delta);

	virtual void LockCameraToThing() override;
	bool IsOrbiting() const;
	void StartOrbit();
	void StopOrbit();

	virtual void UnlockCameraFromThing() override;


	virtual void Tick(float DeltaTime) override;
	//void AddLook(const FVector2D& Axis);
	//--Camera parameters--
	AActor* OriginalViewTarget = nullptr;

	bool bOrbiting = false;

	float OrbitYaw = 0.f;
	float OrbitPitch = 0.f;

	float OrbitDistance = 150.f;

protected:
	virtual void BeginPlay() override;
	void UpdateBoundingBox();
	virtual void OnConstruction(const FTransform& Transform) override;
	APlayerController* PC = nullptr;
	APawn* PlayerPawn = nullptr;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
#if WITH_EDITOR
	virtual void PostEditMove(bool bFinished) override;

#endif

};
