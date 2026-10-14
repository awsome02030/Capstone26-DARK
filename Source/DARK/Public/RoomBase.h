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

    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Room")
    void ToggleKeypads();

protected:
    virtual void BeginPlay() override;
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
    FName RoomID;

    UFUNCTION(BlueprintPure, Category = "Room")
    AGridManager* GetGridManager() const { return GridManager; }

    UFUNCTION(BlueprintCallable, Category = "Room")
    void SetGridManager(AGridManager* InGridManager);

    UPROPERTY(BlueprintReadOnly, Category = "Room")
    bool bDoorOpened = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
    float PuzzleTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle", DisplayName = "Puzzle Won?")
    bool bPuzzleWon = false;

private:
    UPROPERTY()
    AGridManager* GridManager = nullptr;
};