#pragma once

#include "CoreMinimal.h"
#include "scut/Character/NPC/SCUTNPCCharacter.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SCUTCatCharacter.generated.h"


// 定义猫的状态枚举
UENUM(BlueprintType)
enum class ECatState : uint8
{
    Standing    UMETA(DisplayName = "Standing"),
    Running     UMETA(DisplayName = "Running"),
    Sitting     UMETA(DisplayName = "Sitting")
};

UCLASS()
class SCUT_API ASCUTCatCharacter : public ASCUTNPCCharacter
{
    GENERATED_BODY()

public:
    ASCUTCatCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // 玩家靠近时喵叫（只叫一次，避免重复）
    UFUNCTION(BlueprintImplementableEvent, Category = "Cat|Sound")
        void Meow(); // 蓝图实现喵叫音效

    // 被抚摸时触发（蓝图实现动画/音效）
    UFUNCTION(BlueprintImplementableEvent, Category = "Cat|Interaction")
        void OnPet();

    // 供玩家交互调用
    UFUNCTION(BlueprintCallable, Category = "Cat|Interaction")
        void PetByPlayer();

    // 获取当前猫的状态
    UFUNCTION(BlueprintPure, Category = "Cat|State")
        ECatState GetCatState() const { return CurrentCatState; }

    // 获取玩家是否在非常接近的范围内
    UFUNCTION(BlueprintPure, Category = "Cat|State")
        bool IsPlayerVeryClose() const { return bIsPlayerVeryClose; }

    // 获取玩家是否静止
    UFUNCTION(BlueprintPure, Category = "Cat|State")
        bool IsPlayerStatic() const { return bIsPlayerStatic; }

protected:
    // 开始追踪的距离
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cat|Follow")
        float FollowDistance = 1000.0f;

    // 停止追踪的距离（与非常接近距离相同）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cat|Follow")
        float StopFollowDistance = 200.0f;

    // 非常接近的距离（触发坐下行为）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cat|Follow")
        float VeryCloseDistance = 200.0f;

    // 判定玩家静止的速度阈值
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cat|Behavior")
        float PlayerStaticSpeedThreshold = 10.0f;

    // 玩家需要保持静止的时间才触发坐下
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cat|Behavior")
        float RequiredStaticTime = 5.0f;

    // 是否已经喵叫过
    bool bHasMeowed;

    // 追随目标（玩家）
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cat|Follow")
        APawn* FollowTarget;

    // 当前猫的状态
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cat|State")
        ECatState CurrentCatState;

    // 玩家是否在非常接近的范围内
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cat|State")
        bool bIsPlayerVeryClose;

    // 玩家是否静止
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cat|State")
        bool bIsPlayerStatic;

    // 玩家保持静止的时间
    float PlayerStaticTime;

    // 上一帧玩家的位置
    FVector LastPlayerLocation;

    // 检查玩家距离并追随
    void CheckAndFollowPlayer();

    // 更新玩家状态（是否静止等）
    void UpdatePlayerState();

    // 更新猫的状态
    void UpdateCatState();
};