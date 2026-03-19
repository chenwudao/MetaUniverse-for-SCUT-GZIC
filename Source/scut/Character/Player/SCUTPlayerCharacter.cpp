#include "scut/Character/Player/SCUTPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "scut/Character/NPC/SCUTNPCCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "scut/Character/NPC/SCUTDormManagerCharacter.h" // 确保包含宿管角色头文件
#include "scut/Character/NPC/SCUTTeacherCharacter.h"       // 确保包含老师角色头文件
#include "scut/Character/NPC/SCUTStudentCharacter.h"       // 确保包含学生角色头文件
#include "scut/Character/NPC/SCUTVendingMachineCharacter.h" // 确保包含售货机角色头文件
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h" // For GetCapsuleComponent()
#include "Components/SkeletalMeshComponent.h" // For Mesh1P
#include "scut/Game/BusTrigger.h" // 确保包含 BusTrigger 的头文件
#include "GameFramework/PlayerController.h" // For PlayerController

ASCUTPlayerCharacter::ASCUTPlayerCharacter()
{
    // 设置角色旋转
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // 配置角色移动
    GetCharacterMovement()->bOrientRotationToMovement = true; // 默认第三人称面向移动方向
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 600.f;
    GetCharacterMovement()->AirControl = 0.2f;

    // 设置碰撞胶囊体的大小
    GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

    // --- 创建第一人称摄像机组件 (以 firstpersonCharacter 为准) ---
    FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    // 摄像机附加到胶囊体
    FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
    // 设置摄像机的相对位置，使用你定义的属性
    FirstPersonCameraComponent->SetRelativeLocation(FirstPersonCameraRelativeLocation);
    // 摄像机旋转跟随控制器（用于第一人称视角）
    FirstPersonCameraComponent->bUsePawnControlRotation = true;

    // --- 创建第一人称网格体组件（通常是手臂；只被拥有者看到）(以 firstpersonCharacter 为准) ---
    Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
    // 只对拥有者可见
    Mesh1P->SetOnlyOwnerSee(true);
    // 1P网格体附加到1P摄像机
    Mesh1P->SetupAttachment(FirstPersonCameraComponent);
    // 1P手臂通常不需要投射动态阴影
    Mesh1P->bCastDynamicShadow = false;
    Mesh1P->CastShadow = false;
    // 调整1P手臂的相对位置
    Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

    // --- 配置ACharacter自带的默认网格体 (GetMesh()) 用于第三人称视角 (以 firstpersonCharacter 为准) ---
    // 这个网格体将被其他玩家看到，或者在第三人称视角时被自己看到。
    // 调整到胶囊体底部
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
    // 调整默认朝向
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
    // 初始时，让拥有者看不到自己的身体（因为默认是第一人称）
    GetMesh()->SetOwnerNoSee(true);
    // 确保第三人称网格体投射动态阴影
    GetMesh()->bCastDynamicShadow = true;
    GetMesh()->CastShadow = true;

    // --- 创建摄像机臂组件（用于第三人称视角）(以 firstpersonCharacter 为准) ---
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    // 附加到角色胶囊体
    CameraBoom->SetupAttachment(GetCapsuleComponent());
    // 设置默认距离
    CameraBoom->TargetArmLength = ThirdPersonCameraDistance;
    // 摄像机臂旋转跟随角色控制器
    CameraBoom->bUsePawnControlRotation = true;
    // 启用碰撞测试，防止摄像机穿墙
    CameraBoom->bDoCollisionTest = true;

    // 初始化为第一人称视角
    bIsFirstPersonView = true; // 默认以第一人称视角开始
    // 调用函数设置初始状态
    SetFirstPersonView();

    // 初始化其他状态
    bIsRunning = false;
    bIsInDialogue = false;
    CurrentDialogueTarget = nullptr;
    bIsSitting = false;
    bIsSittingDown = false;
    bIsStandingUp = false;
    bIsMovingToSeat = false; // 新增，用于平滑移动
    bIsMovementLocked = false;
    bIsBeingQuestioned = false;
    bIsTalkingToDormManager = false; // 新增
    PlayerDrinkState = EPlayerDrinkState::None; // 新增
    CurrentOverlappingBusTrigger = nullptr; // 初始化 CurrentOverlappingBusTrigger
}

void ASCUTPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    // 初始设置步行速度
    SetMovementSpeed(WalkSpeed);

    // 添加输入映射上下文
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

void ASCUTPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // --- 实现平滑移动到座位 ---
    if (bIsMovingToSeat)
    {
        FVector TargetLocation = ChairLocation;
        FVector CurrentLocation = GetActorLocation();
        FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();

        // 计算移动距离
        float MoveDistance = SittingMoveSpeed * DeltaTime;
        FVector NewLocation = CurrentLocation + Direction * MoveDistance;

        // 如果已经足够接近目标位置
        if (FVector::Distance(NewLocation, TargetLocation) < MoveDistance)
        {
            NewLocation = TargetLocation;
            bIsMovingToSeat = false;
            // 达到位置后，开始坐下动画
            StartSittingAnimation();
        }

        SetActorLocation(NewLocation);

        // 平滑旋转到目标方向 (假设 ChairRotation 包含了面向老师后的旋转)
        FRotator CurrentRotation = GetActorRotation();
        FRotator TargetRotation = ChairRotation;
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 5.0f); // 5.0f 是插值速度
        SetActorRotation(NewRotation);
    }
    // 根据移动状态更新角色状态
    else if (GetVelocity().Size() > 0.0f)
    {
        UpdateCharacterState(bIsRunning ? ECharacterState::Running : ECharacterState::Walking);
    }
    else
    {
        UpdateCharacterState(ECharacterState::Idle);
    }
}

void ASCUTPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // 获取增强输入组件
    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // 绑定移动输入
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASCUTPlayerCharacter::Move);
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ASCUTPlayerCharacter::Move);

        // 绑定视角输入
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASCUTPlayerCharacter::Look);

        // 绑定奔跑输入
        if (RunAction)
        {
            EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Triggered, this, &ASCUTPlayerCharacter::OnRun);
            EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &ASCUTPlayerCharacter::OnStopRunning);
        }

        // 绑定跳跃输入 - 修改为调用 InputJump 和 InputStopJumping
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASCUTPlayerCharacter::InputJump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ASCUTPlayerCharacter::InputStopJumping);

        // <<< 关键修改：将 InteractAction 绑定到新的 OnInteractInput 函数 >>>
        if (InteractAction)
        {
            EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ASCUTPlayerCharacter::OnInteractInput);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("InteractAction is not set on PlayerCharacter Blueprint! Please assign it in editor."));
        }

        // 绑定切换视角输入
        if (ToggleViewAction)
        {
            EnhancedInputComponent->BindAction(ToggleViewAction, ETriggerEvent::Started, this, &ASCUTPlayerCharacter::OnToggleView);
        }

        // 绑定坐下输入 (这个 OnSittingAction 将处理坐下和站起的逻辑)
        if (SittingAction)
        {
            EnhancedInputComponent->BindAction(SittingAction, ETriggerEvent::Started, this, &ASCUTPlayerCharacter::OnSittingAction);
        }

        // 绑定老师相关输入 (确保这里被正确绑定)
        if (RollCallAction)
        {
            EnhancedInputComponent->BindAction(RollCallAction, ETriggerEvent::Started, this, &ASCUTPlayerCharacter::OnTeacherRollCallAction);
        }
        if (QuestionAction)
        {
            EnhancedInputComponent->BindAction(QuestionAction, ETriggerEvent::Started, this, &ASCUTPlayerCharacter::OnTeacherQuestionAction);
        }
        if (EndQuestionAction)
        {
            EnhancedInputComponent->BindAction(EndQuestionAction, ETriggerEvent::Started, this, &ASCUTPlayerCharacter::OnTeacherEndQuestionAction);
        }

        // 绑定学生相关输入
        if (StudentDialogueAction)
        {
            EnhancedInputComponent->BindAction(StudentDialogueAction, ETriggerEvent::Started, this, &ASCUTPlayerCharacter::OnStudentDialogueAction);
        }
        if (StudentEndDialogueAction)
        {
            EnhancedInputComponent->BindAction(StudentEndDialogueAction, ETriggerEvent::Started, this, &ASCUTPlayerCharacter::OnStudentEndDialogueAction);
        }

        // 绑定贩卖机交互输入
        if (VendingMachineInteractAction)
        {
            EnhancedInputComponent->BindAction(VendingMachineInteractAction, ETriggerEvent::Started, this, &ASCUTPlayerCharacter::OnVendingMachineInteract);
        }
    }
}

// 绑定输入动作的辅助函数，如果需要动态绑定
void ASCUTPlayerCharacter::BindInputActions()
{
    // 这部分逻辑已在 SetupPlayerInputComponent 中处理，此函数可以作为集中管理输入动作的接口
    // 或在其他地方需要动态重新绑定时使用。
    // 为了避免重复绑定，如果 SetupPlayerInputComponent 已经完成了大部分工作，这个函数可能就不需要额外逻辑了。
    // 如果是动态绑定，需要先解除之前的绑定。
}

void ASCUTPlayerCharacter::SetupInputActions(UInputAction* InMoveAction, UInputAction* InLookAction, UInputAction* InRunAction,
    UInputAction* InJumpAction, UInputAction* InInteractAction, UInputAction* InDialogueAction,
    UInputAction* InEndDialogueAction, UInputAction* InSittingAction, UInputAction* InRollCallAction,
    UInputAction* InQuestionAction, UInputAction* InEndQuestionAction)
{
    MoveAction = InMoveAction;
    LookAction = InLookAction;
    RunAction = InRunAction;
    JumpAction = InJumpAction;
    InteractAction = InInteractAction;
    DialogueAction = InDialogueAction;
    EndDialogueAction = InEndDialogueAction;
    SittingAction = InSittingAction;
    RollCallAction = InRollCallAction;
    QuestionAction = InQuestionAction;
    EndQuestionAction = InEndQuestionAction;
    // BindInputActions(); // 在 SetupPlayerInputComponent 中已经绑定，这里如果调用会重复绑定
}

void ASCUTPlayerCharacter::SetupSittingAction(UInputAction* InSittingAction)
{
    SittingAction = InSittingAction;
    // 如果需要动态绑定，可以在这里重新绑定
    // if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    // {
    //     if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerController->InputComponent))
    //     {
    //         EnhancedInputComponent->BindAction(SittingAction, ETriggerEvent::Started, this, &ASCUTPlayerCharacter::OnSittingAction);
    //     }
    // }
}

void ASCUTPlayerCharacter::SetupDormManagerInput(UInputAction* InInteractAction, UInputAction* InDialogueAction, UInputAction* InEndDialogueAction)
{
    // 理论上这里应该使用传入的InInteractAction等，并绑定到宿管专有的处理函数
    // 简化处理，假设 InteractAction 等被重用
    // 目前逻辑已在 OnInteractInput 函数中通过 FindDormManagerInRange 统一处理，宿管是NPC的一种
    // 如果需要宿管专有输入，应在此处绑定
    DialogueAction = InDialogueAction;
    EndDialogueAction = InEndDialogueAction;
}

