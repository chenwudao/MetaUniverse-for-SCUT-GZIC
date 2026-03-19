#pragma once

#include "CoreMinimal.h"
#include "SCUTNPCCharacter.h"
#include "InputActionValue.h"
#include "scut/Public/UI/DormManagerDialogWidget.h"
#include "SCUTDormManagerCharacter.generated.h"

class UInputAction;
class UInputComponent;

UCLASS()
class SCUT_API ASCUTDormManagerCharacter : public ASCUTNPCCharacter
{
    GENERATED_BODY()

public:
    ASCUTDormManagerCharacter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // 开始对话
    virtual void StartDialogue() override;

    // 结束对话
    virtual void EndDialogue() override;

    // 开始巡逻
    virtual void StartPatrol() override;

    // 停止巡逻
    virtual void StopPatrol() override;

    // 设置巡逻点
    virtual void SetPatrolPoints(const TArray<FVector>& Points) override;

    // 设置椅子位置
    UFUNCTION(BlueprintCallable, Category = "NPC|Movement")
        void SetChairLocation(const FVector& Location);

    // 设置椅子朝向
    UFUNCTION(BlueprintCallable, Category = "NPC|Movement")
        void SetChairRotation(const FRotator& Rotation);

    // 坐下
    UFUNCTION(BlueprintCallable, Category = "NPC|Movement")
        void SitDown();

    // 站起来
    UFUNCTION(BlueprintCallable, Category = "NPC|Movement")
        void StandUp();

    // 获取是否坐着
    UFUNCTION(BlueprintCallable, Category = "NPC|State")
        bool GetIsSitting() const { return bIsSitting; }

    // 获取是否在对话
    UFUNCTION(BlueprintCallable, Category = "NPC|State")
        bool GetIsTalking() const { return bIsTalking; }

    // 获取 DormManager 实例
    UFUNCTION(BlueprintCallable, Category = "NPC|Manager", meta = (WorldContext = "WorldContextObject"))
        static ASCUTDormManagerCharacter* GetDormManager(const UObject* WorldContextObject);

    // 设置输入动作
    UFUNCTION(BlueprintCallable, Category = "NPC|Input")
        void SetupInputActions(UInputAction* InteractAction, UInputAction* DialogueAction, UInputAction* EndDialogueAction);

    // 处理交互输入
    UFUNCTION(BlueprintCallable, Category = "NPC|Input")
        void HandleInteractInput();

    // 处理对话输入
    UFUNCTION(BlueprintCallable, Category = "NPC|Input")
        void HandleDialogueInput();

    // 处理结束对话输入
    UFUNCTION(BlueprintCallable, Category = "NPC|Input")
        void HandleEndDialogueInput();

    UFUNCTION(BlueprintCallable, Category = "NPC|Animation")
        void SetIsStandingUp(bool bNewValue) { bIsStandingUp = bNewValue; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Animation")
        void SetIsSittingDown(bool bNewValue) { bIsSittingDown = bNewValue; }

    void OnStandUpFinished();
    void OnSitDownFinished();

    // <--- 在这里添加 GetIsStandingUp 和 GetIsSittingDown 的声明 --->
    UFUNCTION(BlueprintPure, Category = "NPC|Animation")
        bool GetIsStandingUp() const { return bIsStandingUp; } // 直接返回成员变量

    UFUNCTION(BlueprintPure, Category = "NPC|Animation")
        bool GetIsSittingDown() const { return bIsSittingDown; } // 直接返回成员变量

public:
    // 巡逻间隔时间
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DormManager|Behavior")
        float PatrolInterval;

    // 巡逻计时器
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DormManager|Behavior")
        float PatrolTimer;

    // 更新巡逻
    virtual void UpdatePatrol(float DeltaTime) override;

    // 宿管的工作位置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DormManager|Position")
        FVector WorkPosition;

    // 宿管的工作朝向
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DormManager|Position")
        FRotator WorkRotation;

    // 是否在工作位置
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DormManager|State")
        bool bIsAtWorkPosition;

    // 椅子位置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement")
        FVector ChairLocation;

    // 椅子朝向
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement")
        FRotator ChairRotation;

    // 是否坐着
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
        bool bIsSitting;

    // 是否在对话
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NPC|State")
        bool bIsTalking = false;

    // 交互范围
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Interaction")
        float InteractionRange;

    // 移动到工作位置
    void MoveToWorkPosition();

    // 移动到椅子位置
    void MoveToChair();

    // 检查玩家是否在交互范围内
    bool IsPlayerInRange() const;

    // 输入动作
    UPROPERTY()
        UInputAction* InteractAction;

    UPROPERTY()
        UInputAction* DialogueAction;

    UPROPERTY()
        UInputAction* EndDialogueAction;

    // 输入处理函数
    void OnInteractAction(const FInputActionValue& Value);
    void OnDialogueAction(const FInputActionValue& Value);
    void OnEndDialogueAction(const FInputActionValue& Value);

    // 绑定输入
    void BindInputActions();

    UPROPERTY(BlueprintReadWrite, Category = "NPC|State")
        bool bIsStandingUp = false;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|State")
        bool bIsSittingDown = false;

    // 椅子坐下时的偏移（用于修正动画和实际位置的差异）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement")
        FVector SittingOffset = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dialog")
        TSubclassOf<UDormManagerDialogWidget> DialogWidgetClass;

protected:
    UPROPERTY()
        UDormManagerDialogWidget* DialogWidget = nullptr;

private:
    FTimerHandle StandUpTimerHandle;
    FTimerHandle SitDownTimerHandle;
};