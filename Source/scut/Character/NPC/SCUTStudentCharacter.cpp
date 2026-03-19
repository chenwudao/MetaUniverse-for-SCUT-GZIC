#include "scut/Character/NPC/SCUTStudentCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "scut/Character/Player/SCUTPlayerCharacter.h"
#include "SCUTStudentAnimInstance.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ASCUTStudentCharacter::ASCUTStudentCharacter()
{
    // 设置默认值
    CurrentPointIndex = 0;
    StudentState = EStudentState::Walking;
    DialogueTarget = nullptr;

    // 设置移动速度
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }
}

void ASCUTStudentCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 确保有导航点
    if (NavigationPoints.Num() > 0)
    {
        CurrentPointIndex = 0;
        StudentState = EStudentState::Walking;
    }
    else
    {
        StudentState = EStudentState::Idle;
    }
}

void ASCUTStudentCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (StudentState == EStudentState::Walking && NavigationPoints.Num() > 0 && CurrentPointIndex < NavigationPoints.Num())
    {
        FVector TargetLocation = NavigationPoints[CurrentPointIndex];
        FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
        AddMovementInput(Direction, 1.0f);
        if (HasReachedDestination())
        {
            MoveToNextPoint();
        }
        else
        {
            UpdateRotation();
        }
    }

    // 通过AnimInstance的bIsTalking弹窗
    USCUTStudentAnimInstance* AnimInstance = Cast<USCUTStudentAnimInstance>(GetMesh() ? GetMesh()->GetAnimInstance() : nullptr);
    bool bAnimIsTalking = AnimInstance ? AnimInstance->bIsTalking : false;
    UE_LOG(LogTemp, Warning, TEXT("[SCUTStudentCharacter] Tick: AnimInstance bIsTalking=%d, DialogWidget=%p, DialogWidgetClass=%p"), bAnimIsTalking, DialogWidget, DialogWidgetClass ? *DialogWidgetClass->GetName() : TEXT("None"));
    if (bAnimIsTalking && DialogWidget == nullptr && DialogWidgetClass)
    {
        DialogWidget = CreateWidget<UStudentDialogWidget>(GetWorld(), DialogWidgetClass);
        if (DialogWidget)
        {
            UE_LOG(LogTemp, Warning, TEXT("[SCUTStudentCharacter] DialogWidget created and added to viewport!"));
            DialogWidget->AddToViewport();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[SCUTStudentCharacter] CreateWidget failed!"));
        }
    }
    else if (!bAnimIsTalking && DialogWidget)
    {
        DialogWidget->RemoveFromParent();
        DialogWidget = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("[SCUTStudentCharacter] DialogWidget removed from viewport!"));
    }
}

void ASCUTStudentCharacter::StartDialogue()
{
    if (StudentState != EStudentState::Talking)
    {
        // 查找玩家
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASCUTPlayerCharacter::StaticClass(), FoundActors);

        if (FoundActors.Num() > 0)
        {
            DialogueTarget = Cast<ASCUTPlayerCharacter>(FoundActors[0]);
            if (DialogueTarget)
            {
                StudentState = EStudentState::Talking;
                UpdateRotation(); // 立即更新朝向

                // 停止移动
                if (GetCharacterMovement())
                {
                    GetCharacterMovement()->DisableMovement();
                }
            }
        }
    }
}

void ASCUTStudentCharacter::EndDialogue()
{
    if (StudentState == EStudentState::Talking)
    {
        StudentState = EStudentState::Walking;
        DialogueTarget = nullptr;

        // 恢复移动
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        }

        // 更新朝向到下一个目标点
        UpdateRotation();
    }
}

void ASCUTStudentCharacter::MoveToNextPoint()
{
    CurrentPointIndex++;

    // 如果到达最后一个点，销毁角色
    if (CurrentPointIndex >= NavigationPoints.Num())
    {
        Destroy();
        return;
    }

    // 更新朝向
    UpdateRotation();
}

bool ASCUTStudentCharacter::HasReachedDestination() const
{
    if (CurrentPointIndex >= NavigationPoints.Num())
        return false;

    FVector CurrentLocation = GetActorLocation();
    FVector TargetLocation = NavigationPoints[CurrentPointIndex];
    float Distance = FVector::Distance(CurrentLocation, TargetLocation);

    return Distance <= ArrivalThreshold;
}

void ASCUTStudentCharacter::UpdateRotation()
{
    if (StudentState == EStudentState::Walking && CurrentPointIndex < NavigationPoints.Num())
    {
        // 朝向目标点
        FVector TargetLocation = NavigationPoints[CurrentPointIndex];
        FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
        FRotator NewRotation = Direction.Rotation();
        SetActorRotation(NewRotation);
    }
    else if (StudentState == EStudentState::Talking && DialogueTarget)
    {
        // 朝向对话目标（玩家）
        FVector Direction = (DialogueTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        FRotator NewRotation = Direction.Rotation();
        SetActorRotation(NewRotation);
    }
}

EStudentState ASCUTStudentCharacter::GetStudentState() const
{
    return StudentState;
}

ASCUTStudentCharacter* ASCUTStudentCharacter::GetStudent(const UObject* WorldContextObject)
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
    UGameplayStatics::GetAllActorsOfClass(World, ASCUTStudentCharacter::StaticClass(), FoundActors);

    if (FoundActors.Num() > 0)
    {
        return Cast<ASCUTStudentCharacter>(FoundActors[0]);
    }

    return nullptr;
}