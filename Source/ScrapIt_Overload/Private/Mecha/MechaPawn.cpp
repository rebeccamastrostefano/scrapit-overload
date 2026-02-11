// Fill out your copyright notice in the Description page of Project Settings.


#include "Mecha/MechaPawn.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Interfaces/Scrappable.h"
#include "Components/BoxComponent.h"
#include "Core/ScrapItGameInstance.h"
#include "Engine/OverlapResult.h"

// Sets default values
AMechaPawn::AMechaPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// Mesh creation
	MechaMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MechaMesh"));
	RootComponent = MechaMesh;
	
	// Enable Physics
	MechaMesh->SetSimulatePhysics(true);
	MechaMesh->SetEnableGravity(false);
	MechaMesh->SetLinearDamping(1.0f); //Air Resistance
	MechaMesh->SetAngularDamping(2.0f); //Turn Resistance
	
	// Lock rotation so it can't tilt forward or lean sideways
	MechaMesh->GetBodyInstance()->bLockXRotation = true;
	MechaMesh->GetBodyInstance()->bLockYRotation = true;
	MechaMesh->GetBodyInstance()->bLockZTranslation = true;
	
	// Spring Arm creation
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = CameraHeightOffset;
	
	//Camera Lag
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = CameraSmoothness;
	SpringArm->CameraLagMaxDistance = MaxCameraLagDistance;
	
	// Disable Rotation inheritance of SpringArm to prevent it from rotating with player
	SpringArm->SetUsingAbsoluteRotation(true);
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bInheritYaw = false;
	
	// Camera creation
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	
	//Hurtbox creation
	Hurtbox = CreateDefaultSubobject<UBoxComponent>(TEXT("Hurtbox"));
	Hurtbox->SetupAttachment(RootComponent);
	Hurtbox->SetSimulatePhysics(false);
	Hurtbox->SetCollisionProfileName(TEXT("Trigger"));
	
	//Create Weapon system
	WeaponSystem = CreateDefaultSubobject<UWeaponSystemComponent>(TEXT("WeaponSystem"));
	
	SocketFront = CreateDefaultSubobject<USceneComponent>(TEXT("SocketFront"));
	SocketFront->SetupAttachment(RootComponent);
	WeaponSystem->Sockets.Add(EWeaponSocket::Front, SocketFront);
	
	SocketBack = CreateDefaultSubobject<USceneComponent>(TEXT("SocketBack"));
	SocketBack->SetupAttachment(RootComponent);
	WeaponSystem->Sockets.Add(EWeaponSocket::Back, SocketBack);
	
	SocketLeft = CreateDefaultSubobject<USceneComponent>(TEXT("SocketLeft"));
	SocketLeft->SetupAttachment(RootComponent);
	WeaponSystem->Sockets.Add(EWeaponSocket::Left, SocketLeft);
	
	SocketRight = CreateDefaultSubobject<USceneComponent>(TEXT("SocketRight"));
	SocketRight->SetupAttachment(RootComponent);
	WeaponSystem->Sockets.Add(EWeaponSocket::Right, SocketRight);
	
	//Creat Tier System
	TierSystem = CreateDefaultSubobject<UTierSystemComponent>(TEXT("TierSystem"));
	
	//Wheels creation
	WheelFrontLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelFrontLeft"));
	WheelFrontLeft->SetupAttachment(RootComponent);
	
	WheelFrontRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelFrontRight"));
	WheelFrontRight->SetupAttachment(RootComponent);
	
	WheelBackLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelBackLeft"));
	WheelBackLeft->SetupAttachment(RootComponent);
	
	WheelBackRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelBackRight"));
	WheelBackRight->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AMechaPawn::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeVariables();
	SetupEnhancedInput();
	TierSystem->InitializeTierSystem();
	BindEvents();
	
	if (PersistentManager != nullptr)
	{
		//Load saved Mecha State from Persistent Manager
		LoadMechaState(PersistentManager->GetMechaState());
	}
	else 
	{
		UE_LOG(LogTemp, Warning, TEXT("Error: No Persistent Manager found"));
	}
}

/* --- Initialization Functions --- */
void AMechaPawn::InitializeVariables()
{
	SpringArm->CameraLagSpeed = CameraSmoothness;
	SpringArm->CameraLagMaxDistance = MaxCameraLagDistance;
	SpringArm->SetRelativeRotation(FRotator(CameraRotationOffset, 0.f, 0.f));
	
	CurrentHealth = CoreMaxHealth;
	CurrentAcceleration = BaseAccelerationForce;
	CurrentSteerSpeed = BaseSteeringSpeed;
	
	PersistentManager = GetGameInstance()->GetSubsystem<UPersistentManager>();
	GameInstance = Cast<UScrapItGameInstance>(GetGameInstance());
}

