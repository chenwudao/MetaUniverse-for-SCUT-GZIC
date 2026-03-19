#include "SCUTTeacherCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

ASCUTTeacherCharacter::ASCUTTeacherCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bIsSitting = false;
    bIsRollCalling = false;
    bIsQuestioning = false;
    bIsAsking = false;
    bIsAtWorkPosition = false;
    RollCallInterval = 5.0f;
    RollCallTimer = 0.0f;
    InteractionRange = 200.0f;
    bHasShownQuestion = false;

    // 设置移动组件参数
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->bOrientRotationToMovement = false;
        MovementComp->bUseControllerDesiredRotation = true;
        MovementComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        MovementComp->MaxWalkSpeed = 150.0f;
    }
}

void ASCUTTeacherCharacter::BeginPlay()
{
    Super::BeginPlay();
    //MoveToWorkPosition();
}

void ASCUTTeacherCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateRollCall(DeltaSeconds);
    // RollCall UI
    if (bIsRollCalling && RollCallWidget == nullptr && RollCallWidgetClass)
    {
        RollCallWidget = CreateWidget<UTeacherRollCallWidget>(GetWorld(), RollCallWidgetClass);
        if (RollCallWidget) RollCallWidget->AddToViewport();
    }
    else if (!bIsRollCalling && RollCallWidget)
    {
        RollCallWidget->RemoveFromParent();
        RollCallWidget = nullptr;
    }
    // Talking UI（只在bIsTalking==true时显示）
    if (bIsTalking && TalkingWidget == nullptr && TalkingWidgetClass)
    {
        TalkingWidget = CreateWidget<UTeacherTalkingWidget>(GetWorld(), TalkingWidgetClass);
        if (TalkingWidget) TalkingWidget->AddToViewport();
    }
    else if (!bIsTalking && TalkingWidget)
    {
        TalkingWidget->RemoveFromParent();
        TalkingWidget = nullptr;
    }
}

void ASCUTTeacherCharacter::StartRollCall()
{
    UE_LOG(LogTemp, Warning, TEXT("[Teacher] StartRollCall called. bIsRollCalling=%d, bIsSitting=%d, bIsSittingDown=%d, bIsStandingUp=%d, bIsQuestioning=%d"), bIsRollCalling, bIsSitting, bIsSittingDown, bIsStandingUp, bIsQuestioning);
    if (!bIsRollCalling)
    {
        bIsRollCalling = true;
        RollCallTimer = 0.0f;
        SitDown();
        UE_LOG(LogTemp, Warning, TEXT("[Teacher] StartRollCall: bIsRollCalling=%d, bIsSitting=%d, bIsSittingDown=%d, bIsStandingUp=%d, bIsQuestioning=%d"), bIsRollCalling, bIsSitting, bIsSittingDown, bIsStandingUp, bIsQuestioning);
    }
}

void ASCUTTeacherCharacter::EndRollCall()
{
    if (bIsRollCalling)
    {
        bIsRollCalling = false;
        if (RollCallWidget)
        {
            RollCallWidget->RemoveFromParent();
            RollCallWidget = nullptr;
        }
        // 点名结束后显示题目
        if (!bHasShownQuestion && QuestionWidgetClass)
        {
            QuestionWidget = CreateWidget<UTeacherQuestionWidget>(GetWorld(), QuestionWidgetClass);
            if (QuestionWidget)
            {
                QuestionWidget->AddToViewport();
                bHasShownQuestion = true;
            }
        }
    }
}

