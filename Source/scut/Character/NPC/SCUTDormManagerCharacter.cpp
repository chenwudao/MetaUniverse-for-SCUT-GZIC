#include "scut/Character/NPC/SCUTDormManagerCharacter.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

ASCUTDormManagerCharacter::ASCUTDormManagerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    PatrolTimer = 0.0f;
    PatrolInterval = 300.0f; // 默认5分钟
    bIsSitting = true;  // 默认坐着
    bIsTalking = false;
    InteractAction = nullptr;
    DialogueAction = nullptr;
    EndDialogueAction = nullptr;

    // 设置移动组件参数
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->bOrientRotationToMovement = false;
        MovementComp->bUseControllerDesiredRotation = true;
        MovementComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        MovementComp->MaxWalkSpeed = 200.0f;
    }

    // 设置默认值
    InteractionRange = 200.0f;
}

void ASCUTDormManagerCharacter::BeginPlay()
{
    Super::BeginPlay();
    StartPatrol();
    SitDown(); // 游戏开始时自动坐下
}

void ASCUTDormManagerCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    // 弹出UI
    if (bIsTalking && DialogWidget == nullptr && DialogWidgetClass)
    {
        DialogWidget = CreateWidget<UDormManagerDialogWidget>(GetWorld(), DialogWidgetClass);
        if (DialogWidget)
        {
            DialogWidget->AddToViewport();
            // 可在此处调用DialogWidget->UpdateDialogText("你好，我是宿管");
        }
    }
    // 关闭UI
    else if (!bIsTalking && DialogWidget)
    {
        DialogWidget->RemoveFromParent();
        DialogWidget = nullptr;
    }

    if (bIsPatrolling)
    {
        UpdatePatrol(DeltaSeconds);
    }
}

void ASCUTDormManagerCharacter::StartDialogue()
{
    if (!bIsTalking)
    {
        bIsTalking = true;
        UpdateNPCState(ENPCState::Talking);
        UE_LOG(LogTemp, Warning, TEXT("[DormManager] StartDialogue: bIsSitting=%d, bIsStandingUp=%d, bIsSittingDown=%d, bIsTalking=%d"), bIsSitting, bIsStandingUp, bIsSittingDown, bIsTalking);
    }
}

void ASCUTDormManagerCharacter::EndDialogue()
{
    if (bIsTalking)
    {
        bIsTalking = false;
        SitDown();
        UpdateNPCState(ENPCState::Idle);
        UE_LOG(LogTemp, Warning, TEXT("[DormManager] EndDialogue: bIsSitting=%d, bIsStandingUp=%d, bIsSittingDown=%d, bIsTalking=%d"), bIsSitting, bIsStandingUp, bIsSittingDown, bIsTalking);
    }
}

void ASCUTDormManagerCharacter::StartPatrol()
{
    if (!bIsPatrolling)
    {
        bIsPatrolling = true;
        PatrolTimer = 0.0f;
        UpdateNPCState(ENPCState::Working);
        // 开始第一次巡逻
        if (PatrolPoints.Num() > 0)
        {
            CurrentPatrolIndex = 0;
            MoveToNextPatrolPoint();
        }
    }
}

void ASCUTDormManagerCharacter::StopPatrol()
{
    if (bIsPatrolling)
    {
        bIsPatrolling = false;
        PatrolTimer = 0.0f;
        UpdateNPCState(ENPCState::Idle);
        // 停止移动
        if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
        {
            MovementComp->Velocity = FVector::ZeroVector;
        }
    }
}

void ASCUTDormManagerCharacter::SetPatrolPoints(const TArray<FVector>& Points)
{
    PatrolPoints = Points;
    if (bIsPatrolling)
    {
        CurrentPatrolIndex = 0;
        MoveToNextPatrolPoint();
    }
}

void ASCUTDormManagerCharacter::UpdatePatrol(float DeltaTime)
{
    PatrolTimer += DeltaTime;

    // 如果到达巡逻间隔时间
    if (PatrolTimer >= PatrolInterval)
    {
        PatrolTimer = 0.0f;
        MoveToNextPatrolPoint();
    }
}

void ASCUTDormManagerCharacter::SetChairLocation(const FVector& Location)
{
    ChairLocation = Location;
}

void ASCUTDormManagerCharacter::SetChairRotation(const FRotator& Rotation)
{
    ChairRotation = Rotation;
}

void ASCUTDormManagerCharacter::StandUp()
{
    if (bIsSitting)
    {
        // 起立时面向主角
        ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
        if (PlayerCharacter)
        {
            FVector ToPlayer = PlayerCharacter->GetActorLocation() - GetActorLocation();
            FRotator LookAtRotation = FRotationMatrix::MakeFromX(ToPlayer).Rotator();
            LookAtRotation.Pitch = 0.f;
            LookAtRotation.Roll = 0.f;
            SetActorRotation(LookAtRotation);
        }
        bIsSitting = false;
        bIsStandingUp = true;
        UpdateNPCState(ENPCState::Idle);
        UE_LOG(LogTemp, Warning, TEXT("[DormManager] StandUp: bIsSitting=%d, bIsStandingUp=%d, bIsSittingDown=%d, bIsTalking=%d"), bIsSitting, bIsStandingUp, bIsSittingDown, bIsTalking);
        // 2秒后自动结束起立
        GetWorldTimerManager().SetTimer(StandUpTimerHandle, this, &ASCUTDormManagerCharacter::OnStandUpFinished, 4.0f, false);
    }
}

