#include "SemanticGraphSubsystem.h"
#include "EngineUtils.h"
#include "AbstractMuseumActor.h"
#include "UObject/ObjectSaveContext.h"

//#include "EditorSubsystem.h"
#if WITH_EDITOR
//#include "Editor.h"
#include "UnrealEdMisc.h"
#endif

void USemanticGraphSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if WITH_EDITOR
	//FEditorDelegates::OnMapOpened.AddUObject(this, &USemanticGraphSubsystem::HandleMapOpened);
	//FEditorDelegates::PreSaveWorldWithContext.AddUObject(this, &USemanticGraphSubsystem::HandlePreSaveWorld);
	//FEditorDelegates::PostSaveWorldWithContext.AddUObject(this, &USemanticGraphSubsystem::HandlePostSaveWorld);
#endif

	if (UWorld* World = GetWorld())
	{
		World->OnWorldBeginPlay.AddUObject(this, &USemanticGraphSubsystem::OnBeginPlay);
	}
}
void USemanticGraphSubsystem::OnBeginPlay()
{
	UWorld* World = GetWorld();
	for (TActorIterator<AAbstractMuseumActor> It(World); It; ++It)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found: %s"), *It->GetName());
	}
}

void USemanticGraphSubsystem::Deinitialize()
{
	ClearGraph();
	Super::Deinitialize();
}

void USemanticGraphSubsystem::BuildGraph()
{
	AllNodes.Empty();
	UWorld* World = GetWorld();
	if (!World) return;
	for (TActorIterator<AAbstractMuseumActor> It(World); It; ++It)
	{
		AAbstractMuseumActor* Actor = *It;
		if (!Actor) continue;

		TArray<USemanticNodeComponent*> Components;
		Actor->GetComponents(Components);
		for (USemanticNodeComponent* Comp : Components)
		{
			if (Comp)
			{
				AllNodes.Add(Comp);
			}
		}
		//create connections by tags
		for (int32 i = 0; i < AllNodes.Num(); ++i)
		{
			auto* NodeA = AllNodes[i].Get();
			if (!NodeA) continue;
			for (int32 j = i + 1; j < AllNodes.Num(); ++j)
			{
				auto* NodeB = AllNodes[j].Get();
				if (!NodeB) continue;

				bool bHasCommon = false;
				for (const FName& TagA : NodeA->SemanticTags)
				{
					if (NodeB->SemanticTags.Contains(TagA))
					{
						bHasCommon = true;
						break;
					}
				}
				if (bHasCommon == true)
				{
					NodeA->ConnectedNodes.Add(NodeB);
					NodeB->ConnectedNodes.Add(NodeA);
					NodeA->ConnectedNodeIDs.Add(NodeB->NodeID);
					NodeB->ConnectedNodeIDs.Add(NodeA->NodeID);
				}
			}

		}
	}
	UE_LOG(LogTemp, Log, TEXT("Semantic graph built: %d nodes"), AllNodes.Num());
}
#if WITH_EDITOR
void USemanticGraphSubsystem::BuildGraphEd(UObject* Asset)
{
	AllNodes.Empty();

#if WITH_EDITOR
	if (!Asset)
		return;

	TArray<USemanticNodeComponent*> TempNodes;

	// 1. Get NodeComponents note
	if (AAbstractMuseumActor* Actor = Cast<AAbstractMuseumActor>(Asset))
	{
		Actor->GetComponents(TempNodes);
	}
	else if (UBlueprint* BP = Cast<UBlueprint>(Asset))
	{
		if (BP->GeneratedClass)
		{
			// get CDO, to get component
			UObject* CDO = BP->GeneratedClass->GetDefaultObject();
			if (AAbstractMuseumActor* ActorCDO = Cast<AAbstractMuseumActor>(CDO))
			{
				ActorCDO->GetComponents(TempNodes);
			}
		}
	}

	// 2. Copy to WeakPtr array
	for (USemanticNodeComponent* Comp : TempNodes)
	{
		if (Comp)
		{
			AllNodes.Add(Comp);
		}
	}

	// 3. Find all connected components to SemanticNodeComponent
	for (int32 i = 0; i < AllNodes.Num(); ++i)
	{
		USemanticNodeComponent* NodeA = AllNodes[i].Get();
		if (!NodeA) continue;

		for (int32 j = i + 1; j < AllNodes.Num(); ++j)
		{
			USemanticNodeComponent* NodeB = AllNodes[j].Get();
			if (!NodeB) continue;

			bool bHasCommon = false;
			for (const FName& TagA : NodeA->SemanticTags)
			{
				if (NodeB->SemanticTags.Contains(TagA))
				{
					bHasCommon = true;
					break;
				}
			}

			if (bHasCommon)
			{
				NodeA->ConnectedNodes.Add(NodeB);
				NodeB->ConnectedNodes.Add(NodeA);
				NodeA->ConnectedNodeIDs.Add(NodeB->NodeID);
				NodeB->ConnectedNodeIDs.Add(NodeA->NodeID);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("BuildGraphEd: built semantic graph from asset %s, total %d nodes"),
		*Asset->GetName(), AllNodes.Num());
#endif
}
#endif
void USemanticGraphSubsystem::ClearGraph()
{
	AllNodes.Empty();
}
#if WITH_EDITOR
void USemanticGraphSubsystem::HandleMapOpened(const FString& Filename, bool bAsTemplate)
{
	UE_LOG(LogTemp, Log, TEXT("Map opened: %s"), *Filename);
	BuildGraph();
}

void USemanticGraphSubsystem::HandlePreSaveWorld(UWorld* World, FObjectPreSaveContext Context)
{
	ClearGraph();
}

void USemanticGraphSubsystem::HandlePostSaveWorld(UWorld* World, FObjectPostSaveContext Context)
{

	BuildGraph();
}

#endif