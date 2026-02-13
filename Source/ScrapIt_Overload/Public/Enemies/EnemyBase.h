// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/Damageable.h"
#include "EnemyBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeathData, FVector, DeathLocation, int32, ScrapAmount);

UENUM()
enum class EState : uint8
{
	Idle,
	Chasing,
	Attacking,
	Cooldown,
	Hurt
};

UCLASS()
class SCRAPIT_OVERLOAD_API AEnemyBase : public APawn, public IDamageable
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AEnemyBase();

protected:
	UPROPERTY(EditAnywhere, Category = "Enemy Settings")
	float BaseHealth;
	
	UPROPERTY(EditAnywhere, Category = "Enemy Settings")
	float Damage = 5.0f;
	
	UPROPERTY(EditAnywhere, Category = "Enemy Settings")
	float MoveSpeed = 500.0f;
	
	UPROPERTY(EditAnywhere, Category = "Enemy Settings")
	int32 BaseDropAmount;
	
	UPROPERTY(VisibleAnywhere)
	float CurrentHealth;
	
	UPROPERTY(VisibleAnywhere)
	EState CurrentState = EState::Idle;
	
	UPROPERTY()
	APawn* MechaTarget;
	
	UFUNCTION()
	virtual void Die() override;
	
	virtual void TakeDamage(const float DamageAmount) override PURE_VIRTUAL(AEnemyBase::TakeDamage,);

public:	
	FOnDeathData OnDeath;
};
