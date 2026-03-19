#include "Portal.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Engine/TargetPoint.h" // 如果你直接在这里查找目标点 (通常在GameMode做)
#include "MyGameInstance.h" // 假设你创建了一个自定义的GameInstance

APortal::APortal()
{
    PrimaryActorTick.bCanEverTick = false; // 通常传送门不需要每帧Tick

    // 创建碰撞体积
    CollisionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionVolume"));
    RootComponent = CollisionVolume; // 将碰撞体积设为根组件
    CollisionVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic")); // 设置碰撞预设，确保能与玩家Pawn重叠
    CollisionVolume->SetGenerateOverlapEvents(true); // 开启重叠事件

    // 创建静态网格体 (可选，用于可视化)
    PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
    PortalMesh->SetupAttachment(RootComponent); // 附加到碰撞体积上
    PortalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 传送门外观通常不需要碰撞
}

void APortal::BeginPlay()
{
    Super::BeginPlay();

    // 绑定重叠事件回调函数
    if (CollisionVolume)
    {
        CollisionVolume->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnOverlapBegin);
    }
}

void APortal::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 检查进入的是否是玩家角色
    ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
    if (PlayerCharacter && PlayerCharacter->IsLocallyControlled()) // 确保是本地玩家
    {
        if (TargetLevelName.IsNone())
        {
            UE_LOG(LogTemp, Warning, TEXT("APortal: TargetLevelName is not set!"));
            return;
        }

        UE_LOG(LogTemp, Log, TEXT("Player overlapped with portal. Teleporting to level: %s with spawn tag: %s"), *TargetLevelName.ToString(), *TargetSpawnPointTag.ToString());

        // --- 方式一: 使用GameInstance传递SpawnTag (推荐) ---
        UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetGameInstance());
        if (GameInstance)
        {
            GameInstance->SetTargetSpawnTag(TargetSpawnPointTag); // 在GameInstance中保存目标Tag
            UGameplayStatics::OpenLevel(this, TargetLevelName);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("APortal: MyGameInstance not found! Falling back to simple OpenLevel. Spawn tag may not work."));
            // --- 方式二: 简单打开关卡 (不方便传递Tag到新关卡的GameMode) ---
            // UGameplayStatics::OpenLevel(this, TargetLevelName);

            // --- 方式三: OpenLevel时通过URL传递参数 (较复杂，需要GameMode解析) ---
            // FString URL = FString::Printf(TEXT("%s?SpawnTag=%s"), *TargetLevelName.ToString(), *TargetSpawnPointTag.ToString());
            // UGameplayStatics::OpenLevel(this, FName(*URL));
        }
    }
}