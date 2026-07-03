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

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

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
	PrimaryActorTick.bCanEverTick = true;

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



void AAbstractMuseumItem::LockCameraToThing()
{
	if (!PC) return;

	ShowCursorWidget();
	OriginalViewTarget = PC->GetViewTarget();

	PC->SetViewTargetWithBlend(this, 0.5f);
	StartOrbit();
	if (ACharacter* Char = Cast<ACharacter>(PlayerPawn))
	{
		Char->GetCharacterMovement()->DisableMovement();
	}
}


void AAbstractMuseumItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bOrbiting || !AMCamera || !StaticMesh)
		return;

	const FRotator Rot(OrbitPitch, OrbitYaw, 0.f);
	const FVector Center = StaticMesh->GetComponentLocation();
	const FVector CamPos =
		Center + Rot.Vector() * OrbitDistance;

	AMCamera->SetWorldLocation(CamPos);
	AMCamera->SetWorldRotation((Center - CamPos).Rotation());
}





bool AAbstractMuseumItem::IsOrbiting() const
{
	return bOrbiting;
}
void AAbstractMuseumItem::StartOrbit() {bOrbiting = true;
const FVector Center = StaticMesh->GetComponentLocation();

const FVector Offset = AMCamera->GetComponentLocation() - Center;

OrbitDistance = Offset.Length();

const FRotator Rot = Offset.Rotation();

OrbitYaw = Rot.Yaw;
OrbitPitch = Rot.Pitch;

}
void AAbstractMuseumItem::StopOrbit() {bOrbiting = false;}

void AAbstractMuseumItem::UnlockCameraFromThing()
{
	if (!PC) return;
	StopOrbit();
	if (OriginalViewTarget)
	{
		PC->SetViewTargetWithBlend(OriginalViewTarget, 0.5f);
	}
	if (ACharacter* Char = Cast<ACharacter>(PlayerPawn))
	{
		Char->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

}
void AAbstractMuseumItem::AMInspectLook_Implementation(const FVector2D& Delta)
{
	if (!bOrbiting) return;

	OrbitYaw += Delta.X;

	OrbitPitch = FMath::Clamp(
		OrbitPitch + Delta.Y,
		-80.f,
		80.f
	);
}
