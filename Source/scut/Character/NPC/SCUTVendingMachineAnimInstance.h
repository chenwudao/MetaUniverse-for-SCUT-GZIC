#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "scut/Character/NPC/SCUTVendingMachineCharacter.h"
#include "SCUTVendingMachineAnimInstance.generated.h"

UCLASS()
class SCUT_API USCUTVendingMachineAnimInstance : public UAnimInstance
{
    GENERATED_BODY()
public:
    USCUTVendingMachineAnimInstance();
protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    UPROPERTY(BlueprintReadOnly, Category = "VendingMachine|Animation")
        EVendingMachineState VendingMachineAnimState;

private:
    UPROPERTY()
        class ASCUTVendingMachineCharacter* VendingMachineCharacter;
};