#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/DataTable.h" // 包含DataTable
#include "MyGameMode.generated.h"

// 前向声明，避免循环引用
// class UMyLevelSelectWidget; // GameMode不直接操作UI，无需声明
// class UMyMainMenuWidget; // GameMode不直接操作UI，无需声明
class AMyPlayerController; // 声明你的PlayerController类

// 定义一个结构体来存储关卡信息
USTRUCT(BlueprintType)
struct FLevelInfo : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
        FName LevelID; // 关卡ID，用于内部标识和匹配PlayerStart Tag

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
        FString LevelName; // 显示在UI上的关卡名称

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
        FString MapAssetName; // 实际的地图资产名称 (例如：/Game/Maps/MyLevel01)

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
        UTexture2D* Thumbnail; // 关卡缩略图

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level", meta = (MultiLine = "true"))
        FText Description; // 关卡描述
};

// 声明一个委托，当游戏开始时触发 (用于蓝图)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameStarted);
// 声明一个委托，当游戏退出时触发 (用于蓝图)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameQuit);

UCLASS()
class SCUT_API AMyGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMyGameMode();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Travel")
        UDataTable* LevelInfoDataTable; // 存储关卡信息的DataTable

    // AMyGameMode.h
    UFUNCTION(Server, Reliable, WithValidation) // 添加 WithValidation
        void Server_TravelToLevel(FName LevelID);

    // 重写选择玩家出生点的函数
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

    // StartGame 函数 (可以在主菜单的Start按钮中调用)
    UFUNCTION(BlueprintCallable, Category = "Game Control")
        void StartGame();

    // QuitGame 函数 (可以在主菜单的Quit按钮中调用)
    UFUNCTION(BlueprintCallable, Category = "Game Control")
        void QuitGame();

    // 当游戏开始时触发的事件 (由蓝图监听，以执行UI操作)
    UPROPERTY(BlueprintAssignable, Category = "Game Events")
        FOnGameStarted OnGameStarted;

    // 当游戏退出时触发的事件 (由蓝图监听，以执行UI操作)
    UPROPERTY(BlueprintAssignable, Category = "Game Events")
        FOnGameQuit OnGameQuit;

    // ... 其他 GameMode 的功能，例如分数、游戏状态等 ...
};