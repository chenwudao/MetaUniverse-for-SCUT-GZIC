#include "SCUTDormManagerAnimInstance.h"
#include "scut/Character/NPC/SCUTDormManagerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

USCUTDormManagerAnimInstance::USCUTDormManagerAnimInstance()
{
    bIsSitting = false;
    bIsTalking = false;
}

void USCUTDormManagerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    if (APawn* Pawn = TryGetPawnOwner())
    {
        DormManagerCharacter = Cast<ASCUTDormManagerCharacter>(Pawn);
    }
}

void USCUTDormManagerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (DormManagerCharacter == nullptr)
    {
        DormManagerCharacter = Cast<ASCUTDormManagerCharacter>(TryGetPawnOwner());
    }

    if (DormManagerCharacter)
    {
        UpdateAnimationProperties();
    }
}

void USCUTDormManagerAnimInstance::UpdateAnimationProperties()
{
    // 从角色获取坐姿状态
    bIsSitting = DormManagerCharacter->GetIsSitting();
    // 从角色获取对话状态
    bIsTalking = DormManagerCharacter->GetIsTalking();
    // 新增：同步起立、坐下、站立状态
    bIsStandingUp = DormManagerCharacter->bIsStandingUp;
    bIsSittingDown = DormManagerCharacter->bIsSittingDown;
    bIsStanding = !DormManagerCharacter->bIsSitting && !DormManagerCharacter->bIsStandingUp && !DormManagerCharacter->bIsSittingDown;
}