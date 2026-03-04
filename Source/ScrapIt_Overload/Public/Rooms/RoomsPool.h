// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RoomsPool.generated.h"

USTRUCT(BlueprintType)
struct FRoomArray
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<TSoftObjectPtr<UWorld>> Rooms;
};

UENUM(BlueprintType)
enum class ERoomType : uint8
{
	Start,
	Combat,
	Special,
	Exit,
};


UCLASS()
class SCRAPIT_OVERLOAD_API URoomsPool : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Level Generation")
	TMap<ERoomType, FRoomArray> RoomPool;

	UFUNCTION(BlueprintCallable, Category = "Level Generation")
	TSoftObjectPtr<UWorld> GetRandomRoomByType(const ERoomType Type);
};
