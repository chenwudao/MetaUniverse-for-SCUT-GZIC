// YourGame/Public/MyPlayerController.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

// Include all necessary Widget class headers for TSubclassOf
#include "scut/MyMainMenuWidget.h"
#include "scut/MyMainHUDWidget.h"
#include "MyEndMenuWidget.h"
#include "MyPlayerController.generated.h"

// Forward declarations for classes used as pointers/references
class UUserWidget;
class UInputMappingContext;
class UInputAction;
class ABusTrigger; // Forward declare ABusTrigger as we use a pointer to it

UCLASS()
class SCUT_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMyPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// --- Input System Properties (Set in Blueprint) ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
		UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
		UInputAction* ToggleMenuAction; // For ESC key

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
		UInputAction* InteractAction; // For E key

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
		UInputAction* OpenLevelSelectAction; // For H key

	// --- UI Class References (Set in Blueprint) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SCU UI")
		TSubclassOf<UMyMainMenuWidget> MainMenuWidgetClass; // Main menu (now includes level select)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SCU UI")
		TSubclassOf<UMyMainHUDWidget> MainHUDWidgetClass; // HUD Widget

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SCU UI")
		TSubclassOf<UMyEndMenuWidget> EndMenuWidgetClass; // End menu Widget

	// **New:** Class reference for the bus interaction prompt UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SCU UI")
		TSubclassOf<UUserWidget> TravelPromptWidgetClass; // Class for the "Press E to Interact" prompt

	// --- UI Instance References ---
	UPROPERTY(BlueprintReadOnly, Category = "SCU UI")
		UMyMainMenuWidget* MainMenuWidgetInstance;

	UPROPERTY(BlueprintReadOnly, Category = "SCU UI")
		UMyMainHUDWidget* MainHUDWidgetInstance;

	UPROPERTY(BlueprintReadOnly, Category = "SCU UI")
		UMyEndMenuWidget* EndMenuWidgetInstance;

	UPROPERTY(BlueprintReadOnly, Category = "SCU UI")
		UUserWidget* CurrentPromptWidget; // Instance of the current prompt widget (e.g., bus interact)

	// --- State Variables ---
	UPROPERTY(BlueprintReadOnly, Category = "Bus")
		bool bIsInBusTriggerRange; // Is the player currently within a bus trigger?

	// --- UI Management Functions ---
	UFUNCTION(BlueprintCallable, Category = "SCU UI")
		void ShowMainMenu();

	UFUNCTION(BlueprintCallable, Category = "SCU UI")
		void HideMainMenu();

	UFUNCTION(BlueprintCallable, Category = "SCU UI")
		void ToggleMainMenu();

	UFUNCTION(BlueprintCallable, Category = "SCU UI")
		void ShowEndMenu();

	UFUNCTION(BlueprintCallable, Category = "SCU UI")
		void HideEndMenu();

protected:
	// --- Input Event Handlers ---
	void OnEscapeKeyPressed();
	void OnInteractKeyPressed();
	void OnOpenLevelSelectKeyPressed();

	// --- BusTrigger Delegate Handlers ---
	UFUNCTION()
		void OnPlayerEnterBusTrigger(bool bIsEntering); // Handles player entering/leaving bus trigger

	// **New:** Handles changes in the bus interaction locked status
	UFUNCTION()
		void HandleBusInteractionLockChanged(bool bIsLocked);

	// --- Main Menu Delegate Handlers ---
	UFUNCTION()
		void OnLevelSelectedInMainMenu(FName SelectedLevelID);

	// --- RPC Examples (if your project uses them) ---
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_SendGenericAction(const FString& ActionName);
	bool Server_SendGenericAction_Validate(const FString& ActionName);
	void Server_SendGenericAction_Implementation(const FString& ActionName);

	UFUNCTION(Client, Reliable)
		void Client_ReceiveNotification(const FText& NotificationText);
	void Client_ReceiveNotification_Implementation(const FText& NotificationText);
};