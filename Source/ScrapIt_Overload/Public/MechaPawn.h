// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "Interfaces/Damageable.h"
#include "MechaPawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScrapCountChanged, int32, NewScrapCount);

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

UENUM()
enum class EWeaponSocket : uint8
{
	Front,
	Back,
	Left,
	Right
};

UCLASS()
class SCRAPIT_OVERLOAD_API AMechaPawn : public APawn, public IDamageable
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMechaPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// INPUT
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* ThrustAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* TurnAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* MagnetAction;
	
	void ApplyThrust(const FInputActionValue& Value);
	void ApplySteer(const FInputActionValue& Value);
	
	// COMPONENTS
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MechaMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USpringArmComponent* SpringArm;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCameraComponent* Camera;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Components")
	class UBoxComponent* Hurtbox;
	
	//Sockets
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mecha Sockets")
	USceneComponent* SocketFront;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mecha Sockets")
	USceneComponent* SocketBack;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mecha Sockets")
	USceneComponent* SocketLeft;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mecha Sockets")
	USceneComponent* SocketRight;
	
	// MOVEMENT SETTINGS
	UPROPERTY(EditAnywhere, Category = "Mecha Movement")
	float BaseAccelerationForce = 400000.0f;
	
	float CurrentSteerAngle = 0.0f;
	float TargetSteerAngle = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = "Mecha Movement")
	float MaxSteerAngle = 35.0f;
	
	UPROPERTY(EditAnywhere, Category = "Mecha Movement")
	float BaseSteeringSpeed = 20.0f;
	
	UPROPERTY(EditAnywhere, Category = "Mecha Movement")
	float HandlingFactor = 0.05f;
	
	UPROPERTY(EditAnywhere, Category = "Mecha Movement")
	float GripStrength = 0.5f;
	
	// CAMERA SETTINGS
	UPROPERTY(EditAnywhere, Category = "Mecha Camera")
	float CameraSmoothness = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Mecha Camera")
	float MaxCameraLagDistance = 100.0f;
	
	// MAGNET SETTINGS
	UPROPERTY(EditAnywhere, Category = "Mecha Magnet")
	float MagnetStrength = 2.0f;
	
	UPROPERTY(EditAnywhere, Category = "Mecha Magnet")
	float MagnetRadius = 50.0f;
	
	UPROPERTY(EditAnywhere, Category = "Mecha Magnet")
	float CollectionRadius = 150.0f;
	
	UPROPERTY(EditAnywhere, meta = (UIMin = "0.1", ClampMin = "0.1", ClampMax = "1.0"), Category = "Mecha Magnet")
	float MagnetSpeedDecrease = 0.5f;
	
	bool bIsMagnetActive = false;

	// Movement Functions
	void UpdateThrust(float Value);
	void UpdateSteer(float DeltaTime);
	void ApplyLateralFriction();
	
	//Magnet Functions
	void ActivateMagnet();
	void ToggleMagnet();
	
	//Stats
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mecha Stats")
	int32 CurrentScraps = 0;
	
	float CurrentAcceleration = 0.0f;
	float CurrentSteerSpeed = 0.0f;
	
	//Tiers Management
	UPROPERTY(EditAnywhere, Category = "Mecha Mass Tiers")
	TArray<FMassTier> MassTiers;
	
	UPROPERTY(VisibleAnywhere, Category = "Mecha Mass Tiers")
	TArray<UStaticMeshComponent*> MassMeshParts;
	
	UPROPERTY(VisibleAnywhere, Category = "Mecha Mass Tiers")
	FMassTier CurrentTier;
	
	void CheckTierUpgrade();
	void UpdateMassStats(const FMassTier& Tier);
	void UpdateMassVisuals(const FMassTier& Tier);
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	//Scrap Management
	void AddScrap(int32 Amount);
	
	UFUNCTION(BlueprintPure, Category = "Mecha Stats")
	int32 GetCurrentScrapCount() const
	{
		return CurrentScraps;
	}
	
	UPROPERTY(BlueprintAssignable, Category = "Mecha Stats")
	FOnScrapCountChanged OnScrapCountChanged;
	
	//Weapons
	void EquipWeapon(TSubclassOf<AActor> WeaponClass, EWeaponSocket Socket);
	
	UPROPERTY(VisibleAnywhere, Category = "Mecha Weapons")
	TArray<AActor*> EquippedWeapons;
	
	void TakeDamage(float Amount);
};
