// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/PersistentManager.h"
#include "TierSystemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTierChanged, FMassTier, Tier);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SCRAPIT_OVERLOAD_API UTierSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTierSystemComponent();
	
	UPROPERTY(EditAnywhere, Category = "Mecha Settings")
	TArray<FMassTier> MassTiers;
	
	UPROPERTY(VisibleAnywhere, Category = "Mecha Settings")
	TMap<UStaticMeshComponent*, int32> MassMeshesForTiers;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mecha State")
	FMassTier CurrentTier;
	
	//Functions
	void InitializeTierSystem();
	void LoadTierState(int32 SavedTierNumber);
	
	UFUNCTION()
	void CheckForTierChange(const int32 CurrentScraps);
	
	void ApplyNewTier(const FMassTier& Tier);
	void UpdateTierVisuals(const FMassTier& Tier);
	
	//Events
	UPROPERTY(BlueprintAssignable, Category = "Mecha State")
	FOnTierChanged OnTierChanged;
	
	//Helpers
	UFUNCTION(BlueprintPure, Category = "Mecha Helpers")
	FMassTier GetTierByNumber(const int32 TierNumber) const;
};
