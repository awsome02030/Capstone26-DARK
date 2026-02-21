#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridManager.generated.h"

UCLASS()
class DARK_API AGridManager : public AActor
{
    GENERATED_BODY()

public:
    AGridManager();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditAnywhere, Category = "Rooms")
    TSubclassOf<AActor> AnchorRoomBP;

    UPROPERTY(EditAnywhere, Category = "Rooms")
    TArray<TSubclassOf<AActor>> RoomBlueprints;

    UPROPERTY(EditAnywhere, Category = "Grid")
    float GridSpacingX = 3300.f;

    UPROPERTY(EditAnywhere, Category = "Grid")
    float GridSpacingY = 3300.f;

    TMap<FIntPoint, AActor*> SpawnedRooms;

    void SpawnRooms();
    FVector GridToWorld(const FIntPoint& GridPos) const;
};
