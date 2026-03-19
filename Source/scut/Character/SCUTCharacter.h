#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCUTCharacter.generated.h"

UCLASS()
class SCUT_API ASCUTCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ASCUTCharacter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // 获取场景中的宿管角色
    UFUNCTION(BlueprintCallable, Category = "Character|Interaction")
        class ASCUTDormManagerCharacter* GetDormManager() const;

    // 设置输入动作
    UFUNCTION(BlueprintCallable, Category = "Character|Input")
        void SetupDormManagerInput(class UInputAction* InteractAction, class UInputAction* DialogueAction, class UInputAction* EndDialogueAction);

protected:
    // 宿管角色引用
    UPROPERTY()
        class ASCUTDormManagerCharacter* DormManager;
};