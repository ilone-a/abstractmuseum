#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AbstractMuseumGameInstance.generated.h"

UCLASS(BlueprintType, Blueprintable)
class ABSTRACTMUSEUM_API UAbstractMuseumGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UAbstractMuseumGameInstance();

    virtual void Init() override;
};