#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SCUTAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EPlayerDrinkState : uint8
{
    None UMETA(DisplayName = "None"),
    Buying UMETA(DisplayName = "Buying"),
    Drinking UMETA(DisplayName = "Drinking"),
    Finished UMETA(DisplayName = "Finished")
};

UCLASS()
class SCUT_API USCUTAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    USCUTAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VendingMachine|Drink")
        bool bIsBuying = false;

protected:
    // 动画状态变量
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
        float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
        bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
        bool bIsRunning;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
        bool bIsJumping;

    // 坐下相关动画状态
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
        bool bIsSitting;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
        bool bIsSittingDown;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
        bool bIsStandingUp;

    // 喝水动画状态
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VendingMachine|Drink")
        EPlayerDrinkState PlayerDrinkState = EPlayerDrinkState::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VendingMachine|Drink")
        bool bIsDrinking = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VendingMachine|Drink")
        bool bIsDrinkFinished = false;

private:
    // 缓存指向拥有者的指针
    class ASCUTBaseCharacter* CharacterOwner;
};