#pragma once
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "TimedSceneCaptureActor.generated.h"

UCLASS()
class ABSTRACTMUSEUM_API ATimedSceneCaptureActor : public AActor
{
    GENERATED_BODY()

public:
    ATimedSceneCaptureActor();

protected:
    virtual void BeginPlay() override;

public:

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Capture")
    USceneCaptureComponent2D* SceneCapture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capture")
    UTextureRenderTarget2D* RenderTarget;

    // seconds upd interval
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capture")
    float UpdateInterval = 1.0f;

private:
    FTimerHandle CaptureTimerHandle;

    void DoCapture();
};