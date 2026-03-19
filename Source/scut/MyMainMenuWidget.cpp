// YourGame/Private/scut/MyMainMenuWidget.cpp
#include "scut/MyMainMenuWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"
#include "scut/Game/MyGameMode.h"

// 移除这行，因为 FLevelInfo 已经通过 MyGameMode.h 包含
// #include "Data/LevelInfo.h" // 这一行应该不存在了

UMyMainMenuWidget::UMyMainMenuWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    Canvas_MainMenu = nullptr;
    Canvas_LevelSelect = nullptr;
    Button_BackToMainMenu = nullptr;
    VerticalBox_LevelList = nullptr;
    LevelEntryWidgetClass = nullptr;
}

void UMyMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Canvas_MainMenu)
    {
        UE_LOG(LogTemp, Log, TEXT("UMyMainMenuWidget: Canvas_MainMenu bound successfully."));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UMyMainMenuWidget: Canvas_MainMenu is not bound! Check UMG Designer 'Is Variable' and name."));
    }

    if (Canvas_LevelSelect)
    {
        UE_LOG(LogTemp, Log, TEXT("UMyMainMenuWidget: Canvas_LevelSelect bound successfully."));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UMyMainMenuWidget: Canvas_LevelSelect is not bound! Check UMG Designer 'Is Variable' and name."));
    }

    if (Button_BackToMainMenu)
    {
        Button_BackToMainMenu->OnClicked.AddDynamic(this, &UMyMainMenuWidget::OnBackToMainMenuClicked);
        UE_LOG(LogTemp, Log, TEXT("UMyMainMenuWidget: Button_BackToMainMenu bound successfully."));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UMyMainMenuWidget: Button_BackToMainMenu is not bound! Check UMG Designer 'Is Variable' and name."));
    }

    if (VerticalBox_LevelList)
    {
        UE_LOG(LogTemp, Log, TEXT("UMyMainMenuWidget: VerticalBox_LevelList bound successfully."));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UMyMainMenuWidget: VerticalBox_LevelList is not bound! Check UMG Designer 'Is Variable' and name."));
    }

    ShowMainMenuPart();
}

void UMyMainMenuWidget::ShowMainMenuPart()
{
    if (Canvas_MainMenu)
    {
        Canvas_MainMenu->SetVisibility(ESlateVisibility::Visible);
        UE_LOG(LogTemp, Log, TEXT("UMyMainMenuWidget: Main Menu Part Shown."));
    }
    if (Canvas_LevelSelect)
    {
        Canvas_LevelSelect->SetVisibility(ESlateVisibility::Hidden);
        UE_LOG(LogTemp, Log, TEXT("UMyMainMenuWidget: Level Select Part Hidden."));
    }
}

void UMyMainMenuWidget::ShowLevelSelectPart()
{
    if (Canvas_MainMenu)
    {
        Canvas_MainMenu->SetVisibility(ESlateVisibility::Hidden);
        UE_LOG(LogTemp, Log, TEXT("UMyMainMenuWidget: Main Menu Part Hidden."));
    }
    if (Canvas_LevelSelect)
    {
        Canvas_LevelSelect->SetVisibility(ESlateVisibility::Visible);
        UE_LOG(LogTemp, Log, TEXT("UMyMainMenuWidget: Level Select Part Shown."));
    }
    PopulateLevelList();
}

void UMyMainMenuWidget::OnBackToMainMenuClicked()
{
    UE_LOG(LogTemp, Log, TEXT("UMyMainMenuWidget: Back To Main Menu Button Clicked."));
    ShowMainMenuPart();
}

void UMyMainMenuWidget::OnLevelEntryClickedFromBlueprint(FName LevelID)
{
    UE_LOG(LogTemp, Log, TEXT("UMyMainMenuWidget: Received Level Click from Blueprint: %s"), *LevelID.ToString());
    OnLevelSelected.Broadcast(LevelID);
}

