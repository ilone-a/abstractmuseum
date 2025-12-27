#include "../Public/AbstractMuseumItem.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
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

	BoundingBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BBox"));
	check(BoundingBox);
	BoundingBox->SetupAttachment(Origin);

}


void AAbstractMuseumItem::BeginPlay()
{
}

void AAbstractMuseumItem::UpdateBoundingBox()
{
	if (!StaticMesh || !StaticMesh->GetStaticMesh())
		return;

	const FBoxSphereBounds Bounds = StaticMesh->GetStaticMesh()->GetBounds();

	BoundingBox->SetBoxExtent(Bounds.BoxExtent);
	BoundingBox->SetRelativeLocation(Bounds.Origin);
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
		UpdateBoundingBox();
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