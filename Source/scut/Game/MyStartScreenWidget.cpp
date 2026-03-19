// MyStartScreenWidget.cpp
#include "MyStartScreenWidget.h"
#include "Kismet/GameplayStatics.h" // 用于加载关卡和退出游戏

void UMyStartScreenWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 绑定按钮点击事件
    if (Button_StartGame)
    {
        Button_StartGame->OnClicked.AddDynamic(this, &UMyStartScreenWidget::OnStartGameClicked);
    }
    if (Button_QuitGame)
    {
        Button_QuitGame->OnClicked.AddDynamic(this, &UMyStartScreenWidget::OnQuitGameClicked);
    }
}

void UMyStartScreenWidget::OnStartGameClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Start Game Button Clicked!"));

    // 1. 将当前菜单从视口中移除
    // This removes the UMG widget from the screen and allows it to be garbage collected.
    RemoveFromParent();

    // 2. 加载一个新关卡
    UGameplayStatics::OpenLevel(this, FName("domitory")); // 替换为你的游戏关卡名
}

void UMyStartScreenWidget::OnQuitGameClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Quit Game Button Clicked!"));
    // 示例：退出游戏
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (PlayerController)
    {
        UKismetSystemLibrary::QuitGame(this, PlayerController, EQuitPreference::Quit, true);
    }
}