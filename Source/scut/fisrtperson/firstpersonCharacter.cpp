#include "firstpersonCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "scut/Character/NPC/SCUTNPCCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "scut/Character/NPC/SCUTDormManagerCharacter.h"
#include "InputActionValue.h"
#include "scut/Character/NPC/SCUTTeacherCharacter.h"
#include "scut/Character/NPC/SCUTStudentCharacter.h"
#include "scut/Character/NPC/SCUTVendingMachineCharacter.h"

// Constructor remains largely the same, only adding flight state initialization
AfirstpersonCharacter::AfirstpersonCharacter()
{
    // Set this character to call Tick() every frame. You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Set character rotation
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
    GetCharacterMovement()->JumpZVelocity = 600.f;
    GetCharacterMovement()->AirControl = 0.2f; // How much control a character has in the air
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; // Initial walk speed
    GetCharacterMovement()->MaxFlySpeed = FlySpeed; // Set initial fly speed

    // Create a camera boom (pulls in towards the character if there's a collision)
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = ThirdPersonCameraDistance; // The camera follows at this distance behind the character
    CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

    // Create a follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

    // Initialize states
    bIsRunning = false;
    bIsInDialogue = false;
    CurrentDialogueTarget = nullptr;
    bIsFirstPersonView = false;
    bIsSitting = false;
    bIsSittingDown = false;
    bIsStandingUp = false;
    bIsMovingToSeat = false;
    bIsMovementLocked = false;
    bIsBeingQuestioned = false;
    PlayerDrinkState = EPlayerDrinkState::None; // Initialize drink state

    // Initialize flight state
    bIsFlying = false;
}

void AfirstpersonCharacter::BeginPlay()
{
    Super::BeginPlay();
    SetMovementSpeed(WalkSpeed); // Ensure initial speed is set

    // Add Input Mapping Context
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }

        // It's generally best practice to bind all input actions in SetupPlayerInputComponent.
        // However, if you explicitly need these bound here for some reason, ensure they are also
        // correctly set up in SetupPlayerInputComponent to avoid unexpected behavior or rebinding issues.
        // I've moved the key bindings you had here to SetupPlayerInputComponent for consistency.
    }
}

void AfirstpersonCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // If moving to seat, handle interpolated movement
    if (bIsMovingToSeat)
    {
        FVector TargetLocation = ChairLocation;
        FVector CurrentLocation = GetActorLocation();
        FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();

        // Calculate move distance
        float MoveDistance = SittingMoveSpeed * DeltaTime;
        FVector NewLocation = CurrentLocation + Direction * MoveDistance;

        // If already close enough to the target location
        if (FVector::Distance(NewLocation, TargetLocation) < MoveDistance)
        {
            NewLocation = TargetLocation;
            bIsMovingToSeat = false;
            // Start sitting animation
            StartSittingAnimation();
        }

        SetActorLocation(NewLocation);

        // Smoothly rotate to the target orientation
        FRotator CurrentRotation = GetActorRotation();
        FRotator TargetRotation = ChairRotation;
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 5.0f);
        SetActorRotation(NewRotation);
    }
    // Update character state based on movement or other conditions
    else
    {
        if (GetVelocity().Size() > 0.0f)
        {
            if (GetCharacterMovement()->IsFlying())
            {
                UpdateCharacterState(ECharacterState::Flying); // Assuming you have a Flying state in ECharacterState
            }
            else
            {
                UpdateCharacterState(bIsRunning ? ECharacterState::Running : ECharacterState::Walking);
            }
        }
        else
        {
            // If movement is locked (e.g., during dialogue, sitting, or drinking), don't set to Idle unless it's a specific idle state.
            // You might want a more granular state for "locked but not moving" vs "idle".
            // For now, let's keep it simple: if not moving and not specifically flying, assume idle or locked idle.
            if (!bIsMovementLocked)
            {
                UpdateCharacterState(ECharacterState::Idle);
            }
            // else, the state should be handled by the locking mechanism (e.g., Talking, Sitting, Buying, Drinking)
        }
    }
}

void AfirstpersonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Get the Enhanced Input Component
    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Bind Move Input
        if (MoveAction)
        {
            EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AfirstpersonCharacter::Move);
            EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AfirstpersonCharacter::Move); // For when input stops
        }

        // Bind Look Input
        if (LookAction)
            EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AfirstpersonCharacter::Look);

        // Bind Run Input
        if (RunAction)
        {
            EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &AfirstpersonCharacter::StartRunning);
            EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &AfirstpersonCharacter::StopRunning);
        }

        // Bind Jump Input
        if (JumpAction)
        {
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AfirstpersonCharacter::JumpInput);
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AfirstpersonCharacter::StopJumpingInput);
        }

        // Bind Interact Input (Dorm Manager specific, consider generalizing)
        if (InteractAction)
            EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AfirstpersonCharacter::OnInteractDormManager);
        if (DialogueAction)
            EnhancedInputComponent->BindAction(DialogueAction, ETriggerEvent::Started, this, &AfirstpersonCharacter::OnDialogueDormManager);
        if (EndDialogueAction)
            EnhancedInputComponent->BindAction(EndDialogueAction, ETriggerEvent::Started, this, &AfirstpersonCharacter::OnEndDialogueDormManager);

        // Bind Sitting Input
        if (SittingAction)
            EnhancedInputComponent->BindAction(SittingAction, ETriggerEvent::Started, this, &AfirstpersonCharacter::OnSittingAction);

        // Bind Teacher Actions
        if (RollCallAction)
            EnhancedInputComponent->BindAction(RollCallAction, ETriggerEvent::Started, this, &AfirstpersonCharacter::OnTeacherRollCallAction);
        if (QuestionAction)
            EnhancedInputComponent->BindAction(QuestionAction, ETriggerEvent::Started, this, &AfirstpersonCharacter::OnTeacherQuestionAction);
        if (EndQuestionAction)
            EnhancedInputComponent->BindAction(EndQuestionAction, ETriggerEvent::Started, this, &AfirstpersonCharacter::OnTeacherEndQuestionAction);

        // Bind Student Dialogue Input
        if (StudentDialogueAction)
            EnhancedInputComponent->BindAction(StudentDialogueAction, ETriggerEvent::Started, this, &AfirstpersonCharacter::OnStudentDialogueAction);
        if (StudentEndDialogueAction)
            EnhancedInputComponent->BindAction(StudentEndDialogueAction, ETriggerEvent::Started, this, &AfirstpersonCharacter::OnStudentEndDialogueAction);

        // Bind Vending Machine Interact Input
        if (VendingMachineInteractAction)
            EnhancedInputComponent->BindAction(VendingMachineInteractAction, ETriggerEvent::Started, this, &AfirstpersonCharacter::OnVendingMachineInteract);

        // Bind Toggle Flight Input
        if (ToggleFlyAction)
            EnhancedInputComponent->BindAction(ToggleFlyAction, ETriggerEvent::Started, this, &AfirstpersonCharacter::ToggleFlight);

        // Bind Vertical Fly Input
        if (VerticalFlyAction)
            EnhancedInputComponent->BindAction(VerticalFlyAction, ETriggerEvent::Triggered, this, &AfirstpersonCharacter::FlyVertical);
    }
}

void AfirstpersonCharacter::SetCameraDistance(float NewDistance)
{
    if (CameraBoom)
    {
        CameraBoom->TargetArmLength = NewDistance;
    }
}

void AfirstpersonCharacter::SetCameraRotation(const FRotator& NewRotation)
{
    if (CameraBoom)
    {
        CameraBoom->SetWorldRotation(NewRotation);
    }
}

void AfirstpersonCharacter::SetMovementSpeed(float NewSpeed)
{
    if (GetCharacterMovement())
    {
        // Adjust MaxWalkSpeed for walking/running and MaxFlySpeed for flying
        if (GetCharacterMovement()->MovementMode == MOVE_Walking || GetCharacterMovement()->MovementMode == MOVE_Falling || GetCharacterMovement()->MovementMode == MOVE_NavWalking)
        {
            GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
        }
        else if (GetCharacterMovement()->MovementMode == MOVE_Flying)
        {
            GetCharacterMovement()->MaxFlySpeed = NewSpeed;
        }
    }
}

bool AfirstpersonCharacter::IsMoving() const
{
    return GetVelocity().Size() > 0.0f;
}

bool AfirstpersonCharacter::IsRunning() const
{
    return bIsRunning;
}

