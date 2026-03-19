#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "scut/Character/NPC/SCUTStudentCharacter.h"
#include "SCUTStudentAnimInstance.generated.h"

UCLASS()
class SCUT_API USCUTStudentAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    USCUTStudentAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // 动画状态
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
        bool bIsWalking;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
        bool bIsTalking;

    // 移动速度
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
        float Speed;

private:
    // 角色引用
    UPROPERTY()
        class ASCUTStudentCharacter* StudentCharacter;
};