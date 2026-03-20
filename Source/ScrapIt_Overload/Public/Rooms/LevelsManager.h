// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LevelPool.h"
#include "RoomsPool.h"
#include "Core/PersistentManager.h"
#include "Core/ScrapItGameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LevelsManager.generated.h"

class ARoomLayout;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNewLevelGenerated);

USTRUCT(BlueprintType)
struct FLevelNode
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 LevelID = 0;

	UPROPERTY(BlueprintReadOnly)
	ELevelType LevelType;

	UPROPERTY(BlueprintReadOnly)
	TArray<int32> ConnectedLevels;
};

USTRUCT(BlueprintType)
struct FLevelRank
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<FLevelNode> Levels;
};

USTRUCT(BlueprintType)
struct FRoomNode
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FIntPoint Coordinates;

	UPROPERTY(BlueprintReadOnly)
	ERoomType RoomType = ERoomType::Combat;

	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UWorld> Map;

	UPROPERTY(BlueprintReadOnly)
	TArray<int32> ConnectedRoomsIDs;

	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<ARoomLayout> RoomLayout;

	UPROPERTY(BlueprintReadOnly)
	bool bIsVisited = false;
};

UENUM(BlueprintType)
enum EDoorDirection : uint8
{
	North,
	South,
	East,
	West,
	None
};

UCLASS()
class SCRAPIT_OVERLOAD_API ULevelsManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:
	//Run Generation
	UPROPERTY(BlueprintReadOnly, Category = "Run Generation")
	TArray<FLevelRank> RunMap;

	UPROPERTY(BlueprintReadOnly, Category = "Run Generation")
	int32 CurrentLevelID = 0;

	//Level Generation
	UPROPERTY(BlueprintReadOnly, Category = "Level Generation")
	TMap<int32, FRoomNode> LevelMap;

	UPROPERTY(BlueprintReadOnly, Category = "Level Generation")
	ELevelType CurrentLevelType;

	UPROPERTY(BlueprintReadOnly, Category = "Level Generation")
	int32 CurrentRoomID = 0;

	UPROPERTY()
	UScrapItGameInstance* GameInstance;

	UPROPERTY()
	UPersistentManager* PersistentManager;

	UPROPERTY()
	URoomsPool* RoomsPool;

	EDoorDirection LastExitDirection = None;

	const TArray<FIntPoint> Directions = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};

	void GenerateSpecialLevel(const ELevelType LevelType);
	void CreateBaseLevelMap(const int32 NumRooms);
	void SetupSpecialRooms(const int32 NumRooms, TArray<int32>& OutSpecialNodes);
	void ConnectRooms(const int32 From, const int32 To, TArray<FIntPoint>& OccupiedCoordinates);

	//Helper functions
	FIntPoint GetRandomEmptyNeighbor(const FIntPoint Origin, const TArray<FIntPoint>& Occupied) const;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//Run Generation
	UFUNCTION(BlueprintCallable, Category = "Run Generation")
	void GenerateRun();

	UFUNCTION(Category = "Run Generation")
	void ConnectToNextRank(FLevelNode& LevelToConnect, TArray<FLevelNode> NextRankLevels);

	UFUNCTION(Category = "Run Generation")
	void EnsureLevelConnections(const int32 ParentRank, const int32 NextRank);

	UFUNCTION(BlueprintCallable, Category = "Run Generation")
	void SelectNextLevel(int32 TargetLevelID);

	//Level Generation
	UFUNCTION(BlueprintCallable, Category = "Level Generation")
	void GenerateStandardLevel();

	UFUNCTION(BlueprintCallable, Category = "Level Generation")
	void MarkRoomAsVisited(const int32 RoomID);

	UFUNCTION(BlueprintCallable, Category = "Level Generation")
	void TransitionToRoomByID(const int32 RoomID);

	//GETTERS
	UFUNCTION(BlueprintCallable, Category = "Run Generation")
	FORCEINLINE TArray<FLevelRank>& GetRunMap()
	{
		return RunMap;
	}

	UFUNCTION(BlueprintCallable, Category = "Level Generation")
	FORCEINLINE TMap<int32, FRoomNode>& GetLevelMap()
	{
		return LevelMap;
	}

	UFUNCTION(BlueprintCallable, Category = "Level Generation")
	FORCEINLINE int32 GetCurrentRoomID() const
	{
		return CurrentRoomID;
	}

	UFUNCTION(BlueprintCallable, Category = "Level Generation")
	FORCEINLINE EDoorDirection GetLastExitDirection() const
	{
		return LastExitDirection;
	}

	UFUNCTION(BlueprintCallable, Category = "Level Generation")
	FORCEINLINE ELevelType GetLevelType() const
	{
		return CurrentLevelType;
	}

	UFUNCTION(BlueprintCallable, Category = "Level Generation")
	FORCEINLINE FLevelNode GetLevelNodeByID(const int32 LevelID)
	{
		const FLevelRank* Rank = RunMap.FindByPredicate([=](const FLevelRank& R)
		{
			return R.Levels.ContainsByPredicate([=](const FLevelNode& Node)
			{
				return Node.LevelID == LevelID;
			});
		});

		return *Rank->Levels.FindByPredicate([=](const FLevelNode& Node)
		{
			return Node.LevelID == LevelID;
		});
	}

	//SETTERS
	UFUNCTION()
	FORCEINLINE void SetCurrentRoomID(const int32 RoomID)
	{
		CurrentRoomID = RoomID;
	}

	UFUNCTION()
	FORCEINLINE void SetLastExitDirection(const EDoorDirection Direction)
	{
		LastExitDirection = Direction;
	}

	//Helper Functions
	FORCEINLINE static EDoorDirection GetOppositeDoorDirection(const EDoorDirection& Direction)
	{
		switch (Direction)
		{
		case North: return South;
		case South: return North;
		case East: return West;
		case West: return East;
		default: return None;
		}
	};

	//Events
	UPROPERTY(BlueprintAssignable, Category = "Level Generation")
	FOnNewLevelGenerated OnNewLevelGenerated;
};
