#include "AbstractMuseumCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ArtInteractInterface.h"
#include "Kismet/KismetMathLibrary.h"
AAbstractMuseumCharacter::AAbstractMuseumCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    FirstPersonCamera =
        CreateDefaultSubobject<UCameraComponent>(
            TEXT("FirstPersonCamera"));

    FirstPersonCamera->SetupAttachment(GetCapsuleComponent());

    FirstPersonCamera->SetRelativeLocation(
        FVector(0.f, 0.f, 70.f));
}

void AAbstractMuseumCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!EditMode) return;
	if (!FirstPersonCamera) return;

	FVector StartTrace = FirstPersonCamera->GetComponentLocation();
	FVector EndTrace = StartTrace + (FirstPersonCamera->GetForwardVector()) * 300.f;
	FHitResult Hit;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit,
        StartTrace,
        EndTrace,
        ECC_Visibility,
        Params
    );


    if (bHit)
    {
        OutlineBegin(Hit);
    }
}

void AAbstractMuseumCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PC =
        Cast<APlayerController>(GetController()))
    {
        if (ULocalPlayer* LP =
            PC->GetLocalPlayer())
        {
            if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
                LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
            {
                Subsystem->AddMappingContext(
                    DefaultMappingContext,
                    0
                );
            }
        }
    }
    if (EditMode)
    {
        CreateEditModeWidget();
    }
}

void AAbstractMuseumCharacter::OutlineBegin(const FHitResult& Hit)
{
    AActor* Actor = Hit.GetActor();
    if (!Actor)
    {
        return;
    }

    if (Actor->Implements<UArtInteractInterface>())
    {
        IArtInteractInterface::Execute_ArtOnFocus(Actor);
    }

    if (Actor->Implements<UEnvInteractInterface>())
    {
        IEnvInteractInterface::Execute_EnvOnFocus(Actor);
    }
}
void AAbstractMuseumCharacter::SetupPlayerInputComponent(
    UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(
        PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInput =
        Cast<UEnhancedInputComponent>(
            PlayerInputComponent))
    {
        if (MoveAction)
        {
            EnhancedInput->BindAction(
                MoveAction,
                ETriggerEvent::Triggered,
                this,
                &AAbstractMuseumCharacter::Move);
        };

        if (LookAction)
        {
            EnhancedInput->BindAction(
                LookAction,
                ETriggerEvent::Triggered,
                this,
                &AAbstractMuseumCharacter::Look);
        };

        if (JumpAction)
        {
            EnhancedInput->BindAction(
                JumpAction,
                ETriggerEvent::Started,
                this,
                &ACharacter::Jump);

            EnhancedInput->BindAction(
                JumpAction,
                ETriggerEvent::Completed,
                this,
                &ACharacter::StopJumping);
        };
        if (LeftClickAction)
        {
            EnhancedInput->BindAction(
                LeftClickAction,
                ETriggerEvent::Started,
                this,
                &AAbstractMuseumCharacter::OnLeftClick);
        }

    };
}
void AAbstractMuseumCharacter::Move(
    const FInputActionValue& Value)
{
    FVector2D Input = Value.Get<FVector2D>();
    FRotator InRot = GetControlRotation();
    FVector WDirection = UKismetMathLibrary::GetRightVector(InRot);
    FVector WDirection2 = UKismetMathLibrary::GetForwardVector(InRot);
    AddMovementInput(WDirection, Input.X);
    AddMovementInput(WDirection2, Input.Y);
}

void AAbstractMuseumCharacter::Look(const FInputActionValue& Value)
{
    FVector2D Input =Value.Get<FVector2D>();
    AddControllerYawInput(Input.X);
    AddControllerPitchInput(Input.Y);
}

void AAbstractMuseumCharacter::OnLeftClick(const FInputActionValue& Value)
{
    if (!EditMode)   return;
    APlayerController* PC =
        Cast<APlayerController>(GetController());
    if (!PC) return;

    FVector StartTrace = FirstPersonCamera->GetComponentLocation();
    FVector EndTrace = StartTrace + (FirstPersonCamera->GetForwardVector()) * 300.f;
    FHitResult Hit;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit,
        StartTrace,
        EndTrace,
        ECC_Visibility,
        Params
    );

    if (!bHit) return;
    AActor* Actor = Hit.GetActor();
    if (!Actor) return;

    if (Actor->Implements<UArtInteractInterface>())
    {
        IArtInteractInterface::Execute_ArtOnInteract(Actor);
    }
    if (Actor->Implements<UEnvInteractInterface>())
    {
        IEnvInteractInterface::Execute_EnvOnInteract(Actor);
    }
}
void AAbstractMuseumCharacter::CreateEditModeWidget()
{
    if (EditModeWidget) return;
    if (!GetWorld()) return;

    TSubclassOf<UUserWidget> WidgetClass =
        LoadClass<UUserWidget>(nullptr,TEXT("/Game/UMG/Edit_Mode_Widget.Edit_Mode_Widget_C"));

    if (!WidgetClass) return;
    APlayerController* PC =Cast<APlayerController>(GetController());
    if (!PC) return;
    EditModeWidget =CreateWidget<UUserWidget>(PC,WidgetClass);
    if (EditModeWidget)
    {
        EditModeWidget->AddToViewport();
    }
}