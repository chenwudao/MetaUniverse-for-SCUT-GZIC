# SCUT GZIC 校园元宇宙

## 项目简介

本仓库是华南理工大学广州国际校区（SCUT GZIC）的校园元宇宙课程小组项目，使用 Unreal Engine 5.2 开发。

校园地图使用 Luma AI 插件导入的高斯泼溅场景搭建，资源位于 `Content/Scenes`。项目包含角色蓝图、动画蓝图和 Enhanced Input 输入资源；玩家角色支持跳跃与奔跑。

- 角色蓝图位于 `Content/_Game/Character`，玩家蓝图为 `BP_SCUTPlayerCharacter`。
- 动画蓝图位于 `Content/Animations`。玩家的待机、行走、奔跑与跳跃动画位于 `Content/Source/BP_SCUTPlayerCharacter`。
- 输入动作包括 `IA_Move`、`IA_Look`、`IA_Jump` 和 `IA_Run`，位于 `Content/_Game/Character/MainCharacter/Input`。
- 关卡包括菜单、教室、食堂、图书馆、教学楼、宿舍（地图文件名为 `domitory`）和主路。默认启动关卡为 `Content/Menu`。

## 环境要求

- Unreal Engine 5.2，与 `scut.uproject` 中的引擎版本一致。
- Visual Studio 2022，并安装 `.vsconfig` 中列出的 C++ 与游戏开发组件。
- Git 与 Git LFS。`.uasset`、`.umap` 等资源通过 Git LFS 管理。

`LumaAIPlugin` 已在工程中启用，插件来自虚幻商城，不包含在本仓库内。若打开工程时提示缺少该插件，请先安装后再进入编辑器。

## 如何使用

### 1. 克隆仓库

```bash
git lfs install
git clone https://github.com/chenwudao/MetaUniverse-for-SCUT-GZIC.git
```

克隆后请确认 `Content` 中的资源已由 Git LFS 检出，而不是只留下指针文件。

### 2. 生成 Visual Studio 工程

1. 在仓库根目录找到 `scut.uproject`。
2. 右键该文件，选择 **Generate Visual Studio project files**（生成 Visual Studio 工程文件）。
3. 也可以在命令行执行。请按本机引擎安装路径调整：

```bat
"C:\Program Files\Epic Games\UE_5.2\Engine\Binaries\Win64\UnrealVersionSelector.exe" /projectfiles scut.uproject
```

生成成功后，根目录会出现 `scut.sln`。该文件已列入 `.gitignore`，无需提交。

### 3. 编译

1. 用 Visual Studio 2022 打开 `scut.sln`。
2. 配置选择 **Development Editor**，平台选择 **Win64**。
3. 生成解决方案。

### 4. 打开编辑器

双击 `scut.uproject` 启动 Unreal Editor。首次打开时会编译 C++ 代码。

## 资源文件

蓝图、动画、输入和场景资源都在 `Content` 中。编辑器会直接识别这些 `.uasset`，无需单独导入。

请保持 `Content` 完整。资源缺失时，角色蓝图、动画蓝图和关卡将无法打开。拉取或推送前，请确认 Git LFS 已安装。

## 常见问题

### 蓝图、动画蓝图或输入资源丢失

检查 `Content` 是否完整，并确认 Git LFS 已将 `.uasset` 检出为实际文件。

### 编译失败

确认使用的是 Unreal Engine 5.2，且 Visual Studio 已安装 `.vsconfig` 中的 C++ 组件。必要时右键 `scut.uproject`，重新生成 Visual Studio 工程后再编译。

### 跳跃或动画没有反应

检查玩家蓝图上的变量、Enhanced Input 映射，以及动画蓝图是否指向当前角色。跳跃与奔跑分别对应 `IA_Jump` 和 `IA_Run`。

## 协作约定

- 提交前先拉取最新代码，以减少冲突。
- 提交源码、配置和 `Content`。不要提交 `Binaries`、`DerivedDataCache`、`Intermediate`、`Saved` 等生成目录，这些目录已在 `.gitignore` 中忽略。

其他问题请在课程小组内沟通。
