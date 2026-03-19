#pragma once

#include "CoreMinimal.h"
#include "scut/Character/Base/SCUTBaseCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "InputActionValue.h" // Needed for FInputActionValue
#include "scut/Animation/SCUTAnimInstance.h" // Assuming this is needed for character animation
#include "GameFramework/CharacterMovementComponent.h" // Needed for GetCharacterMovement()->MaxWalkSpeed
#include "Engine/LocalPlayer.h" // For ULocalPlayer::GetSubsystem
#include "SCUTPlayerCharacter.generated.h" // <<< 保持在文件末尾，但在所有前向声明和实际类定义之前

// --- Forward Declarations (place after all #includes, before UCLASS()) ---
class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class USkeletalMeshComponent;
class USpringArmComponent;
class ASCUTNPCCharacter;
class ASCUTDormManagerCharacter;
class ASCUTTeacherCharacter;
class ASCUTStudentCharacter;
class ASCUTVendingMachineCharacter;
class ABusTrigger;

UCLASS()
class SCUT_API ASCUTPlayerCharacter : public ASCUTBaseCharacter
{
    GENERATED_BODY()

public:
    ASCUTPlayerCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:
    virtual void Tick(float DeltaTime) override;

    USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
    UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
    USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

    UFUNCTION(BlueprintCallable, Category = "Player|Camera")
        void ToggleView();

    UFUNCTION(BlueprintCallable, Category = "Player|Camera")
        void SetFirstPersonView();

    UFUNCTION(BlueprintCallable, Category = "Player|Camera")
        void SetThirdPersonView();

    UFUNCTION(BlueprintPure, Category = "Player|Camera")
        bool IsFirstPersonView() const { return bIsFirstPersonView; }

    UFUNCTION(BlueprintCallable, Category = "Player|Movement")
        void SetMovementSpeed(float NewSpeed);

    UFUNCTION(BlueprintPure, Category = "Player|Input")
        bool IsMoving() const;

    virtual bool IsRunning() const override;

    UFUNCTION(BlueprintCallable, Category = "Player|Dialogue")
        void StartDialogue(ASCUTNPCCharacter* NPCTarget);

    UFUNCTION(BlueprintCallable, Category = "Player|Dialogue")
        void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Player|Dialogue")
        void SelectDialogueOption(int32 OptionIndex);

    UFUNCTION(BlueprintPure, Category = "Player|Dialogue")
        bool IsInDialogue() const { return bIsInDialogue; }

    UFUNCTION(BlueprintPure, Category = "Player|Dialogue")
        ASCUTNPCCharacter* GetCurrentDialogueTarget() const { return CurrentDialogueTarget; }

    UFUNCTION(BlueprintCallable, Category = "Player|Input")
        void SetupDormManagerInput(UInputAction* InInteractAction, UInputAction* InDialogueAction, UInputAction* InEndDialogueAction);

    UFUNCTION()
        void OnInteractDormManager(const FInputActionValue& Value);

    UFUNCTION()
        void OnDialogueDormManager(const FInputActionValue& Value);

    UFUNCTION()
        void OnEndDialogueDormManager(const FInputActionValue& Value);

    ASCUTDormManagerCharacter* FindDormManagerInRange(float Range);

    ASCUTTeacherCharacter* FindTeacherInRange(float Range);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|State")
        bool bIsSitting;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|State")
        bool bIsSittingDown;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|State")
        bool bIsStandingUp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Movement")
        FVector ChairLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Movement")
        FRotator ChairRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Movement")
        FVector SittingOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Movement")
        float SittingMoveSpeed = 200.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|State")
        bool bIsMovingToSeat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Movement")
        float SittingLockTime = 2.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|State")
        bool bIsMovementLocked;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|State")
        bool bIsBeingQuestioned;

    UFUNCTION(BlueprintCallable, Category = "Player|Movement")
        void SitDown();

    UFUNCTION(BlueprintCallable, Category = "Player|Movement")
        void StandUp();

    UFUNCTION(BlueprintCallable, Category = "Player|State")
        bool GetIsSitting() const { return bIsSitting; }

    void StartSittingAnimation();

    void OnStandUpFinished();
    void OnSitDownFinished();

