#pragma once
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Blueprint/UserWidget.h"
#include "../Public/AbstractMuseumActor.h"
#include "AbstractMuseumCharacter.generated.h"

class UCameraComponent;
class UInputMappingContext;
class UInputAction;

UCLASS()
class ABSTRACTMUSEUM_API AAbstractMuseumCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	AAbstractMuseumCharacter();

	//Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "My Character")
	UCameraComponent* FirstPersonCamera;
	//Overrides
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(		UInputComponent* PlayerInputComponent) override;

	//Input
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	UFUNCTION()
	void HandleInspectChanged(AAbstractMuseumActor* Actor);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LeftClickAction;

	//Functions
	void OutlineBegin(const FHitResult& Hit);
	void OnLeftClick(const FInputActionValue& Value);
	void CreateEditModeWidget();
	//Edit Mode
	UPROPERTY(EditAnywhere, Category = "Edit")
	bool EditMode = true;
	UPROPERTY()
	UUserWidget* EditModeWidget;

	UPROPERTY()
	TObjectPtr<AAbstractMuseumActor> CurrentInteractActor = nullptr;


private:
	void EditModeOn() { EditMode = true; };
	void EditModeOff() { EditMode = false; };

};