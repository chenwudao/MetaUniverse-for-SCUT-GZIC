// Fill out your copyright notice in the Description page of Project Settings.


#include "scut/Public/UI/TeacherRollCallWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "scut/Character/NPC/SCUTTeacherCharacter.h"

void UTeacherRollCallWidget::StartRollCall(float Duration)
{
    // 这里可实现倒计时逻辑
}

void UTeacherRollCallWidget::OnSignInClicked()
{
    ASCUTTeacherCharacter* Teacher = Cast<ASCUTTeacherCharacter>(UGameplayStatics::GetActorOfClass(GetWorld(), ASCUTTeacherCharacter::StaticClass()));
    if (Teacher)
        Teacher->EndRollCall();
}

void UTeacherRollCallWidget::OnCloseClicked()
{
    ASCUTTeacherCharacter* Teacher = Cast<ASCUTTeacherCharacter>(UGameplayStatics::GetActorOfClass(GetWorld(), ASCUTTeacherCharacter::StaticClass()));
    if (Teacher)
        Teacher->EndRollCall();
}

void UTeacherRollCallWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        PC->bShowMouseCursor = true;
        PC->SetInputMode(FInputModeUIOnly());
    }
    if (SignInButton)
        SignInButton->OnClicked.AddDynamic(this, &UTeacherRollCallWidget::OnSignInClicked);
    if (CloseButton)
        CloseButton->OnClicked.AddDynamic(this, &UTeacherRollCallWidget::OnCloseClicked);
}

void UTeacherRollCallWidget::NativeDestruct()
{
    Super::NativeDestruct();
    // 不再在这里隐藏鼠标和切回GameOnly，交由最后一个UI关闭时处理
}

