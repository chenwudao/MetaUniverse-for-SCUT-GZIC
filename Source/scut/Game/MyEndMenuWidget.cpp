// MyEndMenuWidget.cpp

#include "MyEndMenuWidget.h"
#include "Kismet/GameplayStatics.h" // 包含 GameplayStatics 以访问游戏相关函数
#include "GameFramework/PlayerController.h" // 包含 APlayerController 头文件

UMyEndMenuWidget::UMyEndMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 可以在构造函数中做一些初始化，但对于Widget，通常在NativeConstruct中更合适
}

void UMyEndMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 确保按钮是有效的指针，避免空指针崩溃
	if (BTN_ResumeGame)
	{
		// 绑定 OnClicked 事件到 C++ 函数 OnResumeGameClicked
		BTN_ResumeGame->OnClicked.AddDynamic(this, &UMyEndMenuWidget::OnResumeGameClicked);
	}

	if (BTN_QuitGame)
	{
		// 绑定 OnClicked 事件到 C++ 函数 OnQuitGameClicked
		BTN_QuitGame->OnClicked.AddDynamic(this, &UMyEndMenuWidget::OnQuitGameClicked);
	}
}

void UMyEndMenuWidget::OnResumeGameClicked()
{
	// 从视口移除当前Widget
	RemoveFromParent();

	// 获取拥有此Widget的玩家控制器
	APlayerController* PlayerController = GetOwningPlayer();
	if (PlayerController)
	{
		// 取消游戏暂停
		UGameplayStatics::SetGamePaused(GetWorld(), false);

		// 将输入模式切换回游戏独占
		PlayerController->SetInputMode(FInputModeGameOnly());

		// 隐藏鼠标光标
		PlayerController->bShowMouseCursor = false;
	}
}

void UMyEndMenuWidget::OnQuitGameClicked()
{
	// 退出游戏
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, true);
	// 或者更简单的: FGenericPlatformMisc::RequestExit(false); (如果不需要蓝图的额外功能)
}