// YourGame/Private/MyPlayerController.cpp
#include "MyPlayerController.h"
#include "BusTrigger.h" // Include BusTrigger header
#include "Blueprint/UserWidget.h" // Include UserWidget header
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MyGameMode.h" // Include your MyGameMode header
#include "scut/MyMainMenuWidget.h" // Include your main menu Widget header
#include "scut/MyMainHUDWidget.h" // Include your HUD Widget header
#include "MyEndMenuWidget.h" // Include your end menu Widget header (if you have a separate C++ class)

AMyPlayerController::AMyPlayerController()
{
	CurrentPromptWidget = nullptr;
	EndMenuWidgetInstance = nullptr;
	bIsInBusTriggerRange = false;

	DefaultMappingContext = nullptr;
	ToggleMenuAction = nullptr;
	InteractAction = nullptr;
	OpenLevelSelectAction = nullptr;

	MainMenuWidgetClass = nullptr;
	MainHUDWidgetClass = nullptr;
	EndMenuWidgetClass = nullptr;
	TravelPromptWidgetClass = nullptr; // Initialize the new UI class reference
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Find all BusTrigger actors in the world and bind to their delegates
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABusTrigger::StaticClass(), FoundActors);
	for (AActor* Actor : FoundActors)
	{
		ABusTrigger* BusTrigger = Cast<ABusTrigger>(Actor);
		if (BusTrigger)
		{
			// Bind to player enter/leave trigger event
			BusTrigger->OnPlayerEnterBusTrigger.AddDynamic(this, &AMyPlayerController::OnPlayerEnterBusTrigger);
			// **New:** Bind to the interaction locked status change event
			BusTrigger->OnInteractionLockedChanged.AddDynamic(this, &AMyPlayerController::HandleBusInteractionLockChanged);
		}
	}

	// Add default input mapping context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Show HUD at BeginPlay
	if (IsLocalPlayerController() && MainHUDWidgetClass)
	{
		MainHUDWidgetInstance = CreateWidget<UMyMainHUDWidget>(this, MainHUDWidgetClass);
		if (MainHUDWidgetInstance)
		{
			MainHUDWidgetInstance->AddToViewport();
			UE_LOG(LogTemp, Warning, TEXT("Main HUD Added to Viewport."));
		}
	}
}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (ToggleMenuAction)
		{
			EnhancedInputComponent->BindAction(ToggleMenuAction, ETriggerEvent::Started, this, &AMyPlayerController::OnEscapeKeyPressed);
		}
		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AMyPlayerController::OnInteractKeyPressed);
		}
		if (OpenLevelSelectAction)
		{
			EnhancedInputComponent->BindAction(OpenLevelSelectAction, ETriggerEvent::Started, this, &AMyPlayerController::OnOpenLevelSelectKeyPressed);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AMyPlayerController: EnhancedInputComponent not found. Ensure InputComponent is configured for Enhanced Input."));
	}
}

// **New Function:** Handles changes in the bus interaction locked status
void AMyPlayerController::HandleBusInteractionLockChanged(bool bIsLocked)
{
	// If interaction is locked and a prompt widget is currently displayed, hide it
	if (bIsLocked && CurrentPromptWidget)
	{
		CurrentPromptWidget->RemoveFromParent();
		CurrentPromptWidget = nullptr;
		UE_LOG(LogTemp, Log, TEXT("AMyPlayerController: Bus interaction prompt hidden due to lock."));
	}
	// If interaction is unlocked, and player is still in range, you might re-show it,
	// but typically the player's state (on/off bus) will change, so the prompt won't be needed immediately.
}


void AMyPlayerController::OnPlayerEnterBusTrigger(bool bIsEntering)
{
	bIsInBusTriggerRange = bIsEntering;

	ABusTrigger* FoundBusTrigger = nullptr;
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABusTrigger::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		FoundBusTrigger = Cast<ABusTrigger>(FoundActors[0]); // Assuming you only have one BusTrigger or want the first one
	}

	if (bIsEntering)
	{
		// Only show the prompt if player enters, BusTrigger exists, it's NOT locked, and UI class is set
		if (FoundBusTrigger && !FoundBusTrigger->bIsInteractionLocked && TravelPromptWidgetClass)
		{
			// Ensure no old prompt widget is lingering
			if (CurrentPromptWidget)
			{
				CurrentPromptWidget->RemoveFromParent();
				CurrentPromptWidget = nullptr;
			}
			CurrentPromptWidget = CreateWidget<UUserWidget>(this, TravelPromptWidgetClass);
			if (CurrentPromptWidget)
			{
				CurrentPromptWidget->AddToViewport();
				UE_LOG(LogTemp, Log, TEXT("AMyPlayerController: Bus interaction prompt shown."));
			}
		}
	}
	else // Player leaving trigger range
	{
		if (CurrentPromptWidget)
		{
			CurrentPromptWidget->RemoveFromParent();
			CurrentPromptWidget = nullptr;
			UE_LOG(LogTemp, Log, TEXT("AMyPlayerController: Bus interaction prompt hidden."));
		}
	}
}

