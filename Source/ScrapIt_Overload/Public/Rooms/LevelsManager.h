// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LevelPool.h"
#include "RoomsPool.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LevelsManager.generated.h"

USTRUCT(BlueprintType)
struct FRoomNode
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 RoomID = -1;

	UPROPERTY(BlueprintReadOnly)
	FIntPoint Coordinates;

	UPROPERTY(BlueprintReadOnly)
	ERoomType RoomType = ERoomType::Combat;

	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UWorld> Layout;

	UPROPERTY(BlueprintReadOnly)
	TArray<int32> ConnectedRoomsIDs;

	UPROPERTY(BlueprintReadOnly)
	bool bIsVisited = false;
};

UCLASS()
class SCRAPIT_OVERLOAD_API ULevelsManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Level Generation")
	TMap<int32, FRoomNode> LevelMap;

	UPROPERTY(BlueprintReadOnly, Category = "Level Generation")
	ELevelType CurrentLevelType;

	UPROPERTY(BlueprintReadOnly, Category = "Level Generation")
	int32 CurrentRoomID = 0;

	UPROPERTY()
	URoomsPool* RoomsPool;

	void CreateBaseLevelMap(const int32 NumRooms);

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Level Generation")
	void GenerateLevel(const int32 NumRooms);

	UFUNCTION(BlueprintCallable, Category = "Level Generation")
	void MarkRoomAsVisited(const int32 RoomID);

	UFUNCTION(BlueprintCallable, Category = "Level Generation")
	void LoadRoomByID(const int32 RoomID) const;

	//GETTERS
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
	FORCEINLINE ELevelType GetLevelType() const
	{
		return CurrentLevelType;
	}
};