void ASCUTTeacherCharacter::StartQuestioning()
{
    if (!bIsQuestioning && !bIsRollCalling)
    {
        bIsQuestioning = true;
        // 如果已经显示过题目，则使用 TalkingWidget
        if (bHasShownQuestion)
        {
            bIsTalking = true;
        }
        // 否则显示题目
        else if (QuestionWidgetClass)
        {
            QuestionWidget = CreateWidget<UTeacherQuestionWidget>(GetWorld(), QuestionWidgetClass);
            if (QuestionWidget)
            {
                QuestionWidget->AddToViewport();
                bHasShownQuestion = true;
            }
        }
        // 面向玩家
        if (ASCUTPlayerCharacter* PlayerCharacter = Cast<ASCUTPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
        {
            FVector ToPlayer = PlayerCharacter->GetActorLocation() - GetActorLocation();
            FRotator LookAtRotation = FRotationMatrix::MakeFromX(ToPlayer).Rotator();
            LookAtRotation.Pitch = 0.f;
            LookAtRotation.Roll = 0.f;
            SetActorRotation(LookAtRotation);
        }
        UE_LOG(LogTemp, Warning, TEXT("[Teacher] StartQuestioning: bIsRollCalling=%d, bIsSitting=%d, bIsSittingDown=%d, bIsStandingUp=%d, bIsQuestioning=%d"), bIsRollCalling, bIsSitting, bIsSittingDown, bIsStandingUp, bIsQuestioning);
    }
}

void ASCUTTeacherCharacter::EndQuestioning()
{
    UE_LOG(LogTemp, Warning, TEXT("[Teacher] EndQuestioning called. bIsRollCalling=%d, bIsSitting=%d, bIsSittingDown=%d, bIsStandingUp=%d, bIsQuestioning=%d"), bIsRollCalling, bIsSitting, bIsSittingDown, bIsStandingUp, bIsQuestioning);
    // 关闭题目界面
    if (QuestionWidget)
    {
        QuestionWidget->RemoveFromParent();
        QuestionWidget = nullptr;
    }
    // 关闭对话界面
    if (TalkingWidget)
    {
        TalkingWidget->RemoveFromParent();
        TalkingWidget = nullptr;
    }
    // 重置状态
    bIsQuestioning = false;
    bIsTalking = false;
    SetActorRotation(WorkRotation);
    UE_LOG(LogTemp, Warning, TEXT("[Teacher] EndQuestioning: bIsRollCalling=%d, bIsSitting=%d, bIsSittingDown=%d, bIsStandingUp=%d, bIsQuestioning=%d"), bIsRollCalling, bIsSitting, bIsSittingDown, bIsStandingUp, bIsQuestioning);
}

void ASCUTTeacherCharacter::SetChairLocation(const FVector& Location)
{
    ChairLocation = Location;
}

void ASCUTTeacherCharacter::SetChairRotation(const FRotator& Rotation)
{
    ChairRotation = Rotation;
}

void ASCUTTeacherCharacter::SitDown()
{
    UE_LOG(LogTemp, Warning, TEXT("[Teacher] SitDown called. bIsRollCalling=%d, bIsSitting=%d, bIsSittingDown=%d, bIsStandingUp=%d, bIsQuestioning=%d"), bIsRollCalling, bIsSitting, bIsSittingDown, bIsStandingUp, bIsQuestioning);
    if (!bIsSitting)
    {
        bIsSitting = true;
        bIsSittingDown = true;
        SetActorLocation(ChairLocation + SittingOffset);
        SetActorRotation(ChairRotation);
        UE_LOG(LogTemp, Warning, TEXT("[Teacher] SitDown: bIsRollCalling=%d, bIsSitting=%d, bIsSittingDown=%d, bIsStandingUp=%d, bIsQuestioning=%d"), bIsRollCalling, bIsSitting, bIsSittingDown, bIsStandingUp, bIsQuestioning);
        GetWorldTimerManager().SetTimer(SitDownTimerHandle, this, &ASCUTTeacherCharacter::OnSitDownFinished, 2.0f, false);
    }
}

void ASCUTTeacherCharacter::StandUp()
{
    UE_LOG(LogTemp, Warning, TEXT("[Teacher] StandUp called. bIsRollCalling=%d, bIsSitting=%d, bIsSittingDown=%d, bIsStandingUp=%d, bIsQuestioning=%d"), bIsRollCalling, bIsSitting, bIsSittingDown, bIsStandingUp, bIsQuestioning);
    if (bIsSitting)
    {
        bIsSitting = false;
        bIsStandingUp = true;
        UE_LOG(LogTemp, Warning, TEXT("[Teacher] StandUp: bIsRollCalling=%d, bIsSitting=%d, bIsSittingDown=%d, bIsStandingUp=%d, bIsQuestioning=%d"), bIsRollCalling, bIsSitting, bIsSittingDown, bIsStandingUp, bIsQuestioning);
        GetWorldTimerManager().SetTimer(StandUpTimerHandle, this, &ASCUTTeacherCharacter::OnStandUpFinished, 2.0f, false);
    }
}

void ASCUTTeacherCharacter::OnStandUpFinished()
{
    bIsStandingUp = false;
    UE_LOG(LogTemp, Warning, TEXT("[Teacher] OnStandUpFinished: bIsRollCalling=%d, bIsSitting=%d, bIsSittingDown=%d, bIsStandingUp=%d, bIsQuestioning=%d"), bIsRollCalling, bIsSitting, bIsSittingDown, bIsStandingUp, bIsQuestioning);
}

void ASCUTTeacherCharacter::OnSitDownFinished()
{
    bIsSittingDown = false;
    UE_LOG(LogTemp, Warning, TEXT("[Teacher] OnSitDownFinished: bIsRollCalling=%d, bIsSitting=%d, bIsSittingDown=%d, bIsStandingUp=%d, bIsQuestioning=%d"), bIsRollCalling, bIsSitting, bIsSittingDown, bIsStandingUp, bIsQuestioning);
}

ASCUTTeacherCharacter* ASCUTTeacherCharacter::GetTeacher(const UObject* WorldContextObject)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (World)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ASCUTTeacherCharacter::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            return Cast<ASCUTTeacherCharacter>(FoundActors[0]);
        }
    }
    return nullptr;
}

