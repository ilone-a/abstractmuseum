#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "ArtInteractInterface.h"
#include "AbstractMuseumActor.generated.h"

// Other base structs
USTRUCT(BlueprintType)
struct FLinetraceProjectionData
{
	GENERATED_BODY()

	FVector TraceStart;
	FVector TraceEnd;
	FVector HitLocation;

	bool bHit = false;

	float Distance = 0.0f;
	FLinetraceProjectionData()
		: TraceStart(FVector::ZeroVector)
		, TraceEnd(FVector::ZeroVector)
		, HitLocation(FVector::ZeroVector)
		, bHit(false)
		, Distance(0.0f)
	{
	};
};

UCLASS(Abstract)
class ABSTRACTMUSEUM_API AAbstractMuseumActor : public AActor, public IArtInteractInterface
{
	GENERATED_BODY()
public:
	AAbstractMuseumActor();

	//--Interface
	virtual void ArtOnInteract_Implementation() override;
	virtual void ArtOnFocus_Implementation() override;
	bool bOutline = false;
	bool bIsInteracted = false; //sort of state machine
	void HandleCreateWidget();
	void RestartHideTimer();

	virtual void UpdateLinetrace();
	virtual void UpdateProjectionDecal();
	virtual void ScaleMeshes();

	virtual void LockCameraToThing();
	virtual void UnlockCameraFromThing();

	UPROPERTY(EditAnywhere, Category = "Path")
	FString LocalFilePath;

	//non-overridable
	void SetHash(const FString& FileHash);
	FString& GetHash() { return FileHash; }
	const FString& GetHash() const { return FileHash; }

	FLinetraceProjectionData EditorProjection;
	UPROPERTY(EditAnywhere, Category = "Projection")
	bool bEnableProjection = false;
	virtual void CalculateCameraPositionEditor();

protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<USceneComponent> Origin;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UDecalComponent> ProjectionDecal;

// camera zoom logic
	UPROPERTY(VisibleAnywhere, Category = "Default")
	UCameraComponent* AMCamera;

	FVector DefaultLocation;
	FRotator DefaultRotation;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	// Main cursor class for all child classes
	UPROPERTY()
	TSubclassOf<UUserWidget> CursorWidgetClass;

	// ActiveCursorWidget - Magnifying glass widget
	UPROPERTY()
	UUserWidget* ActiveCursorWidget = nullptr;
	UPROPERTY()
	bool bWidgetCreated = false;

	FTimerHandle HideWidgetTimer;
	void ShowCursorWidget();
	void HideCursorWidget();

private:
	FString FileHash;

};


