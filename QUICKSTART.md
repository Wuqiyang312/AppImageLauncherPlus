# Quick Start Guide: Custom Launch Arguments

## 简介 (Introduction)

AppImageLauncherPlus 现在支持为每个 AppImage 配置独立的启动参数！

AppImageLauncherPlus now supports custom launch arguments for each AppImage!

---

## 快速开始 (Quick Start)

### 1. 打开 AppImage 管理器 (Open AppImage Manager)

**方式一 (Method 1)**: 从应用菜单搜索 "AppImage Manager"

**方式二 (Method 2)**: 在终端运行
```bash
AppImageLauncherManager
```

### 2. 配置启动参数 (Configure Launch Arguments)

1. 在表格中找到你的 AppImage (Find your AppImage in the table)
2. 右键点击 → "编辑启动参数" (Right-click → "Edit Launch Arguments")
3. 输入参数，例如: `--fullscreen --debug`
4. 点击 OK 保存 (Click OK to save)

### 3. 使用配置的参数运行 (Launch with Arguments)

**方式一 (Method 1)**: 从应用菜单启动 AppImage（自动应用参数）
Launch from app menu (arguments applied automatically)

**方式二 (Method 2)**: 在管理器中右键 → "运行"
Right-click in manager → "Run"

---

## 配置文件位置 (Config File Location)

配置文件与 AppImage 在同一目录，同名但扩展名为 `.cfg`：

Config file is in the same directory as AppImage, with `.cfg` extension:

```
~/Applications/MyApp.AppImage  → ~/Applications/MyApp.cfg
```

---

## 配置文件格式 (Config File Format)

```ini
[Launch]
Arguments=--your-args-here
```

### 示例 (Examples):

**全屏模式 (Fullscreen)**:
```ini
[Launch]
Arguments=--fullscreen
```

**调试模式 (Debug mode)**:
```ini
[Launch]
Arguments=--debug --verbose
```

**自定义数据目录 (Custom data directory)**:
```ini
[Launch]
Arguments=--data-dir=/custom/path
```

---

## 其他功能 (Other Features)

### 创建桌面快捷方式 (Create Desktop Shortcut)
右键 AppImage → "创建桌面快捷方式"
Right-click → "Create Desktop Shortcut"

### 直接运行 (Run Directly)
右键 AppImage → "运行"
Right-click → "Run"

### 移除 (Remove)
右键 AppImage → "移除"（会删除 AppImage 和配置文件）
Right-click → "Remove" (removes AppImage and config file)

---

## 常见问题 (FAQ)

### Q: 配置文件会自动创建吗？
**A**: 不会，需要通过管理器手动配置。

### Q: Will config files be created automatically?
**A**: No, you need to configure through the manager manually.

---

### Q: 参数中可以包含空格吗？
**A**: 可以，但暂不支持引号内的空格（如 `"file name with spaces"`）。

### Q: Can arguments contain spaces?
**A**: Yes, but quoted strings with spaces are not yet supported.

---

### Q: 配置文件可以手动编辑吗？
**A**: 可以！用任何文本编辑器打开 `.cfg` 文件即可。

### Q: Can I edit config files manually?
**A**: Yes! Open the `.cfg` file with any text editor.

---

### Q: 配置是否随 AppImage 移动？
**A**: 是的，配置文件在同一目录，移动 AppImage 时记得一起移动。

### Q: Does config move with AppImage?
**A**: Yes, but remember to move the `.cfg` file together.

---

## 高级用法 (Advanced Usage)

### 手动创建配置文件 (Create config manually)

```bash
# 为 MyApp.AppImage 创建配置
cat > ~/Applications/MyApp.cfg << EOF
[Launch]
Arguments=--custom-option value --flag
EOF
```

### 批量配置 (Batch configuration)

```bash
# 为所有 AppImage 添加相同参数
for app in ~/Applications/*.AppImage; do
    cfg="${app%.AppImage}.cfg"
    echo "[Launch]" > "$cfg"
    echo "Arguments=--common-arg" >> "$cfg"
done
```

### 查看所有配置 (View all configs)

```bash
# 列出所有配置文件
find ~/Applications -name "*.cfg" -exec echo "=== {} ===" \; -exec cat {} \;
```

---

## 需要帮助？ (Need Help?)

- 📖 详细文档: `CUSTOM_LAUNCH_ARGS.md`
- 🐛 报告问题: GitHub Issues
- 💬 讨论: GitHub Discussions

---

**享受更灵活的 AppImage 使用体验！**

**Enjoy more flexible AppImage usage!**
