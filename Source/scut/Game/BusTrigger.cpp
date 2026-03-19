// YourGame/Private/BusTrigger.cpp
#include "BusTrigger.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "TimerManager.h"

// IMPORTANT: Include your player character's header here!
// Replace "YourPlayerCharacter.h" with the actual path to your player character's header file.
// For example: #include "YourGame/Public/YourPlayerCharacter.h"
// Or if it's in a common location: #include "YourPlayerCharacter.h"
#include "scut/Character/Player/SCUTPlayerCharacter.h" // <<< 使用正确的路径和文件名

ABusTrigger::ABusTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;
	TriggerBox->SetBoxExtent(FVector(200.0f, 200.0f, 200.0f));
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ABusTrigger::OnOverlapBegin);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ABusTrigger::OnOverlapEnd);

	BusMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BusMesh"));
	BusMesh->SetupAttachment(RootComponent);
	BusMesh->SetCollisionProfileName(TEXT("BlockAllDynamic")); // Ensure bus has collision

	bPlayerInRange = false;
	bIsPlayerOnBus = false; // Initialize as not on bus
	bIsInteractionLocked = false; // Initialize as not locked
	BusPawn = nullptr; // This will be set in the editor for a placed bus actor
	OriginalPlayerCharacter = nullptr;
	BusInputMappingContext = nullptr; // This will be set in the editor
}

void ABusTrigger::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController)
	{
		// Get the original player character reference
		// Cast to your specific player character type: ASCUTPlayerCharacter
		OriginalPlayerCharacter = Cast<ASCUTPlayerCharacter>(PlayerController->GetPawn()); // <<< 修改这里
		if (!OriginalPlayerCharacter)
		{
			UE_LOG(LogTemp, Error, TEXT("ABusTrigger::BeginPlay: Could not get OriginalPlayerCharacter! Ensure player pawn is ASCUTPlayerCharacter.")); // <<< 修改日志信息
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ABusTrigger::BeginPlay: Could not get PlayerController!"));
	}

	if (!BusPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ABusTrigger::BeginPlay: BusPawn is not assigned in the editor. Please assign the bus actor to the BusPawn slot."));
	}
	if (!BusInputMappingContext)
	{
		UE_LOG(LogTemp, Warning, TEXT("ABusTrigger::BeginPlay: BusInputMappingContext is not assigned in the editor. Please assign the bus IMC."));
	}
}

void ABusTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABusTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Cast the OtherActor to your specific player character class
	ASCUTPlayerCharacter* PlayerChar = Cast<ASCUTPlayerCharacter>(OtherActor); // <<< 修改这里

	// Check if it's your player character and it's locally controlled
	if (PlayerChar && PlayerChar->IsLocallyControlled() && PlayerChar == OriginalPlayerCharacter)
	{
		// Only if the player is entering, not on bus, AND interaction is NOT locked
		if (!bPlayerInRange && !bIsPlayerOnBus && !bIsInteractionLocked)
		{
			bPlayerInRange = true;
			UE_LOG(LogTemp, Warning, TEXT("Player entered bus trigger."));
			OnPlayerEnterBusTrigger.Broadcast(true); // Notify UI to show prompt

			// Notify the player character that they are overlapping this bus trigger
			PlayerChar->SetCurrentOverlappingBusTrigger(this);
		}
	}
}

void ABusTrigger::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Cast the OtherActor to your specific player character class
	ASCUTPlayerCharacter* PlayerChar = Cast<ASCUTPlayerCharacter>(OtherActor); // <<< 修改这里

	// Check if it's your player character and it's locally controlled
	if (PlayerChar && PlayerChar->IsLocallyControlled() && PlayerChar == OriginalPlayerCharacter)
	{
		// Only if the player is leaving, not on bus, AND interaction is NOT locked
		if (bPlayerInRange && !bIsPlayerOnBus && !bIsInteractionLocked)
		{
			bPlayerInRange = false;
			UE_LOG(LogTemp, Warning, TEXT("Player left bus trigger."));
			OnPlayerEnterBusTrigger.Broadcast(false); // Notify UI to hide prompt

			// Notify the player character that they are no longer overlapping this bus trigger
			PlayerChar->SetCurrentOverlappingBusTrigger(nullptr); // Clear the reference
		}
	}
}

// Helper function to manage interaction lock state and broadcast
void ABusTrigger::SetInteractionLocked(bool bLocked)
{
	if (bIsInteractionLocked != bLocked) // Only update if state changes
	{
		bIsInteractionLocked = bLocked;
		OnInteractionLockedChanged.Broadcast(bLocked); // Broadcast the new state
		UE_LOG(LogTemp, Warning, TEXT("Interaction Locked: %s"), bLocked ? TEXT("True") : TEXT("False"));
	}
}

