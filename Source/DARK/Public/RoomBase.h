#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomBase.generated.h"

class AGridManager;
class APuzzleDoor;

UCLASS(Blueprintable)
class DARK_API ARoomBase : public AActor
{
    GENERATED_BODY()
public:
    ARoomBase();
protected:
    virtual void BeginPlay() override;
public:
    UFUNCTION(BlueprintPure, Category = "Room")
    AGridManager* GetGridManager() const { return GridManager; }
    UFUNCTION(BlueprintCallable, Category = "Room")
    void SetGridManager(AGridManager* InGridManager);
private:
    UPROPERTY()
    AGridManager* GridManager = nullptr;
};