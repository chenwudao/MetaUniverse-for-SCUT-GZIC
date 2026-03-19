#include "scut/Character/NPC/SCUTCatCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"

ASCUTCatCharacter::ASCUTCatCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bHasMeowed = false;
    FollowTarget = nullptr;

    // 初始化新添加的变量
    CurrentCatState = ECatState::Standing;
    bIsPlayerVeryClose = false;
    bIsPlayerStatic = true;
    PlayerStaticTime = 0.0f;
    LastPlayerLocation = FVector::ZeroVector;

    // 设置AI控制器
    AIControllerClass = AAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    // 设置移动组件参数
    UCharacterMovementComponent* MovementComp = GetCharacterMovement();
    if (MovementComp)
    {
        MovementComp->bOrientRotationToMovement = false;
        MovementComp->bUseControllerDesiredRotation = false;
        MovementComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        MovementComp->MaxWalkSpeed = 400.0f;
        MovementComp->MinAnalogWalkSpeed = 20.0f;
        MovementComp->bRequestedMoveUseAcceleration = true;
    }
}

void ASCUTCatCharacter::BeginPlay()
{
    Super::BeginPlay();
    // 默认追随第一个玩家
    if (UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        FollowTarget = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        LastPlayerLocation = FollowTarget->GetActorLocation();
    }
}

void ASCUTCatCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (FollowTarget)
    {
        UpdatePlayerState();
        CheckAndFollowPlayer();
        UpdateCatState();
    }
}

void ASCUTCatCharacter::CheckAndFollowPlayer()
{
    if (!FollowTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cat: No follow target!"));
        return;
    }

    float Distance = FVector::Distance(GetActorLocation(), FollowTarget->GetActorLocation());

    // 临时修正值，将Follow和Stop颠倒回来 - 强制用正确的值覆盖
    float TrueFollowDistance = 1000.0f;
    float TrueStopDistance = 200.0f;

    UE_LOG(LogTemp, Log, TEXT("Cat: Distance to player: %f (True Follow: %f, True Stop: %f)"),
        Distance, TrueFollowDistance, TrueStopDistance);

    // 更新是否非常接近
    bIsPlayerVeryClose = (Distance <= TrueStopDistance);

    // 总是朝向玩家
    FVector DirectionToPlayer = (FollowTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FRotator TargetRotation = DirectionToPlayer.Rotation();

    // 使用平滑插值进行旋转
    FRotator CurrentRotation = GetActorRotation();
    FRotator NewRotation = FMath::RInterpTo(
        CurrentRotation,
        FRotator(0.0f, TargetRotation.Yaw, 0.0f),
        GetWorld()->GetDeltaSeconds(),
        8.0f
    );

    SetActorRotation(NewRotation);

    // 根据距离决定行为 - 使用硬编码的临时修正值而不是成员变量
    if (Distance <= TrueFollowDistance && Distance >= TrueStopDistance)
    {
        UE_LOG(LogTemp, Log, TEXT("Cat: In follow range, starting to move"));

        // 在追踪范围内，向玩家移动
        if (!bHasMeowed)
        {
            Meow();
            bHasMeowed = true;
        }

        // 计算目标位置 - 保持在停止距离外
        FVector TargetLocation = FollowTarget->GetActorLocation() - (DirectionToPlayer * TrueStopDistance * 0.8f);

        // 直接设置速度而不是使用AI移动
        UCharacterMovementComponent* MovementComp = GetCharacterMovement();
        if (MovementComp)
        {
            // 向目标位置移动的方向
            FVector MoveDirection = (TargetLocation - GetActorLocation()).GetSafeNormal();

            // 计算当前朝向和移动方向的夹角
            float DotProduct = FVector::DotProduct(GetActorForwardVector(), MoveDirection);

            // 设置速度 - 当猫朝向目标方向时速度更快
            float AlignmentFactor = FMath::Clamp(DotProduct, 0.3f, 1.0f);
            float Speed = 400.0f * AlignmentFactor;

            // 更新速度 - 方向始终基于猫的朝向
            FVector ForwardDirection = GetActorForwardVector();
            MovementComp->Velocity = ForwardDirection * Speed;
            MovementComp->bOrientRotationToMovement = false;

            UE_LOG(LogTemp, Log, TEXT("Cat: Setting velocity to: %s (Speed: %f, Alignment: %f)"),
                *MovementComp->Velocity.ToString(), Speed, AlignmentFactor);
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Cat: Outside follow range or too close, stopping"));

        // 超出追踪范围或太近，停止移动
        UCharacterMovementComponent* MovementComp = GetCharacterMovement();
        if (MovementComp)
        {
            MovementComp->Velocity = FVector::ZeroVector;
        }

        // 只有当超出追踪范围时才重置喵叫状态
        if (Distance > TrueFollowDistance)
        {
            bHasMeowed = false;
        }
    }
}

void ASCUTCatCharacter::UpdatePlayerState()
{
    if (!FollowTarget) return;

    // 获取玩家当前位置
    FVector CurrentPlayerLocation = FollowTarget->GetActorLocation();

    // 计算玩家移动速度
    float PlayerSpeed = FVector::Distance(CurrentPlayerLocation, LastPlayerLocation) / GetWorld()->GetDeltaSeconds();

    // 更新玩家是否静止
    bool WasStatic = bIsPlayerStatic;
    bIsPlayerStatic = (PlayerSpeed <= PlayerStaticSpeedThreshold);

    // 更新静止时间
    if (bIsPlayerStatic)
    {
        if (WasStatic)
        {
            PlayerStaticTime += GetWorld()->GetDeltaSeconds();
        }
        else
        {
            PlayerStaticTime = 0.0f;
        }
    }
    else
    {
        PlayerStaticTime = 0.0f;
    }

    // 保存当前位置用于下一帧计算
    LastPlayerLocation = CurrentPlayerLocation;
}

void ASCUTCatCharacter::UpdateCatState()
{
    // 获取当前速度
    float Speed = GetVelocity().Size();

    // 更新状态
    if (bIsPlayerVeryClose && bIsPlayerStatic && PlayerStaticTime >= RequiredStaticTime)
    {
        // 玩家非常接近且静止足够长时间，坐下
        CurrentCatState = ECatState::Sitting;
    }
    else if (Speed > PlayerStaticSpeedThreshold)
    {
        // 在移动
        CurrentCatState = ECatState::Running;
    }
    else
    {
        // 站立
        CurrentCatState = ECatState::Standing;
    }
}

void ASCUTCatCharacter::PetByPlayer()
{
    OnPet();
}