#pragma once

#include "CoreMinimal.h"
#include "SCUTNPCCharacter.h"
#include "InputActionValue.h"
#include "scut/Public/UI/TeacherQuestionWidget.h"
#include "scut/Public/UI/TeacherRollCallWidget.h"
#include "scut/Public/UI/TeacherTalkingWidget.h"
#include "SCUTTeacherCharacter.generated.h"

class UInputAction;
class UInputComponent;

UCLASS()
class SCUT_API ASCUTTeacherCharacter : public ASCUTNPCCharacter
{
    GENERATED_BODY()

public:
    ASCUTTeacherCharacter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // 开始点名
    UFUNCTION(BlueprintCallable, Category = "Teacher|Interaction")
        void StartRollCall();

    // 结束点名
    UFUNCTION(BlueprintCallable, Category = "Teacher|Interaction")
        void EndRollCall();

    // 开始提问
    UFUNCTION(BlueprintCallable, Category = "Teacher|Interaction")
        void StartQuestioning();

    // 结束提问
    UFUNCTION(BlueprintCallable, Category = "Teacher|Interaction")
        void EndQuestioning();

    // 设置椅子位置
    UFUNCTION(BlueprintCallable, Category = "Teacher|Movement")
        void SetChairLocation(const FVector& Location);

    // 设置椅子朝向
    UFUNCTION(BlueprintCallable, Category = "Teacher|Movement")
        void SetChairRotation(const FRotator& Rotation);

    // 坐下
    UFUNCTION(BlueprintCallable, Category = "Teacher|Movement")
        void SitDown();

    // 站起来
    UFUNCTION(BlueprintCallable, Category = "Teacher|Movement")
        void StandUp();

    // 获取是否坐着
    UFUNCTION(BlueprintCallable, Category = "Teacher|State")
        bool GetIsSitting() const { return bIsSitting; }

    // 获取是否在点名
    UFUNCTION(BlueprintCallable, Category = "Teacher|State")
        bool GetIsRollCalling() const { return bIsRollCalling; }

    // 获取是否在提问
    UFUNCTION(BlueprintCallable, Category = "Teacher|State")
        bool GetIsQuestioning() const { return bIsQuestioning; }

    // 获取是否在主动提问
    UFUNCTION(BlueprintCallable, Category = "Teacher|State")
        bool GetIsAsking() const { return bIsAsking; }

    // 获取 Teacher 实例
    UFUNCTION(BlueprintCallable, Category = "Teacher|Manager", meta = (WorldContext = "WorldContextObject"))
        static ASCUTTeacherCharacter* GetTeacher(const UObject* WorldContextObject);

    // 设置输入动作
    UFUNCTION(BlueprintCallable, Category = "Teacher|Input")
        void SetupInputActions(UInputAction* InRollCallAction, UInputAction* InQuestionAction, UInputAction* InEndQuestionAction);

    // 处理点名输入
    UFUNCTION(BlueprintCallable, Category = "Teacher|Input")
        void HandleRollCallInput();

    // 处理提问输入
    UFUNCTION(BlueprintCallable, Category = "Teacher|Input")
        void HandleQuestionInput();

    // 处理结束提问输入
    UFUNCTION(BlueprintCallable, Category = "Teacher|Input")
        void HandleEndQuestionInput();

    UFUNCTION(BlueprintCallable, Category = "Teacher|Animation")
        void SetIsStandingUp(bool bNewValue) { bIsStandingUp = bNewValue; }

    UFUNCTION(BlueprintCallable, Category = "Teacher|Animation")
        void SetIsSittingDown(bool bNewValue) { bIsSittingDown = bNewValue; }

    void OnStandUpFinished();
    void OnSitDownFinished();

public:
    // 点名间隔时间
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teacher|Behavior")
        float RollCallInterval;

    // 点名计时器
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Teacher|Behavior")
        float RollCallTimer;

    // 更新点名
    virtual void UpdateRollCall(float DeltaTime);

    // 老师的工作位置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teacher|Position")
        FVector WorkPosition;

    // 老师的工作朝向
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teacher|Position")
        FRotator WorkRotation;

    // 是否在工作位置
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Teacher|State")
        bool bIsAtWorkPosition;

    // 椅子位置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teacher|Movement")
        FVector ChairLocation;

    // 椅子朝向
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teacher|Movement")
        FRotator ChairRotation;

    // 是否坐着
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Teacher|State")
        bool bIsSitting;

    // 是否在点名
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Teacher|State")
        bool bIsRollCalling;

    // 是否在提问
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Teacher|State")
        bool bIsQuestioning;

    // 是否在主动提问（上课时随机提问）
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Teacher|State")
        bool bIsAsking;

    // 交互范围
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teacher|Interaction")
        float InteractionRange;

    // 移动到工作位置
    void MoveToWorkPosition();

    // 移动到椅子位置
    void MoveToChair();

    // 检查玩家是否在交互范围内
    bool IsPlayerInRange() const;

    // 输入动作
    UPROPERTY()
        UInputAction* RollCallAction;

    UPROPERTY()
        UInputAction* QuestionAction;

    UPROPERTY()
        UInputAction* EndQuestionAction;

    // 输入处理函数
    void OnRollCallAction(const FInputActionValue& Value);
    void OnQuestionAction(const FInputActionValue& Value);
    void OnEndQuestionAction(const FInputActionValue& Value);

    // 绑定输入
    void BindInputActions();

    UPROPERTY(BlueprintReadWrite, Category = "Teacher|State")
        bool bIsStandingUp = false;

    UPROPERTY(BlueprintReadWrite, Category = "Teacher|State")
        bool bIsSittingDown = false;

    // 椅子坐下时的偏移（用于修正动画和实际位置的差异）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teacher|Movement")
        FVector SittingOffset = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dialog")
        bool bIsTalking = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
        TSubclassOf<UTeacherRollCallWidget> RollCallWidgetClass;
    UPROPERTY()
        UTeacherRollCallWidget* RollCallWidget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
        TSubclassOf<UTeacherQuestionWidget> QuestionWidgetClass;
    UPROPERTY()
        UTeacherQuestionWidget* QuestionWidget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
        TSubclassOf<UTeacherTalkingWidget> TalkingWidgetClass;
    UPROPERTY()
        UTeacherTalkingWidget* TalkingWidget = nullptr;

protected:
    UPROPERTY()
        UTeacherQuestionWidget* DialogWidget = nullptr;

private:
    FTimerHandle StandUpTimerHandle;
    FTimerHandle SitDownTimerHandle;

    bool bHasShownQuestion = false;
};