#include "scut/Public/UI/VendingItemWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "scut/Public/UI/VendingMachineWidget.h"

void UVendingItemWidget::InitItem(const FVendingItem& Item, int32 Index, UVendingMachineWidget* ParentWidget)
{
    if (ItemNameText) ItemNameText->SetText(FText::FromString(Item.ItemName));
    if (PriceText) PriceText->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), Item.Price)));
    if (ItemImageWidget && Item.ItemImage) ItemImageWidget->SetBrushFromTexture(Item.ItemImage);
    Parent = ParentWidget;
    ItemIndex = Index;
}

void UVendingItemWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (BuyButton)
    {
        BuyButton->OnClicked.AddDynamic(this, &UVendingItemWidget::OnBuyClicked);
    }
}

void UVendingItemWidget::OnBuyClicked()
{
    if (Parent)
    {
        Parent->PurchaseItem(ItemIndex);
    }
}