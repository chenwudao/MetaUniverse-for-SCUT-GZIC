# SCUT 小组作业项目

## 项目简介
本项目为大学小组作业，基于 Unreal Engine 5 开发，包含角色蓝图、动画蓝图、输入系统等内容。支持跳跃、奔跑等基础功能。

---

## 环境要求
- Unreal Engine 5.x（建议与组长/小组统一版本）
- Visual Studio 2022（或兼容的VS版本，需安装C++开发组件）
- Git（用于版本管理）

---

## 如何使用本项目

### 1. 克隆项目代码
```bash
git clone https://github.com/你们组名/项目名.git
```

### 2. 生成 Visual Studio 工程文件
1. 找到项目根目录（有 `.uproject` 文件的那个目录）。
2. 右键 `.uproject` 文件，选择 **"Generate Visual Studio project files"（生成VS工程文件）**。
   - 或者在命令行输入：
     ```
     "C:\Program Files\Epic Games\UE_5.x\Engine\Binaries\Win64\UnrealVersionSelector.exe" /projectfiles YourProject.uproject
     ```
3. 生成后会出现 `.sln` 文件。

### 3. 用 Visual Studio 打开并编译
1. 双击 `.sln` 文件，用 Visual Studio 打开。
2. 选择 `Development Editor` 配置，平台选 `Win64`。
3. 右键解决方案，选择 **生成（Build）**。

### 4. 启动 Unreal Editor
1. 双击 `.uproject` 文件，启动 Unreal Editor。
2. 第一次打开会自动编译C++代码。

---

## 关于 .uasset 资源文件
- 所有蓝图、动画、输入设置等资源都在 `Content/` 文件夹下。
- **确保 `Content/` 文件夹完整上传和下载，否则蓝图和动画会丢失！**
- 不需要手动导入 `.uasset`，只要 `Content/` 文件夹在项目里，UE会自动识别。

---

## 常见问题

### Q: 蓝图/动画蓝图/输入设置丢失？
A: 检查 `Content/` 文件夹是否完整，`.uasset` 文件是否都在。

### Q: 编译报错？
A: 检查UE版本和VS组件是否一致，必要时右键 `.uproject` 重新生成VS工程文件。

### Q: 运行时跳跃/动画没反应？
A: 检查蓝图变量、输入映射、动画蓝图设置是否正确，参考小组文档或联系组员。

---

## 贡献说明
- 提交前请拉取最新代码，避免冲突。
- 只提交源码和 `Content/` 文件夹，`Binaries/`、`Intermediate/`、`Saved/` 文件夹无需上传。

---

如有其他问题，请联系组长或在小组群内讨论。 