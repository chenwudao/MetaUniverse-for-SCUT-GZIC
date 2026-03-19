#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SCUTDormManagerAnimInstance.generated.h"

UCLASS()
class SCUT_API USCUTDormManagerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    USCUTDormManagerAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
        bool bIsSitting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
        bool bIsTalking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
        class ASCUTDormManagerCharacter* DormManagerCharacter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
        bool bIsStandingUp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
        bool bIsSittingDown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
        bool bIsStanding;

protected:
    void UpdateAnimationProperties();
};