void ASCUTPlayerCharacter::OnInteractDormManager(const FInputActionValue& Value)
{
    // 具体的宿管交互逻辑 - 注意这个函数目前可能不会被直接绑定，而是通过 OnInteractInput 统一调用
    // 为了保持第二段代码的直接性，我们可以在 OnInteractInput 里优先处理宿管的坐下/站起
    ASCUTDormManagerCharacter* DormManager = FindDormManagerInRange(DormManagerInteractionRange);
    if (DormManager)
    {
        // 模仿第二段代码中，交互键直接触发宿管站起
        if (DormManager->GetIsSitting())
        {
            UE_LOG(LogTemp, Warning, TEXT("玩家通过通用交互键触发宿管站起！"));
            DormManager->StandUp();
        }
        else if (!DormManager->GetIsStandingUp() && !DormManager->GetIsSittingDown())
        {
            // 如果宿管站着或在过渡中，尝试让她坐下 (这部分逻辑在第二段代码中没有直接对应，但符合逻辑)
             UE_LOG(LogTemp, Warning, TEXT("宿管已经站着或在过渡中，尝试通过通用交互键让她坐下。"));
             DormManager->SitDown();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("宿管正在坐下或站起过渡中，无法执行新的交互。"));
        }
    }
}

void ASCUTPlayerCharacter::OnDialogueDormManager(const FInputActionValue& Value)
{
    // 具体的宿管对话触发逻辑 (与第二段代码保持一致)
    ASCUTDormManagerCharacter* DormManager = FindDormManagerInRange(DormManagerInteractionRange);
    if (DormManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player triggered DormManager dialogue!"));
        StartDialogue(DormManager); // 使用通用 StartDialogue
        bIsTalkingToDormManager = true;
        // 锁定玩家移动
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->DisableMovement();
        }
    }
}

void ASCUTPlayerCharacter::OnEndDialogueDormManager(const FInputActionValue& Value)
{
    // 具体的宿管对话结束逻辑 (与第二段代码保持一致)
    if (bIsTalkingToDormManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player ended DormManager dialogue!"));
        EndDialogue(); // 使用通用 EndDialogue
        bIsTalkingToDormManager = false;
        // 解锁玩家移动
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        }
    }
}


ASCUTDormManagerCharacter* ASCUTPlayerCharacter::FindDormManagerInRange(float Range)
{
    FVector PlayerLocation = GetActorLocation();
    TArray<AActor*> FoundActors;
    // 尝试获取世界中的所有 ASCUTDormManagerCharacter 实例
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASCUTDormManagerCharacter::StaticClass(), FoundActors);

    ASCUTDormManagerCharacter* ClosestDormManager = nullptr;
    float ClosestDistanceSq = FMath::Square(Range); // 使用平方距离避免开方

    UE_LOG(LogTemp, Warning, TEXT("FindDormManagerInRange 被调用，检测范围: %.2f。找到 %d 个宿管类的Actor。"), Range, FoundActors.Num());

    // 遍历所有找到的 Actor
    for (AActor* Actor : FoundActors)
    {
        ASCUTDormManagerCharacter* DormManager = Cast<ASCUTDormManagerCharacter>(Actor);
        if (DormManager)
        {
            float DistanceSq = FVector::DistSquared(PlayerLocation, DormManager->GetActorLocation());
            UE_LOG(LogTemp, Warning, TEXT("  检查宿管: %s。距离平方: %.2f (范围平方: %.2f)"), *DormManager->GetName(), DistanceSq, ClosestDistanceSq);
            if (DistanceSq < ClosestDistanceSq)
            {
                ClosestDistanceSq = DistanceSq;
                ClosestDormManager = DormManager;
                UE_LOG(LogTemp, Warning, TEXT("    找到更近的宿管: %s"), *DormManager->GetName());
            }
        }
    }

    if (ClosestDormManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("FindDormManagerInRange 返回宿管: %s"), *ClosestDormManager->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("FindDormManagerInRange 返回 NULL (在指定范围内未找到宿管)。"));
    }
    return ClosestDormManager;
}

ASCUTTeacherCharacter* ASCUTPlayerCharacter::FindTeacherInRange(float Range)
{
    FVector PlayerLocation = GetActorLocation();
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASCUTTeacherCharacter::StaticClass(), FoundActors);

    ASCUTTeacherCharacter* ClosestTeacher = nullptr;
    float ClosestDistanceSq = FMath::Square(Range);

    for (AActor* Actor : FoundActors)
    {
        ASCUTTeacherCharacter* Teacher = Cast<ASCUTTeacherCharacter>(Actor);
        if (Teacher)
        {
            float DistanceSq = FVector::DistSquared(PlayerLocation, Teacher->GetActorLocation());
            if (DistanceSq < ClosestDistanceSq)
            {
                ClosestDistanceSq = DistanceSq;
                ClosestTeacher = Teacher;
            }
        }
    }
    return ClosestTeacher;
}

void ASCUTPlayerCharacter::OnTeacherRollCallAction(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("[Player] RollCall input pressed!"));
    ASCUTTeacherCharacter* Teacher = FindTeacherInRange(TeacherInteractionRange);
    if (Teacher)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Player] Found Teacher, calling StartRollCall."));
        // 实际调用老师的点名函数
        Teacher->StartRollCall();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Player] No Teacher found in range!"));
    }
}

void ASCUTPlayerCharacter::OnTeacherQuestionAction(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("[Player] Question input pressed!"));
    ASCUTTeacherCharacter* Teacher = FindTeacherInRange(TeacherInteractionRange);
    if (Teacher)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Player] Found Teacher, calling StartQuestioning."));
        // 实际调用老师的提问函数
        Teacher->StartQuestioning();
        bIsBeingQuestioned = true; // 假设玩家正在被提问
        // 锁定玩家移动
        bIsMovementLocked = true;
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->DisableMovement();
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Player] No Teacher found in range!"));
    }
}

