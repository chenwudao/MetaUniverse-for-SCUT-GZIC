#include "scut/Character/NPC/SCUTNPCCharacter.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "AIController.h"

ASCUTNPCCharacter::ASCUTNPCCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentNPCState = ENPCState::Idle;
    CurrentDialogueIndex = 0;
    CurrentPatrolIndex = 0;
    PatrolSpeed = 200.0f;
    MovementSpeed = 200.0f;
    bIsPatrolling = false;
    bIsMoving = false;

    // 设置AI控制器
    AIControllerClass = AAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    // 设置移动组件参数
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->bOrientRotationToMovement = true;
        MovementComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        MovementComp->MaxWalkSpeed = 300.0f;
    }
}

void ASCUTNPCCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void ASCUTNPCCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsPatrolling)
    {
        UpdatePatrol(DeltaTime);
    }
}

void ASCUTNPCCharacter::UpdateNPCState(ENPCState NewState)
{
    if (CurrentNPCState != NewState)
    {
        CurrentNPCState = NewState;
        // 这里可以添加状态改变时的回调或事件
    }
}

void ASCUTNPCCharacter::StartDialogue()
{
    if (CurrentNPCState != ENPCState::Talking)
    {
        UpdateNPCState(ENPCState::Talking);
        CurrentDialogueIndex = 0;
        // 这里可以触发对话UI显示
    }
}

void ASCUTNPCCharacter::EndDialogue()
{
    if (CurrentNPCState == ENPCState::Talking)
    {
        UpdateNPCState(ENPCState::Idle);
        // 这里可以隐藏对话UI
    }
}

void ASCUTNPCCharacter::SetPatrolPoints(const TArray<FVector>& Points)
{
    PatrolPoints = Points;
    CurrentPatrolIndex = 0;
}

void ASCUTNPCCharacter::StartPatrol()
{
    if (!bIsPatrolling && PatrolPoints.Num() > 0)
    {
        bIsPatrolling = true;
        UpdateNPCState(ENPCState::Walking);
        MoveToNextPatrolPoint();
    }
}

void ASCUTNPCCharacter::StopPatrol()
{
    if (bIsPatrolling)
    {
        bIsPatrolling = false;
        UpdateNPCState(ENPCState::Idle);
        StopMovement();
    }
}

void ASCUTNPCCharacter::MoveToLocation(const FVector& TargetLocation)
{
    AController* MyController = GetController();
    if (MyController)
    {
        UAIBlueprintHelperLibrary::SimpleMoveToLocation(MyController, TargetLocation);
        bIsMoving = true;
        UpdateNPCState(ENPCState::Walking);
    }
}

void ASCUTNPCCharacter::StopMovement()
{
    AController* MyController = GetController();
    if (MyController)
    {
        MyController->StopMovement();
    }
    bIsMoving = false;
    UpdateNPCState(ENPCState::Idle);
}

void ASCUTNPCCharacter::SetRotation(const FRotator& NewRotation)
{
    SetActorRotation(NewRotation);
}

void ASCUTNPCCharacter::UpdatePatrol(float DeltaTime)
{
    if (PatrolPoints.Num() == 0)
    {
        StopPatrol();
        return;
    }

    if (HasReachedLocation(PatrolPoints[CurrentPatrolIndex]))
    {
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
        MoveToNextPatrolPoint();
    }
}

void ASCUTNPCCharacter::MoveToNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        StopPatrol();
        return;
    }

    // 获取导航系统
    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (NavSystem)
    {
        // 移动到下一个巡逻点
        FNavLocation NavLocation;
        if (NavSystem->GetRandomReachablePointInRadius(PatrolPoints[CurrentPatrolIndex], 100.0f, NavLocation))
        {
            UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(), NavLocation.Location);
        }
    }
}

bool ASCUTNPCCharacter::HasReachedLocation(const FVector& TargetLocation, float AcceptanceRadius) const
{
    return FVector::Distance(GetActorLocation(), TargetLocation) <= AcceptanceRadius;
}

UNavigationSystemV1* ASCUTNPCCharacter::GetNavigationSystem() const
{
    return FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
}