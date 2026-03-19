#include "SCUTCatAnimInstance.h"
#include "SCUTCatCharacter.h"

USCUTCatAnimInstance::USCUTCatAnimInstance()
{
    CatCharacter = nullptr;
    CurrentCatState = ECatState::Standing;
}

void USCUTCatAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // 获取所属的猫角色
    if (APawn* Owner = TryGetPawnOwner())
    {
        CatCharacter = Cast<ASCUTCatCharacter>(Owner);
    }
}

void USCUTCatAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (CatCharacter)
    {
        CurrentCatState = CatCharacter->GetCatState();
    }
}

ECatState USCUTCatAnimInstance::GetCatState() const
{
    return CurrentCatState;
}