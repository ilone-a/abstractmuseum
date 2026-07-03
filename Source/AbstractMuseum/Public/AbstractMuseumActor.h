#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "ArtInteractInterface.h"
#include "InfoComponent.h"
#include "AbstractMuseumActor.generated.h"



UCLASS(Abstract)
class ABSTRACTMUSEUM_API AAbstractMuseumActor : public AActor, public IAMInteractInterface
{
	GENERATED_BODY()
public:
	AAbstractMuseumActor();
	//--Interface
	virtual void AMOnInteract_Implementation() override;
	virtual void AMOnFocus_Implementation() override;
	virtual void AMBeginInspect_Implementation() override;
	virtual void AMEndInspect_Implementation() override;
	virtual void AMInspectLook_Implementation(const FVector2D& Delta) override;
	virtual bool AMIsInspecting_Implementation() override;

	bool bIsInspecting = false;
	bool bIsInteracted = false; //sort of state machine
	void HandleCreateWidget();
	void RestartHideTimer();

	virtual void ScaleMeshes();

	virtual void LockCameraToThing();
	virtual void UnlockCameraFromThing();

	UPROPERTY(EditAnywhere, Category = "Path")
	FString LocalFilePath;

	//non-overridable
	void SetHash(const FString& SourceFileHash);
	FString& GetHash() { return SourceFileHash; }
	const FString& GetHash() const { return SourceFileHash; }

	virtual void CalculateCameraPositionEditor();

protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<USceneComponent> Origin;

	//info
	UPROPERTY(EditAnywhere, Category = "Info")
	FAMInfoComponent InfoComponent;

	// camera zoom logic
	UPROPERTY(VisibleAnywhere, Category = "Default")
	UCameraComponent* AMCamera;
	FVector DefaultLocation;
	FRotator DefaultRotation;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	// Cursors: Main cursor class for all child classes
	// ActiveCursorWidget - Magnifying glass widget
	UPROPERTY()
	TSubclassOf<UUserWidget> CursorWidgetClass;
	UPROPERTY()
	UUserWidget* ActiveCursorWidget = nullptr;
	UPROPERTY()
	bool bWidgetCreated = false;
	FTimerHandle HideWidgetTimer;
	void ShowCursorWidget();
	void HideCursorWidget();

private:
	UPROPERTY()
	FString SourceFileHash;
};


