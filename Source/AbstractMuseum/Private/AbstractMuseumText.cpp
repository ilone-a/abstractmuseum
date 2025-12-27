#include "../Public/AbstractMuseumText.h"
#include "AbstractMuseumSettings.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CoreGlobals.h"
#include "DrawDebugHelpers.h"
#include "Engine/Font.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Interfaces/IPluginManager.h"
#include "Materials/MaterialInterface.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/ConfigCacheIni.h"
#include "UObject/ConstructorHelpers.h"

static UStaticMesh* GTextPlaneMesh = nullptr;
static UMaterialInterface* GTextBackgroundMat = nullptr;
static UFont* GCyrillicFont = nullptr;
static bool GTextAssetsLoaded = false;

static void LoadTextAssetsOnce()
{
	if (GTextAssetsLoaded)
		return;

	GTextAssetsLoaded = true;

	FString PlaneMeshPath;
	FString MaterialPath;
	FString FontPath;

	const FString ConfigPath = FPaths::Combine(
		IPluginManager::Get().FindPlugin(TEXT("AbstractMuseum"))->GetBaseDir(),
		TEXT("Config/AbstractMuseumPaths.ini")
	);

	if (GConfig)
	{
		GConfig->LoadFile(ConfigPath);

		GConfig->GetString(TEXT("AssetPaths"), TEXT("TextPlaneMesh"), PlaneMeshPath, ConfigPath);
		GConfig->GetString(TEXT("AssetPaths"), TEXT("TextMaterial"), MaterialPath, ConfigPath);
		GConfig->GetString(TEXT("AssetPaths"), TEXT("CyrillicFont"), FontPath, ConfigPath);
	}

	if (!PlaneMeshPath.IsEmpty())
	{
		GTextPlaneMesh = Cast<UStaticMesh>(
			StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *PlaneMeshPath)
		);
	}

	if (!MaterialPath.IsEmpty())
	{
		GTextBackgroundMat = Cast<UMaterialInterface>(
			StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *MaterialPath)
		);
	}

	if (!FontPath.IsEmpty())
	{
		GCyrillicFont = Cast<UFont>(
			StaticLoadObject(UFont::StaticClass(), nullptr, *FontPath)
		);
	}
}

AAbstractMuseumText::AAbstractMuseumText()
{
	PrimaryActorTick.bCanEverTick = false;
	Origin = CreateDefaultSubobject<USceneComponent>("Origin");
	check(Origin);
	SetRootComponent(Origin);
	//---Text component settings---

	TextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextComponent"));
	TextComponent->SetupAttachment(Origin);
	EHorizTextAligment UEAlignX =
		(HorizontalAlignment == EMuseumTextHorizontal::Left) ? EHTA_Left :
		(HorizontalAlignment == EMuseumTextHorizontal::Right) ? EHTA_Right :
		EHTA_Center;

	EVerticalTextAligment UEAlignY =
		(VerticalAlignment == EMuseumTextVertical::Top) ? EVRTA_TextTop :
		(VerticalAlignment == EMuseumTextVertical::Bottom) ? EVRTA_TextBottom :
		EVRTA_TextCenter;

	TextComponent->SetHorizontalAlignment(UEAlignX);
	TextComponent->SetVerticalAlignment(UEAlignY);

	//---Plane settings---
	TextPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BackgroundPlane"));
	TextPlane->SetupAttachment(Origin);
	TextPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	LoadTextAssetsOnce();
	if (GTextPlaneMesh)
	{
		TextPlane->SetStaticMesh(GTextPlaneMesh);
		TextPlane->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
		TextPlane->SetRelativeLocation(FVector(0.f, 0.01f, 0.f));
		TextPlane->SetVisibility(bIsPlaneVisible);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TextPlaneMesh not loaded"));
	}
	if (GTextBackgroundMat)
	{
		TextPlane->SetMaterial(0, GTextBackgroundMat);
	}

	// font
	if (GCyrillicFont)
	{
		TextComponent->SetFont(GCyrillicFont);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CyrillicFont not loaded"));
	}

	TextComponent->SetTextRenderColor(TextColor);
	TextComponent->SetWorldSize(TextScale * UAbstractMuseumSettings::TextWorldScale);
}

void AAbstractMuseumText::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateText();
	SetTextProperties(TextColor, TextScale, nullptr);
	UpdateTextPlane();

}

void AAbstractMuseumText::BeginPlay()
{
	Super::BeginPlay();
	UpdateText();
	UpdateTextPlane();
}
//---Text background plane---
void AAbstractMuseumText::UpdateTextPlane()
{
	if (!TextComponent || !TextPlane)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateTextPlane: Missing components"));
		return;
	}
	if (!GetWorld() || !TextComponent->IsRegistered())
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateTextPlane: World not ready or TextComponent not registered"));
		return;
	}
	if (!TextComponent->Font)
	{
		UE_LOG(LogTemp, Warning, TEXT("TextComponent has no valid font!"));
		return;
	}

	//---Move to not cross text and plane---
	TextPlane->SetRelativeLocation(FVector(-1.f, 0.f, 0.f));

	//---Material---
	if (BackgroundMaterial)
	{
		TextPlane->SetMaterial(0, BackgroundMaterial);
	}


	//---Get text size in local space---
	FVector TextSize = TextComponent->GetTextLocalSize();

	//---Local Scale TextComponent---
	FVector LocalScale = TextComponent->GetComponentScale();
	FVector TargetScale;
	TargetScale.Y = (TextSize.X + UAbstractMuseumSettings::TextPlaneScaleOffset) / 100.0f / LocalScale.X;
	TargetScale.X = (TextSize.Y + UAbstractMuseumSettings::TextPlaneScaleOffset) / 100.0f / LocalScale.Y;
	TargetScale.Z = (TextSize.Z + UAbstractMuseumSettings::TextPlaneScaleOffset) / 100.0f / LocalScale.Z;

	//---Set scale to Plane---
	TextPlane->SetRelativeScale3D(TargetScale);
	FVector TextCenter = TextComponent->GetLocalBounds().Origin;
	TextPlane->SetRelativeLocation(TextCenter);
}
//---Text projection---

