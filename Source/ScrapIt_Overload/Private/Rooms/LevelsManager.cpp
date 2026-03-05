// Fill out your copyright notice in the Description page of Project Settings.


#include "Rooms/LevelsManager.h"
#include "Core/ScrapItGameInstance.h"
#include "Kismet/GameplayStatics.h"

void ULevelsManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (const UScrapItGameInstance* GameInstance = Cast<UScrapItGameInstance>(GetGameInstance()))
	{
		RoomsPool = GameInstance->RoomsPool;
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
		return;
	}

	//Reset vars for new level generation
	LevelMap.Empty();
	CurrentRoomID = 0;

	/* --- 1) Setup Room Nodes --- */

	//Populate Level Map with Base Nodes
	CreateBaseLevelMap(NumRooms);

	//Setup Special Rooms based on how many rooms we have
	TArray<int32> SpecialNodes;
	if (NumRooms > 3)
	{
		//If we have more than 3 rooms, we can have a special room
		SpecialNodes.Add(1);
		LevelMap[SpecialNodes[0]].RoomType = ERoomType::Special;

		if (NumRooms > 4)
		{
			//If we have more than 4 rooms, we can have two special rooms
			SpecialNodes.Add(2);
			LevelMap[SpecialNodes[1]].RoomType = ERoomType::Special;
		}
	}

	/* --- 2) Setup Coordinates --- */
	TArray<FIntPoint> OccupiedCoordinates;
	FIntPoint StartingCoordinate = FIntPoint(0, 0);
	LevelMap[0].Coordinates = StartingCoordinate; //First room is at origin
	OccupiedCoordinates.Add(StartingCoordinate);

	/* --- 3) Connect Rooms --- */
	TArray<int32> MainPathNodes;

	//form the main path (guarantees reaching exit)
	for (int32 i = 0; i < NumRooms; i++)
	{
		if (!SpecialNodes.Contains(i))
		{
			MainPathNodes.Add(i);
		}
	}

	//Link the rooms on main path
	for (int32 i = 0; i < MainPathNodes.Num() - 1; i++)
	{
		int32 Current = MainPathNodes[i];
		int32 Next = MainPathNodes[i + 1];

		//Place next room at an empty neighbor
		FIntPoint ConnectedCoordinates = GetRandomEmptyNeighbor(LevelMap[Current].Coordinates,
		                                                        OccupiedCoordinates);
		LevelMap[Next].Coordinates = ConnectedCoordinates;
		OccupiedCoordinates.Add(ConnectedCoordinates);

		//Link them together
		LevelMap[Current].ConnectedRoomsIDs.Add(Next);
		LevelMap[Next].ConnectedRoomsIDs.Add(Current);
		UE_LOG(LogTemp, Warning, TEXT("Node %d connected to %d"), Current, Next)
	}

	/* --- 4) Attach special rooms if we have them (not on the exit) --- */
	if (SpecialNodes.Num() > 0)
	{
		for (int32 Node : SpecialNodes)
		{
			const int32 ConnectToID = MainPathNodes[FMath::RandRange(0, MainPathNodes.Num() - 2)];
			const FIntPoint SpecialCoordinates = GetRandomEmptyNeighbor(LevelMap[ConnectToID].Coordinates,
			                                                            OccupiedCoordinates);

			LevelMap[Node].Coordinates = SpecialCoordinates;
			OccupiedCoordinates.Add(SpecialCoordinates);

			LevelMap[Node].ConnectedRoomsIDs.Add(ConnectToID);
			LevelMap[ConnectToID].ConnectedRoomsIDs.Add(Node);
			UE_LOG(LogTemp, Warning, TEXT("Special Room connected to Node %d"), ConnectToID)
		}
	}

	/* --- 6) Assign Layouts to Rooms --- */
	for (auto& Room : LevelMap)
	{
		Room.Value.Layout = RoomsPool->GetRandomRoomByType(Room.Value.RoomType);
	}

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
	case EDoorDirection::North: return EDoorDirection::South;
	case EDoorDirection::South: return EDoorDirection::North;
	case EDoorDirection::East: return EDoorDirection::West;
	case EDoorDirection::West: return EDoorDirection::East;
	default: return EDoorDirection::None;
	}
}
