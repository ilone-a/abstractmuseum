
#include "AbstractPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/InputComponent.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Widgets/Input/SButton.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "AbstractMuseumProjectionLogic.h"
#include "AbstractMuseumActor.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

void AStartMenuController::BeginPlay()
{
	Super::BeginPlay();
	ProjectionLogic = AAbstractMuseumProjectionLogic::Get(GetWorld());
	auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem && IMC_Player)
	{
		Subsystem->AddMappingContext(IMC_Player, 0);
	}

	//---Find BP Start widget---
	if (!StartMenuClass)
	{
		static ConstructorHelpers::FClassFinder<UUserWidget> WidgetBPClass(TEXT("/Game/World/BP_StartMenu"));
		if (WidgetBPClass.Succeeded())
		{
			StartMenuClass = WidgetBPClass.Class;
		}
	}
	if (!EndMenuClass)
	{
		static ConstructorHelpers::FClassFinder<UUserWidget> EndBP(TEXT("/Game/BP_EndMenu"));
		if (EndBP.Succeeded())
			EndMenuClass = EndBP.Class;
	}

	if (StartMenuClass)
	{
		StartMenuWidget = CreateWidget<UUserWidget>(this, StartMenuClass);
		if (StartMenuWidget)
		{
			StartMenuWidget->AddToViewport(100);

			//---Find button in BP Start widget---
			if (UButton* StartButton = Cast<UButton>(StartMenuWidget->GetWidgetFromName(TEXT("StartButton"))))
			{
				StartButton->OnClicked.AddDynamic(this, &AStartMenuController::OnStartClicked);
			}

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(StartMenuWidget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create StartMenuWidget"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("StartMenuClass not set and not found"));
	}


	bEnableMouseOverEvents = true;
	bEnableClickEvents = true;

	bShowMouseCursor = true;

	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	SetInputMode(Mode);
}

void AStartMenuController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAction("Pause", IE_Pressed, this, &AStartMenuController::ShowEndMenu);
}

void AStartMenuController::OnStartClicked()
{
	if (StartMenuWidget)
	{
		StartMenuWidget->RemoveFromParent();
	}

	bShowMouseCursor = false;

	bEnableMouseOverEvents = true;
	bEnableClickEvents = true;

	FInputModeGameAndUI Mode;
	Mode.SetHideCursorDuringCapture(false); // 
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	SetInputMode(Mode);
}

void AStartMenuController::ShowEndMenu()
{
	if (EndMenuWidget)
		return; // already opened

	if (EndMenuClass)
	{
		EndMenuWidget = CreateWidget<UUserWidget>(this, EndMenuClass);
		if (EndMenuWidget)
		{
			EndMenuWidget->AddToViewport(100);

			if (UButton* YesBtn = Cast<UButton>(EndMenuWidget->GetWidgetFromName(TEXT("YesButton"))))
				YesBtn->OnClicked.AddDynamic(this, &AStartMenuController::OnYesClicked);
			if (UButton* NoBtn = Cast<UButton>(EndMenuWidget->GetWidgetFromName(TEXT("NoButton"))))
				NoBtn->OnClicked.AddDynamic(this, &AStartMenuController::OnNoClicked);

			FInputModeUIOnly InputMode;
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
	}
}

void AStartMenuController::OnYesClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), this, EQuitPreference::Quit, false);
}

void AStartMenuController::OnNoClicked()
{
	if (EndMenuWidget)
	{
		EndMenuWidget->RemoveFromParent();
		EndMenuWidget = nullptr;
	}

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
}
