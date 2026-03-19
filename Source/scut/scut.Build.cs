using UnrealBuildTool;

public class scut : ModuleRules
{
    public scut(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "MediaAssets",
            "EnhancedInput",
            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",
            "AIModule",
            "NavigationSystem",
            // ****** 添加以下 UI 相关的模块 ******
            "UMG",           // 用于UserWidget
            "Slate",         // UMG的底层依赖
            "SlateCore",      // UMG的底层依赖
            "Json",         // <--- 添加这个
            "JsonUtilities", // <--- 添加这个
            "HTTP"
		});

        PrivateDependencyModuleNames.AddRange(new string[] { });

        // 你这里注释掉的 Slate 和 SlateCore 实际上应该加到 PublicDependencyModuleNames
        // 因为你的头文件直接使用了 UMG 相关的类型，这些类型在公共接口中。
        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" }); 
    }
}