void AfirstpersonCharacter::Move(const FInputActionValue& Value)
{
    // Check if movement is locked
    if (bIsMovementLocked)
    {
        return;
    }

    if (Controller != nullptr)
    {
        const FVector2D MovementVector = Value.Get<FVector2D>();

        // Get the control rotation to determine forward/right directions
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0); // Only consider yaw for movement direction

        // Get forward and right vectors
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // Add movement input in the corresponding directions
        // For flying, movement is handled more directly, but AddMovementInput will still work.
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void AfirstpersonCharacter::Look(const FInputActionValue& Value)
{
    if (Controller != nullptr)
    {
        const FVector2D LookAxisVector = Value.Get<FVector2D>();
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void AfirstpersonCharacter::StartRunning(const FInputActionValue& Value)
{
    if (!bIsFlying) // Only run if not flying
    {
        bIsRunning = true;
        SetMovementSpeed(RunSpeed);
    }
}

void AfirstpersonCharacter::StopRunning(const FInputActionValue& Value)
{
    if (!bIsFlying) // Only stop running if not flying
    {
        bIsRunning = false;
        SetMovementSpeed(WalkSpeed);
    }
}

void AfirstpersonCharacter::JumpInput(const FInputActionValue& Value)
{
    // Only allow jumping if not flying and movement is not locked
    if (!bIsFlying && !bIsMovementLocked)
    {
        Super::Jump(); // Call the base ACharacter's default jump behavior
    }
}

void AfirstpersonCharacter::StopJumpingInput(const FInputActionValue& Value)
{
    // Call the base ACharacter's default stop jumping behavior
    Super::StopJumping();
}


void AfirstpersonCharacter::StartDialogue(ASCUTNPCCharacter* NPCTarget)
{
    if (NPCTarget && !bIsInDialogue)
    {
        bIsInDialogue = true;
        CurrentDialogueTarget = NPCTarget;
        UpdateCharacterState(ECharacterState::Talking); // Update character state to Talking
        NPCTarget->StartDialogue();
        bIsMovementLocked = true; // Lock player movement during dialogue
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->DisableMovement();
        }
    }
}

void AfirstpersonCharacter::EndDialogue()
{
    if (bIsInDialogue && CurrentDialogueTarget)
    {
        bIsInDialogue = false;
        CurrentDialogueTarget->EndDialogue();
        CurrentDialogueTarget = nullptr;
        UpdateCharacterState(ECharacterState::Idle); // Reset character state to Idle
        bIsMovementLocked = false; // Unlock player movement
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        }
    }
}

void AfirstpersonCharacter::SelectDialogueOption(int32 OptionIndex)
{
    if (bIsInDialogue && CurrentDialogueTarget)
    {
        // Add dialogue option processing logic here
        // e.g., trigger specific events or change NPC state
    }
}

ASCUTNPCCharacter* AfirstpersonCharacter::FindInteractableNPC()
{
    FVector PlayerLocation = GetActorLocation();
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASCUTNPCCharacter::StaticClass(), FoundActors);

    ASCUTNPCCharacter* ClosestNPC = nullptr;
    float ClosestDistance = TeacherInteractionRange; // Use teacher interaction range as default for general NPC

    for (AActor* Actor : FoundActors)
    {
        ASCUTNPCCharacter* NPC = Cast<ASCUTNPCCharacter>(Actor);
        if (NPC)
        {
            float Distance = FVector::Distance(PlayerLocation, NPC->GetActorLocation());
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                ClosestNPC = NPC;
            }
        }
    }
    return ClosestNPC;
}

void AfirstpersonCharacter::Interact(const FInputActionValue& Value)
{
    if (bIsInDialogue)
    {
        EndDialogue(); // If in dialogue, interact action ends dialogue
    }
    else
    {
        ASCUTNPCCharacter* InteractableNPC = FindInteractableNPC();
        if (InteractableNPC)
        {
            StartDialogue(InteractableNPC); // Otherwise, attempt to start dialogue with an NPC
        }
    }
}

void AfirstpersonCharacter::SetFirstPersonView()
{
    if (!bIsFirstPersonView)
    {
        bIsFirstPersonView = true;

        if (CameraBoom)
        {
            CameraBoom->TargetArmLength = FirstPersonCameraOffset;
            CameraBoom->bDoCollisionTest = false;
            CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, FirstPersonCameraHeight)); // Set camera height
        }

        if (FollowCamera)
        {
            FollowCamera->bUsePawnControlRotation = true;
        }

        if (GetMesh())
        {
            GetMesh()->SetVisibility(false); // Hide character mesh
        }
    }
}

