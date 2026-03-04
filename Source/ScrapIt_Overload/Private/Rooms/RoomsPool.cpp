// Fill out your copyright notice in the Description page of Project Settings.


#include "Rooms/RoomsPool.h"

TSoftObjectPtr<UWorld> URoomsPool::GetRandomRoomByType(const ERoomType Type)
{
	if (const FRoomArray* FoundRooms = RoomPool.Find(Type))
	{
		if (FoundRooms->Rooms.Num() > 0)
		{
			const int32 RandomIndex = FMath::RandRange(0, FoundRooms->Rooms.Num() - 1);
			return FoundRooms->Rooms[RandomIndex];
		}
	}
	return nullptr;
}