void UMyMainMenuWidget::PopulateLevelList()
{
    if (!VerticalBox_LevelList)
    {
        UE_LOG(LogTemp, Error, TEXT("UMyMainMenuWidget: VerticalBox_LevelList is null, cannot populate level list."));
        return;
    }

    VerticalBox_LevelList->ClearChildren();

    AMyGameMode* GameMode = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (!GameMode || !GameMode->LevelInfoDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("UMyMainMenuWidget: GameMode or LevelInfoDataTable is null. Cannot populate level list."));
        return;
    }

    TArray<FName> RowNames = GameMode->LevelInfoDataTable->GetRowNames();

    for (const FName& RowName : RowNames)
    {
        FLevelInfo* LevelInfo = GameMode->LevelInfoDataTable->FindRow<FLevelInfo>(RowName, TEXT("Looking for Level Info"));
        if (LevelInfo)
        {
            if (!LevelEntryWidgetClass)
            {
                UE_LOG(LogTemp, Error, TEXT("UMyMainMenuWidget: LevelEntryWidgetClass is not set in BP_MyMainMenuWidget, cannot create level entries."));
                return;
            }

            UUserWidget* LevelEntryWidget = CreateWidget<UUserWidget>(this, LevelEntryWidgetClass);
            if (LevelEntryWidget)
            {
                // ---- 调用蓝图函数 SetLevelEntryInfo ----
                UFunction* SetLevelInfoFunction = LevelEntryWidget->GetClass()->FindFunctionByName(TEXT("SetLevelEntryInfo"));
                if (SetLevelInfoFunction)
                {
                    struct FSetLevelEntryInfo_Params
                    {
                        FName InLevelID;
                        FString InLevelName;
                        FString InMapAssetName;
                        UTexture2D* InThumbnail;
                        FText InDescription;
                    };
                    FSetLevelEntryInfo_Params Params;
                    Params.InLevelID = LevelInfo->LevelID;
                    Params.InLevelName = LevelInfo->LevelName;
                    Params.InMapAssetName = LevelInfo->MapAssetName;
                    Params.InThumbnail = LevelInfo->Thumbnail;
                    Params.InDescription = LevelInfo->Description;

                    LevelEntryWidget->ProcessEvent(SetLevelInfoFunction, &Params);
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("UMyMainMenuWidget: WBP_LevelEntryWidget does not have 'SetLevelEntryInfo' BlueprintCallable function! Check name and parameters."));
                }

                // ---- 核心修改在这里：通过 FScriptDelegate 和 BindUFunction 绑定蓝图委托 ----
                // 获取委托的属性指针
                FProperty* LevelClickedDelegateProperty = LevelEntryWidget->GetClass()->FindPropertyByName(TEXT("OnLevelEntryClickedEvent"));
                if (LevelClickedDelegateProperty)
                {
                    // 尝试将属性转换为多播委托属性
                    FMulticastDelegateProperty* MulticastDelegateProperty = CastField<FMulticastDelegateProperty>(LevelClickedDelegateProperty);
                    if (MulticastDelegateProperty)
                    {
                        // 获取委托的实际值（来自 LevelEntryWidget 实例）
                        FMulticastScriptDelegate* LevelClickedDelegate = MulticastDelegateProperty->ContainerPtrToValuePtr<FMulticastScriptDelegate>(LevelEntryWidget);
                        if (LevelClickedDelegate)
                        {
                            // 使用 FScriptDelegate::BindUFunction 来绑定 C++ 函数
                            FScriptDelegate NewDelegate;
                            // `this` 是绑定对象
                            // `TEXT("OnLevelEntryClickedFromBlueprint")` 是要在 `this` 上调用的 UFUNCTION 名称
                            NewDelegate.BindUFunction(this, TEXT("OnLevelEntryClickedFromBlueprint"));

                            // 将这个 FScriptDelegate 添加到蓝图的多播委托中
                            LevelClickedDelegate->Add(NewDelegate);
                            UE_LOG(LogTemp, Log, TEXT("UMyMainMenuWidget: Bound OnLevelEntryClickedFromBlueprint to LevelEntryWidget using FScriptDelegate."));
                        }
                        else
                        {
                            UE_LOG(LogTemp, Error, TEXT("UMyMainMenuWidget: Failed to get delegate pointer for 'OnLevelEntryClickedEvent'."));
                        }
                    }
                    else
                    {
                        UE_LOG(LogTemp, Error, TEXT("UMyMainMenuWidget: 'OnLevelEntryClickedEvent' is not a MulticastDelegateProperty!"));
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("UMyMainMenuWidget: WBP_LevelEntryWidget does not have 'OnLevelEntryClickedEvent' BlueprintAssignable delegate!"));
                }

                VerticalBox_LevelList->AddChildToVerticalBox(LevelEntryWidget);
            }
        }
    }
}