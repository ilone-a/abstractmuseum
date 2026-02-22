// Fill out your copyright notice in the Description page of Project Settings.


#include "AbstractMuseumArt.h"
#include "AbstractMuseumFileHelper.h"
#include "AbstractPlayerController.h"
#include "AbstractMuseumSettings.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Camera/CameraActor.h"
#include "Components/SceneComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CoreGlobals.h"

#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/ConfigCacheIni.h"
#include "Interfaces/IPluginManager.h"
#include "Materials/MaterialInstanceConstant.h"


static UStaticMesh* CachedArtPlaneMesh = nullptr;
static UStaticMesh* CachedFrameCubeMesh = nullptr;

void LoadArtAssetsOnce()
{
	// only if not loaded
	if (CachedArtPlaneMesh && CachedFrameCubeMesh) return;

	const FString ConfigPath = FPaths::Combine(
		IPluginManager::Get().FindPlugin(TEXT("AbstractMuseum"))->GetBaseDir(),
		TEXT("Config/AbstractMuseumPaths.ini")
	);

	if (!GConfig)
		return;

	GConfig->LoadFile(ConfigPath);

	// --- Plane Mesh ---
	if (!CachedArtPlaneMesh)
	{
		FString PlaneMeshPath;
		GConfig->GetString(TEXT("AssetPaths"), TEXT("ArtPlaneMesh"), PlaneMeshPath, ConfigPath);
		if (!PlaneMeshPath.IsEmpty())
		{
			CachedArtPlaneMesh = Cast<UStaticMesh>(
				StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *PlaneMeshPath)
			);
			if (!CachedArtPlaneMesh)
				UE_LOG(LogTemp, Warning, TEXT("Failed to load ArtPlaneMesh from path: %s"), *PlaneMeshPath);
		}
	}

	// --- Frame Cube Mesh ---
	if (!CachedFrameCubeMesh)
	{
		FString FrameMeshPath;
		GConfig->GetString(TEXT("AssetPaths"), TEXT("ArtFrameMesh"), FrameMeshPath, ConfigPath);
		if (!FrameMeshPath.IsEmpty())
		{
			CachedFrameCubeMesh = Cast<UStaticMesh>(
				StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *FrameMeshPath)
			);
			if (!CachedFrameCubeMesh)
				UE_LOG(LogTemp, Warning, TEXT("Failed to load ArtFrameMesh from path: %s"), *FrameMeshPath);
		}
	}
}

AAbstractMuseumArt::AAbstractMuseumArt()
{
	PrimaryActorTick.bCanEverTick = false;
	Origin = CreateDefaultSubobject<USceneComponent>("Origin");
	check(Origin);
	SetRootComponent(Origin);

	//camera
	AMCamera = CreateDefaultSubobject<UCameraComponent>("AMCamera");
	AMCamera->SetupAttachment(RootComponent);

	if (AMCamera) { AMCamera->SetupAttachment(Origin); }


	//Art texture storage and placement
	Plane = CreateDefaultSubobject<UStaticMeshComponent>("Plane");
	check(Plane);
	Plane->SetupAttachment(Origin);

	//plane mesh
	LoadArtAssetsOnce();

	if (Plane && CachedArtPlaneMesh)
	{
		Plane->SetStaticMesh(CachedArtPlaneMesh);

		Plane->SetRelativeLocation(FVector(0.0f, 0.01f, 0.0f));//fix overlay material
		Plane->SetRenderCustomDepth(true);
		Plane->SetTranslucentSortPriority(1);

		Plane->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Plane->SetGenerateOverlapEvents(false); //
		Plane->SetNotifyRigidBodyCollision(false);
		Plane->SetCollisionResponseToAllChannels(ECR_Block);

	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load default Plane mesh"));
	}
	//--Custom frame for art. May be hidden
	Frame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Frame"));
	check(Frame);
	Frame->SetupAttachment(Origin);
	FrameBaseMaterial = nullptr;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> GridMatFinder(
		TEXT("/AbstractMuseum/Materials/FrameMaterial.FrameMaterial"));
	if (GridMatFinder.Succeeded())
	{
		FrameBaseMaterial = GridMatFinder.Object;
	}


	ProjectionDecal = CreateDefaultSubobject<UDecalComponent>("ProjectionDecal");
	check(ProjectionDecal);
	ProjectionDecal->SetupAttachment(Origin);

	// Set default decal properties
	ProjectionDecal->SetVisibility(false); // Hidden by default, shown only when projecting

}

