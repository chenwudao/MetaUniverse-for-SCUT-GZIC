// Fill out your copyright notice in the Description page of Project Settings.
#include "VideoScreenActor.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "MediaPlayer.h"
#include "MediaSource.h"
#include "MediaTexture.h" // 再次确认包含 MediaTexture
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
AVideoScreenActor::AVideoScreenActor()
{
    // Set this actor to call Tick() every frame. You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

    // Create and attach Components
    ScreenMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScreenMesh"));
    RootComponent = ScreenMesh;

    InteractionTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionTrigger"));
    InteractionTrigger->SetupAttachment(RootComponent);
    InteractionTrigger->SetBoxExtent(FVector(200.0f, 200.0f, 100.0f));
    InteractionTrigger->SetCollisionProfileName(TEXT("Trigger"));

    MediaSoundComponent = CreateDefaultSubobject<UMediaSoundComponent>(TEXT("MediaSoundComponent"));
    MediaSoundComponent->SetupAttachment(RootComponent);

    bLoopVideo = false;
}

// Called when the game starts or when spawned
void AVideoScreenActor::BeginPlay()
{
    Super::BeginPlay();

    InteractionTrigger->OnComponentBeginOverlap.AddDynamic(this, &AVideoScreenActor::OnOverlapBegin);
    InteractionTrigger->OnComponentEndOverlap.AddDynamic(this, &AVideoScreenActor::OnOverlapEnd);

    InitializeMediaPlayerAndMaterial();
}

// Called every frame
void AVideoScreenActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AVideoScreenActor::InitializeMediaPlayerAndMaterial()
{
    if (!MediaPlayer)
    {
        UE_LOG(LogTemp, Error, TEXT("AVideoScreenActor: MediaPlayer is not set on %s. Video will not play."), *GetName());
        return;
    }
    if (!VideoSource)
    {
        UE_LOG(LogTemp, Error, TEXT("AVideoScreenActor: VideoSource is not set on %s. Video will not play."), *GetName());
        return;
    }
    // <--- 新增：检查 VideoTexture 是否已设置
    if (!VideoTexture)
    {
        UE_LOG(LogTemp, Error, TEXT("AVideoScreenActor: VideoTexture is not set on %s. Video will not be displayed."), *GetName());
        return;
    }


    // Associate the MediaPlayer with the MediaSoundComponent
    // 注意：MediaPlayer 只需要设置一次。
    if (!MediaSoundComponent->GetMediaPlayer()) // 避免重复设置
    {
        MediaSoundComponent->SetMediaPlayer(MediaPlayer);
    }
    // 当 MediaPlayer 播放时，它会更新其关联的 MediaTexture。
    // 所以，我们直接将这个 MediaTexture 传给材质。

    // 获取第一个材质，创建动态实例
    UMaterialInterface* BaseMaterial = ScreenMesh->GetMaterial(0);
    if (!BaseMaterial)
    {
        UE_LOG(LogTemp, Error, TEXT("AVideoScreenActor: ScreenMesh does not have a material assigned. Video might not show."));
        return;
    }

    UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
    if (DynamicMaterial)
    {
        // 关键更改：直接使用我们通过 UPROPERTY 暴露的 VideoTexture 变量
        // 这个 VideoTexture 资产会在编辑器中被你配置为指向 MediaPlayer 的 MediaTexture
        DynamicMaterial->SetTextureParameterValue(TEXT("VideoTexture"), VideoTexture);
        ScreenMesh->SetMaterial(0, DynamicMaterial);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AVideoScreenActor: Failed to create Dynamic Material Instance for ScreenMesh."));
    }

    // 设置循环播放
    MediaPlayer->SetLooping(bLoopVideo);

    // 初始暂停
    MediaPlayer->Pause();
}

void AVideoScreenActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (Cast<ACharacter>(OtherActor) && MediaPlayer && VideoSource && VideoTexture) // 新增 VideoTexture 检查
    {
        UE_LOG(LogTemp, Log, TEXT("Player entered video trigger. Opening and playing video."));

        // 如果当前没有播放且没有暂停，则打开源
        if (!MediaPlayer->IsPlaying() && !MediaPlayer->IsPaused())
        {
            MediaPlayer->OpenSource(VideoSource);
        }

        MediaPlayer->Play();
    }
}

void AVideoScreenActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (Cast<ACharacter>(OtherActor) && MediaPlayer)
    {
        UE_LOG(LogTemp, Log, TEXT("Player exited video trigger. Pausing and rewinding video."));
        MediaPlayer->Pause();
        MediaPlayer->Rewind();
    }
}