void AMechaPawn::SetupEnhancedInput() const
{
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("MechaPawn is not Possessed by Player Controller"));
		return;
	}
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (Subsystem == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Mapping Context is Missing in Blueprint"));
		return;
	}
	
	if (DefaultMappingContext)
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
		UE_LOG(LogTemp, Warning, TEXT("Mapping Context Added"));
	}
}

void AMechaPawn::BindEvents()
{
	OnScrapCountChanged.AddDynamic(TierSystem, &UTierSystemComponent::CheckForTierChange);
	TierSystem->OnTierChanged.AddDynamic(WeaponSystem, &UWeaponSystemComponent::UpgradeAllWeapons);
	TierSystem->OnTierChanged.AddDynamic(this, &AMechaPawn::UpdateTierModifiers);
}

void AMechaPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(ThrustAction, ETriggerEvent::Triggered, this, &AMechaPawn::ApplyThrust);
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &AMechaPawn::ApplySteer);
		EnhancedInputComponent->BindAction(MagnetAction, ETriggerEvent::Triggered, this, &AMechaPawn::PullScraps);
		EnhancedInputComponent->BindAction(MagnetAction, ETriggerEvent::Started, this, &AMechaPawn::ToggleMagnet);
		EnhancedInputComponent->BindAction(MagnetAction, ETriggerEvent::Canceled, this, &AMechaPawn::ToggleMagnet);
		EnhancedInputComponent->BindAction(MagnetAction, ETriggerEvent::Completed, this, &AMechaPawn::ToggleMagnet);
	}
}

void AMechaPawn::LoadMechaState(const FMechaRunState& MechaRunState)
{
	//Apply Health
	if (MechaRunState.CurrentHealth > 0)
	{
		CurrentHealth = MechaRunState.CurrentHealth;
		OnHealthChanged.Broadcast(CurrentHealth);
	}
	
	//Load Mass Tier
	TierSystem->LoadTierState(MechaRunState.CurrentMassTierNumber);
	
	//Set Current Scraps
	if (MechaRunState.CurrentScraps > 0)
	{
		CurrentScraps = MechaRunState.CurrentScraps;
	}
	OnScrapCountChanged.Broadcast(CurrentScraps);
	
	//Load Weapons
	if (WeaponSystem != nullptr)
	{
		WeaponSystem->LoadWeaponLoadout(MechaRunState.WeaponLoadout);
	}
}

void AMechaPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UpdateSteer(DeltaTime);
	ApplyLateralFriction();
	UpdateMagnetDrag(DeltaTime);
	AnimateWheels(DeltaTime);
}

/* --- Movement Functions --- */
void AMechaPawn::ApplyThrust(const FInputActionValue& Value)
{
	const float FloatValue = Value.Get<float>();
	UpdateThrust(FloatValue);
}

void AMechaPawn::ApplySteer(const FInputActionValue& Value)
{
	const float FloatValue = Value.Get<float>();
	TargetSteerAngle = FloatValue * MaxSteerAngle;
}

void AMechaPawn::UpdateThrust(const float Value) const
{
	if (Value == 0.0f)
	{
		return;
	}
	
	//Calculate Force taking into account Magnet Status
	float const SpeedPenalty = bIsMagnetActive ? MagnetSpeedDecrease : 1.0f;
	float const FinalAcceleration = CurrentAcceleration * SpeedPenalty;
	
	// Apply Force (facing direction * Input * Power) to the Mecha
	FVector const ForceToAdd = MechaMesh->GetForwardVector() * Value * FinalAcceleration;
	MechaMesh->AddForce(ForceToAdd);
}

void AMechaPawn::UpdateSteer(const float DeltaTime)
{
	CurrentSteerAngle = FMath::FInterpTo(CurrentSteerAngle, TargetSteerAngle, DeltaTime, CurrentSteerSpeed);
	FVector const Velocity = MechaMesh->GetPhysicsLinearVelocity();
	float const ForwardSpeed = FVector::DotProduct(Velocity, MechaMesh->GetForwardVector());
	
	//Only turn if we are moving
	if (FMath::Abs(ForwardSpeed) > 10.0f)
	{
		const float TorqueAmount = ForwardSpeed * CurrentSteerAngle * HandlingFactor;
		FVector const RotationTorque = FVector(0.f, 0.f, TorqueAmount);
		
		MechaMesh->AddTorqueInDegrees(RotationTorque, NAME_None, true);
	}
}

void AMechaPawn::ApplyLateralFriction() const
{
	FVector const Velocity = MechaMesh->GetPhysicsLinearVelocity();
	FVector const RightVector = MechaMesh->GetRightVector();
	
	float const LateralSpeed = FVector::DotProduct(Velocity, RightVector);
	FVector const ImpulseToApply = -RightVector * LateralSpeed * GripStrength;
	
	MechaMesh->AddImpulse(ImpulseToApply, NAME_None, true);
}

