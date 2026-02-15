# AppImageLauncherPlus 中文简介

## 项目简介
AppImageLauncherPlus 是一款让你的 Linux 桌面原生支持 AppImage 应用的工具。它可以一键集成 AppImage 到系统应用菜单，方便管理、更新和移除 AppImage 应用，无需手动赋予可执行权限，双击即可运行。

## 主要功能
- **桌面集成**：自动将 AppImage 添加到应用菜单/启动器，并集中管理。
- **AppImage 管理器**：图形化管理界面，可查看所有已集成的 AppImage，支持：
  - 自定义启动参数：为每个 AppImage 配置独立的启动参数
  - 创建桌面快捷方式：右键创建当前用户的桌面启动项
  - 直接运行、移除 AppImage
- **更新管理**：集成后可直接在菜单中一键更新 AppImage。
- **移除管理**：支持从系统中彻底移除 AppImage 及其集成信息。
- **命令行工具**：提供 `ail-cli`，支持集成/去集成等自动化操作。
- **Lite 版**：无需 root 权限，用户可通过 AppImage 方式自集成 Lite 版。

## 安装方法
### 1. 系统包安装（推荐）
- 支持多种主流发行版（Ubuntu、Debian、openSUSE、Arch、Manjaro 等）。
- 直接下载安装对应的 `.deb` 或 `.rpm` 包，或通过发行版仓库/AUR 安装。
- 具体兼容性和包名请参考[英文 README](https://github.com/TheAssassin/AppImageLauncher#installation)。

### 2. 源码编译
1. 安装依赖（以 Ubuntu/Debian 为例）：
   ```bash
   sudo apt install make cmake libglib2.0-dev libcairo2-dev librsvg2-dev libfuse-dev libarchive-dev libxpm-dev libcurl4-openssl-dev libboost-all-dev qtbase5-dev qtdeclarative5-dev qttools5-dev-tools patchelf libc6-dev gcc-multilib g++-multilib
   ```
2. 克隆源码并编译：
   ```bash
   git clone https://github.com/TheAssassin/AppImageLauncher.git -b stable
   cd AppImageLauncher
   git submodule update --init --recursive
   export PREFIX="/usr/local/"
   mkdir build && cd build
   cmake .. -DCMAKE_INSTALL_PREFIX="$PREFIX" -DUSE_SYSTEM_BOOST=true
   make
   sudo make install
   ```

### 3. Lite 版（无需 root 权限）
- 下载 AppImageLauncher Lite 的 AppImage 文件。
- 运行 `./appimagelauncher-lite-*.AppImage install` 完成自集成。

## 基本用法
- **双击 AppImage 文件**：首次运行会弹窗提示是否集成到系统，选择后自动移动并集成。
- **应用菜单管理**：集成后可在菜单中直接启动、更新或移除 AppImage。
- **AppImage 管理器**：
  - 启动方式：在应用菜单中搜索"AppImage Manager"或运行 `AppImageLauncherManager` 命令
  - 查看所有已集成的 AppImage
  - 右键菜单支持：
    - **编辑启动参数**：为 AppImage 设置独立的启动参数（保存在同名 .cfg 文件中）
    - **创建桌面快捷方式**：在桌面创建当前用户的启动项
    - **运行**：使用配置的参数直接运行 AppImage
    - **移除**：彻底删除 AppImage 及其配置
- **命令行工具**：使用 `ail-cli` 进行批量集成/去集成等操作。

### 自定义启动参数说明
- 启动参数保存在 AppImage 所在目录的同名 .cfg 配置文件中
- 配置文件格式为 INI 格式，参数保存在 `[Launch]` 节的 `Arguments` 键下
- 示例：如果 AppImage 路径为 `~/Applications/MyApp.AppImage`，配置文件为 `~/Applications/MyApp.cfg`
- 配置文件内容示例：
  ```ini
  [Launch]
  Arguments=--some-option value --another-flag
  ```
- 只运行 AppImage 不会自动创建配置文件，需要通过 AppImage 管理器手动配置

## 依赖说明
- 主要依赖：glib2、cairo、librsvg、fuse、libarchive、libXpm、qt5、libcurl、boost 等。
- 详细依赖及安装方法见上文源码编译部分。

## 贡献指南
- 欢迎通过 [Weblate](https://translate.assassinate-you.net/projects/appimagelauncher/) 参与翻译。
- 可通过 PR 改进文档、教程或提交新功能/修复 bug。
- 遇到问题请在 [issue 区](https://github.com/TheAssassin/AppImageLauncher/issues/new) 反馈，尽量详细描述问题。
- 详细贡献流程见 CONTRIBUTING.md。

## 许可证
本项目采用 MIT 许可证，允许自由使用、修改和分发。详情见 LICENSE.txt。

## 常见问题与注意事项
- 推荐优先使用系统包安装，体验更完整。
- Lite 版适合无 root 权限用户，但功能有限。
- 如遇兼容性或集成问题，请查阅 [Wiki](https://github.com/TheAssassin/AppImageLauncher/wiki) 或 issue 区。

---

> 本文档为非官方中文简介，内容参考英文 README、BUILD.md、CONTRIBUTING.md 及 LICENSE.txt。
