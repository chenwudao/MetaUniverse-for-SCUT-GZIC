#pragma once

#include "CoreMinimal.h"
#include "scut/Character/NPC/SCUTNPCCharacter.h"
#include "scut/Public/UI/StudentDialogWidget.h"
#include "SCUTStudentCharacter.generated.h"

class USCUTStudentAnimInstance; // 前向声明

UENUM(BlueprintType)
enum class EStudentState : uint8
{
    Walking UMETA(DisplayName = "Walking"),
    Talking UMETA(DisplayName = "Talking"),
    Idle UMETA(DisplayName = "Idle")
};

UCLASS()
class SCUT_API ASCUTStudentCharacter : public ASCUTNPCCharacter
{
    GENERATED_BODY()

public:
    ASCUTStudentCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // 导航点
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
        TArray<FVector> NavigationPoints;

    // 当前目标点索引
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
        int32 CurrentPointIndex;

    // 移动速度
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
        float WalkSpeed = 200.0f;

    // 到达目标点的距离阈值
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
        float ArrivalThreshold = 100.0f;

    // 开始对话
    virtual void StartDialogue() override;

    // 结束对话
    virtual void EndDialogue() override;

    // 获取当前状态
    UFUNCTION(BlueprintPure, Category = "State")
        EStudentState GetStudentState() const;

    // 获取学生角色实例
    UFUNCTION(BlueprintCallable, Category = "Student|Manager", meta = (WorldContext = "WorldContextObject"))
        static ASCUTStudentCharacter* GetStudent(const UObject* WorldContextObject);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
        TSubclassOf<UStudentDialogWidget> DialogWidgetClass;

protected:
    // 移动到下一个导航点
    void MoveToNextPoint();

    // 检查是否到达目标点
    bool HasReachedDestination() const;

    // 更新朝向
    void UpdateRotation();

    // 对话目标（玩家）
    UPROPERTY()
        class ASCUTPlayerCharacter* DialogueTarget;

    // 学生状态
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
        EStudentState StudentState;

    UPROPERTY()
        UStudentDialogWidget* DialogWidget = nullptr;
};