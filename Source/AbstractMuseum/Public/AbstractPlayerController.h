#pragma once


#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "InputMappingContext.h"
#include "AbstractPlayerController.generated.h"

// To use, create BP UI widgets with correct names

UCLASS()
class ABSTRACTMUSEUM_API AStartMenuController : public APlayerController
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;


protected:
    FTimerHandle LogTimer;
    UPROPERTY()
    class AAbstractMuseumProjectionLogic* ProjectionLogic = nullptr;
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputMappingContext* IMC_Player;
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* LeftClick;

private:
    UPROPERTY()
    class UUserWidget* StartMenuWidget;
    UPROPERTY()
    UUserWidget* EndMenuWidget;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UUserWidget> StartMenuClass;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UUserWidget> EndMenuClass;

    UFUNCTION()
    void OnStartClicked();
    UFUNCTION()
    void OnYesClicked();
    UFUNCTION()
    void OnNoClicked();
    void ShowEndMenu();
};