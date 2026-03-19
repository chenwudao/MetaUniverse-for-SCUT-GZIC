#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

class UBoxComponent; // 前向声明碰撞体组件
class UStaticMeshComponent; // 前向声明静态网格体组件 (可选，用于可视化)
class ACharacter; // 前向声明角色类

UCLASS()
class SCUT_API APortal : public AActor
{
    GENERATED_BODY()

public:
    APortal();

protected:
    virtual void BeginPlay() override;

    // 碰撞组件，用于检测玩家进入
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
        UBoxComponent* CollisionVolume;

    // 静态网格体组件，用于显示传送门的外观 (可选)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal Visual")
        UStaticMeshComponent* PortalMesh;

    // 目标关卡的名称 (在编辑器中设置)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
        FName TargetLevelName;

    // 目标关卡中目标点的Actor Tag (在编辑器中设置)
    // 我们将使用这个Tag在目标关卡中找到对应的ATargetPoint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
        FName TargetSpawnPointTag;

    // 当有Actor进入碰撞体积时调用的函数
    UFUNCTION()
        void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
    // 辅助函数，获取目标生成点的Tag (如果需要跨关卡传递)
    FName GetTargetSpawnPointTag() const { return TargetSpawnPointTag; }
};