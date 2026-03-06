// Fill out your copyright notice in the Description page of Project Settings.


#include "Rooms/LevelsManager.h"

#include "Core/PersistentManager.h"
#include "Core/ScrapItGameInstance.h"
#include "Kismet/GameplayStatics.h"

void ULevelsManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(UPersistentManager::StaticClass());

	if (const UScrapItGameInstance* GameInstance = Cast<UScrapItGameInstance>(GetGameInstance()))
	{
		RoomsPool = GameInstance->RoomsPool;
	}

	if (UPersistentManager* PersistentManager = GetGameInstance()->GetSubsystem<UPersistentManager>())
	{
		OnNewLevelGenerated.AddDynamic(PersistentManager, &UPersistentManager::AdvanceLevel);
		UE_LOG(LogTemp, Warning, TEXT("Subscribed to Level Generation"));
	}
}

void ULevelsManager::Deinitialize()
{
	Super::Deinitialize();
}

void ULevelsManager::GenerateLevel(const int32 NumRooms)
{
	if (RoomsPool == nullptr || NumRooms <= 2)
	{
		//TODO: Handle special levels with few rooms
		return;
	}

	//1. Reset state
	LevelMap.Empty();
	CurrentRoomID = 0;
	TArray<FIntPoint> OccupiedCoordinates;
	TArray<int32> SpecialNodes;

	//2. Define node roles
	CreateBaseLevelMap(NumRooms);
	SetupSpecialRooms(NumRooms, SpecialNodes);

	//3. Build Main Path
	TArray<int32> MainPathNodes;
	for (int32 i = 0; i < NumRooms; i++)
	{
		if (!SpecialNodes.Contains(i))
		{
			MainPathNodes.Add(i);
		}
	}

	//Place starting room at the origin
	const FIntPoint StartingCoordinate = FIntPoint(0, 0);
	LevelMap[0].Coordinates = FIntPoint(0, 0); //First room is at origin
	OccupiedCoordinates.Add(StartingCoordinate);

	//Link Rooms on Main Path
	for (int32 i = 0; i < MainPathNodes.Num() - 1; i++)
	{
		ConnectRooms(MainPathNodes[i], MainPathNodes[i + 1], OccupiedCoordinates);
	}

	//4. Link Special Rooms to Main Path
	for (const int32 Node : SpecialNodes)
	{
		const int32 ConnectToID = MainPathNodes[FMath::RandRange(0, MainPathNodes.Num() - 2)];
		ConnectRooms(ConnectToID, Node, OccupiedCoordinates);
	}

	//5. Finalize by assigning levels to each room
	for (auto& Room : LevelMap)
	{
		Room.Value.Layout = RoomsPool->GetRandomRoomByType(Room.Value.RoomType);
	}

	OnNewLevelGenerated.Broadcast();
	UE_LOG(LogTemp, Warning, TEXT("Level Generation Complete with %d Rooms"), NumRooms);
}

void ULevelsManager::CreateBaseLevelMap(const int32 NumRooms)
{
	for (int32 i = 0; i < NumRooms; ++i)
	{
		FRoomNode NewNode;
		NewNode.RoomID = i;

		if (i == 0)
		{
			NewNode.RoomType = ERoomType::Start;
		}
		else if (i == NumRooms - 1)
		{
			NewNode.RoomType = ERoomType::Exit;
		}
		else
		{
			NewNode.RoomType = ERoomType::Combat;
		}

		LevelMap.Add(i, NewNode);
	}
}

void ULevelsManager::SetupSpecialRooms(const int32 NumRooms, TArray<int32>& OutSpecialNodes)
{
	if (NumRooms > 3)
	{
		OutSpecialNodes.Add(1);
	}
	else if (NumRooms > 4)
	{
		OutSpecialNodes.Add(2);
	}

	for (const int32 SpecialNode : OutSpecialNodes)
	{
		LevelMap[SpecialNode].RoomType = ERoomType::Special;
	}
}

void ULevelsManager::ConnectRooms(const int32 From, const int32 To, TArray<FIntPoint>& OccupiedCoordinates)
{
	const FIntPoint ConnectedCoordinates = GetRandomEmptyNeighbor(LevelMap[From].Coordinates, OccupiedCoordinates);
	LevelMap[To].Coordinates = ConnectedCoordinates;
	OccupiedCoordinates.Add(ConnectedCoordinates);

	//Link them together
	LevelMap[From].ConnectedRoomsIDs.AddUnique(To);
	LevelMap[To].ConnectedRoomsIDs.AddUnique(From);
	UE_LOG(LogTemp, Warning, TEXT("Node %d connected to %d"), From, To)
}

FIntPoint ULevelsManager::GetRandomEmptyNeighbor(const FIntPoint Origin, const TArray<FIntPoint>& Occupied) const
{
	//Shuffle Directions
	TArray<FIntPoint> ShuffledDirections = Directions;
	for (int32 i = Directions.Num() - 1; i > 0; i--)
	{
		const int32 j = FMath::RandRange(0, i);
		Swap(ShuffledDirections[i], ShuffledDirections[j]);
	}

	for (const FIntPoint& Direction : ShuffledDirections)
	{
		const FIntPoint NewCoordinate = Origin + Direction;
		if (!Occupied.Contains(NewCoordinate))
		{
			return NewCoordinate;
		}
	}

	//Fallback, just return first direction (might cause overlap but better than crashing)
	return Origin + Directions[0];
}

void ULevelsManager::MarkRoomAsVisited(const int32 RoomID)
{
	LevelMap[RoomID].bIsVisited = true;
}

void ULevelsManager::LoadRoomByID(const int32 RoomID) const
{
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, LevelMap[RoomID].Layout);
}

EDoorDirection ULevelsManager::GetEntryDirection() const
{
	switch (LastExitDirection)
	{
	case North: return South;
	case South: return North;
	case East: return West;
	case West: return East;
	default: return None;
	}
}
