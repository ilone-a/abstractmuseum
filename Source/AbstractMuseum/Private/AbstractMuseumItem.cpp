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
#include "AbstractMuseumCharacter.h"
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
	//TODO what is target camera
	OriginalViewTarget = PC->GetViewTarget();
	PC->SetViewTargetWithBlend(this, 0.5f);

	if (AAbstractMuseumCharacter* Char = Cast<AAbstractMuseumCharacter>(PlayerPawn))
	{
		Char->CurrentItem = this;
	}

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

	const FVector Center = StaticMesh->GetComponentLocation();

	FRotator Rot(OrbitPitch, OrbitYaw, 0.f);

	const FVector Offset = Rot.Vector() * OrbitDistance;

	const FVector CamPos = Center - Offset;

	AMCamera->SetWorldLocation(CamPos);
	AMCamera->SetWorldRotation((Center - CamPos).Rotation());

}

void AAbstractMuseumItem::AddLook(const FVector2D& Axis)
{
	if (!bOrbiting) return;

	OrbitYaw += Axis.X;

	OrbitPitch = FMath::Clamp(
		OrbitPitch + Axis.Y,
		-80.f,
		80.f
	);
}



bool AAbstractMuseumItem::IsOrbiting() const
{
	return bOrbiting;
}
void AAbstractMuseumItem::StartOrbit()
{
	bOrbiting = true;
}

void AAbstractMuseumItem::StopOrbit()
{
	bOrbiting = false;
}
void AAbstractMuseumItem::Look(const FInputActionValue& Value)
{
	if (!bOrbiting) return;

	FVector2D Axis = Value.Get<FVector2D>();

	OrbitYaw += Axis.X;
	OrbitPitch = FMath::Clamp(
		OrbitPitch + Axis.Y,
		-80.f,
		80.f
	);
}
void AAbstractMuseumItem::UnlockCameraFromThing()
{
	if (!PC) return;
	StopOrbit();

	if (auto Char = Cast<AAbstractMuseumCharacter>(PlayerPawn))
	{
		Char->CurrentItem = nullptr;
	}

	if (OriginalViewTarget)
	{
		PC->SetViewTargetWithBlend(OriginalViewTarget, 0.5f);
	}


	// Unblock movement
	if (auto* Char = Cast<ACharacter>(PlayerPawn))
	{
		Char->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		UE_LOG(LogTemp, Warning, TEXT("unLockCamera_Item"));
	}


}

