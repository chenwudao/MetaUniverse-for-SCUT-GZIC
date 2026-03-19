// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TeacherQuestionWidget.generated.h"

USTRUCT(BlueprintType)
struct FQuestion
{
	GENERATED_BODY()
		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString QuestionText;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FString> Options;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 CorrectOptionIndex;
};

/**
 *
 */
UCLASS()
class SCUT_API UTeacherQuestionWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* QuestionText;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UVerticalBox* OptionsList;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* ResultText;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UButton* CloseButton;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UButton* AButton;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UButton* BButton;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UButton* CButton;

	UFUNCTION(BlueprintCallable, Category = "Question")
		void ShowQuestion(const FQuestion& Question);
	UFUNCTION(BlueprintCallable, Category = "Question")
		void OnOptionSelected(int32 OptionIndex);
	UFUNCTION()
		void OnCloseClicked();
	UFUNCTION()
		void OnOptionAClicked();
	UFUNCTION()
		void OnOptionBClicked();
	UFUNCTION()
		void OnOptionCClicked();
};
