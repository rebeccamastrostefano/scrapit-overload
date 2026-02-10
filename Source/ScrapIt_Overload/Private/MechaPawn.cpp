// Fill out your copyright notice in the Description page of Project Settings.


#include "MechaPawn.h"

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
	
	CurrentTier = FMassTier{0, 1, 1};
	
	//Socket creation
	SocketFront = CreateDefaultSubobject<USceneComponent>(TEXT("SocketFront"));
	SocketFront->SetupAttachment(RootComponent);
	
	SocketBack = CreateDefaultSubobject<USceneComponent>(TEXT("SocketBack"));
	SocketBack->SetupAttachment(RootComponent);
	
	SocketLeft = CreateDefaultSubobject<USceneComponent>(TEXT("SocketLeft"));
	SocketLeft->SetupAttachment(RootComponent);
	
	SocketRight = CreateDefaultSubobject<USceneComponent>(TEXT("SocketRight"));
	SocketRight->SetupAttachment(RootComponent);
	
	//Hurtbox creation
	Hurtbox = CreateDefaultSubobject<UBoxComponent>(TEXT("Hurtbox"));
	Hurtbox->SetupAttachment(RootComponent);
	Hurtbox->SetSimulatePhysics(false);
	Hurtbox->SetCollisionProfileName(TEXT("Trigger"));
	
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
	
	CurrentHealth = CoreMaxHealth;
	SpringArm->CameraLagSpeed = CameraSmoothness;
	SpringArm->CameraLagMaxDistance = MaxCameraLagDistance;
	CurrentAcceleration = BaseAccelerationForce;
	CurrentSteerSpeed = BaseSteeringSpeed;
	SpringArm->SetRelativeRotation(FRotator(CameraRotationOffset, 0.f, 0.f));
	
	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
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
	
	//Populate Mass Mesh Parts array
	TArray<UStaticMeshComponent*> Meshes; 
	GetComponents<UStaticMeshComponent>(Meshes);
	
	for (UStaticMeshComponent* Mesh : Meshes)
	{
		if (Mesh->ComponentHasTag("MassTier"))
		{
			const FName* NumericTag = Mesh->ComponentTags.FindByPredicate([](const FName Tag)
			{
				return Tag.ToString().IsNumeric();
			});
			
			if (NumericTag != nullptr)
			{
				int32 TierNumber = FCString::Atoi(*NumericTag->ToString());
				MassMeshesForTiers.Add(Mesh, TierNumber);
			}
		}
	}
	
	
	
	PersistentManager = GetGameInstance()->GetSubsystem<UPersistentManager>();
	GameInstance = Cast<UScrapItGameInstance>(GetGameInstance());
	
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

// Called to bind functionality to input
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

void AMechaPawn::LoadMechaState(FMechaRunState MechaRunState)
{
	//Apply Health
	if (MechaRunState.CurrentHealth > 0)
	{
		CurrentHealth = MechaRunState.CurrentHealth;
		OnHealthChanged.Broadcast(CurrentHealth);
	}
	
	//Apply Mass Tier
	ApplyNewTier(GetTierByNumber(MechaRunState.CurrentMassTierNumber));
	
	//Set Current Scraps
	if (MechaRunState.CurrentScraps > 0)
	{
		CurrentScraps = MechaRunState.CurrentScraps;
	}
	OnScrapCountChanged.Broadcast(CurrentScraps);
	
	//Apply Weapons
	for (const auto& [Type, CurrentLevel, Socket] : MechaRunState.WeaponLoadout)
	{
		EquipWeaponTypeToSocket(Type, Socket, CurrentLevel);
	}
}

// Called every frame
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

void AMechaPawn::UpdateMagnetDrag(const float DeltaTime) const
{
	const float TargetDamping = bIsMagnetActive ? MagnetLinearDamping : BaseLinearDamping;
	const float CurrentDamping = MechaMesh->GetLinearDamping();
	
	const float NewDamping = FMath::FInterpTo(CurrentDamping, TargetDamping, DeltaTime, DampingInterpSpeed);
	MechaMesh->SetLinearDamping(NewDamping);
}

