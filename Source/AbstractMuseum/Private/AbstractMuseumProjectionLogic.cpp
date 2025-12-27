// Fill out your copyright notice in the Description page of Project Settings.


#include "AbstractMuseumProjectionLogic.h"
#include "AbstractMuseumSettings.h"
#include "AbstractMuseumArt.h"
#include "AbstractMuseumItem.h"
#include "AbstractMuseumText.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "EngineUtils.h"

#if WITH_EDITOR
#include "Editor.h"
#include "Selection.h"
#include "Editor/UnrealEdEngine.h"
#include "Editor/EditorEngine.h"
#include "EditorModeManager.h"

#endif

AAbstractMuseumProjectionLogic::AAbstractMuseumProjectionLogic()
{
#if WITH_EDITOR
	bEditorHookRegistered = false;
#endif
	PrimaryActorTick.bCanEverTick = true;
}


AAbstractMuseumProjectionLogic* AAbstractMuseumProjectionLogic::Get(UWorld* World)
{
	if (!World) return nullptr;
	for (TActorIterator<AAbstractMuseumProjectionLogic> It(World); It; ++It) {
		return *It;
	}
	return nullptr;
}

void AAbstractMuseumProjectionLogic::CollectAbstractMuseumActors()
{
	AMActors.Empty();

	TArray<AActor*> MActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAbstractMuseumActor::StaticClass(), MActors);

	for (AActor* Actor : MActors)
	{
		if (AAbstractMuseumActor* Art = Cast<AAbstractMuseumActor>(Actor))
		{
			AMActors.Add(Art);
		}
	}
#if WITH_EDITOR
	RegisterEditorSelect();
#endif
}

#if WITH_EDITOR

void AAbstractMuseumProjectionLogic::RegisterEditorSelect()
{
	if (GEditor)
	{
		bEditorHookRegistered = true;
		GEditor->GetSelectedActors()->SelectObjectEvent.AddUObject(this, &AAbstractMuseumProjectionLogic::OnEditorSelectionChanged);
	}
}


//---Main walls projection logic---
void AAbstractMuseumProjectionLogic::OnEditorSelectionChanged(UObject* NewSelection)
{
	if (!NewSelection) return;
	if (AAbstractMuseumActor* SActor = Cast<AAbstractMuseumActor>(NewSelection))
	{

		if ((SActor) && (SActor != SelectedActor))
		{
			//---Only for museum actors with projection enabled---
			if (!SActor->bEnableProjection) return;
			SActor->UpdateLinetrace();
			SelectedActor = SActor;
		}
	}

	{
		//---Select walls with active SelectedActor child class---
		FHitResult Hit;
		FCollisionQueryParams Params;
		if (SelectedActor)
		{
			Params.AddIgnoredActor(SelectedActor);
			Params.bTraceComplex = true;

			UWorld* World = GetWorld();
			if (!World) return;

			const FVector& Start = SelectedActor->EditorProjection.TraceStart;
			const FVector& End = SelectedActor->EditorProjection.TraceEnd;

			if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
			{
				if (Hit.GetComponent() == NewSelection || Hit.GetActor() == Cast<AActor>(NewSelection))
				{
					//--Create projection. Move actor to hit point, null selected actor
					FVector Offset = Hit.Normal * UAbstractMuseumSettings::OffsetFromWall; // 2 offset from wall 
					SelectedActor->SetActorLocation(Hit.Location + Offset);

					//FVector Offset2 = Hit.Normal * OffsetFromWall;
					//SelectedActor->SetActorLocation(Hit.Location + Offset2);

					//---Get wall normal and forward of actor---
					FVector WallNormal = Hit.Normal.GetSafeNormal();
					FVector ArtForward = SelectedActor->GetActorForwardVector().GetSafeNormal();

					//---Calculate rotation angle---
					float Dot = FVector::DotProduct(ArtForward, WallNormal);
					if (!FMath::IsNearlyZero(Dot, 0.01f))
					{
						//---Fix forward: projection of current forward to wall
						FVector DesiredForward = (ArtForward - Dot * WallNormal).GetSafeNormal();
						//---Do quat rotate to old from new state 
						FQuat DeltaQuat = FQuat::FindBetweenNormals(ArtForward, DesiredForward);
						SelectedActor->AddActorWorldRotation(DeltaQuat);
					}
					SelectedActor->EditorProjection = {};
					SelectedActor->UpdateProjectionDecal();

					//Undo selection
					GEditor->SelectNone(false, true);
					SelectedActor = nullptr;
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Clicked object is not under projection hit."));
				}
			}
		}
	}
}

#endif

void AAbstractMuseumProjectionLogic::BeginPlay()
{
	Super::BeginPlay();
	CollectAbstractMuseumActors();
}


void AAbstractMuseumProjectionLogic::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

#if WITH_EDITOR
	if (bEditorHookRegistered && GEditor && GEditor->GetSelectedActors())
	{
		GEditor->GetSelectedActors()->SelectObjectEvent.RemoveAll(this);
		bEditorHookRegistered = false;
	}
#endif
}

void AAbstractMuseumProjectionLogic::PostLoad()
{
	Super::PostLoad();
	CollectAbstractMuseumActors();
}