void AfirstpersonCharacter::SetThirdPersonView()
{
    if (bIsFirstPersonView)
    {
        bIsFirstPersonView = false;

        if (CameraBoom)
        {
            CameraBoom->TargetArmLength = ThirdPersonCameraDistance;
            CameraBoom->bDoCollisionTest = true;
            CameraBoom->SetRelativeLocation(FVector::ZeroVector); // Reset camera position
        }

        if (FollowCamera)
        {
            FollowCamera->bUsePawnControlRotation = false;
        }

        if (GetMesh())
        {
            GetMesh()->SetVisibility(true); // Show character mesh
        }
    }
}

void AfirstpersonCharacter::SetupDormManagerInput(UInputAction* InInteractAction, UInputAction* InDialogueAction, UInputAction* InEndDialogueAction)
{
    // This function seems to be setting input actions on a DormManager instance.
    // Ensure that DormManager actually exists and is cast correctly.
    // It's usually better to have DormManager handle its own input binding if it's a separate character.
    // If this is meant to *give* the player control over DormManager's actions, then this pattern is ok.
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASCUTDormManagerCharacter::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        ASCUTDormManagerCharacter* DormManager = Cast<ASCUTDormManagerCharacter>(FoundActors[0]);
        if (DormManager)
        {
            DormManager->SetupInputActions(InInteractAction, InDialogueAction, InEndDialogueAction);
        }
    }
}

ASCUTDormManagerCharacter* AfirstpersonCharacter::FindDormManagerInRange(float Range)
{
    TArray<AActor*> FoundActors;
    if (!this->GetWorld()) return nullptr;
    UGameplayStatics::GetAllActorsOfClass(this->GetWorld(), ASCUTDormManagerCharacter::StaticClass(), FoundActors);

    ASCUTDormManagerCharacter* ClosestDormManager = nullptr;
    float ClosestDistance = Range;

    for (AActor* Actor : FoundActors)
    {
        ASCUTDormManagerCharacter* DormManager = Cast<ASCUTDormManagerCharacter>(Actor);
        if (DormManager)
        {
            float Distance = FVector::Distance(this->GetActorLocation(), DormManager->GetActorLocation());
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                ClosestDormManager = DormManager;
            }
        }
    }
    return ClosestDormManager;
}

bool AfirstpersonCharacter::IsInChairRange() const
{
    return FVector::Distance(GetActorLocation(), ChairLocation) <= ChairInteractionRange;
}

