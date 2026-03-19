#pragma once

#include "CoreMinimal.h"
#include "scut/Character/Base/SCUTBaseCharacter.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "SCUTNPCCharacter.generated.h"

// NPC状态枚举
UENUM(BlueprintType)
enum class ENPCState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Talking     UMETA(DisplayName = "Talking"),
    Working     UMETA(DisplayName = "Working"),
    Custom      UMETA(DisplayName = "Custom")
};

UCLASS()
class SCUT_API ASCUTNPCCharacter : public ASCUTBaseCharacter
{
    GENERATED_BODY()

public:
    ASCUTNPCCharacter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // 获取NPC当前状态
    UFUNCTION(BlueprintPure, Category = "NPC|State")
        ENPCState GetNPCState() const { return CurrentNPCState; }

    // 更新NPC状态
    UFUNCTION(BlueprintCallable, Category = "NPC|State")
        virtual void UpdateNPCState(ENPCState NewState);

    // 开始对话
    UFUNCTION(BlueprintCallable, Category = "NPC|Dialogue")
        virtual void StartDialogue();

    // 结束对话
    UFUNCTION(BlueprintCallable, Category = "NPC|Dialogue")
        virtual void EndDialogue();

    // 设置巡逻点
    UFUNCTION(BlueprintCallable, Category = "NPC|Movement")
        virtual void SetPatrolPoints(const TArray<FVector>& Points);

    // 开始巡逻
    UFUNCTION(BlueprintCallable, Category = "NPC|Movement")
        virtual void StartPatrol();

    // 停止巡逻
    UFUNCTION(BlueprintCallable, Category = "NPC|Movement")
        virtual void StopPatrol();

    // 移动到指定位置
    UFUNCTION(BlueprintCallable, Category = "NPC|Movement")
        virtual void MoveToLocation(const FVector& TargetLocation);

    // 停止移动
    UFUNCTION(BlueprintCallable, Category = "NPC|Movement")
        virtual void StopMovement();

    // 设置朝向
    UFUNCTION(BlueprintCallable, Category = "NPC|Movement")
        virtual void SetRotation(const FRotator& NewRotation);

protected:
    // 当前NPC状态
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
        ENPCState CurrentNPCState;

    // NPC对话内容
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Dialogue")
        TArray<FString> DialogueLines;

    // 当前对话索引
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Dialogue")
        int32 CurrentDialogueIndex;

    // 巡逻点数组
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement")
        TArray<FVector> PatrolPoints;

    // 当前巡逻点索引
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Movement")
        int32 CurrentPatrolIndex;

    // 巡逻速度
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement")
        float PatrolSpeed;

    // 是否正在巡逻
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Movement")
        bool bIsPatrolling;

    // 是否正在移动
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Movement")
        bool bIsMoving;

    // 移动速度
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement")
        float MovementSpeed;

    // 更新巡逻
    virtual void UpdatePatrol(float DeltaTime);

    // 移动到下一个巡逻点
    virtual void MoveToNextPatrolPoint();

    // 检查是否到达目标位置
    virtual bool HasReachedLocation(const FVector& TargetLocation, float AcceptanceRadius = 100.0f) const;

    // 获取导航系统
    UNavigationSystemV1* GetNavigationSystem() const;
};