#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SCUTCatCharacter.h"
#include "SCUTCatAnimInstance.generated.h"

/**
 * 猫的动画实例类
 */
UCLASS()
class SCUT_API USCUTCatAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    USCUTCatAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // 获取猫的状态
    UFUNCTION(BlueprintPure, Category = "Cat|Animation")
        ECatState GetCatState() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Cat|Animation")
        ASCUTCatCharacter* CatCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Cat|Animation")
        ECatState CurrentCatState;
};