void ASCUTPlayerCharacter::OnTeacherEndQuestionAction(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("[Player] EndQuestion input pressed!"));
    if (bIsBeingQuestioned)
    {
        ASCUTTeacherCharacter* Teacher = FindTeacherInRange(TeacherInteractionRange); // 再次查找老师，确保仍有效
        if (Teacher)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Player] Found Teacher, calling EndQuestioning."));
            // 结束提问状态
            Teacher->EndQuestioning();
            bIsBeingQuestioned = false;
            // 解锁玩家移动
            bIsMovementLocked = false;
            if (GetCharacterMovement())
            {
                GetCharacterMovement()->SetMovementMode(MOVE_Walking);
            }
        }
        else
        {
            // 如果老师不在范围内了，也应该重置状态
            UE_LOG(LogTemp, Warning, TEXT("[Player] Teacher no longer in range, ending question state."));
            bIsBeingQuestioned = false;
            bIsMovementLocked = false;
            if (GetCharacterMovement())
            {
                GetCharacterMovement()->SetMovementMode(MOVE_Walking);
            }
        }
    }
}

void ASCUTPlayerCharacter::SetupStudentInput(UInputAction* InStudentDialogueAction, UInputAction* InStudentEndDialogueAction)
{
    StudentDialogueAction = InStudentDialogueAction;
    StudentEndDialogueAction = InStudentEndDialogueAction;

    // 绑定学生相关输入 (这里只是设置InputAction变量，实际绑定在SetupPlayerInputComponent)
    // 如果 SetupStudentInput 是在运行时调用的，并且需要动态绑定，则需要这里的逻辑
    // if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
    // {
    //     if (StudentDialogueAction)
    //     {
    //         EnhancedInputComponent->BindAction(StudentDialogueAction, ETriggerEvent::Started, this, &ASCUTPlayerCharacter::OnStudentDialogueAction);
    //     }
    //     if (StudentEndDialogueAction)
    //     {
    //         EnhancedInputComponent->BindAction(StudentEndDialogueAction, ETriggerEvent::Started, this, &ASCUTPlayerCharacter::OnStudentEndDialogueAction);
    //     }
    // }
}

void ASCUTPlayerCharacter::OnStudentDialogueAction(const FInputActionValue& Value)
{
    ASCUTStudentCharacter* Student = FindStudentInRange(StudentInteractionRange);
    if (Student)
    {
        StartDialogue(Student); // 使用通用 StartDialogue
        // 锁定玩家移动
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->DisableMovement();
        }
    }
}

void ASCUTPlayerCharacter::OnStudentEndDialogueAction(const FInputActionValue& Value)
{
    if (bIsInDialogue && CurrentDialogueTarget && CurrentDialogueTarget->IsA(ASCUTStudentCharacter::StaticClass()))
    {
        EndDialogue(); // 使用通用 EndDialogue
        // 解锁玩家移动
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        }
    }
}

ASCUTStudentCharacter* ASCUTPlayerCharacter::FindStudentInRange(float Range)
{
    FVector PlayerLocation = GetActorLocation();
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASCUTStudentCharacter::StaticClass(), FoundActors);

    ASCUTStudentCharacter* ClosestStudent = nullptr;
    float ClosestDistanceSq = FMath::Square(Range);

    for (AActor* Actor : FoundActors)
    {
        ASCUTStudentCharacter* Student = Cast<ASCUTStudentCharacter>(Actor);
        if (Student)
        {
            float DistanceSq = FVector::DistSquared(PlayerLocation, Student->GetActorLocation());
            if (DistanceSq < ClosestDistanceSq)
            {
                ClosestDistanceSq = DistanceSq;
                ClosestStudent = Student;
            }
        }
    }
    return ClosestStudent;
}

void ASCUTPlayerCharacter::OnVendingMachineInteract(const FInputActionValue& Value)
{
    ASCUTVendingMachineCharacter* VendingMachine = FindVendingMachineInRange(VendingMachineInteractionRange);
    if (VendingMachine)
    {
        if (PlayerDrinkState == EPlayerDrinkState::None)
        {
            UE_LOG(LogTemp, Warning, TEXT("Player trying to buy from Vending Machine! (State: Buying)"));
            PlayerDrinkState = EPlayerDrinkState::Buying; // 播放A1动画
            bIsMovementLocked = true; // 锁定移动
            // 通常这里会触发UI或贩卖机的购买逻辑
            VendingMachine->InteractWithPlayer(this); // 假设贩卖机有一个交互函数来处理购买
        }
        else if (PlayerDrinkState == EPlayerDrinkState::Buying)
        {
            UE_LOG(LogTemp, Warning, TEXT("Player trying to drink from Vending Machine! (State: Drinking)"));
            PlayerDrinkState = EPlayerDrinkState::Drinking; // 播放A2动画
            bIsMovementLocked = true; // 锁定移动
            GetWorldTimerManager().SetTimer(PlayerDrinkTimerHandle, this, &ASCUTPlayerCharacter::OnPlayerDrinkFinished, PlayerDrinkingDuration, false);
        }
        // 其他状态下不处理
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No Vending Machine in range to interact with."));
    }
}

ASCUTVendingMachineCharacter* ASCUTPlayerCharacter::FindVendingMachineInRange(float Range)
{
    FVector PlayerLocation = GetActorLocation();
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASCUTVendingMachineCharacter::StaticClass(), FoundActors);

    ASCUTVendingMachineCharacter* ClosestVendingMachine = nullptr;
    float ClosestDistanceSq = FMath::Square(Range);

    for (AActor* Actor : FoundActors)
    {
        ASCUTVendingMachineCharacter* VendingMachine = Cast<ASCUTVendingMachineCharacter>(Actor);
        if (VendingMachine)
        {
            float DistanceSq = FVector::DistSquared(PlayerLocation, VendingMachine->GetActorLocation());
            if (DistanceSq < ClosestDistanceSq)
            {
                ClosestDistanceSq = DistanceSq;
                ClosestVendingMachine = VendingMachine;
            }
        }
    }
    return ClosestVendingMachine;
}

