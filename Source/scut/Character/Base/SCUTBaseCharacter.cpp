#include "scut/Character/Base/SCUTBaseCharacter.h"

ASCUTBaseCharacter::ASCUTBaseCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentState = ECharacterState::Idle;
}

void ASCUTBaseCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void ASCUTBaseCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ASCUTBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

float ASCUTBaseCharacter::GetCharacterSpeed() const
{
    return GetVelocity().Size();
}

void ASCUTBaseCharacter::UpdateCharacterState(ECharacterState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        // 这里可以添加状态改变时的回调或事件
    }
}

void ASCUTBaseCharacter::HandleMovementInput(const FVector2D& MovementVector)
{
    if (Controller != nullptr)
    {
        // 获取前向和右向向量
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // 添加移动输入
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void ASCUTBaseCharacter::HandleRotationInput(const FVector2D& RotationVector)
{
    if (Controller != nullptr)
    {
        AddControllerYawInput(RotationVector.X);
        AddControllerPitchInput(RotationVector.Y);
    }
} 