void ASCUTDormManagerCharacter::OnStandUpFinished()
{
    bIsStandingUp = false;
    UE_LOG(LogTemp, Warning, TEXT("[DormManager] OnStandUpFinished: bIsSitting=%d, bIsStandingUp=%d, bIsSittingDown=%d, bIsTalking=%d"), bIsSitting, bIsStandingUp, bIsSittingDown, bIsTalking);
}

void ASCUTDormManagerCharacter::SitDown()
{
    if (!bIsSitting)
    {
        bIsSitting = true;
        bIsSittingDown = true;
        UpdateNPCState(ENPCState::Idle);
        UE_LOG(LogTemp, Warning, TEXT("[DormManager] SitDown: bIsSitting=%d, bIsStandingUp=%d, bIsSittingDown=%d, bIsTalking=%d"), bIsSitting, bIsStandingUp, bIsSittingDown, bIsTalking);
        // 2秒后自动结束坐下
        GetWorldTimerManager().SetTimer(SitDownTimerHandle, this, &ASCUTDormManagerCharacter::OnSitDownFinished, 4.0f, false);
    }
}

void ASCUTDormManagerCharacter::OnSitDownFinished()
{
    bIsSittingDown = false;
    UE_LOG(LogTemp, Warning, TEXT("[DormManager] OnSitDownFinished: bIsSitting=%d, bIsStandingUp=%d, bIsSittingDown=%d, bIsTalking=%d"), bIsSitting, bIsStandingUp, bIsSittingDown, bIsTalking);
}

void ASCUTDormManagerCharacter::MoveToChair()
{
    MoveToLocation(ChairLocation);
}

void ASCUTDormManagerCharacter::SetupInputActions(UInputAction* NewInteractAction, UInputAction* NewDialogueAction, UInputAction* NewEndDialogueAction)
{
    InteractAction = NewInteractAction;
    DialogueAction = NewDialogueAction;
    EndDialogueAction = NewEndDialogueAction;
    BindInputActions();
}

void ASCUTDormManagerCharacter::BindInputActions()
{
    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerController->InputComponent))
        {
            if (InteractAction)
            {
                EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ASCUTDormManagerCharacter::OnInteractAction);
            }
            if (DialogueAction)
            {
                EnhancedInputComponent->BindAction(DialogueAction, ETriggerEvent::Triggered, this, &ASCUTDormManagerCharacter::OnDialogueAction);
            }
            if (EndDialogueAction)
            {
                EnhancedInputComponent->BindAction(EndDialogueAction, ETriggerEvent::Triggered, this, &ASCUTDormManagerCharacter::OnEndDialogueAction);
            }
        }
    }
}

void ASCUTDormManagerCharacter::HandleInteractInput()
{
    // 检查玩家是否在范围内
    if (!IsPlayerInRange())
    {
        return;
    }

    // 如果正在坐着，则站起来
    if (bIsSitting)
    {
        StandUp();
    }
}

void ASCUTDormManagerCharacter::HandleDialogueInput()
{
    // 检查玩家是否在范围内
    if (!IsPlayerInRange())
    {
        return;
    }

    // 如果已经站起来且不在对话中，则开始对话
    if (!bIsTalking)
    {
        bIsTalking = true;
        StartDialogue();
    }
}

void ASCUTDormManagerCharacter::HandleEndDialogueInput()
{
    // 如果正在对话，则结束对话
    if (bIsTalking)
    {
        bIsTalking = false;
        EndDialogue();
        SitDown(); // 对话结束后坐下
    }
}

void ASCUTDormManagerCharacter::OnInteractAction(const FInputActionValue& Value)
{
    HandleInteractInput();
}

void ASCUTDormManagerCharacter::OnDialogueAction(const FInputActionValue& Value)
{
    HandleDialogueInput();
}

void ASCUTDormManagerCharacter::OnEndDialogueAction(const FInputActionValue& Value)
{
    HandleEndDialogueInput();
}

ASCUTDormManagerCharacter* ASCUTDormManagerCharacter::GetDormManager(const UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        return nullptr;
    }

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return nullptr;
    }

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ASCUTDormManagerCharacter::StaticClass(), FoundActors);

    if (FoundActors.Num() > 0)
    {
        return Cast<ASCUTDormManagerCharacter>(FoundActors[0]);
    }

    return nullptr;
}

bool ASCUTDormManagerCharacter::IsPlayerInRange() const
{
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!PlayerCharacter)
    {
        return false;
    }

    float Distance = FVector::Distance(GetActorLocation(), PlayerCharacter->GetActorLocation());
    return Distance <= InteractionRange;
}