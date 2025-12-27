// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"

#include "AbstractMuseumProjectionLogic.generated.h"
class AAbstractMuseumActor;
class AAbstractMuseumArt;
class AAbstractMuseumText;
class AAbstractMuseumItem;
/**
 * 
 */
UCLASS()
class ABSTRACTMUSEUM_API AAbstractMuseumProjectionLogic : public AInfo
{
	GENERATED_BODY()

public:
	AAbstractMuseumProjectionLogic();
	static AAbstractMuseumProjectionLogic* Get(UWorld* World);
	AAbstractMuseumActor* SelectedActor = nullptr;

	void OnEditorSelectionChanged(UObject* SelectedObject);
	AAbstractMuseumActor* Hovered = nullptr;

protected:
	virtual void BeginPlay() override;
	virtual void PostLoad() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
#if WITH_EDITOR
	bool bEditorHookRegistered = false;

#endif
	

private:
	//Abstract Museum actor child array
	UPROPERTY() TArray <AAbstractMuseumActor*> AMActors;
	void CollectAbstractMuseumActors();

#if WITH_EDITOR
	void RegisterEditorSelect();
#endif
};
