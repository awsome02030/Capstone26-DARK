#include "GridManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

AGridManager::AGridManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AGridManager::BeginPlay()
{
    Super::BeginPlay();
    SpawnRoomsAndHallways();
}

FVector AGridManager::GridToWorld(const FIntPoint& GridPos) const
{
    return FVector(
        GridPos.X * GridSpacingX,
        GridPos.Y * GridSpacingY,
        0.f
    );
}

void AGridManager::SpawnRoomsAndHallways()
{
    if (!AnchorRoomBP)
    {
        UE_LOG(LogTemp, Error, TEXT("AnchorRoomBP is not set"));
        return;
    }

    FIntPoint AnchorGrid(0, 0);
    AActor* AnchorRoom = GetWorld()->SpawnActor<AActor>(
        AnchorRoomBP,
        FVector(0.f, 0.f, 0.f),
        FRotator::ZeroRotator
    );

    if (!AnchorRoom)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn AnchorRoom"));
        return;
    }

    SpawnedRooms.Add(AnchorGrid, AnchorRoom);

    for (int32 X = -1; X <= 1; X++)
    {
        for (int32 Y = -3; Y <= -1; Y++)
        {
            FIntPoint GridPos(X, Y);

            if (RoomBlueprints.Num() == 0)
            {
                UE_LOG(LogTemp, Error, TEXT("RoomBlueprints array is empty"));
                return;
            }

            int32 RandomIndex = FMath::RandRange(0, RoomBlueprints.Num() - 1);
            TSubclassOf<AActor> RoomBP = RoomBlueprints[RandomIndex];

            FVector WorldPos = GridToWorld(GridPos);
            AActor* Room = GetWorld()->SpawnActor<AActor>(RoomBP, WorldPos, FRotator::ZeroRotator);

            if (Room)
            {
                SpawnedRooms.Add(GridPos, Room);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to spawn room at grid (%d,%d)"), X, Y);
            }
        }
    }

    if (PermanentEndRoom)
    {
        SpawnedRooms.Add(FIntPoint(0, -999), PermanentEndRoom);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PermanentEndRoom was not assigned"));
    }

    ConnectRoomsWithHallways();
}

void AGridManager::ConnectRoomsWithHallways()
{
    TArray<FIntPoint> Keys;
    SpawnedRooms.GetKeys(Keys);

    for (int32 i = 0; i < Keys.Num(); i++)
    {
        AActor* RoomA = SpawnedRooms[Keys[i]];
        if (!RoomA) continue;

        TArray<USceneComponent*> DoorsA;
        GetDoorComponents(RoomA, DoorsA);

        for (int32 j = i + 1; j < Keys.Num(); j++)
        {
            AActor* RoomB = SpawnedRooms[Keys[j]];
            if (!RoomB) continue;

            TArray<USceneComponent*> DoorsB;
            GetDoorComponents(RoomB, DoorsB);

            for (USceneComponent* DoorA : DoorsA)
            {
                for (USceneComponent* DoorB : DoorsB)
                {
                    if (!DoorA || !DoorB) continue;
                    if (AlreadyConnected(DoorA, DoorB)) continue;

                    FVector DirA = DoorA->GetForwardVector();
                    FVector DirB = DoorB->GetForwardVector();
                    float Dist = FVector::Dist(
                        DoorA->GetComponentLocation(),
                        DoorB->GetComponentLocation()
                    );

                    bool bFacing = FVector::DotProduct(DirA, -DirB) > DoorFacingThreshold;
                    bool bInRange = Dist < DoorConnectionMaxDist;

                    if (bFacing && bInRange)
                    {
                        SpawnHallway(DoorA, DoorB);
                        ConnectedDoors.Add(TPair<USceneComponent*, USceneComponent*>(DoorA, DoorB));
                    }
                }
            }
        }
    }
}

bool AGridManager::AlreadyConnected(USceneComponent* DoorA, USceneComponent* DoorB) const
{
    for (const auto& Pair : ConnectedDoors)
    {
        if ((Pair.Key == DoorA && Pair.Value == DoorB) ||
            (Pair.Key == DoorB && Pair.Value == DoorA))
        {
            return true;
        }
    }
    return false;
}

void AGridManager::GetDoorComponents(AActor* Room, TArray<USceneComponent*>& OutDoors) const
{
    if (!Room) return;

    TArray<USceneComponent*> AllComponents;
    Room->GetComponents<USceneComponent>(AllComponents);

    for (USceneComponent* Comp : AllComponents)
    {
        if (Comp && Comp->ComponentHasTag(FName("DoorPlaceHolder")))
        {
            OutDoors.Add(Comp);
        }
    }
}

void AGridManager::SpawnHallway(USceneComponent* DoorA, USceneComponent* DoorB)
{
    if (!DoorA || !DoorB || !HallwayBP) return;

    FVector Start = DoorA->GetComponentLocation();
    FVector End = DoorB->GetComponentLocation();

    FVector Direction = (End - Start).GetSafeNormal();
    FVector Midpoint = (Start + End) * 0.5f;
    FRotator Rotation = Direction.Rotation();
    Rotation.Yaw += 90.f;

    AActor* Hallway = GetWorld()->SpawnActor<AActor>(HallwayBP, Midpoint, Rotation);
    if (!Hallway) return;

    float Distance = FVector::Dist(Start, End);
    float ScaleX = Distance / FMath::Max(HallwayLength, 1.f);

    Hallway->SetActorScale3D(FVector(ScaleX, 1.f, 1.f));
}