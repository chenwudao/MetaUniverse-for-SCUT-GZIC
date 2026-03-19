// YourGame/Public/BusTrigger.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputSubsystems.h" // For Enhanced Input Subsystems
#include "BusTrigger.generated.h"

class ACharacter;
class APawn;
class UInputMappingContext; // Forward declaration for UInputMappingContext
class ASCUTPlayerCharacter; // <<< Correct forward declaration

// Delegates for Blueprint communication
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerEnterBusTrigger, bool, bPlayerInArea);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerMountedBus, bool, bMounted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionLockedChanged, bool, bIsLocked); // New: Broadcasts interaction lock status

UCLASS()
class SCUT_API ABusTrigger : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABusTrigger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UBoxComponent* TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UStaticMeshComponent* BusMesh;

	// Properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bus")
		APawn* BusPawn; // The actual bus Pawn to possess. Assign this in the editor for your placed bus actor.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
		UInputMappingContext* BusInputMappingContext; // Input Mapping Context for bus controls. Assign this in the editor.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bus State")
		bool bPlayerInRange; // Is player currently within the trigger box?

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bus State")
		bool bIsPlayerOnBus; // Is player currently possessing the bus?

	// Is the interaction currently locked (e.g., during mount/dismount animation)?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bus State")
		bool bIsInteractionLocked;

	// Reference to the player's original character.
	// We'll get this in BeginPlay.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bus State")
		ACharacter* OriginalPlayerCharacter;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnPlayerEnterBusTrigger OnPlayerEnterBusTrigger; // Broadcasts when player enters/leaves trigger

	UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnPlayerMountedBus OnPlayerMountedBus; // Broadcasts when player mounts/dismounts bus

	// New Delegate: Broadcasts when the interaction lock status changes
	UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnInteractionLockedChanged OnInteractionLockedChanged;

	// Overlap event handlers
	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Public functions for bus control, callable from Blueprint
	UFUNCTION(BlueprintCallable, Category = "Bus Control")
		void MountBus();

	UFUNCTION(BlueprintCallable, Category = "Bus Control")
		void DismountBus();

protected:
	// Helper Function: To set the interaction lock status and broadcast changes
	void SetInteractionLocked(bool bLocked);

	// Helper Functions: For timer callbacks to unlock interaction
	void SetInteractionLocked_False_Mount();
	void SetInteractionLocked_False_Dismount();
};