void AAbstractMuseumArt::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);



	if (!Plane)
	{
		UE_LOG(LogTemp, Error, TEXT("Plane is NULL in OnConstruction"));
		return;
	}
	if (ArtMaterialAsset)
	{
		Plane->SetMaterial(0, ArtMaterialAsset);
		ScaleMeshes();
	}

	CreateDynamicFrameMaterial();

	if (Frame && CachedFrameCubeMesh)
	{
		Frame->SetStaticMesh(CachedFrameCubeMesh);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load default Cube mesh for Frame"));
	}

	//if (LoadedTexture )
	{


#if WITH_EDITOR
		CalculateCameraPositionEditor();
#endif

	}
	if (Frame)
	{
		Frame->SetVisibility(bIsFrameVisible);
	}

}

void AAbstractMuseumArt::BeginPlay()
{
	Super::BeginPlay();
	SetFrameVisible(false);
	// check init
	if (!BaseMaterial || !Plane)
	{
		UE_LOG(LogTemp, Error, TEXT("ArtMaterialStruct/BaseMaterial/Plane not set"));
		return;
	}

	//CreateDynamicMaterial();
	if (ArtMaterialAsset)
	{
		Plane->SetMaterial(0, ArtMaterialAsset);
		ScaleMeshes();
	}

	

	// frame
	if (Frame)
	{
		Frame->SetVisibility(bIsFrameVisible);
		if (CachedFrameCubeMesh)
		{
			Frame->SetStaticMesh(CachedFrameCubeMesh);
		}
	}

	// camera settings
	if (AMCamera)
	{
		AMCamera->SetWorldLocation(SavedCameraLocation);
	}

	// PlayerController
	PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PlayerPawn = PC->GetPawn();
		OriginalViewTarget = PC->GetViewTarget();
	}
}

//-------Frame settings-----
void AAbstractMuseumArt::CreateDynamicFrameMaterial()
{

	if (FrameBaseMaterial && !HasAnyFlags(RF_ClassDefaultObject))
	{
		FrameMaterial = UMaterialInstanceDynamic::Create(FrameBaseMaterial, this);
		if (FrameMaterial && Frame)
		{
			Frame->SetMaterial(0, FrameMaterial);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CreateDynamicMaterial failed: no BaseMaterial and no fallback"));
	}

}

void AAbstractMuseumArt::SetFrameVisible(bool bVisible)
{
	bIsFrameVisible = bVisible;
}

void AAbstractMuseumArt::UpdateFrame()
{/**/
	if (!bIsFrameVisible) return;
	FVector PlaneScale = Plane->GetRelativeScale3D();
	float frameAspect = 0.0f;
	//--scale Frame
	float frameZ = Frame->Bounds.BoxExtent.Z;
	frameAspect = FrameDepth / frameZ;

	FVector FrameScale(
		PlaneScale.X + FrameBorder,
		frameAspect,			
		PlaneScale.Y + FrameBorder

	);
	Frame->SetRelativeScale3D(FrameScale);
	Frame->UpdateBounds();
		FVector t = (Frame->Bounds.BoxExtent);
	const float MinValue = FMath::Min3(
		(float)t.X,
		(float)t.Y,
		(float)t.Z
	);
	Frame->SetRelativeLocation(FVector(0.f, -( MinValue * UAbstractMuseumSettings::FrameOffset), 0.f));
}

//-------Camera actions-----
void AAbstractMuseumArt::CalculateCameraPositionEditor()
{

	const FBoxSphereBounds Bounds = Plane->Bounds;

	const float HalfHeight = Bounds.BoxExtent.Z;
	const float HalfWidth = Bounds.BoxExtent.Y;

	const float FOV = AMCamera->FieldOfView * (PI / 180.f);
	const float DistanceH = HalfHeight / FMath::Tan(FOV * 0.5f);
	const float DistanceW = HalfWidth / (FMath::Tan(FOV * 0.5f) * AMCamera->AspectRatio);

	float FrameDistance = FMath::Max(DistanceH, DistanceW);

	FrameDistance += UAbstractMuseumSettings::CameraDistanceOffset;

	FVector Target = Bounds.Origin;
	// WTF!
	const FVector BackDirection = Plane->GetRightVector().GetSafeNormal();

	FVector NewCamLocation = Target + BackDirection * FrameDistance;
	FRotator NewCamRotation = (Target - NewCamLocation).Rotation();

	SavedCameraLocation = NewCamLocation;
	SavedCameraRotation = NewCamRotation;

	// apply in Editor
	AMCamera->SetWorldLocation(NewCamLocation);
	AMCamera->SetWorldRotation(NewCamRotation);

	UE_LOG(LogTemp, Log, TEXT("Camera placement updated and saved for %s"), *GetName());
}
void AAbstractMuseumArt::LockCameraToThing()
{
	if (!PC) return;
	ShowCursorWidget();
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

void AAbstractMuseumArt::UnlockCameraFromThing()
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
		UE_LOG(LogTemp, Warning, TEXT("UnlockCamera_Child"));
	}
}
//-------Walls projection linetrace-----
void AAbstractMuseumArt::UpdateLinetrace()
{
	if (!Plane) return;
	EditorProjection = FLinetraceProjectionData();

	FVector Start = Plane->GetComponentLocation();
	FVector Forward = Plane->GetRightVector();
	FVector End = Start - Forward * UAbstractMuseumSettings::ProjectionOffset;
	//End.Y = -End.Y;

	FCollisionQueryParams Params;
	Params.bTraceComplex = true;
	Params.AddIgnoredActor(this);

	FHitResult Hit;
	UWorld* World = GetWorld();
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		EditorProjection.TraceStart = Start;
		EditorProjection.TraceEnd = End;
		EditorProjection.HitLocation = Hit.Location;
		EditorProjection.Distance = FVector::Dist(Start, Hit.Location);
		EditorProjection.bHit = true;

	}
}