void ASCUTTeacherCharacter::SetupInputActions(UInputAction* InRollCallAction, UInputAction* InQuestionAction, UInputAction* InEndQuestionAction)
{
    RollCallAction = InRollCallAction;
    QuestionAction = InQuestionAction;
    EndQuestionAction = InEndQuestionAction;
    BindInputActions();
}

void ASCUTTeacherCharacter::HandleRollCallInput()
{
    if (!bIsRollCalling)
    {
        StartRollCall();
    }
    else
    {
        EndRollCall();
    }
}

void ASCUTTeacherCharacter::HandleQuestionInput()
{
    if (!bIsQuestioning)
    {
        StartQuestioning();
    }
}

void ASCUTTeacherCharacter::HandleEndQuestionInput()
{
    if (bIsQuestioning)
    {
        EndQuestioning();
    }
}

void ASCUTTeacherCharacter::UpdateRollCall(float DeltaTime)
{
    if (bIsRollCalling)
    {
        RollCallTimer += DeltaTime;
        UE_LOG(LogTemp, Warning, TEXT("[Teacher] UpdateRollCall: Timer=%.2f/%.2f, bIsRollCalling=%d, bIsSitting=%d, bIsSittingDown=%d, bIsStandingUp=%d, bIsQuestioning=%d"), RollCallTimer, RollCallInterval, bIsRollCalling, bIsSitting, bIsSittingDown, bIsStandingUp, bIsQuestioning);
        if (RollCallTimer >= RollCallInterval)
        {
            EndRollCall();
        }
    }
}

void ASCUTTeacherCharacter::MoveToWorkPosition()
{
    SetActorLocation(WorkPosition);
    SetActorRotation(WorkRotation);
    bIsAtWorkPosition = true;
}

void ASCUTTeacherCharacter::MoveToChair()
{
    SetActorLocation(ChairLocation);
    SetActorRotation(ChairRotation);
}

bool ASCUTTeacherCharacter::IsPlayerInRange() const
{
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (PlayerCharacter)
    {
        float Distance = FVector::Distance(GetActorLocation(), PlayerCharacter->GetActorLocation());
        return Distance <= InteractionRange;
    }
    return false;
}

void ASCUTTeacherCharacter::OnRollCallAction(const FInputActionValue& Value)
{
    HandleRollCallInput();
}

void ASCUTTeacherCharacter::OnQuestionAction(const FInputActionValue& Value)
{
    HandleQuestionInput();
}

void ASCUTTeacherCharacter::OnEndQuestionAction(const FInputActionValue& Value)
{
    HandleEndQuestionInput();
}

void ASCUTTeacherCharacter::BindInputActions()
{
    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerController->InputComponent))
        {
            if (RollCallAction)
            {
                EnhancedInputComponent->BindAction(RollCallAction, ETriggerEvent::Triggered, this, &ASCUTTeacherCharacter::OnRollCallAction);
            }
            if (QuestionAction)
            {
                EnhancedInputComponent->BindAction(QuestionAction, ETriggerEvent::Triggered, this, &ASCUTTeacherCharacter::OnQuestionAction);
            }
            if (EndQuestionAction)
            {
                EnhancedInputComponent->BindAction(EndQuestionAction, ETriggerEvent::Triggered, this, &ASCUTTeacherCharacter::OnEndQuestionAction);
            }
        }
    }
}