void ASCUTPlayerCharacter::OnPlayerDrinkFinished()
{
    PlayerDrinkState = EPlayerDrinkState::Finished; // 播放A3动画
    UE_LOG(LogTemp, Warning, TEXT("Player finished drinking. (State: Finished)"));
    GetWorldTimerManager().ClearTimer(PlayerDrinkTimerHandle);

    // 过一小段时间后重置状态，允许再次购买
    // bIsMovementLocked 保持锁定直到状态完全重置
    GetWorldTimerManager().SetTimer(DrinkFinishedResetTimerHandle, [this]()
    {
        PlayerDrinkState = EPlayerDrinkState::None;
        bIsMovementLocked = false;
        UE_LOG(LogTemp, Warning, TEXT("Player drinking state reset to None. Movement unlocked."));
        if (GetCharacterMovement())
        {
             GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        }
    }, 1.0f, false); // 1秒后重置，可根据需要调整
}

void ASCUTPlayerCharacter::SetMovementSpeed(float NewSpeed)
{
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
    }
}

bool ASCUTPlayerCharacter::IsMoving() const
{
    return GetVelocity().Size() > 0.0f;
}

bool ASCUTPlayerCharacter::IsRunning() const
{
    return bIsRunning;
}

void ASCUTPlayerCharacter::Move(const FInputActionValue& Value)
{
    if (Controller != nullptr && !bIsMovementLocked) // 检查移动锁定
    {
        const FVector2D MovementVector = Value.Get<FVector2D>();

        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void ASCUTPlayerCharacter::Look(const FInputActionValue& Value)
{
    if (Controller != nullptr)
    {
        const FVector2D LookAxisVector = Value.Get<FVector2D>();
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

// 跑步开始
void ASCUTPlayerCharacter::OnRun(const FInputActionValue& Value)
{
    if (!bIsMovementLocked) // 跑步也受移动锁定影响
    {
        bIsRunning = true;
        SetMovementSpeed(RunSpeed);
        UE_LOG(LogTemp, Warning, TEXT("Character is running!"));
    }
}

// 跑步结束
void ASCUTPlayerCharacter::OnStopRunning(const FInputActionValue& Value)
{
    if (!bIsMovementLocked) // 跑步也受移动锁定影响
    {
        bIsRunning = false;
        SetMovementSpeed(WalkSpeed);
        UE_LOG(LogTemp, Warning, TEXT("Character stopped running."));
    }
}

// 新增：处理跳跃输入
void ASCUTPlayerCharacter::InputJump(const FInputActionValue& Value)
{
    // 只有当角色不在空中且移动未被锁定时才允许跳跃
    if (GetCharacterMovement() && !GetCharacterMovement()->IsFalling() && !bIsMovementLocked)
    {
        GetCharacterMovement()->JumpZVelocity = JumpHeight; // 设置跳跃高度
        Super::Jump(); // 调用 ACharacter 的 Jump 函数
        UpdateCharacterState(ECharacterState::Jumping); // 设置为跳跃状态
    }
}

// 新增：处理停止跳跃输入
void ASCUTPlayerCharacter::InputStopJumping(const FInputActionValue& Value)
{
    Super::StopJumping(); // 调用 ACharacter 的 StopJumping 函数

    // 如果角色在地面上，恢复到之前的状态
    if (GetCharacterMovement() && !GetCharacterMovement()->IsFalling())
    {
        if (GetVelocity().Size() > 0.0f)
        {
            UpdateCharacterState(bIsRunning ? ECharacterState::Running : ECharacterState::Walking);
        }
        else
        {
            UpdateCharacterState(ECharacterState::Idle);
        }
    }
}

// 原始的 Jump 和 StopJumping 函数不再需要 FInputActionValue 参数
// 它们现在作为基类函数的覆盖存在
void ASCUTPlayerCharacter::Jump()
{
    // 这个函数是 ACharacter 的虚函数重载，它不接受 FInputActionValue 参数。
    // 跳跃逻辑现在由 InputJump 处理。
    // 在 InputJump 中调用 Super::Jump();
}

void ASCUTPlayerCharacter::StopJumping()
{
    // 这个函数是 ACharacter 的虚函数重载，它不接受 FInputActionValue 参数。
    // 停止跳跃逻辑现在由 InputStopJumping 处理。
    // 在 InputStopJumping 中调用 Super::StopJumping();
}

void ASCUTPlayerCharacter::StartDialogue(ASCUTNPCCharacter* NPCTarget)
{
    if (NPCTarget && !bIsInDialogue)
    {
        bIsInDialogue = true;
        CurrentDialogueTarget = NPCTarget;
        UpdateCharacterState(ECharacterState::Talking);
        NPCTarget->StartDialogue();
        // 锁定玩家移动
        bIsMovementLocked = true;
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->DisableMovement();
        }
    }
}

void ASCUTPlayerCharacter::EndDialogue()
{
    if (bIsInDialogue && CurrentDialogueTarget)
    {
        bIsInDialogue = false;
        CurrentDialogueTarget->EndDialogue();
        CurrentDialogueTarget = nullptr;
        UpdateCharacterState(ECharacterState::Idle);
        // 解锁玩家移动
        bIsMovementLocked = false;
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        }
    }
}

void ASCUTPlayerCharacter::SelectDialogueOption(int32 OptionIndex)
{
    if (bIsInDialogue && CurrentDialogueTarget)
    {
        // 这里可以添加对话选项的处理逻辑
        // 比如触发特定事件或改变NPC状态
    }
}

ASCUTNPCCharacter* ASCUTPlayerCharacter::FindInteractableNPC()
{
    // 获取玩家位置
    FVector PlayerLocation = GetActorLocation();

    // 获取所有NPC
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASCUTNPCCharacter::StaticClass(), FoundActors);

    // 查找最近的NPC
    ASCUTNPCCharacter* ClosestNPC = nullptr;
    float ClosestDistanceSq = FMath::Square(InteractionRange); // 使用一个通用的 InteractionRange

    for (AActor* Actor : FoundActors)
    {
        ASCUTNPCCharacter* NPC = Cast<ASCUTNPCCharacter>(Actor);
        if (NPC)
        {
            float DistanceSq = FVector::DistSquared(PlayerLocation, NPC->GetActorLocation());
            if (DistanceSq < ClosestDistanceSq)
            {
                ClosestDistanceSq = DistanceSq;
                ClosestNPC = NPC;
            }
        }
    }

    return ClosestNPC;
}

// <<< 替换原来的 Interact 函数为 OnInteractInput >>>
void ASCUTPlayerCharacter::OnInteractInput(const FInputActionValue& Value) // 注意：这里不再接收 FInputActionValue 参数
{
    UE_LOG(LogTemp, Warning, TEXT("ASCUTPlayerCharacter::OnInteractInput() 被调用。"));

    // 0. 如果移动被锁定，不允许任何交互（除非是明确设计为允许交互的UI操作）
    if (bIsMovementLocked)
    {
        UE_LOG(LogTemp, Warning, TEXT("Movement is locked, cannot interact."));
        return;
    }

    // 1. 检查 BusTrigger 交互 (高优先级)
    if (CurrentOverlappingBusTrigger)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player interacted with BusTrigger: %s"), *CurrentOverlappingBusTrigger->GetName());
        // 检查交互是否被锁定
        if (CurrentOverlappingBusTrigger->bIsInteractionLocked)
        {
            UE_LOG(LogTemp, Warning, TEXT("Interaction is currently locked. Cannot mount/dismount."));
            return; // 如果锁定，直接返回，不执行任何操作
        }

        if (CurrentOverlappingBusTrigger->bIsPlayerOnBus)
        {
            // 如果玩家已经在巴士上，则尝试下车
            CurrentOverlappingBusTrigger->DismountBus();
            UE_LOG(LogTemp, Warning, TEXT("Attempting to dismount bus."));
        }
        else if (CurrentOverlappingBusTrigger->bPlayerInRange)
        {
            // 如果玩家在范围内但不在巴士上，则尝试上车
            CurrentOverlappingBusTrigger->MountBus();
            UE_LOG(LogTemp, Warning, TEXT("Attempting to mount bus."));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Not in bus trigger range to interact, or not on bus."));
        }
        return; // 如果与巴士交互了，就不要再检查其他交互了
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No overlapping BusTrigger found. Checking other interactions."));
    }

    // 3. 检查宿管交互 (相对高优先级，因为宿管可能需要特殊处理)
    ASCUTDormManagerCharacter* DormManager = FindDormManagerInRange(DormManagerInteractionRange);
    if (DormManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnInteractInput: 在范围内找到了宿管！名称: %s"), *DormManager->GetName());
        // 直接调用宿管的 StandUp/SitDown 逻辑
        // 注意：第二段代码的 InteractAction 绑定的 OnInteractDormManager 仅触发 StandUp
        // 为了兼容，这里可以根据需求调整：
        if (DormManager->GetIsSitting())
        {
            UE_LOG(LogTemp, Warning, TEXT("玩家通过通用交互键触发宿管站起！"));
            DormManager->StandUp();
        }
        else if (!DormManager->GetIsStandingUp() && !DormManager->GetIsSittingDown())
        {
            UE_LOG(LogTemp, Warning, TEXT("宿管已经站着或在过渡中，尝试通过通用交互键让她坐下。"));
            DormManager->SitDown();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("宿管正在坐下或站起过渡中，无法执行新的交互。"));
        }
        return;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("OnInteractInput: 在宿管交互范围内 (%.2f) 未找到宿管。继续检查贩卖机..."), DormManagerInteractionRange);
    }

    // 2. 检查与椅子的交互 (高优先级，因为玩家可能想主动坐下/站起)
    // 这里的 OnSittingAction 会自动处理坐下和站起的逻辑
    if (IsInChairRange())
    {
        UE_LOG(LogTemp, Warning, TEXT("玩家在椅子范围内，尝试坐下/站起。"));
        OnSittingAction(Value); // 调用坐下/站起函数
        return;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("OnInteractInput: 不在椅子交互范围内 (%.2f)。继续检查宿管..."), ChairInteractionRange);
    }

    // 4. 检查与贩卖机的交互
    ASCUTVendingMachineCharacter* VendingMachine = FindVendingMachineInRange(VendingMachineInteractionRange);
    if (VendingMachine)
    {
        UE_LOG(LogTemp, Warning, TEXT("玩家尝试与贩卖机交互。"));
        OnVendingMachineInteract(Value); // 调用贩卖机交互逻辑
        return;
    }

    // 5. 最后，如果没有其他特定交互发生，检查其他通用 NPC 交互 (最低优先级)
    ASCUTNPCCharacter* InteractableNPC = FindInteractableNPC();
    if (InteractableNPC)
    {
        UE_LOG(LogTemp, Warning, TEXT("玩家尝试与普通NPC交互。"));
        StartDialogue(InteractableNPC); // 触发通用NPC对话
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("No interactable object found in range."));
}


