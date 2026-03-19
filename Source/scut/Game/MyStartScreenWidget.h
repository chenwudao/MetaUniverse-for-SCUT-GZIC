// MyStartScreenWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h" // 包含按钮头文件
#include "MyStartScreenWidget.generated.h"

/**
 *
 */
UCLASS()
class SCUT_API UMyStartScreenWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 在蓝图中暴露的按钮引用
    UPROPERTY(meta = (BindWidget))
        UButton* Button_StartGame; // 确保蓝图中按钮的名字和这里一致

    UPROPERTY(meta = (BindWidget))
        UButton* Button_QuitGame; // 确保蓝图中按钮的名字和这里一致

protected:
    virtual void NativeConstruct() override; // 对应蓝图的 Event Construct

    UFUNCTION()
        void OnStartGameClicked(); // 按钮点击事件处理函数

    UFUNCTION()
        void OnQuitGameClicked(); // 按钮点击事件处理函数
};