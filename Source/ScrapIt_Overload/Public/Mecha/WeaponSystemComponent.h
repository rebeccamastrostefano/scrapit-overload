// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapons/WeaponBase.h"
#include "Core/PersistentManager.h"
#include "WeaponSystemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponAcquired, EScrapType, WeaponScrapType, int32, WeaponLevel);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SCRAPIT_OVERLOAD_API UWeaponSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponSystemComponent();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mecha Weapons")
	TMap<EWeaponSocket, USceneComponent*> Sockets;
	
	UPROPERTY(VisibleAnywhere, Category = "Mecha Weapons")
	TArray<FWeaponData> WeaponLoadout;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mecha Weapons")
	TMap<EWeaponSocket, AWeaponBase*> SocketsToWeapons;
	
	// Functions
	void LoadWeaponLoadout(TArray<FWeaponData> SavedWeaponLoadout);
	void NotifyWeaponAcquired(const EScrapType WeaponScrapType, const int32 WeaponLevel) const;
	
	UFUNCTION(BlueprintCallable, Category = "Mecha Weapons")
	void EquipWeaponTypeToSocket(const EScrapType WeaponScrapType, const EWeaponSocket Socket, const int32 WeaponLevel);
	
	void DropWeaponOnSocket(const EWeaponSocket Socket);
	
	UFUNCTION()
	void UpgradeAllWeapons(FMassTier Tier);
	
	//Events
	UPROPERTY(BlueprintAssignable, Category = "Mecha Weapons")
	FOnWeaponAcquired OnWeaponAcquired;
	
	//Helpers
	UFUNCTION(BlueprintPure, Category = "Mecha Helpers")
	TArray<EWeaponSocket> GetAvailableSockets() const;
	
};
