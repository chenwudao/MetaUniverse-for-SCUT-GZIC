#include "MyGameMode.h"
#include "Engine/TargetPoint.h"
#include "Kismet/GameplayStatics.h"
#include "MyGameInstance.h" // 包含自定义GameInstance头文件
#include "GameFramework/PlayerStart.h" // 包含默认PlayerStart头文件
#include "MyPlayerController.h" // 包含你的PlayerController类，用于设置输入模式和鼠标
#include "Kismet/KismetSystemLibrary.h" // 用于 QuitGame
#include "Engine/World.h" // For ServerTravel
#include "Engine/DataTable.h" // For UDataTable (确保包含)

AMyGameMode::AMyGameMode()
{
    // 在这里设置默认的PlayerController和Pawn类
    PlayerControllerClass = AMyPlayerController::StaticClass();
    // DefaultPawnClass = AYourCharacterClass::StaticClass(); // 设置你的默认角色类

    // 确保你的LevelInfoDataTable在蓝图中被设置
    LevelInfoDataTable = nullptr;
}

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();

    // 可以在这里进行一些游戏模式的初始化设置
}

AActor* AMyGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetGameInstance());
    FName SpawnTagToFind = NAME_None;

    if (GameInstance)
    {
        SpawnTagToFind = GameInstance->GetAndClearTargetSpawnTag(); // 从GameInstance获取Tag
    }

    // 如果我们从GameInstance获得了一个有效的SpawnTag
    if (!SpawnTagToFind.IsNone())
    {
        TArray<AActor*> TargetPoints;
        UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ATargetPoint::StaticClass(), SpawnTagToFind, TargetPoints);

        if (TargetPoints.Num() > 0)
        {
            // 简单起见，我们取第一个找到的
            UE_LOG(LogTemp, Log, TEXT("AMyGameMode: Found TargetPoint with tag '%s'. Spawning player there."), *SpawnTagToFind.ToString());
            return TargetPoints[0];
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AMyGameMode: SpawnTag '%s' provided, but no ATargetPoint with this tag found in the level. Falling back to default PlayerStart."), *SpawnTagToFind.ToString());
        }
    }

    // 如果没有指定Tag，或者找不到对应的TargetPoint，则使用默认的PlayerStart逻辑
    UE_LOG(LogTemp, Log, TEXT("AMyGameMode: No specific spawn tag. Using default PlayerStart logic."));

    // 遍历所有PlayerStart Actor
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);

    if (PlayerStarts.Num() > 0)
    {
        // 简单返回第一个找到的PlayerStart，或者你可以实现更复杂的选择逻辑
        return PlayerStarts[FMath::RandRange(0, PlayerStarts.Num() - 1)];
    }

    // 作为最终的回退，调用父类的实现
    return Super::ChoosePlayerStart_Implementation(Player);
}

// StartGame 函数的实现
void AMyGameMode::StartGame()
{
    UE_LOG(LogTemp, Warning, TEXT("Game Mode: Starting Game!"));

    // 广播游戏开始事件，让蓝图（例如BP_MyGameMode中的事件图表）去处理UI的隐藏
    OnGameStarted.Broadcast();

    // 通常，从主菜单点击开始游戏后，会直接打开第一个游戏关卡地图
    // 对于单人游戏，UGameplayStatics::OpenLevel 足够
    UGameplayStatics::OpenLevel(this, FName("domitory")); // 替换为你的第一个游戏关卡名

    // 如果是多人游戏，并且 StartGame 按钮是用来启动一个服务器，则应该调用 ServerTravel：
    // Server_TravelToLevel(FName("domitory")); // 假设 "domitory" 也是你的LevelID
}

// QuitGame 函数的实现
void AMyGameMode::QuitGame()
{
    UE_LOG(LogTemp, Warning, TEXT("Game Mode: Quitting Game!"));
    // 广播游戏退出事件，让蓝图去处理UI的隐藏等操作
    OnGameQuit.Broadcast();

    // 从GameMode退出游戏：
    AMyPlayerController* PlayerController = Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
    if (PlayerController)
    {
        UKismetSystemLibrary::QuitGame(this, PlayerController, EQuitPreference::Quit, true);
    }
    else
    {
        // 如果没有PlayerController（例如在服务器上），仍然可以尝试退出
        UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
    }
}

// ==================== RPC 验证与实现 ====================

bool AMyGameMode::Server_TravelToLevel_Validate(FName LevelID)
{
    if (!LevelInfoDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Server_TravelToLevel_Validate: LevelInfoDataTable is null."));
        return false;
    }
    // 检查 LevelID (数据表行名) 是否存在于数据表中
    FLevelInfo* LevelInfo = LevelInfoDataTable->FindRow<FLevelInfo>(LevelID, TEXT("Server_TravelToLevel_Validate"));
    if (!LevelInfo)
    {
        UE_LOG(LogTemp, Warning, TEXT("Server_TravelToLevel_Validate: LevelID '%s' not found in data table."), *LevelID.ToString());
        return false;
    }
    return true; // 验证通过
}

void AMyGameMode::Server_TravelToLevel_Implementation(FName LevelID)
{
    // 确保只在服务器端执行传送
    if (GetLocalRole() == ROLE_Authority)
    {
        if (LevelInfoDataTable)
        {
            FLevelInfo* LevelRow = LevelInfoDataTable->FindRow<FLevelInfo>(LevelID, TEXT("Looking for LevelInfo"));
            if (LevelRow)
            {
                // 拼接完整的地图路径。根据你的 FLevelInfo，MapAssetName 字段是 FString。
                // 假设你的地图都存放在 /Game/Maps/ 目录下，且 MapAssetName 只是地图名称 (例如："Dormitory")
                FString FullMapPath = FString::Printf(TEXT("/Game/%s"), *LevelRow->MapAssetName);

                UE_LOG(LogTemp, Log, TEXT("AMyGameMode: Server travelling to level: %s (MapAssetName: %s, FullPath: %s)"),
                    *LevelRow->LevelID.ToString(), *LevelRow->MapAssetName, *FullMapPath);

                // 在多人游戏中，使用 ServerTravel
                // URL可以是地图名，也可以包含参数，例如 "?listen"
                GetWorld()->ServerTravel(FullMapPath + TEXT("?listen"), true); // true 表示无缝旅行

                // 对于单人游戏或快速测试，也可以使用 UGameplayStatics::OpenLevel (但这不会经过服务器权威)
                // UGameplayStatics::OpenLevel(GetWorld(), FName(*LevelRow->MapAssetName));
            }
            else
            {
                // 这条错误消息在验证阶段通常会被捕获，但作为双重检查保留
                UE_LOG(LogTemp, Error, TEXT("AMyGameMode: Could not find LevelInfo for LevelID: %s in DataTable (after validation)."), *LevelID.ToString());
            }
        }
        else
        {
            // 这条错误消息在验证阶段通常会被捕获，但作为双重检查保留
            UE_LOG(LogTemp, Error, TEXT("AMyGameMode: LevelInfoDataTable is not set (after validation). Cannot travel to level."));
        }
    }
}