// Fill out your copyright notice in the Description page of Project Settings.


#include "scut/Public/UI/TeacherQuestionWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "scut/Character/NPC/SCUTTeacherCharacter.h"

FQuestion CurrentQuestion;

void UTeacherQuestionWidget::ShowQuestion(const FQuestion& Question)
{
    CurrentQuestion = Question;
    if (QuestionText) QuestionText->SetText(FText::FromString(Question.QuestionText));
    if (OptionsList) OptionsList->ClearChildren();
    for (int32 i = 0; i < Question.Options.Num(); ++i)
    {
        // 这里假设你在蓝图里为每个选项生成按钮并绑定到OnOptionSelected
    }
    if (ResultText) ResultText->SetText(FText::GetEmpty());
}

void UTeacherQuestionWidget::OnOptionSelected(int32 OptionIndex)
{
    if (OptionIndex == 0) // 选择A
    {
        if (ResultText) ResultText->SetText(FText::FromString(TEXT("回答正确！")));
    }
    else
    {
        if (ResultText) ResultText->SetText(FText::FromString(TEXT("回答错误！")));
    }
}

void UTeacherQuestionWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        PC->bShowMouseCursor = true;
        PC->SetInputMode(FInputModeUIOnly());
    }
    if (CloseButton)
        CloseButton->OnClicked.AddDynamic(this, &UTeacherQuestionWidget::OnCloseClicked);
    if (AButton)
        AButton->OnClicked.AddDynamic(this, &UTeacherQuestionWidget::OnOptionAClicked);
    if (BButton)
        BButton->OnClicked.AddDynamic(this, &UTeacherQuestionWidget::OnOptionBClicked);
    if (CButton)
        CButton->OnClicked.AddDynamic(this, &UTeacherQuestionWidget::OnOptionCClicked);
}

void UTeacherQuestionWidget::OnCloseClicked()
{
    ASCUTTeacherCharacter* Teacher = Cast<ASCUTTeacherCharacter>(UGameplayStatics::GetActorOfClass(GetWorld(), ASCUTTeacherCharacter::StaticClass()));
    if (Teacher)
        Teacher->EndQuestioning();
}

void UTeacherQuestionWidget::OnOptionAClicked()
{
    OnOptionSelected(0);
}
void UTeacherQuestionWidget::OnOptionBClicked()
{
    OnOptionSelected(1);
}
void UTeacherQuestionWidget::OnOptionCClicked()
{
    OnOptionSelected(2);
}

void UTeacherQuestionWidget::NativeDestruct()
{
    Super::NativeDestruct();
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
    }
}