/* --- Scrap Management --- */
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

void AMechaPawn::AddScrap(const int32 Amount)
{
	CurrentScraps += Amount;
	CheckForTierChange();
	OnScrapCountChanged.Broadcast(CurrentScraps);
}

void AMechaPawn::RemoveScrap(const int32 Amount)
{
	CurrentScraps -= Amount;
	CheckForTierChange();
	OnScrapCountChanged.Broadcast(CurrentScraps);
}

/* --- Mass Tier Management --- */
void AMechaPawn::CheckForTierChange()
{
	//Determine if we are upgrading, downgrading or staying same tier
	const int32 TierDirection = (CurrentScraps > CurrentTier.UpgradeThreshold) ? 1 : (CurrentScraps < CurrentTier.DowngradeThreshold) ? -1 : 0;
	
	if (TierDirection == 0)
	{
		return; //No change
	}
	
	//Find the target Tier (previous or next)
	const int32 TargetTierNumber = CurrentTier.TierNumber + TierDirection;
	const FMassTier* NewTier= MassTiers.FindByPredicate([TargetTierNumber](const FMassTier& Tier)
	{
		return Tier.TierNumber == TargetTierNumber;
	});

	if (NewTier != nullptr)
	{
		if (TierDirection == 1)
		{
			//If we are upgrading, check if we should upgrade weapons too
			UpdateWeaponLevels(NewTier->TierNumber);
		}
		ApplyNewTier(*NewTier);
	}
}

void AMechaPawn::ApplyNewTier(const FMassTier& Tier)
{
	//We are upgrading or downgrading Tier
	CurrentTier = Tier;
	UpdateTierModifiers(CurrentTier);
	UpdateTierVisuals(CurrentTier);
	
	//Update UI
	OnTierChanged.Broadcast(CurrentTier.TierNumber, CurrentTier.UpgradeThreshold);
	UE_LOG(LogTemp, Warning, TEXT("Tier Changed to: %d"), Tier.TierNumber);
}

void AMechaPawn::UpdateTierModifiers(const FMassTier& Tier)
{
	float const NewAccelerationMult = Tier.SpeedPenalty;
	float const NewSteeringMult = Tier.SteeringPenalty;
	
	CurrentAcceleration = BaseAccelerationForce * NewAccelerationMult;
	CurrentSteerSpeed = BaseSteeringSpeed * NewSteeringMult;
	UE_LOG(LogTemp, Warning, TEXT("New Acceleration: %f"), CurrentAcceleration);
}

void AMechaPawn::UpdateTierVisuals(const FMassTier& Tier)
{
	for (auto& [Mesh, MeshTier] : MassMeshesForTiers)
	{
		if (Mesh != nullptr)
		{
			bool const bIsVisible = MeshTier <= Tier.TierNumber;
			Mesh->SetVisibility(bIsVisible);
		}
	}
}

/* --- Weapon Management --- */
void AMechaPawn::NotifyWeaponAcquired(const EScrapType WeaponScrapType, const int32 WeaponLevel) const
{
	OnWeaponAcquired.Broadcast(WeaponScrapType, WeaponLevel);
}

void AMechaPawn::EquipWeaponTypeToSocket(const EScrapType WeaponScrapType, const EWeaponSocket Socket, const int32 WeaponLevel)
{
	USceneComponent* AttachSocket = GetSocketByEnum(Socket);
	if (AttachSocket == nullptr)
	{
		return;
	}
	
	if (GameInstance != nullptr)
	{
		//If we are equipping a weapon on an occupied socket, drop the old one
		if (SocketsToWeapons.Contains(Socket))
		{
			DropWeaponOnSocket(Socket);
		}
		
		//Spawn new weapon and attach to socket
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Owner = this;
		const TSubclassOf<AWeaponBase> WeaponBP = GameInstance->ScrapTypeToWeaponBP[WeaponScrapType];
		
		if (AWeaponBase* NewWeapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponBP, AttachSocket->GetComponentTransform(), SpawnInfo))
		{
			NewWeapon->AttachToComponent(AttachSocket, FAttachmentTransformRules::KeepWorldTransform);
			WeaponLoadout.Add(FWeaponData{WeaponScrapType, WeaponLevel, Socket});
			SocketsToWeapons.Add(Socket, NewWeapon);
			NewWeapon->TryUpgrade(WeaponLevel); //Set the level of the weapon
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Error: No Game Instance"));
	}
}

