// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageReceived, float, DamageAmount);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SCRAPIT_OVERLOAD_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();
	
protected:
	float MaxHealth;
	float CurrentHealth;
	
public:
	void InitializeHealth(const float BaseHealth);
	void ApplyDamage(const float DamageAmount);
	
	float GetCurrentHealth() const
	{
		return CurrentHealth;
	}
	
	//Events
	FOnDeath OnDeath;
	FOnDamageReceived OnDamageReceived;
};
