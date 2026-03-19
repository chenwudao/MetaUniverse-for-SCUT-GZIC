// YourGame/Public/scut/MyMainMenuWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "MyMainMenuWidget.generated.h"

// 声明委托以在C++中触发关卡选择
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelSelected, FName, SelectedLevelID);

/**
 *
 */
UCLASS()
class SCUT_API UMyMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UMyMainMenuWidget(const FObjectInitializer& ObjectInitializer);

    // 委托：当在菜单中选择关卡时广播
    UPROPERTY(BlueprintAssignable, Category = "UI")
        FOnLevelSelected OnLevelSelected;

    // --- 现有绑定和函数 ---
    UPROPERTY(meta = (BindWidget))
        UCanvasPanel* Canvas_MainMenu; // 主菜单面板

    UPROPERTY(meta = (BindWidget))
        UCanvasPanel* Canvas_LevelSelect; // 关卡选择面板

    UPROPERTY(meta = (BindWidget))
        UButton* Button_BackToMainMenu; // 返回按钮

    // >>>>>>>>>> 这里是需要添加的声明！ <<<<<<<<<<
    UFUNCTION(BlueprintCallable, Category = "UI|MainMenu") // 添加 Category 以便在蓝图菜单中更好地组织
        void ShowMainMenuPart(); // 明确声明函数

    UFUNCTION(BlueprintCallable, Category = "UI|MainMenu") // 添加 Category
        void ShowLevelSelectPart(); // 明确声明函数
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    // --- 新增内容 ---

    // 绑定蓝图中放置关卡条目的容器（例如：Vertical Box 或 Scroll Box）
    UPROPERTY(meta = (BindWidget))
        UVerticalBox* VerticalBox_LevelList;

    // 用于在蓝图编辑器中指定要创建的关卡条目蓝图 Widget 类 (例如 WBP_LevelEntryWidget)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Selection")
        TSubclassOf<UUserWidget> LevelEntryWidgetClass; // 保持 UUserWidget

protected:
    virtual void NativeConstruct() override;

    UFUNCTION()
        void OnBackToMainMenuClicked();

    // 新增：用于在C++中动态生成和填充关卡列表的函数
    void PopulateLevelList(); // 这是在.cpp中实现的一个普通C++函数

    // 新增：用于接收 WBP_LevelEntryWidget 蓝图中广播的点击事件
    // 确保这个函数的签名（参数类型和数量）与 WBP_LevelEntryWidget 中的事件分发器匹配
    UFUNCTION()
        void OnLevelEntryClickedFromBlueprint(FName LevelID); // 匹配蓝图委托的 FName 参数
};