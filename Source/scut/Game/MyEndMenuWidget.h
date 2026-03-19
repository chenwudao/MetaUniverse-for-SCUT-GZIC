// MyEndMenuWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h" // 包含 UButton 头文件
#include "MyEndMenuWidget.generated.h"

/**
 *
 */
UCLASS()
class SCUT_API UMyEndMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 构造函数
	UMyEndMenuWidget(const FObjectInitializer& ObjectInitializer);

protected:
	// 虚函数，在Widget初始化时调用，类似于蓝图的Event Construct
	virtual void NativeConstruct() override;

	// UI 设计器中的“继续游戏”按钮
	UPROPERTY(meta = (BindWidget)) // BindWidget 元数据标签会将此属性绑定到蓝图中的同名Widget
		UButton* BTN_ResumeGame;

	// UI 设计器中的“退出游戏”按钮
	UPROPERTY(meta = (BindWidget))
		UButton* BTN_QuitGame;

	// “继续游戏”按钮点击事件处理函数
	UFUNCTION() // UFUNCTION 宏允许在蓝图和C++之间调用此函数
		void OnResumeGameClicked();

	// “退出游戏”按钮点击事件处理函数
	UFUNCTION()
		void OnQuitGameClicked();
};