/* --- Magnet --- */
void AMechaPawn::UpdateMagnetDrag(const float DeltaTime) const
{
	const float TargetDamping = bIsMagnetActive ? MagnetLinearDamping : BaseLinearDamping;
	const float CurrentDamping = MechaMesh->GetLinearDamping();
	
	const float NewDamping = FMath::FInterpTo(CurrentDamping, TargetDamping, DeltaTime, DampingInterpSpeed);
	MechaMesh->SetLinearDamping(NewDamping);
}

void AMechaPawn::PullScraps()
{
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(MagnetRadius);
	
	if (GetWorld()->OverlapMultiByChannel(Overlaps, GetActorLocation(), FQuat::Identity, ECC_PhysicsBody, Sphere))
	{
		std::for_each(Overlaps.begin(), Overlaps.end(), [this](const FOverlapResult& Result)
		{
			AActor* OverlappedActor = Result.GetActor();
			
			//Check if the overlapped actor implements IScrappable Interface
			if (IScrappable* ScrappableObject = Cast<IScrappable>(OverlappedActor))
			{
				//Pull the scrap towards the mecha
				ScrappableObject->OnMagnetPulled(this, MagnetStrength, MagnetRadius, CollectionRadius);
			}
		});
	}
}

void AMechaPawn::ToggleMagnet()
{
	bIsMagnetActive = !bIsMagnetActive;
}

/* --- Scrap - Tier Management --- */
void AMechaPawn::AddScrap(const int32 Amount)
{
	CurrentScraps += Amount;
	OnScrapCountChanged.Broadcast(CurrentScraps);
}

void AMechaPawn::RemoveScrap(const int32 Amount)
{
	CurrentScraps -= Amount;
	OnScrapCountChanged.Broadcast(CurrentScraps);
}

void AMechaPawn::UpdateTierModifiers(FMassTier Tier)
{
	float const NewAccelerationMult = Tier.SpeedPenalty;
	float const NewSteeringMult = Tier.SteeringPenalty;
	
	CurrentAcceleration = BaseAccelerationForce * NewAccelerationMult;
	CurrentSteerSpeed = BaseSteeringSpeed * NewSteeringMult;
	UE_LOG(LogTemp, Warning, TEXT("New Acceleration: %f"), CurrentAcceleration);
}

/* --- Vitality System --- */
void AMechaPawn::TakeDamage(const float Amount)
{
	UE_LOG(LogTemp, Warning, TEXT("MechaPawn Taking Damage: %f"), Amount);
	if (CurrentScraps > 0)
	{
		float const TotalScrapShieldAbsorption = CurrentScraps * ScrapShieldAbsorption;
		if (TotalScrapShieldAbsorption >= Amount)
		{
			//Scraps absorb all damage, no core health damage
			int32 const ScrapLost = FMath::CeilToInt(Amount / ScrapShieldAbsorption);
			RemoveScrap(ScrapLost);
			return;
		}
	
		//Scraps are lost and Mecha takes remaining damage
		float const RemainingDamage = Amount - TotalScrapShieldAbsorption;
		RemoveScrap(CurrentScraps);
		CurrentHealth -= RemainingDamage;
		OnHealthChanged.Broadcast(CurrentHealth);
	}
	else
	{
		//No Scraps to absorb damage, straight to core
		CurrentHealth -= Amount;
		OnHealthChanged.Broadcast(CurrentHealth);
	}
	
	if (CurrentHealth <= 0)
	{
		Die();
	}
}

void AMechaPawn::Die()
{
	//TODO: Death
	UE_LOG(LogTemp, Warning, TEXT("Game Over"));
}

/* Animation */
void AMechaPawn::AnimateWheels(const float DeltaTime)
{
	FVector const Velocity = MechaMesh->GetPhysicsLinearVelocity();
	if (Velocity == FVector::ZeroVector)
	{
		return;
	}
	
	float const ForwardSpeed = FVector::DotProduct(Velocity, MechaMesh->GetForwardVector());
	
	float const DeltaRoll = (ForwardSpeed * DeltaTime / WheelRadius) * (180.f / PI);
	CurrentWheelRoll += DeltaRoll;
	
	float const TargetAngle = FMath::Clamp(CurrentSteerAngle, -MaxWheelAngle, MaxWheelAngle);
	CurrentWheelAngle = FMath::FInterpTo(CurrentWheelAngle, TargetAngle, DeltaTime, 3.f);
	
	WheelFrontLeft->SetRelativeRotation(FRotator(-CurrentWheelRoll, CurrentWheelAngle, 0.f));
	WheelFrontRight->SetRelativeRotation(FRotator(-CurrentWheelRoll, CurrentWheelAngle, 0.f));
	WheelBackLeft->SetRelativeRotation(FRotator(-CurrentWheelRoll, 0.f, 0.f));
	WheelBackRight->SetRelativeRotation(FRotator(-CurrentWheelRoll, 0.f, 0.f));
}

