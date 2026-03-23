#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridManager.generated.h"

UENUM(BlueprintType)
enum class ERoomType : uint8
{
    Laboratory,
    Equation,
    Briefing,
    CommunicationHub,
    CargoMaintenance,
    FusionReactor
};

USTRUCT(BlueprintType)
struct FRoomData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    ERoomType RoomType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<AActor> RoomClass;
};

UCLASS()
class DARK_API AGridManager : public AActor
{
    GENERATED_BODY()

public:
    AGridManager();

    UFUNCTION()
    void ResetGrid();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditAnywhere, Category = "Rooms")
    TSubclassOf<AActor> AnchorRoomBP;

    UPROPERTY(EditAnywhere, Category = "Rooms")
    TArray<FRoomData> RoomPool;

    UPROPERTY(EditAnywhere, Category = "Grid")
    float GridSpacingX = 3300.f;

    UPROPERTY(EditAnywhere, Category = "Grid")
    float GridSpacingY = 3300.f;

    TMap<FIntPoint, AActor*> SpawnedRooms;

    void SpawnRooms();
    FVector GridToWorld(const FIntPoint& GridPos) const;

    void ClearGrid();
};