int AAbstractMuseumText::GetTextMaxLen()
{
	return UAbstractMuseumSettings::TextMaxLen;
}

int AAbstractMuseumText::GetStringMaxLen()
{
	return UAbstractMuseumSettings::StringMaxLen;
}

void AAbstractMuseumText::UpdateLinetrace()
{
	if (!TextPlane) return;

	EditorProjection = FLinetraceProjectionData();

	FVector Start = TextPlane->GetComponentLocation();
	FVector Forward = TextPlane->GetRightVector();
	FVector End = Start - Forward * 1000.f;

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
		// Add DrawDebug
		//DrawDebugLine(World, Start, Hit.Location, FColor::Green, false, 1.0f, 0, 2.0f);
		//to hit point
		//DrawDebugPoint(World, Hit.Location, 12.0f, FColor::Red, false, 1.5f);
		//DrawDebugLine(World, Hit.Location, Hit.Location + Hit.Normal * 50.f, FColor::Blue, false, 1.5f, 0, 1.5f);
	}
	//else
	//{
		//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.f, 0, 1.f);
	//}
}
//---Text settings and content---
bool AAbstractMuseumText::LoadTextFromFile(const FString& FilePath)
{
	FString FileData;
	if (FFileHelper::LoadFileToString(FileData, *FilePath))
	{
		TextContent = FileData;
		UpdateText();
		UpdateTextPlane();
		return true;
	}
	return false;
}
void AAbstractMuseumText::UpdateText()
{
	if (!TextComponent || TextContent.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateText: no TextComponent or empty content"));
		return;
	}

	// the text
	TextComponent->SetText(FText::FromString(TextContent));
	TextComponent->SetTextRenderColor(TextColor);
	TextComponent->SetWorldSize(TextScale); // 

	TextComponent->SetFont(Font);
}

void AAbstractMuseumText::SetTextProperties(FColor Color, float Scale, UFont* TextFont)
{
	TextColor = Color;
	TextScale = Scale;
	TextComponent->SetTextRenderColor(Color);
	TextComponent->SetWorldSize(Scale);
	if (TextComponent)
	{
		TextComponent->SetTextRenderColor(Color);
		TextComponent->SetWorldSize(Scale);

		if (TextFont)
		{
			Font = TextFont;
			TextComponent->SetFont(TextFont);
		}
	}

}
void AAbstractMuseumText::ApplyAlignmentToText()
{
	if (!TextComponent) return;

	FVector BeforeCenter;
	FBoxSphereBounds BeforeBounds;
	BeforeBounds = TextComponent->GetLocalBounds();
	BeforeCenter = BeforeBounds.Origin;

	FVector OldWorldCenter = TextComponent->GetComponentTransform().TransformPosition(BeforeCenter);
	EHorizTextAligment UEAlignX = EHTA_Left;
	EVerticalTextAligment UEAlignY = EVRTA_TextCenter;

	switch (HorizontalAlignment)
	{
	case EMuseumTextHorizontal::Left:
		UEAlignX = EHTA_Left; break;
	case EMuseumTextHorizontal::Right:
		UEAlignX = EHTA_Right; break;
	case EMuseumTextHorizontal::Center:
		UEAlignX = EHTA_Center; break;
	default:
		UEAlignX = EHTA_Left; break;
	}

	switch (VerticalAlignment)
	{
	case EMuseumTextVertical::Top:
		UEAlignY = EVRTA_TextTop; break;
	case EMuseumTextVertical::Bottom:
		UEAlignY = EVRTA_TextBottom; break;
	case EMuseumTextVertical::Center:
		UEAlignY = EVRTA_TextCenter; break;
	default:
		UEAlignY = EVRTA_TextCenter; break;
	}

	TextComponent->SetHorizontalAlignment(UEAlignX);
	TextComponent->SetVerticalAlignment(UEAlignY);

	//get center afyer move
	FVector AfterCenter;
	FBoxSphereBounds LocalBounds;
	LocalBounds = TextComponent->GetLocalBounds();
	AfterCenter = LocalBounds.Origin;

	FVector NewWorldCenter = TextComponent->GetComponentTransform().TransformPosition(AfterCenter);

	//delta
	FVector Delta = OldWorldCenter - NewWorldCenter;

	TextComponent->SetWorldLocation(TextComponent->GetComponentLocation() + Delta);

}

void AAbstractMuseumText::UpdateTextAlignment()
{
	ApplyAlignmentToText();
	UpdateTextPlane();
	OnTextFormatChanged.Broadcast();
}


#if WITH_EDITOR
void AAbstractMuseumText::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property != nullptr)
	{
		// Compare property name with TextComponent
		FName PropertyName = PropertyChangedEvent.Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(AAbstractMuseumText, TextComponent))
		{
			UpdateText();
			UpdateTextPlane();
		}


		if (PropertyName == GET_MEMBER_NAME_CHECKED(AAbstractMuseumText, HorizontalAlignment) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(AAbstractMuseumText, VerticalAlignment))
		{
			ApplyAlignmentToText();
			OnTextFormatChanged.Broadcast();
		}
	}
}
#endif