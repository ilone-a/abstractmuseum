#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SemanticNodeComponent.h"
#include "SemanticGraphSubsystem.generated.h"

UCLASS()
class ABSTRACTMUSEUM_API USemanticGraphSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void BuildGraph();
#if WITH_EDITOR
	void BuildGraphEd(UObject* Asset);
#endif
	void ClearGraph();

	TArray<TWeakObjectPtr<USemanticNodeComponent>> GetAllNodes() { return AllNodes; };
private:
	void OnBeginPlay();
#if WITH_EDITOR
	void HandleMapOpened(const FString& Filename, bool bAsTemplate);
	void HandlePreSaveWorld(UWorld* World, FObjectPreSaveContext Context);
	void HandlePostSaveWorld(UWorld* World, FObjectPostSaveContext Context);
#endif
	UPROPERTY()
	TArray<TWeakObjectPtr<USemanticNodeComponent>> AllNodes;
};