void AMyPlayerController::OnInteractKeyPressed()
{
	// Get the BusTrigger instance
	ABusTrigger* BusTriggerInstance = nullptr;
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABusTrigger::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		BusTriggerInstance = Cast<ABusTrigger>(FoundActors[0]); // Assuming you only have one BusTrigger or want the first one
	}

	// **Core Logic for Interaction Lock:**
	// Only proceed if a BusTrigger exists AND interaction is NOT locked
	if (BusTriggerInstance && !BusTriggerInstance->bIsInteractionLocked)
	{
		if (bIsInBusTriggerRange)
		{
			// If in range, first hide any active prompt UI
			if (CurrentPromptWidget)
			{
				CurrentPromptWidget->RemoveFromParent();
				CurrentPromptWidget = nullptr;
			}

			// Based on player's current state (on/off bus), call the appropriate function
			if (BusTriggerInstance->bIsPlayerOnBus) // If player is currently on the bus
			{
				BusTriggerInstance->DismountBus();
			}
			else // If player is not on the bus
			{
				BusTriggerInstance->MountBus();
			}
			UE_LOG(LogTemp, Log, TEXT("AMyPlayerController: Interact key pressed. Executing bus action."));
		}
		else // Not in bus trigger range
		{
			UE_LOG(LogTemp, Log, TEXT("AMyPlayerController: Not in bus trigger range to interact."));
			// If the Interact key has other functions outside the bus trigger, they would go here.
			// Currently, it's solely for bus interaction in this example.
		}
	}
	else // BusTrigger not found or interaction is locked
	{
		UE_LOG(LogTemp, Warning, TEXT("AMyPlayerController: BusTrigger not found or interaction is locked. Ignoring Interact input."));
	}
}

void AMyPlayerController::OnOpenLevelSelectKeyPressed()
{
	// If main menu is already displayed, H key can toggle it off
	if (MainMenuWidgetInstance && MainMenuWidgetInstance->IsInViewport())
	{
		HideMainMenu();
	}
	// If end menu is displayed, H key should not open main menu/level select
	else if (EndMenuWidgetInstance && EndMenuWidgetInstance->IsInViewport())
	{
		UE_LOG(LogTemp, Warning, TEXT("AMyPlayerController: End menu is open, H key won't open Main Menu/Level Select."));
	}
	else
	{
		// Hide bus interaction prompt (if displayed)
		if (CurrentPromptWidget)
		{
			CurrentPromptWidget->RemoveFromParent();
			CurrentPromptWidget = nullptr;
		}
		// Show main menu, which will then handle displaying the level select part
		ShowMainMenu();
		// Tell main menu to show its level select part (this requires implementation in UMyMainMenuWidget Blueprint)
		if (MainMenuWidgetInstance)
		{
			MainMenuWidgetInstance->ShowLevelSelectPart(); // <--- This function needs to be implemented in UMyMainMenuWidget Blueprint
		}
	}
}

// --- UI Display/Hide Functions ---

