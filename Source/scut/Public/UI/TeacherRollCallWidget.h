// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TeacherRollCallWidget.generated.h"

/**
 *
 */
UCLASS()
class SCUT_API UTeacherRollCallWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* TitleText;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* TimerText;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* StatusText;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UButton* SignInButton;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UButton* CloseButton;
	UFUNCTION(BlueprintCallable, Category = "RollCall")
		void StartRollCall(float Duration);
	UFUNCTION(BlueprintCallable, Category = "RollCall")
		void OnSignInClicked();
	UFUNCTION()
		void OnCloseClicked();
};