void AfirstpersonCharacter::SitDown()
{
    if (!bIsSitting && !bIsMovingToSeat && !bIsFlying) // Cannot sit down if flying
    {
        float DistanceToChair = FVector::Distance(GetActorLocation(), ChairLocation);
        if (DistanceToChair <= ChairInteractionRange)
        {
            ASCUTTeacherCharacter* Teacher = FindTeacherInRange(TeacherInteractionRange);
            if (Teacher)
            {
                bIsMovementLocked = true;
                if (GetCharacterMovement())
                {
                    GetCharacterMovement()->DisableMovement();
                }

                SetActorLocation(ChairLocation + SittingOffset);
                FVector TeacherLocation = Teacher->GetActorLocation();
                FRotator LookAtRotation = (TeacherLocation - ChairLocation).Rotation();
                SetActorRotation(LookAtRotation);

                StartSittingAnimation();
                UE_LOG(LogTemp, Warning, TEXT("[Player] SitDown: Sitting down at chair and facing teacher. Distance to chair: %f"), DistanceToChair);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("[Player] SitDown: In chair range but no teacher found"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[Player] SitDown: Not close enough to chair. Distance: %f, Required: %f"), DistanceToChair, ChairInteractionRange);
        }
    }
}

void AfirstpersonCharacter::StartSittingAnimation()
{
    if (!bIsSitting)
    {
        bIsSitting = true;
        bIsSittingDown = true;
        bIsMovementLocked = true;

        if (GetCharacterMovement())
        {
            GetCharacterMovement()->DisableMovement();
        }

        UpdateCharacterState(ECharacterState::Custom); // Assuming ECharacterState::Custom handles sitting animation
        UE_LOG(LogTemp, Warning, TEXT("[Player] StartSittingAnimation: bIsSitting=%d, bIsSittingDown=%d"), bIsSitting, bIsSittingDown);
        GetWorldTimerManager().SetTimer(SitDownTimerHandle, this, &AfirstpersonCharacter::OnSitDownFinished, SittingLockTime, false);
    }
}

void AfirstpersonCharacter::OnSitDownFinished()
{
    bIsSittingDown = false;
    // Keep bIsSitting true if you want the character to remain in a sitting pose until StandUp() is called
    // bIsMovementLocked = false; // Only unlock if you want movement immediately after animation
    // Re-enable movement if not locked by other states
    if (GetCharacterMovement() && !bIsMovementLocked) // Ensure it's not locked by other states
    {
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    }
    UpdateCharacterState(ECharacterState::Idle); // Or ECharacterState::SittingIdle if you have one
    UE_LOG(LogTemp, Warning, TEXT("[Player] OnSitDownFinished: bIsSittingDown=%d, bIsSitting=%d"), bIsSittingDown, bIsSitting);
}

void AfirstpersonCharacter::StandUp()
{
    if (bIsSitting)
    {
        bIsSitting = false;
        bIsStandingUp = true;
        bIsMovementLocked = true; // Lock movement during standing up animation

        if (GetCharacterMovement())
        {
            GetCharacterMovement()->DisableMovement();
        }

        UE_LOG(LogTemp, Warning, TEXT("[Player] StandUp: bIsSitting=%d, bIsStandingUp=%d"), bIsSitting, bIsStandingUp);
        GetWorldTimerManager().SetTimer(StandUpTimerHandle, this, &AfirstpersonCharacter::OnStandUpFinished, SittingLockTime, false);
    }
}

void AfirstpersonCharacter::OnStandUpFinished()
{
    bIsStandingUp = false;
    bIsMovementLocked = false; // Unlock movement after standing up

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->SetMovementMode(MOVE_Walking); // Re-enable walking movement
    }
    UE_LOG(LogTemp, Warning, TEXT("[Player] OnStandUpFinished: bIsStandingUp=%d"), bIsStandingUp);
}

void AfirstpersonCharacter::BindInputActions()
{
    // This function is generally not necessary if all bindings are in SetupPlayerInputComponent.
    // If you need to re-bind inputs dynamically, then ensure PlayerInputComponent is valid here.
    // Removing its contents for clarity as SetupPlayerInputComponent handles all bindings.
}

void AfirstpersonCharacter::OnSittingAction(const FInputActionValue& Value)
{
    // If character is sitting or currently in the sitting-down animation, stand up; otherwise try to sit down
    if (bIsSitting || bIsSittingDown)
    {
        StandUp();
    }
    else
    {
        SitDown();
    }
}

ASCUTTeacherCharacter* AfirstpersonCharacter::FindTeacherInRange(float Range)
{
    TArray<AActor*> FoundActors;
    if (!this->GetWorld()) return nullptr;
    UGameplayStatics::GetAllActorsOfClass(this->GetWorld(), ASCUTTeacherCharacter::StaticClass(), FoundActors);

    ASCUTTeacherCharacter* ClosestTeacher = nullptr;
    float ClosestDistance = Range;

    for (AActor* Actor : FoundActors)
    {
        ASCUTTeacherCharacter* Teacher = Cast<ASCUTTeacherCharacter>(Actor);
        if (Teacher)
        {
            float Distance = FVector::Distance(this->GetActorLocation(), Teacher->GetActorLocation());
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                ClosestTeacher = Teacher;
            }
        }
    }
    return ClosestTeacher;
}

void AfirstpersonCharacter::OnTeacherRollCallAction(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("[Player] RollCall input pressed!"));
    ASCUTTeacherCharacter* Teacher = FindTeacherInRange(TeacherInteractionRange);
    if (Teacher)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Player] Found Teacher, calling StartRollCall."));
        Teacher->StartRollCall();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Player] No Teacher found in range!"));
    }
}

void AfirstpersonCharacter::OnTeacherQuestionAction(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("[Player] Question input pressed!"));
    ASCUTTeacherCharacter* Teacher = FindTeacherInRange(TeacherInteractionRange);
    if (Teacher)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Player] Found Teacher, calling StartQuestioning."));
        Teacher->StartQuestioning();
        bIsBeingQuestioned = true;
        bIsMovementLocked = true; // Lock movement when being questioned
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->DisableMovement();
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Player] No Teacher found in range!"));
    }
}

