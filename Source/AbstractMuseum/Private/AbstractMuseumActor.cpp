#include "AbstractMuseumActor.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h" 
#include "UObject/ConstructorHelpers.h"
#include "Components/DecalComponent.h"
#include "AbstractMuseumSettings.h"

AAbstractMuseumActor::AAbstractMuseumActor()
{
	//-------Load widget TODO fix path-----
	static ConstructorHelpers::FClassFinder<UUserWidget> CursorWidgetFinder(
		TEXT("/Game/UMG/ScaleWidget")
	);

	if (CursorWidgetFinder.Succeeded())
	{
		CursorWidgetClass = CursorWidgetFinder.Class;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ScaleWidget NOT FOUND in AAbstractMuseumActor"));
	}
	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");
}

void AAbstractMuseumActor::ArtOnInteract_Implementation()
{
	UE_LOG(LogTemp, Error, TEXT("INTERACTED"));
	//flip flop state machine
	if (!bIsInteracted)
	{
		LockCameraToThing();
	}
	else
	{
		UnlockCameraFromThing();
	}

	bIsInteracted = !bIsInteracted;
}

void AAbstractMuseumActor::ArtOnFocus_Implementation()
{
	//Entry point
	//if event stop triggering more than 0.1s 1) remove widget 2) let it to be created ONCE before Restart

	HandleCreateWidget();// Debounce/inactivity/create widget logic
	RestartHideTimer();// One-shot creation guard/remove widget logic
}


void AAbstractMuseumActor::HandleCreateWidget()
{
	if (bWidgetCreated)
		return;
	bWidgetCreated = true;
	ShowCursorWidget();
}

void AAbstractMuseumActor::RestartHideTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(HideWidgetTimer);

	GetWorld()->GetTimerManager().SetTimer(
		HideWidgetTimer,
		this,
		&AAbstractMuseumActor::HideCursorWidget,
		UAbstractMuseumSettings::LensTimerDelay,
		false
	);
}


//TODO move widget to separate widgets lib
void AAbstractMuseumActor::ShowCursorWidget()
{
	
	if (!CursorWidgetClass)
		return;

	if (ActiveCursorWidget)
		return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
		return;

	ActiveCursorWidget = CreateWidget<UUserWidget>(PC, CursorWidgetClass);

	if (ActiveCursorWidget)
		ActiveCursorWidget->AddToViewport(9999);   // on top of all widgets
		
}

void AAbstractMuseumActor::HideCursorWidget()
{
	
	if (ActiveCursorWidget)
	{
		ActiveCursorWidget->RemoveFromParent();
		ActiveCursorWidget = nullptr;
	}
	bWidgetCreated = false; // <-- Reset для DoN
}
/////////////////////////

void AAbstractMuseumActor::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->bEnableMouseOverEvents = true;
		PC->bEnableClickEvents = true;
	}
}
void AAbstractMuseumActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void AAbstractMuseumActor::UpdateLinetrace() {};

void AAbstractMuseumActor::UpdateProjectionDecal() {};

void AAbstractMuseumActor::LockCameraToThing() {
	UE_LOG(LogTemp, Warning, TEXT("LockCamera_Parent"));
};

void AAbstractMuseumActor::UnlockCameraFromThing() {
	UE_LOG(LogTemp, Warning, TEXT("UnlockCamera_Parent"));
};


void AAbstractMuseumActor::ScaleMeshes() {}
void AAbstractMuseumActor::SetHash(const FString& FileHash_)
{
	FileHash = FileHash_;
	UE_LOG(LogTemp, Log, TEXT("[Art] File Hash %s is set (hash: %s) [%s]"),
		*LocalFilePath,
		*FileHash,
		GIsEditor ? TEXT("Editor") : TEXT("Runtime"));
}

void AAbstractMuseumActor::CalculateCameraPositionEditor()
{
}

;