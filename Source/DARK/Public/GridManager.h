#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridManager.generated.h"

class ARoomBase;

UENUM(BlueprintType)
enum class ERoomType : uint8
{
    None UMETA(DisplayName = "None"),

    Laboratory,
    Equation,
    Briefing,
    CommunicationHub,
    CargoMaintenance,
    FusionReactor
};

UENUM(BlueprintType)
enum class EDoorDirection : uint8
{
    North,
    South,
    East,
    West
};

USTRUCT(BlueprintType)
struct FRoomData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    ERoomType RoomType = ERoomType::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString RoomName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<ARoomBase> RoomClass;
};

UCLASS()
class DARK_API AGridManager : public AActor
{
    GENERATED_BODY()

public:
    AGridManager();
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "Grid")
    void SpawnAnchorRoom();

    UFUNCTION(BlueprintCallable, Category = "Grid")
    void ResetGrid();

    UFUNCTION(BlueprintCallable, Category = "Grid")
    void ShowRoomSelectWidget();

    UFUNCTION()
    void OnRoomChosen(int32 ChosenIndex);

    UFUNCTION(BlueprintCallable, Category = "Grid")
    void RegisterExitDoor(AActor* Door, EDoorDirection Direction);

private:
    UPROPERTY(EditAnywhere, Category = "Rooms")
    TSubclassOf<ARoomBase> AnchorRoomBP;

    UPROPERTY(EditAnywhere, Category = "Rooms")
    TArray<FRoomData> RoomPool;

    UPROPERTY(EditAnywhere, Category = "Rooms")
    TArray<TSubclassOf<AActor>> HallwayPool;

    UPROPERTY(EditAnywhere, Category = "Grid")
    float GridSpacingX = 3300.f;

    UPROPERTY(EditAnywhere, Category = "Grid")
    float GridSpacingY = 3300.f;

    UPROPERTY(EditAnywhere, Category = "Grid")
    float HallwayLengthOffset = 0.0f;

    TMap<FIntPoint, ARoomBase*> SpawnedRooms;
    TMap<FIntPoint, AActor*> SpawnedHallways;
    TArray<FRoomData> PendingRoomChoices;

    AActor* CurrentExitDoor = nullptr;
    EDoorDirection CurrentExitDirection = EDoorDirection::North;
    FIntPoint CurrentExitRoomCell = FIntPoint(0, 0);

    FIntPoint GetNextCell(const FIntPoint& From, EDoorDirection Dir) const;
    FVector GridToWorld(const FIntPoint& GridPos) const;
    FRotator HallwayRotationForDirection(EDoorDirection Dir) const;
    void SpawnChosenRoom(const FRoomData& RoomData);
    void SpawnHallway(const FIntPoint& FromCell, const FIntPoint& ToCell, EDoorDirection Dir);
    void ClearGrid();
};