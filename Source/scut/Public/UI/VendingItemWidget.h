#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VendingMachineWidget.h"
#include "VendingItemWidget.generated.h"

UCLASS()
class SCUT_API UVendingItemWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
        class UTextBlock* ItemNameText;
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
        class UTextBlock* PriceText;
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
        class UImage* ItemImageWidget;
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
        class UButton* BuyButton;
    UFUNCTION(BlueprintCallable, Category = "Vending")
        void InitItem(const FVendingItem& Item, int32 Index, UVendingMachineWidget* ParentWidget);
protected:
    UPROPERTY()
        UVendingMachineWidget* Parent;
    int32 ItemIndex;
    virtual void NativeConstruct() override;
    UFUNCTION()
        void OnBuyClicked();
};