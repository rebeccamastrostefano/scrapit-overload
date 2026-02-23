// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/HealthComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::InitializeHealth(const float BaseHealth)
{
	MaxHealth = BaseHealth;
	CurrentHealth = MaxHealth;
}

void UHealthComponent::ApplyDamage(const float DamageAmount)
{
	if (CurrentHealth <= 0)
	{
		return;
	}
	
	CurrentHealth -= DamageAmount;
	OnDamageReceived.Broadcast(DamageAmount);
	
	if (CurrentHealth <= 0)
	{
		OnDeath.Broadcast();
	}
}



