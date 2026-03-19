#pragma once

#include "CoreMinimal.h"
#include "scut/Character/Base/SCUTBaseCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "InputActionValue.h"
#include "scut/Animation/SCUTAnimInstance.h" // 确保SCUTAnimInstance.h中定义了EPlayerDrinkState或其相关枚举
#include "firstpersonCharacter.generated.h" // 确保这一行是最后一条 include

class UInputMappingContext;
class UInputAction;
class ASCUTDormManagerCharacter;
class ASCUTTeacherCharacter;
class ASCUTStudentCharacter;
class ASCUTVendingMachineCharacter;

// 确保 EPlayerDrinkState 在某个头文件中定义，例如在 SCUTAnimInstance.h 或一个新的枚举头文件
// 如果它没有被定义，这里会报错。
// 例如：
// enum class EPlayerDrinkState : uint8
// {
//     None UMETA(DisplayName = "None"),
//     Buying UMETA(DisplayName = "Buying"), // Add Buying state
//     Drinking UMETA(DisplayName = "Drinking"),
//     Finished UMETA(DisplayName = "Finished")
// };


UCLASS()
class SCUT_API AfirstpersonCharacter : public ASCUTBaseCharacter
{
    GENERATED_BODY()

public:
    AfirstpersonCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:
    virtual void Tick(float DeltaTime) override;

    // 相机控制
    UFUNCTION(BlueprintCallable, Category = "Player|Camera")
        void SetCameraDistance(float NewDistance);

    UFUNCTION(BlueprintCallable, Category = "Player|Camera")
        void SetCameraRotation(const FRotator& NewRotation);

    // 视角控制
    UFUNCTION(BlueprintCallable, Category = "Player|Camera")
        void SetFirstPersonView();

    UFUNCTION(BlueprintCallable, Category = "Player|Camera")
        void SetThirdPersonView();

    UFUNCTION(BlueprintPure, Category = "Player|Camera")
        bool IsFirstPersonView() const { return bIsFirstPersonView; }

    // 移动控制
    UFUNCTION(BlueprintCallable, Category = "Player|Movement")
        void SetMovementSpeed(float NewSpeed);

    // 获取玩家输入状态
    UFUNCTION(BlueprintPure, Category = "Player|Input")
        bool IsMoving() const;

    // 重写奔跑状态获取函数
    virtual bool IsRunning() const override;

    // 对话相关功能
    UFUNCTION(BlueprintCallable, Category = "Player|Dialogue")
        void StartDialogue(class ASCUTNPCCharacter* NPCTarget);

    UFUNCTION(BlueprintCallable, Category = "Player|Dialogue")
        void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Player|Dialogue")
        void SelectDialogueOption(int32 OptionIndex);

    UFUNCTION(BlueprintPure, Category = "Player|Dialogue")
        bool IsInDialogue() const { return bIsInDialogue; }

    UFUNCTION(BlueprintPure, Category = "Player|Dialogue")
        class ASCUTNPCCharacter* GetCurrentDialogueTarget() const { return CurrentDialogueTarget; }

    UFUNCTION(BlueprintCallable, Category = "Player|Input")
        void SetupDormManagerInput(class UInputAction* InInteractAction, class UInputAction* InDialogueAction, class UInputAction* InEndDialogueAction);

    UFUNCTION()
        void OnInteractDormManager(const FInputActionValue& Value);

    UFUNCTION()
        void OnDialogueDormManager(const FInputActionValue& Value);

    UFUNCTION()
        void OnEndDialogueDormManager(const FInputActionValue& Value);

    ASCUTDormManagerCharacter* FindDormManagerInRange(float Range);

    // 查找可交互的老师
    class ASCUTTeacherCharacter* FindTeacherInRange(float Range);

    // 是否坐着
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|State")
        bool bIsSitting;

    // 是否正在坐下
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|State")
        bool bIsSittingDown;

    // 是否正在起立
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|State")
        bool bIsStandingUp;

    // 椅子位置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Movement")
        FVector ChairLocation;

    // 椅子朝向
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Movement")
        FRotator ChairRotation;

    // 椅子坐下时的偏移（用于修正动画和实际位置的差异）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Movement")
        FVector SittingOffset = FVector::ZeroVector;

    // 坐下时的移动速度
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Movement")
        float SittingMoveSpeed = 200.0f;

    // 是否正在移动到座位
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|State")
        bool bIsMovingToSeat;

    // 坐下锁定时间
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Movement")
        float SittingLockTime = 2.0f;

    // 是否锁定移动
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|State")
        bool bIsMovementLocked;

    // 是否正在被提问
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|State")
        bool bIsBeingQuestioned;

    // 坐下
    UFUNCTION(BlueprintCallable, Category = "Player|Movement")
        void SitDown();

    // 站起来
    UFUNCTION(BlueprintCallable, Category = "Player|Movement")
        void StandUp();

    // 获取是否坐着
    UFUNCTION(BlueprintCallable, Category = "Player|State")
        bool GetIsSitting() const { return bIsSitting; }

    // 开始坐下动画
    void StartSittingAnimation();

