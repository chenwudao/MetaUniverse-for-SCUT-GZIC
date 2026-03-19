// Fill out your copyright notice in the Description page of Project Settings.


#include "scut/Public/UI/VendingMachineWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "scut/Public/UI/VendingItemWidget.h"
#include "scut/Animation/SCUTAnimInstance.h"
#include "GameFramework/Character.h"
#include "scut/Character/Player/SCUTPlayerCharacter.h"

void UVendingMachineWidget::PurchaseItem(int32 ItemIndex)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    ASCUTPlayerCharacter* Player = PC ? Cast<ASCUTPlayerCharacter>(PC->GetPawn()) : nullptr;
    if (!Player) return;

    if (AvailableItems.IsValidIndex(ItemIndex))
    {
        float Price = AvailableItems[ItemIndex].Price;
        if (Player->PlayerMoney >= Price)
        {
            Player->PlayerMoney -= Price;
            // 可在此处添加购买成功逻辑
        }
        // 刷新UI
        RefreshUI();
    }
}

void UVendingMachineWidget::RefreshUI()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    ASCUTPlayerCharacter* Player = PC ? Cast<ASCUTPlayerCharacter>(PC->GetPawn()) : nullptr;
    float RealMoney = Player ? Player->PlayerMoney : 0.0f;

    if (ItemList) ItemList->ClearChildren();
    for (int32 i = 0; i < AvailableItems.Num(); ++i)
    {
        if (VendingItemWidgetClass)
        {
            UVendingItemWidget* ItemWidget = CreateWidget<UVendingItemWidget>(GetWorld(), VendingItemWidgetClass);
            if (ItemWidget)
            {
                ItemWidget->InitItem(AvailableItems[i], i, this);
                ItemList->AddChild(ItemWidget);
            }
        }
    }
    if (TotalPriceText)
    {
        TotalPriceText->SetText(FText::FromString(FString::Printf(TEXT("余额：%.2f"), RealMoney)));
    }
}

void UVendingMachineWidget::OnCloseButtonClicked()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    ASCUTPlayerCharacter* Player = PC ? Cast<ASCUTPlayerCharacter>(PC->GetPawn()) : nullptr;
    if (Player)
    {
        Player->PlayerDrinkState = EPlayerDrinkState::Drinking;
        Player->bIsMovementLocked = true;
        Player->GetWorldTimerManager().SetTimer(
            Player->PlayerDrinkTimerHandle,
            Player,
            &ASCUTPlayerCharacter::OnPlayerDrinkFinished,
            Player->PlayerDrinkingDuration,
            false
        );
    }
    RemoveFromParent();
}

void UVendingMachineWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        PC->bShowMouseCursor = true;
        PC->SetInputMode(FInputModeUIOnly());
    }
    if (CloseButton)
    {
        CloseButton->OnClicked.AddDynamic(this, &UVendingMachineWidget::OnCloseButtonClicked);
    }
    RefreshUI();
    bLastIsBuying = true; // 弹出时认为bIsBuying为true
}

void UVendingMachineWidget::NativeDestruct()
{
    Super::NativeDestruct();
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
    }
}

void UVendingMachineWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    // 只要bIsBuying变为false就自动关闭UI，变为true就自动弹出
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

    if (!PC) return;
    APawn* Pawn = PC->GetPawn();
    if (!Pawn) return;
    USkeletalMeshComponent* Mesh = Pawn->FindComponentByClass<USkeletalMeshComponent>();
    if (!Mesh) return;
    USCUTAnimInstance* AnimInstance = Cast<USCUTAnimInstance>(Mesh->GetAnimInstance());
    bool bIsBuying = AnimInstance ? AnimInstance->bIsBuying : false;
    if (bIsBuying && !IsInViewport())
    {
        AddToViewport();
    }
    if (bLastIsBuying && !bIsBuying)
    {
        RemoveFromParent();
    }
    bLastIsBuying = bIsBuying;
}

