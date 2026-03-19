#include "scut/Animation/SCUTAnimInstance.h"
#include "scut/Character/Base/SCUTBaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "scut/Character/Player/SCUTPlayerCharacter.h"

USCUTAnimInstance::USCUTAnimInstance()
{
    Speed = 0.0f;
    bIsMoving = false;
    bIsRunning = false;
    bIsJumping = false;
    bIsSitting = false;
    bIsSittingDown = false;
    bIsStandingUp = false;
    CharacterOwner = nullptr;
}

void USCUTAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // 获取角色引用
    if (AActor* Owner = GetOwningActor())
    {
        CharacterOwner = Cast<ASCUTBaseCharacter>(Owner);
    }
}

void USCUTAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!CharacterOwner)
    {
        return;
    }

    // 更新速度
    Speed = CharacterOwner->GetCharacterSpeed();

    // 更新移动状态
    bIsMoving = Speed > 0.0f;

    // 更新奔跑状态
    bIsRunning = CharacterOwner->IsRunning();

    // 更新跳跃状态
    if (UCharacterMovementComponent* MovementComponent = CharacterOwner->GetCharacterMovement())
    {
        bIsJumping = MovementComponent->IsFalling();
    }

    // 更新坐下状态
    if (ASCUTPlayerCharacter* PlayerCharacter = Cast<ASCUTPlayerCharacter>(CharacterOwner))
    {
        bIsSitting = PlayerCharacter->GetIsSitting();
        bIsSittingDown = PlayerCharacter->bIsSittingDown;
        bIsStandingUp = PlayerCharacter->bIsStandingUp;
        // 同步喝水动画状态
        PlayerDrinkState = PlayerCharacter->PlayerDrinkState;
        bIsBuying = (PlayerDrinkState == EPlayerDrinkState::Buying);
        bIsDrinking = (PlayerDrinkState == EPlayerDrinkState::Drinking);
        bIsDrinkFinished = (PlayerDrinkState == EPlayerDrinkState::Finished);
    }
}