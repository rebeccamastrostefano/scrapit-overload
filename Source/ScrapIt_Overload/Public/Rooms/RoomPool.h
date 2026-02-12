// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RoomPool.generated.h"

UENUM(BlueprintType)
enum class ERoomType : uint8
{
	Standard,
	Shop,
	Maintenance,
	Treasure,
	Gamble,
	Random
};

USTRUCT()
struct FRoomWeight
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	ERoomType Room;
	
	UPROPERTY(EditAnywhere)
	float Weight;
};
UCLASS()
class SCRAPIT_OVERLOAD_API URoomPool : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TArray<FRoomWeight> Rooms;
	
	ERoomType GetRandomRoomType();
};