void AAbstractMuseumArt::ApplyTexture()
{

#if WITH_EDITOR
	if (!ArtMaterialAsset)
		return;
	FMaterialParameterInfo ParamInfo(TEXT("Art"));
	ArtMaterialAsset->PostEditChange();
	ArtMaterialAsset->MarkPackageDirty();
	Plane->SetMaterial(0, ArtMaterialAsset);
#endif
}

void AAbstractMuseumArt::ScaleMeshes()
{
	if (!Plane || !Frame || !ArtMaterialAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("ScaleMeshes: invalid state"));
		return;
	}

	UTexture* Texture = nullptr;

#if WITH_EDITOR
	FMaterialParameterInfo ParamInfo(TEXT("Art"));
	//ArtMaterialAsset->GetTextureParameterValue(ParamInfo, Texture);

	ArtMaterialAsset->GetTextureParameterValue(TEXT("Art"), Texture);
#endif

	UTexture2D* Texture2D = Cast<UTexture2D>(Texture);
	if (!Texture2D)
		return;

	const int32 Width = Texture2D->GetSurfaceWidth();
	const int32 Height = Texture2D->GetSurfaceHeight();

	if (Width <= 0 || Height <= 0)
		return;

	const float AspectRatio = static_cast<float>(Width) / static_cast<float>(Height);
	const float BaseScale = 1.0f;

	FVector PlaneScale(BaseScale * AspectRatio, BaseScale, 1.f);
	Plane->SetRelativeScale3D(PlaneScale);
	Plane->UpdateBounds();

	float frameZ = Frame->Bounds.BoxExtent.Z;
	float frameAspect = FrameDepth / frameZ;

	FVector FrameScale(
		PlaneScale.X + FrameBorder,
		frameAspect,
		PlaneScale.Y + FrameBorder
	);

	Frame->SetRelativeScale3D(FrameScale);
	Frame->UpdateBounds();
	FVector t = Frame->Bounds.BoxExtent;
	const float MinValue = FMath::Min3(t.X, t.Y, t.Z);

	Frame->SetRelativeLocation(
		FVector(0.f, -(MinValue * UAbstractMuseumSettings::FrameOffset), 0.f)
	);
}

#if WITH_EDITOR
void AAbstractMuseumArt::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);
	//UpdateProjectionDecal();
}

void AAbstractMuseumArt::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	//UpdateProjectionDecal();
}
#endif




