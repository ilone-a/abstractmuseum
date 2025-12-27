#pragma once

#include "AbstractMuseumActor.h"
#include "Components/TextRenderComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbstractMuseumText.generated.h"

class USceneComponent;
class UTextRenderComponent;

//---Alignement enums---
UENUM(BlueprintType)
enum class EMuseumTextHorizontal : uint8
{
	Left   UMETA(DisplayName = "Left"),
	Center UMETA(DisplayName = "Center"),
	Right  UMETA(DisplayName = "Right")
};

UENUM(BlueprintType)
enum class EMuseumTextVertical : uint8
{
	Top    UMETA(DisplayName = "Top"),
	Center UMETA(DisplayName = "Center"),
	Bottom UMETA(DisplayName = "Bottom")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTextFormatChanged);

UCLASS()
class ABSTRACTMUSEUM_API AAbstractMuseumText : public AAbstractMuseumActor
{
	GENERATED_BODY()

public:
	AAbstractMuseumText();

	//---Text properties---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Settings")
	FColor TextColor = FColor(180, 180, 180);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Settings")
	float TextScale = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Settings")
	EMuseumTextHorizontal HorizontalAlignment = EMuseumTextHorizontal::Left;
	EMuseumTextVertical VerticalAlignment = EMuseumTextVertical::Top;
	UPROPERTY(EditAnywhere, Category = "Text Content")
	FString TextContent;
	//-----
	UPROPERTY(BlueprintAssignable, Category = "Text|Events")
	FOnTextFormatChanged OnTextFormatChanged;

	//---Text background---
	UPROPERTY(EditAnywhere, Category = "Plane")
	bool bIsPlaneVisible = true;

	//--Get default font (cyrillic Roboto in my case) but can be changed by user--
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Settings")
	UFont* Font;
	//--Plane on text background--
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Plane")
	TObjectPtr<UStaticMeshComponent> TextPlane;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text")
	UMaterialInterface* BackgroundMaterial;

	int GetTextMaxLen();
	int GetStringMaxLen();

	void UpdateLinetrace();
	void UpdateText();
	void UpdateTextPlane();
	void SetTextProperties(FColor Color, float Scale, UFont* TextFont);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	//---Load text from file---
	UFUNCTION(CallInEditor, Category = "Text")
	bool LoadTextFromFile(const FString& FilePath);

	//---TextComponent---
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UTextRenderComponent> TextComponent;

	UFUNCTION(CallInEditor, Category = "Text")
	void UpdateTextAlignment();
	void ApplyAlignmentToText();

};