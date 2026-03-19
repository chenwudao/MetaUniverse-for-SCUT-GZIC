#include "scut/Character/NPC/SCUTVendingMachineAnimInstance.h"
#include "scut/Character/NPC/SCUTVendingMachineCharacter.h"

USCUTVendingMachineAnimInstance::USCUTVendingMachineAnimInstance()
{
    VendingMachineAnimState = EVendingMachineState::Idle;
}

void USCUTVendingMachineAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    VendingMachineCharacter = Cast<ASCUTVendingMachineCharacter>(TryGetPawnOwner());
}

void USCUTVendingMachineAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    if (!VendingMachineCharacter)
    {
        VendingMachineCharacter = Cast<ASCUTVendingMachineCharacter>(TryGetPawnOwner());
        if (!VendingMachineCharacter) return;
    }
    VendingMachineAnimState = VendingMachineCharacter->GetVendingMachineState();
}