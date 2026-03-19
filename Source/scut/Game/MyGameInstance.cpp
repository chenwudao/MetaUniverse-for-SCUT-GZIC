// YourGame/Private/MyGameInstance.cpp
#include "MyGameInstance.h"

UMyGameInstance::UMyGameInstance()
{
    TargetSpawnTag = NAME_None;
}

void UMyGameInstance::SetTargetSpawnTag(FName Tag)
{
    TargetSpawnTag = Tag;
    UE_LOG(LogTemp, Log, TEXT("MyGameInstance: Set TargetSpawnTag to %s"), *Tag.ToString());
}

FName UMyGameInstance::GetAndClearTargetSpawnTag()
{
    FName CurrentTag = TargetSpawnTag;
    TargetSpawnTag = NAME_None; // 清除，避免下次关卡加载时再次使用
    UE_LOG(LogTemp, Log, TEXT("MyGameInstance: Got and Cleared TargetSpawnTag: %s"), *CurrentTag.ToString());
    return CurrentTag;
}