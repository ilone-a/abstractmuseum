#include "../Public/AbstractMuseumItem.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Interfaces/IPluginManager.h"
#include "CoreGlobals.h"
#include "Misc/Paths.h"
#include "Misc/ConfigCacheIni.h"

static UStaticMesh* GDefaultItemCubeMesh = nullptr;

static UStaticMesh* LoadDefaultItemMesh()
{
	if (GDefaultItemCubeMesh)
		return GDefaultItemCubeMesh;

	FString MeshPath;
	const FString ConfigPath = FPaths::Combine(
		IPluginManager::Get().FindPlugin(TEXT("AbstractMuseum"))->GetBaseDir(),
		TEXT("Config/AbstractMuseumPaths.ini")
	);
	if (GConfig)
	{
		GConfig->LoadFile(ConfigPath);
		GConfig->GetString(
			TEXT("AssetPaths"),
			TEXT("ItemCubeMesh"),
			MeshPath,
			ConfigPath
		);
	}
	if (!MeshPath.IsEmpty())
	{
		GDefaultItemCubeMesh = Cast<UStaticMesh>(
			StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *MeshPath)
		);
	}
	return GDefaultItemCubeMesh;
}

AAbstractMuseumItem::AAbstractMuseumItem()
{
	PrimaryActorTick.bCanEverTick = false;

	Origin = CreateDefaultSubobject<USceneComponent>("Origin");
	check(Origin);
	SetRootComponent(Origin);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	check(StaticMesh);
	StaticMesh->SetupAttachment(Origin);
	//camera
	AMCamera = CreateDefaultSubobject<UCameraComponent>("AMCamera");
	AMCamera->SetupAttachment(RootComponent);
	if (AMCamera) { AMCamera->SetupAttachment(Origin); }
}

void AAbstractMuseumItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UStaticMesh* MeshToUse = nullptr;
	if (!MeshAsset.IsNull())
	{
		MeshToUse = MeshAsset.LoadSynchronous();
	}
	else
	{
		MeshToUse = LoadDefaultItemMesh();
	}
	if (MeshToUse && StaticMesh)
	{
		StaticMesh->SetStaticMesh(MeshToUse);
	}
}

void AAbstractMuseumItem::BeginPlay()
{
	Super::BeginPlay();
	PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PlayerPawn = PC->GetPawn();
		OriginalViewTarget = PC->GetViewTarget();
	}
}

void AAbstractMuseumItem::LockCameraToThing()
{
	if (!PC) return;
	ShowCursorWidget();
	//TODO what is target camera
	OriginalViewTarget = PC->GetViewTarget();

	//Art has Camera, Actor + Component
	PC->SetViewTargetWithBlend(this, 0.5f);

	// Blocking player movement to next click
	if (auto* Char = Cast<ACharacter>(PlayerPawn))
	{
		Char->GetCharacterMovement()->DisableMovement();
		bCameraLocked = true;
		UE_LOG(LogTemp, Warning, TEXT("LockCamera_Child"));
	}
}

void AAbstractMuseumItem::UnlockCameraFromThing()
{

	if (!PC) return;
	// Return camera to player
	if (OriginalViewTarget)
		PC->SetViewTargetWithBlend(OriginalViewTarget, 0.5f);

	// Unblock movement
	if (auto* Char = Cast<ACharacter>(PlayerPawn))
	{
		Char->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		bCameraLocked = false;
		UE_LOG(LogTemp, Warning, TEXT("unLockCamera_Item"));
	}
}

#if WITH_EDITOR
void AAbstractMuseumItem::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);
}


void AAbstractMuseumItem::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	static const FName MeshAssetName = GET_MEMBER_NAME_CHECKED(AAbstractMuseumItem, MeshAsset);
	if (PropertyChangedEvent.GetPropertyName() == MeshAssetName)
	{
		if (!MeshAsset.IsNull())
		{
			UStaticMesh* Mesh = MeshAsset.LoadSynchronous();
			if (Mesh)
				StaticMesh->SetStaticMesh(Mesh);
		}
	}
}

#endif