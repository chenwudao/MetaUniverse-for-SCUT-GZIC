// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VendingMachineWidget.generated.h"

USTRUCT(BlueprintType)
struct FVendingItem
{
	GENERATED_BODY()
		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString ItemName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Price;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* ItemImage;
};

/**
 *
 */
UCLASS()
class SCUT_API UVendingMachineWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UVerticalBox* ItemList;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* TotalPriceText;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UButton* PurchaseButton;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UButton* CloseButton;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VendingMachine")
		TArray<FVendingItem> AvailableItems;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VendingMachine")
		TSubclassOf<class UVendingItemWidget> VendingItemWidgetClass;
	UFUNCTION(BlueprintCallable, Category = "VendingMachine")
		void PurchaseItem(int32 ItemIndex);
	UFUNCTION(BlueprintCallable, Category = "VendingMachine")
		void RefreshUI();
	UFUNCTION()
		void OnCloseButtonClicked();
protected:
	bool bLastIsBuying = false;
};
