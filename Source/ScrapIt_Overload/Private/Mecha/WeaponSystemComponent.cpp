// Fill out your copyright notice in the Description page of Project Settings.


#include "Mecha/WeaponSystemComponent.h"

#include "Core/ScrapItGameInstance.h"

// Sets default values for this component's properties
UWeaponSystemComponent::UWeaponSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponSystemComponent::LoadWeaponLoadout(TArray<FWeaponData> SavedWeaponLoadout)
{
	for (const auto& [Type, CurrentLevel, Socket] : SavedWeaponLoadout)
	{
		EquipWeaponTypeToSocket(Type, Socket, CurrentLevel);
	}
}

void UWeaponSystemComponent::NotifyWeaponAcquired(const EScrapType WeaponScrapType, const int32 WeaponLevel) const
{
	OnWeaponAcquired.Broadcast(WeaponScrapType, WeaponLevel);
}

void UWeaponSystemComponent::EquipWeaponTypeToSocket(const EScrapType WeaponScrapType, const EWeaponSocket Socket, const int32 WeaponLevel)
{
	USceneComponent* AttachSocket = Sockets.FindRef(Socket);
	if (AttachSocket == nullptr)
	{
		return;
	}
	
	UScrapItGameInstance* GameInstance = GetWorld()->GetGameInstance<UScrapItGameInstance>();
	if (GameInstance == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Error: No Game Instance"));
		return;
	}
	
	//If we are equipping a weapon on an occupied socket, drop the old one
	if (SocketsToWeapons.Contains(Socket))
	{
		DropWeaponOnSocket(Socket);
	}
	
	//Spawn new weapon and attach to socket
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = GetOwner();
	const TSubclassOf<AWeaponBase> WeaponBP = GameInstance->ScrapTypeToWeaponBP[WeaponScrapType];
	
	if (AWeaponBase* NewWeapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponBP, AttachSocket->GetComponentTransform(), SpawnInfo))
	{
		NewWeapon->AttachToComponent(AttachSocket, FAttachmentTransformRules::KeepWorldTransform);
		WeaponLoadout.Add(FWeaponData{WeaponScrapType, WeaponLevel, Socket});
		SocketsToWeapons.Add(Socket, NewWeapon);
		NewWeapon->TryUpgrade(WeaponLevel); //Set the level of the weapon
	}
}

void UWeaponSystemComponent::DropWeaponOnSocket(const EWeaponSocket Socket)
{
	UScrapItGameInstance* GameInstance = GetWorld()->GetGameInstance<UScrapItGameInstance>();
	if (GameInstance == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Error: No Game Instance"));
		return;
	}
	
	if (AWeaponBase* WeaponActor = SocketsToWeapons.FindRef(Socket))
	{
		//Destroy the old weapon actor
		WeaponActor->Destroy();
		SocketsToWeapons.Remove(Socket);
	}
	
	//Find the weapon data index in the Loadout
	const int32 Index = WeaponLoadout.IndexOfByPredicate([Socket](const FWeaponData& Data)
	{
		return Data.Socket == Socket;
	});
	
	if (Index != INDEX_NONE)
	{
		//Get the old weapon class to drop as scrap
		const FWeaponData OldWeaponData = WeaponLoadout[Index];
		
		if (GameInstance->ScrapTypeToBP.Contains(OldWeaponData.ScrapWeaponType))
		{
			//Drop the old weapon on ground as scrap
			const TSubclassOf<AScrapActor> OldWeaponScrapClass = GameInstance->ScrapTypeToBP[OldWeaponData.ScrapWeaponType];
			if (AScrapActor* WeaponScrap = GetWorld()->SpawnActor<AScrapActor>(OldWeaponScrapClass, GetOwner()->GetActorTransform()))
			{
				WeaponScrap->InitWeaponScrap(OldWeaponData.ScrapWeaponType, OldWeaponData.CurrentLevel);
			}
		}
		
		//Remove the old weapon from the loadout
		WeaponLoadout.RemoveAtSwap(Index);
	}
	
	//Remove the swapped weapon from loadout
	WeaponLoadout.RemoveAll([&](const FWeaponData& Data) { return Data.Socket == Socket; });
}

void UWeaponSystemComponent::UpgradeAllWeapons(FMassTier Tier)
{
	//Check if we should upgrade weapons
	for (FWeaponData& Data : WeaponLoadout)
	{
		if (AWeaponBase* Weapon = SocketsToWeapons.FindRef(Data.Socket))
		{
			if (Weapon->TryUpgrade(Tier.TierNumber))
			{
				Data.CurrentLevel = Tier.TierNumber;
			}
		}
	}
}

TArray<EWeaponSocket> UWeaponSystemComponent::GetAvailableSockets() const
{
	TArray AvailableSockets = { EWeaponSocket::Front, EWeaponSocket::Back, EWeaponSocket::Left, EWeaponSocket::Right };
	
	AvailableSockets.RemoveAll([&](const EWeaponSocket Socket)
	{
		return SocketsToWeapons.Contains(Socket);
	});
	return AvailableSockets;
}