void AfirstpersonCharacter::OnTeacherEndQuestionAction(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("[Player] EndQuestion input pressed!"));
    ASCUTTeacherCharacter* Teacher = FindTeacherInRange(TeacherInteractionRange);
    if (Teacher)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Player] Found Teacher, calling EndQuestioning."));
        Teacher->EndQuestioning();
        bIsBeingQuestioned = false;
        bIsMovementLocked = false; // Unlock movement when questioning ends
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Player] No Teacher found in range!"));
    }
}

void AfirstpersonCharacter::OnInteractDormManager(const FInputActionValue& Value)
{
    ASCUTDormManagerCharacter* DormManager = FindDormManagerInRange(DormManagerInteractionRange);
    if (DormManager)
    {
        DormManager->StandUp(); // Example interaction: make dorm manager stand up
    }
}

void AfirstpersonCharacter::OnDialogueDormManager(const FInputActionValue& Value)
{
    ASCUTDormManagerCharacter* DormManager = FindDormManagerInRange(DormManagerInteractionRange);
    if (DormManager)
    {
        DormManager->StartDialogue();
        bIsTalkingToDormManager = true;
        bIsMovementLocked = true; // Lock player movement during dialogue
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->DisableMovement();
        }
    }
}

void AfirstpersonCharacter::OnEndDialogueDormManager(const FInputActionValue& Value)
{
    ASCUTDormManagerCharacter* DormManager = FindDormManagerInRange(DormManagerInteractionRange);
    if (DormManager)
    {
        DormManager->EndDialogue();
        bIsTalkingToDormManager = false;
        bIsMovementLocked = false; // Unlock player movement after dialogue
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        }
    }
}

// Corrected SetupInputActions signature to include InToggleFlyAction
void AfirstpersonCharacter::SetupInputActions(UInputAction* InMoveAction, UInputAction* InLookAction, UInputAction* InRunAction,
    UInputAction* InJumpAction, UInputAction* InInteractAction, UInputAction* InDialogueAction,
    UInputAction* InEndDialogueAction, UInputAction* InSittingAction, UInputAction* InRollCallAction,
    UInputAction* InQuestionAction, UInputAction* InEndQuestionAction, UInputAction* InToggleFlyAction) // Added InToggleFlyAction
{
    MoveAction = InMoveAction;
    LookAction = InLookAction;
    RunAction = InRunAction;
    JumpAction = InJumpAction;
    InteractAction = InInteractAction;
    DialogueAction = InDialogueAction;
    EndDialogueAction = InEndDialogueAction;
    SittingAction = InSittingAction;
    RollCallAction = InRollCallAction;
    QuestionAction = InQuestionAction;
    EndQuestionAction = InEndQuestionAction;
    ToggleFlyAction = InToggleFlyAction; // Assign the new action
    // BindInputActions(); // This function is not typically called here if SetupPlayerInputComponent handles all bindings
}

void AfirstpersonCharacter::SetupSittingAction(UInputAction* InSittingAction)
{
    SittingAction = InSittingAction;
    // BindInputActions(); // Not typically called here
}

ASCUTStudentCharacter* AfirstpersonCharacter::FindStudentInRange(float Range)
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASCUTStudentCharacter::StaticClass(), FoundActors);

    ASCUTStudentCharacter* ClosestStudent = nullptr;
    float ClosestDistance = Range;

    for (AActor* Actor : FoundActors)
    {
        ASCUTStudentCharacter* Student = Cast<ASCUTStudentCharacter>(Actor);
        if (Student)
        {
            float Distance = FVector::Distance(GetActorLocation(), Student->GetActorLocation());
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                ClosestStudent = Student;
            }
        }
    }
    return ClosestStudent;
}

void AfirstpersonCharacter::OnStudentDialogueAction(const FInputActionValue& Value)
{
    ASCUTStudentCharacter* Student = FindStudentInRange(StudentInteractionRange);
    if (Student)
    {
        Student->StartDialogue();
        bIsInDialogue = true;
        bIsMovementLocked = true; // Lock movement during student dialogue
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->DisableMovement();
        }
    }
}

void AfirstpersonCharacter::OnStudentEndDialogueAction(const FInputActionValue& Value)
{
    ASCUTStudentCharacter* Student = FindStudentInRange(StudentInteractionRange);
    if (Student)
    {
        Student->EndDialogue();
        bIsInDialogue = false;
        bIsMovementLocked = false; // Unlock movement after student dialogue
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        }
    }
}

