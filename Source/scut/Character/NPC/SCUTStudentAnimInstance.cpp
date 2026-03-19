#include "scut/Character/NPC/SCUTStudentAnimInstance.h"
#include "scut/Character/NPC/SCUTStudentCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

USCUTStudentAnimInstance::USCUTStudentAnimInstance()
{
    bIsWalking = false;
    bIsTalking = false;
    Speed = 0.0f;
}

void USCUTStudentAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // 获取角色引用
    StudentCharacter = Cast<ASCUTStudentCharacter>(TryGetPawnOwner());
}

void USCUTStudentAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!StudentCharacter)
    {
        StudentCharacter = Cast<ASCUTStudentCharacter>(TryGetPawnOwner());
        return;
    }

    // 更新速度
    if (UCharacterMovementComponent* MovementComponent = StudentCharacter->GetCharacterMovement())
    {
        Speed = MovementComponent->Velocity.Size();
    }

    // 更新状态
    EStudentState CurrentState = StudentCharacter->GetStudentState();
    bIsWalking = (CurrentState == EStudentState::Walking);
    bIsTalking = (CurrentState == EStudentState::Talking);
}