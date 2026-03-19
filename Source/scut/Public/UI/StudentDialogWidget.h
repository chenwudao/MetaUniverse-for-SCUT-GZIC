// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Http.h"
#include "StudentDialogWidget.generated.h"

/**
 *
 */
UCLASS()
class SCUT_API UStudentDialogWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "Dialog")
		void UpdateDialogText(const FString& NewText);

	UFUNCTION(BlueprintCallable, Category = "Dialog")
		void SendQuestionToAI(const FString& Question);

	UFUNCTION()
		void SendQuestionToAI_Internal();

	void OnAIResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	UPROPERTY(meta = (BindWidget))
		class UTextBlock* DialogText;

	UPROPERTY(meta = (BindWidget))
		class UEditableTextBox* InputBox;

	UPROPERTY(meta = (BindWidget))
		class UButton* SendButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
		FString Context;
};