void ABusTrigger::MountBus()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	// **Lock interaction immediately when MountBus is called**
	SetInteractionLocked(true);

	if (PlayerController && OriginalPlayerCharacter && BusPawn && !bIsPlayerOnBus)
	{
		// 1. Disable player character's input and collision (optional but recommended)
		OriginalPlayerCharacter->DisableInput(PlayerController);
		OriginalPlayerCharacter->SetActorHiddenInGame(true); // Hide player character
		OriginalPlayerCharacter->SetActorEnableCollision(false); // Disable collision

		// 2. Possess the bus Pawn
		PlayerController->Possess(BusPawn);
		bIsPlayerOnBus = true;
		UE_LOG(LogTemp, Warning, TEXT("Player mounted the bus."));

		// 3. Add the bus's Input Mapping Context
		if (BusInputMappingContext)
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				// Add IMC with a higher priority (e.g., 1) to ensure bus controls take precedence
				Subsystem->AddMappingContext(BusInputMappingContext, 1);
				UE_LOG(LogTemp, Warning, TEXT("Added BusInputMappingContext."));
			}
		}

		// 4. Broadcast mount event
		OnPlayerMountedBus.Broadcast(true);

		// 5. Force hide any "enter prompt" UI (as player is now on bus)
		OnPlayerEnterBusTrigger.Broadcast(false);

		// **Important: Delay unlocking interaction to allow for animation/state transition**
		// Adjust the duration (1.0f here) to match your mount animation length or desired delay
		FTimerHandle TimerHandle_MountDelay;
		GetWorldTimerManager().SetTimer(TimerHandle_MountDelay, this, &ABusTrigger::SetInteractionLocked_False_Mount, 1.0f, false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MountBus failed: PlayerController, OriginalPlayerCharacter, or BusPawn not valid, or player already on bus."));
		// If MountBus fails, immediately unlock interaction
		SetInteractionLocked(false);
	}
}

// Helper function called by timer to unlock interaction after mounting
void ABusTrigger::SetInteractionLocked_False_Mount()
{
	SetInteractionLocked(false);
}

void ABusTrigger::DismountBus()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	// **Lock interaction immediately when DismountBus is called**
	SetInteractionLocked(true);

	if (PlayerController && OriginalPlayerCharacter && BusPawn && bIsPlayerOnBus)
	{
		// 1. Remove the bus's Input Mapping Context first
		if (BusInputMappingContext)
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->RemoveMappingContext(BusInputMappingContext);
				UE_LOG(LogTemp, Warning, TEXT("Removed BusInputMappingContext."));
			}
		}

		// 2. Possess the original player character back
		PlayerController->Possess(OriginalPlayerCharacter);

		// 3. Teleport player character to a safe location near the bus (e.g., in front of it)
		FVector DismountLocation = BusPawn->GetActorLocation() + BusPawn->GetActorForwardVector() * 500.0f; // 300 units in front
		DismountLocation.Z += 200.0f; // Lift slightly to prevent clipping
		OriginalPlayerCharacter->SetActorLocation(DismountLocation, false, nullptr, ETeleportType::TeleportPhysics);

		// 4. Enable player character's input and collision
		OriginalPlayerCharacter->EnableInput(PlayerController);
		OriginalPlayerCharacter->SetActorHiddenInGame(false); // Show player character
		OriginalPlayerCharacter->SetActorEnableCollision(true); // Enable collision

		bIsPlayerOnBus = false;
		UE_LOG(LogTemp, Warning, TEXT("Player dismounted the bus."));

		// 5. Broadcast dismount event
		OnPlayerMountedBus.Broadcast(false);

		// 6. If player is still within trigger range, re-show the "enter prompt" UI
		// This handles cases where player dismounts *inside* the trigger area
		if (bPlayerInRange && !bIsInteractionLocked) // Double-check lock state for prompt logic
		{
			OnPlayerEnterBusTrigger.Broadcast(true);
		}

		// **Important: Delay unlocking interaction to allow for animation/state transition**
		// Adjust the duration (1.0f here) to match your dismount animation length or desired delay
		FTimerHandle TimerHandle_DismountDelay;
		GetWorldTimerManager().SetTimer(TimerHandle_DismountDelay, this, &ABusTrigger::SetInteractionLocked_False_Dismount, 1.0f, false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DismountBus failed: PlayerController, OriginalPlayerCharacter, or BusPawn not valid, or player not on bus."));
		// If DismountBus fails, immediately unlock interaction
		SetInteractionLocked(false);
	}
}

// Helper function called by timer to unlock interaction after dismounting
void ABusTrigger::SetInteractionLocked_False_Dismount()
{
	SetInteractionLocked(false);
}