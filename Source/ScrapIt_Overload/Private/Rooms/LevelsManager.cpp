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

	//Decide if level should have a special room
	bool bHasSpecialRoom = false;
	int32 SpecialNode = -1;
	if (NumRooms > 3 && FMath::RandBool())
	{
		bHasSpecialRoom = true;
		SpecialNode = NumRooms - 2;
		LevelMap[SpecialNode].RoomType = ERoomType::Special;
	}

	/* --- 2) Connect Rooms --- */
	TArray<int32> MainPathNodes;
	MainPathNodes.Add(0);

	//Randomly pick some middle rooms to form the main path (guarantees reaching exit)
	for (int32 i = 1; i < NumRooms - 2; i++)
	{
		if (FMath::RandBool())
		{
			MainPathNodes.Add(i);
		}
	}
	MainPathNodes.Add(NumRooms - 1);

	//Link the rooms on main path
	for (int32 i = 0; i < MainPathNodes.Num() - 1; i++)
	{
		int32 Current = MainPathNodes[i];
		int32 Next = MainPathNodes[i + 1];

		LevelMap[Current].ConnectedRoomsIDs.Add(Next);
		LevelMap[Next].ConnectedRoomsIDs.Add(Current);
	}

	//Attach special room if we have it (not on the exit)
	if (bHasSpecialRoom)
	{
		const int32 RandomIndex = FMath::RandRange(0, MainPathNodes.Num() - 2);
		const int32 SpecialRoomConnection = MainPathNodes[RandomIndex];

		LevelMap[SpecialNode].ConnectedRoomsIDs.AddUnique(SpecialRoomConnection);
		LevelMap[SpecialRoomConnection].ConnectedRoomsIDs.AddUnique(SpecialNode);
	}

	/* 3) Assign Layouts to Rooms --- */
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

void ULevelsManager::MarkRoomAsVisited(const int32 RoomID)
{
	LevelMap[RoomID].bIsVisited = true;
}

void ULevelsManager::LoadRoomByID(const int32 RoomID) const
{
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, LevelMap[RoomID].Layout);
}