void AMyPlayerController::ShowMainMenu()
{
	if (MainMenuWidgetClass)
	{
		if (!MainMenuWidgetInstance)
		{
			MainMenuWidgetInstance = CreateWidget<UMyMainMenuWidget>(this, MainMenuWidgetClass);
			if (MainMenuWidgetInstance)
			{
				// Bind delegate for level selection from main menu
				MainMenuWidgetInstance->OnLevelSelected.AddDynamic(this, &AMyPlayerController::OnLevelSelectedInMainMenu);
			}
		}

		if (MainMenuWidgetInstance && !MainMenuWidgetInstance->IsInViewport())
		{
			// Hide any other potentially conflicting UI
			HideEndMenu();
			if (CurrentPromptWidget)
			{
				CurrentPromptWidget->RemoveFromParent();
				CurrentPromptWidget = nullptr;
			}

			MainMenuWidgetInstance->AddToViewport();
			SetInputMode(FInputModeUIOnly());
			bShowMouseCursor = true;
			UE_LOG(LogTemp, Warning, TEXT("Main Menu Shown."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuWidgetClass is not set in AMyPlayerController. Please set it in Blueprint."));
	}
}

void AMyPlayerController::HideMainMenu()
{
	if (MainMenuWidgetInstance && MainMenuWidgetInstance->IsInViewport())
	{
		MainMenuWidgetInstance->RemoveFromParent();
		// MainMenuWidgetInstance can be kept for reuse next time
		// But if its internal state (e.g., showing main part vs. level select part) needs reset,
		// you might need to reset its internal state after RemoveFromParent or recreate it.

		// Restore game input mode and mouse, unless another UI needs focus
		if (!EndMenuWidgetInstance || !EndMenuWidgetInstance->IsInViewport()) // Check if any other UI is still displayed
		{
			SetInputMode(FInputModeGameOnly());
			bShowMouseCursor = false;
		}
		UE_LOG(LogTemp, Warning, TEXT("Main Menu Hidden."));
	}
}

void AMyPlayerController::ToggleMainMenu()
{
	if (MainMenuWidgetInstance && MainMenuWidgetInstance->IsInViewport())
	{
		HideMainMenu();
	}
	else
	{
		ShowMainMenu();
		// By default, show the main part of the main menu, not the level select part
		if (MainMenuWidgetInstance)
		{
			MainMenuWidgetInstance->ShowMainMenuPart(); // <--- This function needs to be implemented in UMyMainMenuWidget Blueprint
		}
	}
}

void AMyPlayerController::ShowEndMenu()
{
	if (EndMenuWidgetClass)
	{
		if (!EndMenuWidgetInstance)
		{
			EndMenuWidgetInstance = CreateWidget<UMyEndMenuWidget>(this, EndMenuWidgetClass);
		}

		if (EndMenuWidgetInstance && !EndMenuWidgetInstance->IsInViewport())
		{
			// Hide any other potentially conflicting UI
			HideMainMenu(); // Hides main menu, including level select
			if (CurrentPromptWidget)
			{
				CurrentPromptWidget->RemoveFromParent();
				CurrentPromptWidget = nullptr;
			}

			EndMenuWidgetInstance->AddToViewport();
			UGameplayStatics::SetGamePaused(GetWorld(), true);
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(EndMenuWidgetInstance->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
			UE_LOG(LogTemp, Warning, TEXT("End Menu Shown."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AMyPlayerController::EndMenuWidgetClass is not set! Please set it in Blueprint."));
	}
}

void AMyPlayerController::HideEndMenu()
{
	if (EndMenuWidgetInstance && EndMenuWidgetInstance->IsInViewport())
	{
		EndMenuWidgetInstance->RemoveFromParent();
		EndMenuWidgetInstance = nullptr; // Clear reference as it's no longer needed
		UGameplayStatics::SetGamePaused(GetWorld(), false);
		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
		UE_LOG(LogTemp, Warning, TEXT("End Menu Hidden."));
	}
}

// --- Event Handlers ---
void AMyPlayerController::OnEscapeKeyPressed()
{
	// If main menu (now including level select) is displayed, prioritize hiding it
	if (MainMenuWidgetInstance && MainMenuWidgetInstance->IsInViewport())
	{
		HideMainMenu();
	}
	// Otherwise, toggle end menu/pause state
	else
	{
		if (UGameplayStatics::IsGamePaused(GetWorld()))
		{
			HideEndMenu();
		}
		else
		{
			ShowEndMenu();
		}
	}
}

// Level selection event handler (now received from UMyMainMenuWidget)
void AMyPlayerController::OnLevelSelectedInMainMenu(FName SelectedLevelID)
{
	UE_LOG(LogTemp, Log, TEXT("PlayerController received level selected from Main Menu: %s"), *SelectedLevelID.ToString());

	HideMainMenu(); // Hide main menu (including level select)

	AMyGameMode* GameMode = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->Server_TravelToLevel(SelectedLevelID);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode is not valid when trying to travel to level."));
	}
}

// --- RPC Example Code (kept unchanged) ---
bool AMyPlayerController::Server_SendGenericAction_Validate(const FString& ActionName) { return !ActionName.IsEmpty(); }
void AMyPlayerController::Server_SendGenericAction_Implementation(const FString& ActionName) { UE_LOG(LogTemp, Log, TEXT("Server received generic action: %s from %s"), *ActionName, *GetName()); }
void AMyPlayerController::Client_ReceiveNotification_Implementation(const FText& NotificationText) { UE_LOG(LogTemp, Log, TEXT("Client received notification: %s"), *NotificationText.ToString()); }