#include "SCUTCharacter.h"
#include "scut/Character/NPC/SCUTDormManagerCharacter.h"
#include "Kismet/GameplayStatics.h"

ASCUTCharacter::ASCUTCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    DormManager = nullptr;
}

void ASCUTCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 在游戏开始时查找宿管角色
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASCUTDormManagerCharacter::StaticClass(), FoundActors);

    if (FoundActors.Num() > 0)
    {
        DormManager = Cast<ASCUTDormManagerCharacter>(FoundActors[0]);
    }
}

void ASCUTCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

ASCUTDormManagerCharacter* ASCUTCharacter::GetDormManager() const
{
    return DormManager;
}

void ASCUTCharacter::SetupDormManagerInput(UInputAction* InteractAction, UInputAction* DialogueAction, UInputAction* EndDialogueAction)
{
    if (DormManager)
    {
        DormManager->SetupInputActions(InteractAction, DialogueAction, EndDialogueAction);
    }
}