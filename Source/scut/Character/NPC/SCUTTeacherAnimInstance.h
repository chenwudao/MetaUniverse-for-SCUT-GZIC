#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SCUTTeacherAnimInstance.generated.h"

UCLASS()
class SCUT_API USCUTTeacherAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    USCUTTeacherAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // 更新动画属性
    void UpdateAnimationProperties();

    // 获取角色引用
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
        class ASCUTTeacherCharacter* TeacherCharacter;

    // 动画状态变量
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
        bool bIsSitting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
        bool bIsStandingUp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
        bool bIsSittingDown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
        bool bIsRollCalling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
        bool bIsQuestioning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
        bool bIsStanding;
};