void ASCUTPlayerCharacter::SitDown()
{
    if (!bIsSitting && !bIsMovingToSeat && !bIsMovementLocked)
    {
        // 1. 查找最近的椅子并获取其位置和旋转
        TArray<AActor*> FoundActors;
        // 假设椅子有一个特定的Tag或者继承自一个特定的类 (这里仍然使用 Tag)
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors); 

        AActor* ClosestChair = nullptr;
        float ClosestDistanceSq = FMath::Square(ChairInteractionRange);
        FVector PlayerLocation = GetActorLocation();

        for (AActor* Actor : FoundActors)
        {
            if (Actor->ActorHasTag(TEXT("Chair"))) // 假设椅子有 "Chair" Tag
            {
                float DistanceSq = FVector::DistSquared(PlayerLocation, Actor->GetActorLocation());
                if (DistanceSq < ClosestDistanceSq)
                {
                    ClosestDistanceSq = DistanceSq;
                    ClosestChair = Actor;
                }
            }
        }

        if (ClosestChair)
        {
            // 更新 ChairLocation 和 ChairRotation
            ChairLocation = ClosestChair->GetActorLocation() + SittingOffset;
            ChairRotation = ClosestChair->GetActorRotation();

            // 2. 检查是否有老师在范围内，并调整朝向
            ASCUTTeacherCharacter* Teacher = FindTeacherInRange(TeacherInteractionRange);
            if (Teacher)
            {
                FVector TeacherLocation = Teacher->GetActorLocation();
                // 计算面向老师的旋转 (只改变Yaw)
                FRotator LookAtRotation = (TeacherLocation - GetActorLocation()).Rotation();
                ChairRotation.Yaw = LookAtRotation.Yaw; // 保持椅子本身的Pitch和Roll，只改变Yaw
                UE_LOG(LogTemp, Warning, TEXT("[Player] SitDown: Adjusting rotation to face teacher."));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("[Player] SitDown: No teacher found in range, using chair default rotation."));
            }

            // 3. 开始平滑移动到座位
            bIsMovingToSeat = true;
            bIsMovementLocked = true; // 锁定移动直到坐下完成
            if (GetCharacterMovement())
            {
                GetCharacterMovement()->DisableMovement();
            }
            UpdateCharacterState(ECharacterState::SittingDown); // 状态设置为坐下中
            UE_LOG(LogTemp, Warning, TEXT("Player starting to move to chair."));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No chair found in range to sit down."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Player is already sitting, moving to seat, or movement is locked. Cannot sit down."));
    }
}