    void OnStandUpFinished();
    void OnSitDownFinished();

    // 设置输入动作
    UFUNCTION(BlueprintCallable, Category = "Player|Input")
        void SetupInputActions(UInputAction* InMoveAction, UInputAction* InLookAction, UInputAction* InRunAction,
            UInputAction* InJumpAction, UInputAction* InInteractAction, UInputAction* InDialogueAction,
            UInputAction* InEndDialogueAction, UInputAction* InSittingAction, UInputAction* InRollCallAction,
            UInputAction* InQuestionAction, UInputAction* InEndQuestionAction, UInputAction* InToggleFlyAction // Added for flying
        );

    // 设置坐下动作
    UFUNCTION(BlueprintCallable, Category = "Player|Input")
        void SetupSittingAction(UInputAction* InSittingAction);

    // 学生交互相关
    UFUNCTION(BlueprintCallable, Category = "Player|Input")
        void SetupStudentInput(class UInputAction* InStudentDialogueAction, class UInputAction* InStudentEndDialogueAction);

    UFUNCTION()
        void OnStudentDialogueAction(const FInputActionValue& Value);

    UFUNCTION()
        void OnStudentEndDialogueAction(const FInputActionValue& Value);

    // 查找学生
    class ASCUTStudentCharacter* FindStudentInRange(float Range);

    // 新增：贩卖机交互相关
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* VendingMachineInteractAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
        float VendingMachineInteractionRange = 200.0f;

    UFUNCTION()
        void OnVendingMachineInteract(const FInputActionValue& Value);

    class ASCUTVendingMachineCharacter* FindVendingMachineInRange(float Range);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VendingMachine|Drink")
        EPlayerDrinkState PlayerDrinkState = EPlayerDrinkState::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VendingMachine|Drink")
        float PlayerDrinkingDuration = 3.0f;

    FTimerHandle PlayerDrinkTimerHandle;

    UFUNCTION()
        void OnPlayerDrinkFinished();

    // Player Money
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Money")
        float PlayerMoney = 100.0f;

    // =============== 新增飞行相关属性和函数声明 ===============

    // 是否正在飞行
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|State")
        bool bIsFlying;

    // 飞行速度
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Movement")
        float FlySpeed = 800.0f;

    // 切换飞行模式的输入动作
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* ToggleFlyAction;

    // 切换飞行模式的函数
    UFUNCTION()
        void ToggleFlight(const FInputActionValue& Value);

    // 垂直方向移动（上升/下降）的输入动作
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* VerticalFlyAction;

    // 垂直方向移动的处理函数
    UFUNCTION()
        void FlyVertical(const FInputActionValue& Value);

protected:
    // 输入映射上下文
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputMappingContext* DefaultMappingContext;

    // 输入动作
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* RunAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* InteractAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* DialogueAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* EndDialogueAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* SittingAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* RollCallAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* QuestionAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* EndQuestionAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* StudentDialogueAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input")
        UInputAction* StudentEndDialogueAction;

    // 相机臂组件
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Camera")
        USpringArmComponent* CameraBoom;

    // 相机组件
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Camera")
        UCameraComponent* FollowCamera;

    // 视角设置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Camera")
        float FirstPersonCameraOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Camera")
        float FirstPersonCameraHeight = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Camera")
        float ThirdPersonCameraDistance = 300.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Camera")
        bool bIsFirstPersonView;

    // 移动速度
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Movement")
        float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Movement")
        float RunSpeed = 600.0f;

    // 跳跃相关
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Movement")
        float JumpHeight = 600.0f;

    // 输入状态
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Input")
        bool bIsRunning;

    // 对话相关
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Dialogue")
        bool bIsInDialogue;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Dialogue")
        class ASCUTNPCCharacter* CurrentDialogueTarget;

    // 交互范围
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
        float ChairInteractionRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
        float TeacherInteractionRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
        float DormManagerInteractionRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
        float StudentInteractionRange = 200.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Dialogue")
        bool bIsTalkingToDormManager = false;

private:
    // 输入处理函数
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartRunning(const FInputActionValue& Value);
    void StopRunning(const FInputActionValue& Value);

    // 新增用于增强输入系统的跳跃函数
    void JumpInput(const FInputActionValue& Value);
    void StopJumpingInput(const FInputActionValue& Value);

    void Interact(const FInputActionValue& Value);
    void OnSittingAction(const FInputActionValue& Value);

    // 查找可交互的NPC
    class ASCUTNPCCharacter* FindInteractableNPC();

    FTimerHandle StandUpTimerHandle;
    FTimerHandle SitDownTimerHandle;

    // 绑定输入 (这个函数通常用于 SetupPlayerInputComponent 中，所以如果你已经在 SetupPlayerInputComponent 中直接绑定，这里可以移除)
    void BindInputActions();

    // 触发老师行为的输入处理函数
    void OnTeacherRollCallAction(const FInputActionValue& Value);
    void OnTeacherQuestionAction(const FInputActionValue& Value);
    void OnTeacherEndQuestionAction(const FInputActionValue& Value);

    // 检查是否在椅子范围内
    bool IsInChairRange() const;
};