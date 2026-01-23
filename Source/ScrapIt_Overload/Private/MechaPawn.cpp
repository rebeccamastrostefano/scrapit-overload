// Fill out your copyright notice in the Description page of Project Settings.


#include "MechaPawn.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Scrappable.h"
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
	SpringArm->TargetArmLength = 800.0f;
	SpringArm->SetRelativeRotation(FRotator(-60.0f, 0.f, 0.f));
	
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
	
	CurrentTier = FMassTier{0, 1, 1};
}

// Called when the game starts or when spawned
void AMechaPawn::BeginPlay()
{
	Super::BeginPlay();
	
	SpringArm->CameraLagSpeed = CameraSmoothness;
	SpringArm->CameraLagMaxDistance = MaxCameraLagDistance;
	CurrentAcceleration = BaseAccelerationForce;
	CurrentSteerSpeed = BaseSteeringSpeed;
	
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
				UE_LOG(LogTemp, Warning, TEXT("Mapping Context Added"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Mapping Context is Missing in Blueprint"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("MechaPawn is not Possessed by Player Controller"));
		}
	}
}

// Called to bind functionality to input
void AMechaPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(ThrustAction, ETriggerEvent::Triggered, this, &AMechaPawn::ApplyThrust);
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &AMechaPawn::ApplySteer);
		EnhancedInputComponent->BindAction(MagnetAction, ETriggerEvent::Triggered, this, &AMechaPawn::ActivateMagnet);
		EnhancedInputComponent->BindAction(MagnetAction, ETriggerEvent::Started, this, &AMechaPawn::ToggleMagnetMovement);
		EnhancedInputComponent->BindAction(MagnetAction, ETriggerEvent::Canceled, this, &AMechaPawn::ToggleMagnetMovement);
		EnhancedInputComponent->BindAction(MagnetAction, ETriggerEvent::Completed, this, &AMechaPawn::ToggleMagnetMovement);
	}
}

// Called every frame
void AMechaPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UpdateSteer(DeltaTime);
	ApplyLateralFriction();
}

void AMechaPawn::ApplyThrust(const FInputActionValue& Value)
{
	float FloatValue = Value.Get<float>();
	UpdateThrust(FloatValue);
}

void AMechaPawn::ApplySteer(const FInputActionValue& Value)
{
	float FloatValue = Value.Get<float>();
	TargetSteerAngle = FloatValue * MaxSteerAngle;
}

void AMechaPawn::UpdateThrust(float Value)
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

void AMechaPawn::UpdateSteer(float DeltaTime)
{
	CurrentSteerAngle = FMath::FInterpTo(CurrentSteerAngle, TargetSteerAngle, DeltaTime, CurrentSteerSpeed);
	FVector const Velocity = MechaMesh->GetPhysicsLinearVelocity();
	float const ForwardSpeed = FVector::DotProduct(Velocity, MechaMesh->GetForwardVector());
	
	//Only turn if we are moving
	if (FMath::Abs(ForwardSpeed) > 10.0f)
	{
		float TorqueAmount = ForwardSpeed * CurrentSteerAngle * HandlingFactor;
		FVector const RotationTorque = FVector(0.f, 0.f, TorqueAmount);
		
		MechaMesh->AddTorqueInDegrees(RotationTorque, NAME_None, true);
	}
}

void AMechaPawn::ApplyLateralFriction()
{
	FVector const Velocity = MechaMesh->GetPhysicsLinearVelocity();
	FVector const RightVector = MechaMesh->GetRightVector();
	
	float const LateralSpeed = FVector::DotProduct(Velocity, RightVector);
	FVector const ImpulseToApply = -RightVector * LateralSpeed * GripStrength;
	
	MechaMesh->AddImpulse(ImpulseToApply, NAME_None, true);
}

void AMechaPawn::ActivateMagnet()
{
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(MagnetRadius);
	
	if (GetWorld()->OverlapMultiByChannel(Overlaps, GetActorLocation(), FQuat::Identity, ECC_PhysicsBody, Sphere))
	{
		for (auto& Result : Overlaps)
		{
			AActor* OverlappedActor = Result.GetActor();
			
			//Check if the overlapped actor implements IScrappable Interface
			IScrappable* ScrappableObject = Cast<IScrappable>(OverlappedActor);
			if (ScrappableObject)
			{
				ScrappableObject->OnMagnetPulled(this, MagnetStrength, MagnetRadius, CollectionRadius);
			}
		}
	}
}

void AMechaPawn::ToggleMagnetMovement()
{
	bIsMagnetActive = !bIsMagnetActive;
}

/* --- Scrap Management --- */

void AMechaPawn::AddScrap(int32 Amount)
{
	CurrentScraps += Amount;
	CheckTierUpgrade();
}

/* --- Mass Tier Management --- */

void AMechaPawn::CheckTierUpgrade()
{
	for (const FMassTier& Tier : MassTiers)
	{
		if (Tier.ScrapThreshold != CurrentTier.ScrapThreshold && CurrentScraps >= Tier.ScrapThreshold)
		{
			UpdateMassStats(Tier);
			UpdateMassVisuals(Tier);
			CurrentTier = Tier;
		}
	}
}

void AMechaPawn::UpdateMassStats(const FMassTier& Tier)
{
	float const NewAccelerationMult = Tier.SpeedPenalty;
	float const NewSteeringMult = Tier.SteeringPenalty;
	
	CurrentAcceleration = BaseAccelerationForce * NewAccelerationMult;
	CurrentSteerSpeed = BaseSteeringSpeed * NewSteeringMult;
	UE_LOG(LogTemp, Warning, TEXT("New Acceleration: %f"), CurrentAcceleration);
}

void AMechaPawn::UpdateMassVisuals(const FMassTier& Tier)
{
	// TODO: Update Visuals based on Tier (turn visible certain MassMeshParts)
}