void AfirstpersonCharacter::SetupStudentInput(UInputAction* InStudentDialogueAction, UInputAction* InStudentEndDialogueAction)
{
    StudentDialogueAction = InStudentDialogueAction;
    StudentEndDialogueAction = InStudentEndDialogueAction;
    // BindInputActions(); // Not typically called here
}

ASCUTVendingMachineCharacter* AfirstpersonCharacter::FindVendingMachineInRange(float Range)
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASCUTVendingMachineCharacter::StaticClass(), FoundActors);

    ASCUTVendingMachineCharacter* ClosestVM = nullptr;
    float ClosestDistance = Range;

    for (AActor* Actor : FoundActors)
    {
        ASCUTVendingMachineCharacter* VM = Cast<ASCUTVendingMachineCharacter>(Actor);
        if (VM)
        {
            float Distance = FVector::Distance(GetActorLocation(), VM->GetActorLocation());
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                ClosestVM = VM;
            }
        }
    }
    return ClosestVM;
}

void AfirstpersonCharacter::OnVendingMachineInteract(const FInputActionValue& Value)
{
    ASCUTVendingMachineCharacter* VM = FindVendingMachineInRange(VendingMachineInteractionRange);
    if (VM)
    {
        if (PlayerDrinkState == EPlayerDrinkState::None)
        {
            PlayerDrinkState = EPlayerDrinkState::Buying; // Play A1 animation
            bIsMovementLocked = true; // Lock movement
            // Optionally, you might want a short timer here before transitioning to Drinking
        }
        else if (PlayerDrinkState == EPlayerDrinkState::Buying)
        {
            PlayerDrinkState = EPlayerDrinkState::Drinking; // Play A2 animation
            bIsMovementLocked = true; // Lock movement
            GetWorldTimerManager().SetTimer(PlayerDrinkTimerHandle, this, &AfirstpersonCharacter::OnPlayerDrinkFinished, PlayerDrinkingDuration, false);
        }
        // No action in other states (e.g., if already drinking)
    }
}

void AfirstpersonCharacter::OnPlayerDrinkFinished()
{
    PlayerDrinkState = EPlayerDrinkState::Finished; // Play A3 animation
    bIsMovementLocked = true; // Still locked during the "finished" animation/state

    // Reset state after a short delay, allowing for new purchases
    GetWorldTimerManager().SetTimer(PlayerDrinkTimerHandle, [this]()
        {
            PlayerDrinkState = EPlayerDrinkState::None;
    bIsMovementLocked = false; // Unlock movement
    // Ensure character movement is set to walking if it was disabled
    if (GetCharacterMovement()->MovementMode == MOVE_None)
    {
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    }
        }, 1.0f, false); // 1 second delay before fully resetting
}

// --- New Flying Functionality Implementations ---

void AfirstpersonCharacter::ToggleFlight(const FInputActionValue& Value)
{
    // Prevent toggling flight if movement is locked by other mechanics (dialogue, sitting, etc.)
    if (bIsMovementLocked)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Player] Cannot toggle flight: Movement is locked."));
        return;
    }

    if (GetCharacterMovement())
    {
        if (GetCharacterMovement()->IsFlying())
        {
            // Currently flying, switch to walking
            GetCharacterMovement()->SetMovementMode(MOVE_Walking);
            bIsFlying = false;
            SetMovementSpeed(WalkSpeed); // Revert to walk speed
            UE_LOG(LogTemp, Warning, TEXT("[Player] Flight mode OFF. Switched to Walking."));
        }
        else
        {
            // Not flying, switch to flying
            GetCharacterMovement()->SetMovementMode(MOVE_Flying);
            bIsFlying = true;
            SetMovementSpeed(FlySpeed); // Apply fly speed
            UE_LOG(LogTemp, Warning, TEXT("[Player] Flight mode ON. Switched to Flying."));
        }
    }
}

void AfirstpersonCharacter::FlyVertical(const FInputActionValue& Value)
{
    // Only apply vertical movement if currently in flight mode and movement is not locked
    if (bIsFlying && !bIsMovementLocked)
    {
        const float AxisValue = Value.Get<float>();
        if (GetCharacterMovement())
        {
            // Add movement input along the UpVector for vertical flight
            AddMovementInput(FVector::UpVector, AxisValue);
        }
    }
}