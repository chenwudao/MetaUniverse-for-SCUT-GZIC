#include "SCUTTeacherAnimInstance.h"
#include "SCUTTeacherCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

USCUTTeacherAnimInstance::USCUTTeacherAnimInstance()
{
    bIsSitting = false;
    bIsStandingUp = false;
    bIsSittingDown = false;
    bIsRollCalling = false;
    bIsQuestioning = false;
    bIsStanding = true;
}

void USCUTTeacherAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    TeacherCharacter = Cast<ASCUTTeacherCharacter>(TryGetPawnOwner());
}

void USCUTTeacherAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    UpdateAnimationProperties();
}

void USCUTTeacherAnimInstance::UpdateAnimationProperties()
{
    if (TeacherCharacter)
    {
        // 同步所有动画状态变量
        bIsSitting = TeacherCharacter->GetIsSitting();
        bIsStandingUp = TeacherCharacter->bIsStandingUp;
        bIsSittingDown = TeacherCharacter->bIsSittingDown;
        bIsRollCalling = TeacherCharacter->GetIsRollCalling();
        bIsQuestioning = TeacherCharacter->GetIsQuestioning();
        bIsStanding = !TeacherCharacter->GetIsSitting() && !TeacherCharacter->bIsStandingUp && !TeacherCharacter->bIsSittingDown;
    }
}