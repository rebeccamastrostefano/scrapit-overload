// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PersistentManager.generated.h"

UENUM(BlueprintType)
enum class EWeaponSocket : uint8
{
	Front,
	Back,
	Left,
	Right
};

USTRUCT(BlueprintType)
struct FMassTier
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	int8 TierNumber;
	
	UPROPERTY(EditAnywhere)
	int32 ScrapThreshold;
	
	UPROPERTY(EditAnywhere, meta = (UIMin = "0.1", ClampMin = "0.1", ClampMax = "1.0"))
	float SpeedPenalty;
	
	UPROPERTY(EditAnywhere, meta = (UIMin = "0.1", ClampMin = "0.1", ClampMax = "1.0"))
	float SteeringPenalty;
};

USTRUCT(BlueprintType)
struct FWeaponData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	TSubclassOf<AActor> WeaponClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	int32 CurrentLevel = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	EWeaponSocket Socket = EWeaponSocket::Back;
};

USTRUCT(BlueprintType)
struct FMechaRunState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mecha Data")
	int CurrentScraps = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mecha Data")
	int32 CurrentTier = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mecha Data")
	TArray<FWeaponData> WeaponLoadout;
};

UCLASS()
class SCRAPIT_OVERLOAD_API UPersistentManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
protected:
	//Run Data
	UPROPERTY(BlueprintReadOnly, Category = "Run Data")
	int32 CurrentRoomRank = 1;
	
	UPROPERTY(BlueprintReadOnly, Category = "Run Data")
	FMechaRunState CurrentMechaState;
	
	UPROPERTY(BlueprintReadOnly, Category = "Run Data")
	int32 TotalGoldenScraps = 0;
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	//GETTERS
	UFUNCTION(BlueprintPure, Category = "Run Data")
	int32 GetRoomRank() const { return CurrentRoomRank; };
	
	UFUNCTION(BlueprintPure, Category = "Run Data")
	FMechaRunState GetMechaState() const { return CurrentMechaState; }
	
	//SETTERS
	UFUNCTION(BlueprintCallable, Category = "Run Data")
	void SaveMechaState(float Scraps, int32 Tier, const TArray<FWeaponData>& Weapons)
	{
		CurrentMechaState.CurrentScraps = Scraps;
		CurrentMechaState.CurrentTier = Tier;
		CurrentMechaState.WeaponLoadout = Weapons;
	}
	
	UFUNCTION(BlueprintCallable, Category = "Run Data")
	void AdvanceRoom() { CurrentRoomRank++; };
	
	UFUNCTION(BlueprintCallable, Category = "Run Data")
	void ResetRun()
	{
		CurrentRoomRank = 1;
		CurrentMechaState = FMechaRunState();
	};
	
	UFUNCTION(BlueprintCallable, Category = "Run Data")
	void AddGoldenScrap(int32 Amount) { TotalGoldenScraps += Amount; };
};
