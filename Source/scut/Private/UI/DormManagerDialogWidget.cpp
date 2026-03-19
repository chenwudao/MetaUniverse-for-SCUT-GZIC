// Fill out your copyright notice in the Description page of Project Settings.


#include "scut/Public/UI/DormManagerDialogWidget.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void UDormManagerDialogWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        PC->bShowMouseCursor = true;
        PC->SetInputMode(FInputModeUIOnly());
    }
    if (SendButton)
    {
        SendButton->OnClicked.AddDynamic(this, &UDormManagerDialogWidget::SendQuestionToAI_Internal);
    }
}

void UDormManagerDialogWidget::NativeDestruct()
{
    Super::NativeDestruct();
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
    }
}

void UDormManagerDialogWidget::SendQuestionToAI_Internal()
{
    if (InputBox)
    {
        FString Question = InputBox->GetText().ToString();
        SendQuestionToAI(Question);
    }
}

void UDormManagerDialogWidget::UpdateDialogText(const FString& NewText)
{
    if (DialogText)
    {
        DialogText->SetText(FText::FromString(NewText));
    }
}

void UDormManagerDialogWidget::SendQuestionToAI(const FString& Question)
{
    UE_LOG(LogTemp, Warning, TEXT("[DormManagerDialogWidget] SendQuestionToAI called, Question: %s"), *Question);
    FString FullQuestion = Context.IsEmpty() ? Question : (Context + TEXT("\n") + Question);
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetStringField(TEXT("question"), FullQuestion);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    // 创建HTTP请求
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(TEXT("http://127.0.0.1:8000/ask"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetContentAsString(OutputString);

    // 绑定回调
    Request->OnProcessRequestComplete().BindUObject(this, &UDormManagerDialogWidget::OnAIResponseReceived);
    Request->ProcessRequest();

    // 显示"思考中..."
    UpdateDialogText(TEXT("思考中..."));
}

void UDormManagerDialogWidget::OnAIResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Warning, TEXT("[DormManagerDialogWidget] OnAIResponseReceived called, Success: %d"), bWasSuccessful);
    FString AnswerText = TEXT("请求失败");
    if (bWasSuccessful && Response.IsValid())
    {
        FString ResponseStr = Response->GetContentAsString();
        UE_LOG(LogTemp, Warning, TEXT("[DormManagerDialogWidget] HTTP Response: %s"), *ResponseStr);
        // 解析JSON
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);
        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            AnswerText = JsonObject->GetStringField(TEXT("answer"));
        }
        else
        {
            AnswerText = TEXT("AI返回格式错误");
        }
    }
    UpdateDialogText(AnswerText);
}