void ASCUTPlayerCharacter::StandUp()
{
    if (bIsSitting && !bIsMovingToSeat && !bIsMovementLocked)
    {
        bIsMovementLocked = true;
        bIsStandingUp = true;
        UpdateCharacterState(ECharacterState::StandingUp);

        // 延迟调用动画结束函数
        GetWorldTimerManager().SetTimer(StandUpTimerHandle, this, &ASCUTPlayerCharacter::OnStandUpFinished, SittingLockTime, false);
        UE_LOG(LogTemp, Warning, TEXT("Player starting to stand up."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Player is not sitting, already moving, or movement is locked. Cannot stand up."));
    }
}

void ASCUTPlayerCharacter::StartSittingAnimation()
{
    // 这只是一个标记函数，您应该在蓝图或动画C++层触发实际的动画蒙太奇
    // 例如：
    // if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
    // {
    //     AnimInstance->Montage_Play(SittingMontage);
    // }
    UE_LOG(LogTemp, Warning, TEXT("Triggering sitting animation... (Actual animation playback should be in Blueprint)"));
    // 坐下动画结束后，调用 OnSitDownFinished
    GetWorldTimerManager().SetTimer(SitDownTimerHandle, this, &ASCUTPlayerCharacter::OnSitDownFinished, SittingLockTime, false);
}

void ASCUTPlayerCharacter::OnSitDownFinished()
{
    bIsSitting = true;
    bIsSittingDown = false;
    bIsMovementLocked = false; // 坐下后解锁移动（可以在坐着时使用Look）
    UpdateCharacterState(ECharacterState::Sitting);
    UE_LOG(LogTemp, Warning, TEXT("Player finished sitting down."));
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->SetMovementMode(MOVE_Walking); // 重新启用移动模式，但实际是否能移动取决于 bIsMovementLocked
    }
}

void ASCUTPlayerCharacter::OnStandUpFinished()
{
    bIsSitting = false;
    bIsStandingUp = false;
    bIsMovementLocked = false;
    UpdateCharacterState(ECharacterState::Idle); // 站起来后回到空闲状态
    UE_LOG(LogTemp, Warning, TEXT("Player finished standing up."));
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->SetMovementMode(MOVE_Walking); // 重新启用移动
    }
}

bool ASCUTPlayerCharacter::IsInChairRange() const
{
    // 查找最近的椅子
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors); // 假设椅子是通用AActor，或者可以有专门的Chair类

    FVector PlayerLocation = GetActorLocation();
    for (AActor* Actor : FoundActors)
    {
        // 假设椅子有一个特定的Tag或者继承自一个特定的类
        if (Actor->ActorHasTag(TEXT("Chair"))) // 或者 Cast<AChair>(Actor)
        {
            if (FVector::Distance(PlayerLocation, Actor->GetActorLocation()) <= ChairInteractionRange)
            {
                // 存储椅子位置和旋转以便坐下
                // 注意：这里用 const_cast 写入非 const 成员变量，仅为方便在 const 函数中实现，
                // 更好的做法是将ChairLocation/ChairRotation作为SitDown的参数传递，或者将ChairInteractionRange
                // 函数改为非 const
                const_cast<ASCUTPlayerCharacter*>(this)->ChairLocation = Actor->GetActorLocation(); // 仅获取椅子基础位置
                const_cast<ASCUTPlayerCharacter*>(this)->ChairRotation = Actor->GetActorRotation();
                UE_LOG(LogTemp, Warning, TEXT("Found chair in range: %s at %s"), *Actor->GetName(), *Actor->GetActorLocation().ToString());
                return true;
            }
        }
    }
    return false;
}

void ASCUTPlayerCharacter::OnSittingAction(const FInputActionValue& Value)
{
    // 这个函数同时处理坐下和站起
    if (bIsSitting)
    {
        StandUp();
    }
    else if (IsInChairRange()) // 只有在椅子范围内才能尝试坐下
    {
        SitDown();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Not in chair range to sit down."));
    }
}