    UFUNCTION(BlueprintCallable, Category = "Player|Input")
        void SetupInputActions(UInputAction* InMoveAction, UInputAction* InLookAction, UInputAction* InRunAction,
            UInputAction* InJumpAction, UInputAction* InInteractAction, UInputAction* InDialogueAction,
            UInputAction* InEndDialogueAction, UInputAction* InSittingAction, UInputAction* InRollCallAction,
            UInputAction* InQuestionAction, UInputAction* InEndQuestionAction);

    UFUNCTION(BlueprintCallable, Category = "Player|Input")
        void SetupSittingAction(UInputAction* InSittingAction);

    UFUNCTION(BlueprintCallable, Category = "Player|Input")
        void SetupStudentInput(UInputAction* InStudentDialogueAction, UInputAction* InStudentEndDialogueAction);

    UFUNCTION()
        void OnStudentDialogueAction(const FInputActionValue& Value);

    UFUNCTION()
        void OnStudentEndDialogueAction(const FInputActionValue& Value);

    ASCUTStudentCharacter* FindStudentInRange(float Range);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* VendingMachineInteractAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
        float VendingMachineInteractionRange = 200.0f;

    UFUNCTION()
        void OnVendingMachineInteract(const FInputActionValue& Value);

    ASCUTVendingMachineCharacter* FindVendingMachineInRange(float Range);

    // Assuming EPlayerDrinkState is now included from another header
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VendingMachine|Drink")
        // Note: EPlayerDrinkState must be defined in an included header now
        EPlayerDrinkState PlayerDrinkState; // Removed default initialization here if enum is from elsewhere

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VendingMachine|Drink")
        float PlayerDrinkingDuration = 3.0f;

    FTimerHandle PlayerDrinkTimerHandle;
    FTimerHandle DrinkFinishedResetTimerHandle; // Added this to resolve C2065 error

    UFUNCTION()
        void OnPlayerDrinkFinished();

    // Player Money
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Money")
        float PlayerMoney = 100.0f;

    // --- BusTrigger Related UPROPERTY ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction|Bus")
        ABusTrigger* CurrentOverlappingBusTrigger;

    // --- BusTrigger Related Function Declaration ---
    void OnInteractInput(const FInputActionValue& Value);

    UFUNCTION(BlueprintCallable, Category = "Interaction|Bus")
        void SetCurrentOverlappingBusTrigger(ABusTrigger* BusTrigger);


protected:
    UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
        USkeletalMeshComponent* Mesh1P;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
        UCameraComponent* FirstPersonCameraComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
        USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
        bool bIsFirstPersonView;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
        float ThirdPersonCameraDistance = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
        FVector FirstPersonCameraRelativeLocation = FVector(-10.f, 0.f, 60.f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* RunAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* InteractAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* DialogueAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* EndDialogueAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* SittingAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* RollCallAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* QuestionAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* EndQuestionAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* StudentDialogueAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* StudentEndDialogueAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
        UInputAction* ToggleViewAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Movement")
        float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Movement")
        float RunSpeed = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Movement")
        float JumpHeight = 600.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Input")
        bool bIsRunning;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Dialogue")
        bool bIsInDialogue;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Dialogue")
        ASCUTNPCCharacter* CurrentDialogueTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
        float ChairInteractionRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
        float TeacherInteractionRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
        float DormManagerInteractionRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
        float StudentInteractionRange = 200.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Dialogue")
        bool bIsTalkingToDormManager = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
        float InteractionRange = 200.0f;

    // Assuming ECharacterState is now included from another header
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = State)
        ECharacterState CurrentCharacterState; // Removed default initialization here if enum is from elsewhere

private:
    // Input handling functions
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void OnRun(const FInputActionValue& Value);
    void OnStopRunning(const FInputActionValue& Value);

    virtual void Jump() override;
    virtual void StopJumping() override;

    void InputJump(const FInputActionValue& Value);
    void InputStopJumping(const FInputActionValue& Value);

    void OnSittingAction(const FInputActionValue& Value);

    void OnToggleView(const FInputActionValue& Value);

    ASCUTNPCCharacter* FindInteractableNPC();

    FTimerHandle StandUpTimerHandle;
    FTimerHandle SitDownTimerHandle;

    void BindInputActions();

    void OnTeacherRollCallAction(const FInputActionValue& Value);
    void OnTeacherQuestionAction(const FInputActionValue& Value);
    void OnTeacherEndQuestionAction(const FInputActionValue& Value);

    bool IsInChairRange() const;

    // Function to update the character's state, now explicitly declared
    // Note: ECharacterState must be defined in an included header
    void UpdateCharacterState(ECharacterState NewState);
};