void AMechaPawn::DropWeaponOnSocket(const EWeaponSocket Socket)
{
	if (AWeaponBase* WeaponActor = SocketsToWeapons.FindRef(Socket))
	{
		//Destroy the old weapon actor
		WeaponActor->Destroy();
		SocketsToWeapons.Remove(Socket);
	}
	
	//Find the weapon data index in the Loadout
	const int32 Index = WeaponLoadout.IndexOfByPredicate([Socket](const FWeaponData& Data)
	{
		return Data.Socket == Socket;
	});
	
	if (Index != INDEX_NONE)
	{
		//Get the old weapon class to drop as scrap
		const FWeaponData OldWeaponData = WeaponLoadout[Index];
			
		if (GameInstance != nullptr && GameInstance->ScrapTypeToBP.Contains(OldWeaponData.ScrapWeaponType))
		{
			//Drop the old weapon on ground as scrap
			const TSubclassOf<AScrapActor> OldWeaponScrapClass = GameInstance->ScrapTypeToBP[OldWeaponData.ScrapWeaponType];
			if (AScrapActor* WeaponScrap = GetWorld()->SpawnActor<AScrapActor>(OldWeaponScrapClass, GetActorTransform()))
			{
				WeaponScrap->InitWeaponScrap(OldWeaponData.ScrapWeaponType, OldWeaponData.CurrentLevel);
			}
		}
		
		//Remove the old weapon from the loadout
		WeaponLoadout.RemoveAtSwap(Index);
	}
	
			
	//Remove the swapped weapon from loadout
	WeaponLoadout.RemoveAll([&](const FWeaponData& Data) { return Data.Socket == Socket; });
}

void AMechaPawn::UpdateWeaponLevels(const int8 TierNumber)
{
	//Check if we should upgrade weapons too
	for (FWeaponData& Data : WeaponLoadout)
	{
		if (AWeaponBase* Weapon = SocketsToWeapons.FindRef(Data.Socket))
		{
			if (Weapon->TryUpgrade(TierNumber))
			{
				Data.CurrentLevel = TierNumber;
			}
		}
	}
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

/* Helper Functions */
FMassTier AMechaPawn::GetTierByNumber(const int32 TierNumber) const
{
	const auto FoundTier = MassTiers.FindByPredicate([=](const FMassTier& Tier)
	{
		return Tier.TierNumber == TierNumber;
	});
		
	return FoundTier ? *FoundTier : MassTiers[0];
}

TArray<EWeaponSocket> AMechaPawn::GetAvailableSockets() const
{
	TArray AvailableSockets = { EWeaponSocket::Front, EWeaponSocket::Back, EWeaponSocket::Left, EWeaponSocket::Right };
		
	AvailableSockets.RemoveAll([&](const EWeaponSocket Socket)
	{
		return SocketsToWeapons.Contains(Socket);
	});
	return AvailableSockets;
}

USceneComponent* AMechaPawn::GetSocketByEnum(const EWeaponSocket SocketEnum) const
{
	switch(SocketEnum)
	{
	case EWeaponSocket::Front:
		return SocketFront;
	case EWeaponSocket::Back:
		return SocketBack;
	case EWeaponSocket::Left:
		return SocketLeft;
	case EWeaponSocket::Right:
		return SocketRight;
	default:
		UE_LOG(LogTemp, Error, TEXT("Invalid Socket"));
		return nullptr;
	}
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

