// YourGame/Public/MyGameInstance.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

UCLASS()
class SCUT_API UMyGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UMyGameInstance();

    // 在关卡切换前设置玩家的出生点Tag
    UFUNCTION(BlueprintCallable, Category = "Game Instance")
        void SetTargetSpawnTag(FName Tag);

    // 获取并清除玩家的出生点Tag (在ChoosePlayerStart_Implementation中调用一次后清除)
    UFUNCTION(BlueprintCallable, Category = "Game Instance")
        FName GetAndClearTargetSpawnTag();

private:
    UPROPERTY()
        FName TargetSpawnTag;
};