// --- 视角切换功能实现 ---

void ASCUTPlayerCharacter::ToggleView()
{
    if (bIsFirstPersonView)
    {
        SetThirdPersonView();
    }
    else
    {
        SetFirstPersonView();
    }
}

void ASCUTPlayerCharacter::SetFirstPersonView()
{
    // 设置当前为第一人称视角
    bIsFirstPersonView = true;

    // 禁用弹簧臂（相机直接附加到角色）
    if (CameraBoom)
    {
        CameraBoom->TargetArmLength = 0.0f;           // 长度设为0，相当于相机就在胶囊体位置
        CameraBoom->bDoCollisionTest = false;         // 禁用碰撞测试
        CameraBoom->bUsePawnControlRotation = true;   // 确保相机臂仍然跟随Pawn控制旋转
    }

    // 第一人称摄像机附加到胶囊体
    FirstPersonCameraComponent->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform);
    // 设置相对位置
    FirstPersonCameraComponent->SetRelativeLocation(FirstPersonCameraRelativeLocation);
    // *** 关键修改：重置第一人称摄像机的相对旋转 ***
    FirstPersonCameraComponent->SetRelativeRotation(FRotator::ZeroRotator);
    // 1P相机旋转跟随Pawn控制器旋转
    FirstPersonCameraComponent->bUsePawnControlRotation = true;

    // 隐藏第三人称网格体（GetMesh()）
    if (GetMesh())
    {
        GetMesh()->SetOwnerNoSee(true); // 让自己的相机看不到自己的身体
    }

    // 显示第一人称网格体（如手臂）
    if (Mesh1P)
    {
        Mesh1P->SetOnlyOwnerSee(true); // 只让拥有者看到
        Mesh1P->SetVisibility(true);   // 确保是可见的
    }

    // 允许控制器Yaw影响角色Yaw（用于第一人称Look）
    bUseControllerRotationYaw = true;
    // 禁用角色面向移动方向（第一人称时通常不希望角色身体自动旋转）
    GetCharacterMovement()->bOrientRotationToMovement = false;

    // 恢复第一人称的默认旋转速度，如果它曾被改变
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f); // 默认速度
    GetCharacterMovement()->bUseControllerDesiredRotation = false; // 第一人称通常不需要此项
    UE_LOG(LogTemp, Warning, TEXT("Switched to First Person View."));
}

void ASCUTPlayerCharacter::SetThirdPersonView()
{
    // 设置当前为第三人称视角
    bIsFirstPersonView = false;

    // 启用弹簧臂
    if (CameraBoom)
    {
        CameraBoom->TargetArmLength = ThirdPersonCameraDistance; // 恢复到第三人称距离
        CameraBoom->bDoCollisionTest = true;                   // 启用碰撞测试
        CameraBoom->bUsePawnControlRotation = true;            // 确保相机臂跟随Pawn控制旋转
    }

    // 摄像机附加到弹簧臂末端
    FirstPersonCameraComponent->AttachToComponent(CameraBoom, FAttachmentTransformRules::SnapToTargetNotIncludingScale, USpringArmComponent::SocketName);
    // 摄像机相对弹簧臂末端为0,0,0
    FirstPersonCameraComponent->SetRelativeLocation(FVector::ZeroVector);
    // 3P相机不直接使用Pawn控制旋转（而是通过弹簧臂来旋转）
    FirstPersonCameraComponent->bUsePawnControlRotation = false;

    // 显示第三人称网格体（GetMesh()）
    if (GetMesh())
    {
        GetMesh()->SetOwnerNoSee(false); // 让拥有者也可以看到自己的身体（即不对拥有者隐藏）
        GetMesh()->SetVisibility(true);   // 确保是可见的
    }

    // 隐藏第一人称网格体（如手臂）
    if (Mesh1P)
    {
        Mesh1P->SetOnlyOwnerSee(false); // 不再只让拥有者看到（因为要隐藏）
        Mesh1P->SetVisibility(false);   // 隐藏
    }

    // 允许控制器Yaw影响角色Yaw，使角色身体跟随控制器（摄像机）的Yaw旋转
    bUseControllerRotationYaw = true; // 保持 true 以便控制器 Yaw 影响角色 Yaw
    // 启用角色面向移动方向（第三人称时通常希望角色身体自动旋转）
    GetCharacterMovement()->bOrientRotationToMovement = true; 
    // 让角色的Yaw旋转速度更快地匹配控制器Yaw
    GetCharacterMovement()->bUseControllerDesiredRotation = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // 调整旋转速度，使其平滑
    UE_LOG(LogTemp, Warning, TEXT("Switched to Third Person View."));
}

void ASCUTPlayerCharacter::OnToggleView(const FInputActionValue& Value)
{
    ToggleView(); // 调用实际的切换逻辑
}

// <<< 实现 SetCurrentOverlappingBusTrigger 函数 >>>
void ASCUTPlayerCharacter::SetCurrentOverlappingBusTrigger(ABusTrigger* BusTrigger)
{
    CurrentOverlappingBusTrigger = BusTrigger;
    if (BusTrigger)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player now overlapping BusTrigger: %s"), *BusTrigger->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Player no longer overlapping BusTrigger."));
    }
}

// 假设的 UpdateCharacterState 函数（如果您的项目中没有，请添加）
void ASCUTPlayerCharacter::UpdateCharacterState(ECharacterState NewState)
{
    // 您可以在这里实现动画状态机逻辑，或者在蓝图中处理
    // 例如，通过蓝图事件通知当前状态
    // BlueprintEvent_OnCharacterStateChanged(NewState);
    // UE_LOG(LogTemp, Log, TEXT("Character State Changed to: %s"), *UEnum::GetValueAsString(NewState));
}