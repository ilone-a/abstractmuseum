#include "TimedSceneCaptureActor.h"
#include "TimerManager.h"
#include "Engine/World.h"

ATimedSceneCaptureActor::ATimedSceneCaptureActor()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
    RootComponent = SceneCapture;
}

void ATimedSceneCaptureActor::BeginPlay()
{
    Super::BeginPlay();

    if (SceneCapture && RenderTarget)
    {
        SceneCapture->TextureTarget = RenderTarget;
    }

    GetWorldTimerManager().SetTimer(
        CaptureTimerHandle,
        this,
        &ATimedSceneCaptureActor::DoCapture,
        UpdateInterval,
        true
    );
}

void ATimedSceneCaptureActor::DoCapture()
{
    if (!SceneCapture)
        return;

    if (RenderTarget && SceneCapture->TextureTarget != RenderTarget)
    {
        SceneCapture->TextureTarget = RenderTarget;
    }

    SceneCapture->CaptureScene();
}