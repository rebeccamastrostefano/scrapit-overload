// Fill out your copyright notice in the Description page of Project Settings.


#include "Rooms/LevelsManager.h"
#include "Core/PersistentManager.h"
#include "Core/ScrapItGameInstance.h"

void ULevelsManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(UPersistentManager::StaticClass());

	GameInstance = Cast<UScrapItGameInstance>(GetGameInstance());
	check(GameInstance != nullptr);

	RoomsPool = GameInstance->RoomsPool;

	PersistentManager = GetGameInstance()->GetSubsystem<UPersistentManager>();
	check(PersistentManager != nullptr);

	OnNewLevelGenerated.AddDynamic(PersistentManager, &UPersistentManager::AdvanceLevel);
	UE_LOG(LogTemp, Warning, TEXT("Subscribed to Level Generation"));
}

void ULevelsManager::Deinitialize()
{
	Super::Deinitialize();
}

/* --- RUN GENERATION --- */
void ULevelsManager::GenerateRun()
{
	UE_LOG(LogTemp, Warning, TEXT("Generating Run..."));
	RunMap.Empty();
	int32 IDCounter = 0;

	//1. Create Ranks
	for (int32 i = 0; i < 10; i++)
	{
		const int32 NodesInRank = i == 0 || i == 9 ? 1 : FMath::RandRange(1, 3);
		FLevelRank NewRank;

		for (int32 j = 0; j < NodesInRank; j++)
		{
			FLevelNode NewNode;
			NewNode.LevelID = IDCounter;
			NewNode.LevelType = i == 0
				                    ? ELevelType::Standard
				                    : (i == 9)
				                    ? ELevelType::FinalBoss
				                    : GameInstance->LevelPool->GetRandomLevelType();
			NewRank.Levels.Add(NewNode);

			IDCounter++;
		}

		RunMap.Add(NewRank);
		UE_LOG(LogTemp, Warning, TEXT("Created Rank %d with %d levels"), i, NodesInRank);
	}

	//2. Link Ranks
	for (int32 i = 0; i < 9; i++)
	{
		for (FLevelNode& Parent : RunMap[i].Levels)
		{
			ConnectToNextRank(Parent, RunMap[i + 1].Levels);
		}

		//Safety check to ensure all levels are reachable
		EnsureLevelConnections(i, i + 1);
	}
}

void ULevelsManager::ConnectToNextRank(FLevelNode& LevelToConnect, TArray<FLevelNode> NextRankLevels)
{
	if (NextRankLevels.Num() == 0)
	{
		//if no next rank (final level), do nothing
		return;
	}

	//1. Decide how many children this node will have (1 or 2)
	//Ensure we don't try to connect to 2 nodes if we only have one
	const int32 MaxConnections = FMath::Min(2, NextRankLevels.Num());
	const int32 NumConnections = FMath::RandRange(1, MaxConnections);

	//2. Pick random unique indices of the level rank to connect the level to
	TArray<int32> ChildIndices;
	while (ChildIndices.Num() < NumConnections)
	{
		const int32 RandomIndex = FMath::RandRange(0, NextRankLevels.Num() - 1);
		if (!ChildIndices.Contains(RandomIndex))
		{
			ChildIndices.Add(RandomIndex);
			LevelToConnect.ConnectedLevels.Add(NextRankLevels[RandomIndex].LevelID);
		}
	}
}

void ULevelsManager::EnsureLevelConnections(const int32 ParentRank, const int32 NextRank)
{
	TArray<FLevelNode>& ParentLevels = RunMap[ParentRank].Levels;
	TArray<FLevelNode>& ChildLevels = RunMap[NextRank].Levels;

	for (FLevelNode& Child : ChildLevels)
	{
		bool bHasParent = false;

		for (const FLevelNode& Parent : ParentLevels)
		{
			if (Parent.ConnectedLevels.Contains(Child.LevelID))
			{
				bHasParent = true;
				break;
			}
		}

		if (!bHasParent)
		{
			//Pick a random parent and connect it
			const int32 RandomParentIndex = FMath::RandRange(0, ParentLevels.Num() - 1);
			ParentLevels[RandomParentIndex].ConnectedLevels.Add(Child.LevelID);
		}
	}
}

void ULevelsManager::SelectNextLevel(int32 TargetLevelID)
{
	if (!RunMap.IsValidIndex(PersistentManager->GetLevelRank()))
	{
		UE_LOG(LogTemp, Error, TEXT("LevelsManager: No Rank found with index %d!"), PersistentManager->GetLevelRank())
		return;
	}

	TArray<FLevelNode>& RankLevels = RunMap[PersistentManager->GetLevelRank()].Levels;
	FLevelNode* SelectedLevel = RankLevels.FindByPredicate([TargetLevelID](const FLevelNode& Level)
	{
		return Level.LevelID == TargetLevelID;
	});

	if (SelectedLevel == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("LevelsManager: No Level found with ID %d!"), TargetLevelID)
		return;
	}
	CurrentLevelID = TargetLevelID;
	SelectedLevel->LevelType == ELevelType::Standard
		? GenerateStandardLevel()
		: GenerateSpecialLevel(SelectedLevel->LevelType);
	TransitionToRoomByID(0);
}

/* --- LEVEL GENERATION --- */
void ULevelsManager::GenerateStandardLevel()
{
	int32 NumRooms = FMath::RandRange(3, 3);
	if (RoomsPool == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("LevelsManager: Rooms Pool is Missing!"));
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
		Room.Value.Map = RoomsPool->GetRandomRoomByType(Room.Value.RoomType);
	}

	OnNewLevelGenerated.Broadcast();
	UE_LOG(LogTemp, Warning, TEXT("Level Generation Complete with %d Rooms"), NumRooms);
}

void ULevelsManager::GenerateSpecialLevel(const ELevelType LevelType)
{
	LevelMap.Empty();
	CurrentRoomID = 0;

	CreateBaseLevelMap(2);
	LevelMap[0].RoomType = ERoomType::Special;

	TArray<FIntPoint> Coordinates;
	ConnectRooms(0, 1, Coordinates);

	const TSoftObjectPtr<UWorld> Map = GameInstance->LevelPool->GetLevelMap(LevelType);
	LevelMap[0].Map = Map;
	LevelMap[1].Map = RoomsPool->GetRandomRoomByType(ERoomType::Exit);

	OnNewLevelGenerated.Broadcast();
}

void ULevelsManager::CreateBaseLevelMap(const int32 NumRooms)
{
	for (int32 i = 0; i < NumRooms; ++i)
	{
		FRoomNode NewNode;

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
	if (NumRooms == 4)
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

void ULevelsManager::TransitionToRoomByID(const int32 RoomID)
{
	GameInstance->LoadLevel(LevelMap[RoomID].Map);
}
