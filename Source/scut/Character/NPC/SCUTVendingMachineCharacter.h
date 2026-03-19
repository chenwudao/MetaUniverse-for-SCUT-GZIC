// SCUTVendingMachineCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "scut/Character/NPC/SCUTNPCCharacter.h"
#include "scut/Character/Player/SCUTPlayerCharacter.h" // <--- ��������
#include "SCUTVendingMachineCharacter.generated.h"

UENUM(BlueprintType)
enum class EVendingMachineState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Buying UMETA(DisplayName = "Buying"),
    Drinking UMETA(DisplayName = "Drinking"),
    Finished UMETA(DisplayName = "Finished")
};

UCLASS()
class SCUT_API ASCUTVendingMachineCharacter : public ASCUTNPCCharacter
{
    GENERATED_BODY()
public:
    ASCUTVendingMachineCharacter();

    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;

    // ����
    UFUNCTION(BlueprintCallable, Category = "VendingMachine|Interaction")
        void StartVendingInteraction();

    UFUNCTION(BlueprintCallable, Category = "VendingMachine|Interaction")
        void EndVendingInteraction();

    // ��ȡ��ǰ״̬
    UFUNCTION(BlueprintPure, Category = "VendingMachine|State")
        EVendingMachineState GetVendingMachineState() const;

    UFUNCTION(BlueprintCallable, Category = "VendingMachine|Interaction")
        void InteractWithPlayer(ASCUTPlayerCharacter* PlayerCharacter);

    bool TryPurchaseItem(float ItemPrice, ASCUTPlayerCharacter* PlayerCharacter);

protected:
    // ����״̬
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VendingMachine|State")
        EVendingMachineState VendingMachineState;

    // ������ʱ��
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VendingMachine|Timing")
        float DrinkingDuration = 3.0f;

    FTimerHandle VendingMachineTimerHandle;

    // ״̬�л�
    void OnDrinkingFinished();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VendingMachine|UI")
        TSubclassOf<class UVendingMachineWidget> VendingWidgetClass;
    UPROPERTY()
        UVendingMachineWidget* VendingWidget = nullptr;

    void ResetToIdle();
};