// SCUTVendingMachineCharacter.cpp
#include "scut/Character/NPC/SCUTVendingMachineCharacter.h"
#include "scut/Character/Player/SCUTPlayerCharacter.h"
#include "TimerManager.h"
#include "scut/Public/UI/VendingMachineWidget.h"
#include "scut/Animation/SCUTAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerController.h" // Added for APlayerController

ASCUTVendingMachineCharacter::ASCUTVendingMachineCharacter()
{
    VendingMachineState = EVendingMachineState::Idle;
}

void ASCUTVendingMachineCharacter::BeginPlay()
{
    Super::BeginPlay();
    VendingMachineState = EVendingMachineState::Idle;
}

void ASCUTVendingMachineCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // UI creation/removal logic has been moved out of Tick.
    // This function should primarily handle continuous updates, not discrete events.
}

void ASCUTVendingMachineCharacter::StartVendingInteraction()
{
    if (VendingMachineState == EVendingMachineState::Idle)
    {
        VendingMachineState = EVendingMachineState::Buying;
        UE_LOG(LogTemp, Warning, TEXT("[Vending] Vending Machine State: Buying."));

        if (VendingWidgetClass)
        {
            if (VendingWidget)
            {
                VendingWidget->RemoveFromParent();
                VendingWidget = nullptr;
            }

            VendingWidget = CreateWidget<UVendingMachineWidget>(GetWorld(), VendingWidgetClass);
            if (VendingWidget)
            {
                VendingWidget->AddToViewport();
                UE_LOG(LogTemp, Warning, TEXT("[Vending] VendingWidget created and added to viewport in StartVendingInteraction!"));

                if (USkeletalMeshComponent* SkeletalMeshComponent = GetMesh())
                {
                    if (USCUTAnimInstance* AnimInstance = Cast<USCUTAnimInstance>(SkeletalMeshComponent->GetAnimInstance()))
                    {
                        AnimInstance->bIsBuying = true;
                    }
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("[Vending] CreateWidget failed in StartVendingInteraction! VendingWidgetClass might be invalid."));
                VendingMachineState = EVendingMachineState::Idle;
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[Vending] VendingWidgetClass is not set!"));
            VendingMachineState = EVendingMachineState::Idle;
        }
    }
}

void ASCUTVendingMachineCharacter::EndVendingInteraction()
{
    if (VendingMachineState == EVendingMachineState::Buying)
    {
        VendingMachineState = EVendingMachineState::Drinking;
        UE_LOG(LogTemp, Warning, TEXT("[Vending] Vending Machine State: Drinking."));

        if (VendingWidget)
        {
            VendingWidget->RemoveFromParent();
            VendingWidget = nullptr;
            UE_LOG(LogTemp, Warning, TEXT("[Vending] VendingWidget removed from viewport in EndVendingInteraction!"));
        }

        GetWorldTimerManager().SetTimer(VendingMachineTimerHandle, this, &ASCUTVendingMachineCharacter::OnDrinkingFinished, DrinkingDuration, false);
    }
}

void ASCUTVendingMachineCharacter::OnDrinkingFinished()
{
    VendingMachineState = EVendingMachineState::Finished;
    UE_LOG(LogTemp, Warning, TEXT("[Vending] Vending Machine State: Finished."));

    GetWorldTimerManager().SetTimer(VendingMachineTimerHandle, this, &ASCUTVendingMachineCharacter::ResetToIdle, 1.0f, false);
}

void ASCUTVendingMachineCharacter::ResetToIdle()
{
    VendingMachineState = EVendingMachineState::Idle;
    UE_LOG(LogTemp, Warning, TEXT("[Vending] Vending Machine State reset to Idle."));
}

void ASCUTVendingMachineCharacter::InteractWithPlayer(ASCUTPlayerCharacter* PlayerCharacter)
{
    UE_LOG(LogTemp, Warning, TEXT("Vending Machine: Player %s interacted. Current state: %d"), *PlayerCharacter->GetName(), (int32)VendingMachineState);

    if (VendingMachineState == EVendingMachineState::Idle)
    {
        StartVendingInteraction();
    }
}

EVendingMachineState ASCUTVendingMachineCharacter::GetVendingMachineState() const
{
    return VendingMachineState;
}

bool ASCUTVendingMachineCharacter::TryPurchaseItem(float ItemPrice, ASCUTPlayerCharacter* PlayerCharacter)
{
    if (!PlayerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("[Vending] TryPurchaseItem: Invalid player character!"));
        return false;
    }

    if (PlayerCharacter->PlayerMoney >= ItemPrice)
    {
        PlayerCharacter->PlayerMoney -= ItemPrice;
        UE_LOG(LogTemp, Warning, TEXT("[Vending] Player bought item for %.2f. Remaining money: %.2f"), ItemPrice, PlayerCharacter->PlayerMoney);
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Vending] Player does not have enough money (%.2f) for item (%.2f)."), PlayerCharacter->PlayerMoney, ItemPrice);
        return false;
    }
}