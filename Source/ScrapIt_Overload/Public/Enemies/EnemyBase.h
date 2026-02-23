// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HealthComponent.h"
#include "AIController.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/Damageable.h"
#include "EnemyBase.generated.h"


UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Idle,
	Chasing,
	Attacking,
	Cooldown,
	Hurt,
	Dead
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeathData, FVector, DeathLocation, int32, ScrapAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStateChanged, EEnemyState, OldState, EEnemyState, NewState);

UCLASS()
class SCRAPIT_OVERLOAD_API AEnemyBase : public APawn, public IDamageable
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AEnemyBase();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* Mesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* HurtboxSphere;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UFloatingPawnMovement* PawnMovement;
	
	UPROPERTY(EditAnywhere, Category = "Enemy Settings")
	class UBehaviorTree* BehaviorTree;
	
	UPROPERTY(EditAnywhere, Category = "Enemy Settings")
	float BaseHealth;
	
	UPROPERTY(EditAnywhere, Category = "Enemy Settings")
	float MoveSpeed = 500.0f;
	
	UPROPERTY(EditAnywhere, Category = "Enemy Settings")
	int32 BaseDropAmount;
	
	UPROPERTY(EditAnywhere, Category = "Enemy Settings")
	float AttackDirectionThreshold = 0.8f;
	
	UPROPERTY(EditAnywhere, Category = "Enemy Settings")
	float Damage = 5.0f;
	
	UPROPERTY(EditAnywhere, Category = "Enemy Settings")
	float DamageTraceRadius = 50.f;
	
	UPROPERTY(EditAnywhere, Category = "Enemy Settings")
	float DamageTraceDistance = 100.f;
	
	UPROPERTY(VisibleAnywhere)
	UHealthComponent* VitalityComponent;
	
	UPROPERTY()
	AAIController* AIController;
	
	UPROPERTY()
	APawn* Player;
	
	UPROPERTY(VisibleAnywhere)
	EEnemyState CurrentState = EEnemyState::Idle;
	
public:
	//Functions
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool IsFacingPlayer() const;
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void Attack() PURE_VIRTUAL(AEnemyBase::Attack,);
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void TriggerDamageTrace();
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	AAIController* GetAIController() const
	{
		return AIController;
	}
	
	UFUNCTION(BlueprintCallable)
	void SetState(const EEnemyState NewState);
	
	UFUNCTION()
	virtual void Die() override;
	
	UFUNCTION()
	virtual void TakeDamage(const float DamageAmount) override;
	
	//Events
	UPROPERTY(BlueprintAssignable)
	FOnStateChanged OnStateChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnDeathData OnDeath;
};
