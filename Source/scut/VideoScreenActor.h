// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h" // 包含静态网格组件头文件
#include "Components/BoxComponent.h"      // 包含盒子碰撞组件头文件
#include "MediaSoundComponent.h"          // 包含媒体声音组件头文件
#include "MediaPlayer.h"                  // 包含媒体播放器头文件
#include "MediaSource.h"                  // 包含媒体源头文件
#include "MediaTexture.h"                 // <--- ！！！新增：包含 MediaTexture 头文件 ！！！

#include "VideoScreenActor.generated.h"

UCLASS()
class SCUT_API AVideoScreenActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVideoScreenActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UStaticMeshComponent* ScreenMesh; // 视频屏幕的静态网格

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UBoxComponent* InteractionTrigger; // 玩家靠近检测触发器

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UMediaSoundComponent* MediaSoundComponent; // 媒体声音组件

	// Media Player Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video Settings")
		UMediaPlayer* MediaPlayer; // 媒体播放器资产

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video Settings")
		UMediaSource* VideoSource; // 视频源资产

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video Settings")
		UMediaTexture* VideoTexture; // <--- ！！！新增：媒体纹理资产的引用 ！！！

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video Settings")
		bool bLoopVideo; // 是否循环播放

	// Overlap Events
	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	// Helper function to initialize Media Player and Material
	void InitializeMediaPlayerAndMaterial();

public:
	// Called every frame (not strictly needed for simple video, can be turned off)
	virtual void Tick(float DeltaTime) override;
};