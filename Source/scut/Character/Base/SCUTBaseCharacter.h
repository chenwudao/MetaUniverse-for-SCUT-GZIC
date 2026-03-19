#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCUTBaseCharacter.generated.h"

// 角色状态枚举
UENUM(BlueprintType)
enum class ECharacterState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"), // 增加跳跃状态
    Talking     UMETA(DisplayName = "Talking"), // 增加对话状态
    Sitting     UMETA(DisplayName = "Sitting"), // 增加坐下状态
    SittingDown UMETA(DisplayName = "SittingDown"), // 坐下动画中
    StandingUp  UMETA(DisplayName = "StandingUp"), // 站起动画中
    Custom      UMETA(DisplayName = "CustomAnimation"), // 用于播放自定义动画，例如跳跃后的落地等
    Flying      UMETA(DisplayName = "飞行"),
};

UCLASS()
class SCUT_API ASCUTBaseCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ASCUTBaseCharacter();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // 获取角色当前状态
    UFUNCTION(BlueprintPure, Category = "Character|State")
    ECharacterState GetCharacterState() const { return CurrentState; }

    // 获取角色位置
    UFUNCTION(BlueprintPure, Category = "Character|Position")
    FVector GetCharacterLocation() const { return GetActorLocation(); }

    // 获取角色速度
    UFUNCTION(BlueprintPure, Category = "Character|Movement")
    float GetCharacterSpeed() const;

    // 获取奔跑状态
    UFUNCTION(BlueprintPure, Category = "Character|Movement")
    virtual bool IsRunning() const { return false; }

protected:
    // 当前角色状态
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|State")
    ECharacterState CurrentState;

    // 角色名称
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Info")
    FString CharacterName;

    // 角色描述
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Info")
    FString CharacterDescription;

    // 更新角色状态
    UFUNCTION(BlueprintCallable, Category = "Character|State")
    virtual void UpdateCharacterState(ECharacterState NewState);

    // 处理移动输入
    virtual void HandleMovementInput(const FVector2D& MovementVector);

    // 处理旋转输入
    virtual void HandleRotationInput(const FVector2D& RotationVector);
}; 