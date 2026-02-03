// Fill out your copyright notice in the Description page of Project Settings.


#include "Rooms/Door.h"
#include "MechaPawn.h"
#include "Core/ScrapItGameInstance.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ADoor::ADoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Sphere"));
	RootComponent = CollisionSphere;
	CollisionSphere->SetCollisionProfileName(TEXT("Trigger"));
}

// Called when the game starts or when spawned
void ADoor::BeginPlay()
{
	Super::BeginPlay();
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ADoor::OnDoorOverlap);
}

void ADoor::SetRoomType(ERoomType NewRoomType)
{
	NextRoomType = NewRoomType;
}

void ADoor::OnDoorOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AMechaPawn* MechaPawn = Cast<AMechaPawn>(OtherActor))
	{
		UPersistentManager* PM = GetGameInstance()->GetSubsystem<UPersistentManager>();
		if (PM)
		{
			//Save the Player's state before progressing
			PM->SaveMechaState(
				MechaPawn->GetCurrentScrapCount(),
				MechaPawn->GetCurrentHealth(),
				MechaPawn->GetCurrentWeaponLoadout()
				);
			PM->AdvanceRoom();
			
			if (UScrapItGameInstance* GI = Cast<UScrapItGameInstance>(GetGameInstance()))
			{
				// Get the Level Pointer from the Map
				if (TSoftObjectPtr<UWorld> LevelPtr = GI->RoomLevels.FindRef(NextRoomType))
				{
					UGameplayStatics::OpenLevelBySoftObjectPtr(this, LevelPtr);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("No Level assigned for Room Type: %d"), (int32)NextRoomType);
				}
			}
		}
	}
}

