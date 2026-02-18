#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridManager.generated.h"

UCLASS()
class TEST_API AGridManager : public AActor
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

    UPROPERTY(EditAnywhere, Category = "Rooms")
    TSubclassOf<AActor> HallwayBP;

    UPROPERTY(EditAnywhere, Category = "Rooms")
    AActor* PermanentEndRoom;

    UPROPERTY(EditAnywhere, Category = "Grid")
    float GridSpacingX = 2203.f;

    UPROPERTY(EditAnywhere, Category = "Grid")
    float GridSpacingY = 2203.f;

    UPROPERTY(EditAnywhere, Category = "Grid")
    float HallwayLength = 507.f;

    UPROPERTY(EditAnywhere, Category = "Grid")
    float HallwayWidth = 1696.f;

    UPROPERTY(EditAnywhere, Category = "Grid")
    float DoorFacingThreshold = 0.95f;

    UPROPERTY(EditAnywhere, Category = "Grid")
    float DoorConnectionMaxDist = 2500.f;

    TMap<FIntPoint, AActor*> SpawnedRooms;
    TArray<TPair<USceneComponent*, USceneComponent*>> ConnectedDoors;

    void SpawnRoomsAndHallways();
    FVector GridToWorld(const FIntPoint& GridPos) const;
    void ConnectRoomsWithHallways();
    void SpawnHallway(USceneComponent* DoorA, USceneComponent* DoorB);
    void GetDoorComponents(AActor* Room, TArray<USceneComponent*>& OutDoors) const;
    bool AlreadyConnected(USceneComponent* DoorA, USceneComponent* DoorB) const;
};