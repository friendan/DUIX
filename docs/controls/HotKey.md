# HotKey

| | |
|--|--|
| 类 | `CHotKeyUI` |
| XML | `<HotKey>` |
| 源码 | `src/DuiLib/Control/UIHotKey.*` |
| 继承属性 | 见 [Label.md](Label.md) |

快捷键录入框：获得焦点后创建系统 `HOTKEY` 子窗口，用户按下组合键后回写显示文本。可用 C++ `SetHotKey` / `GetHotKey` 读写虚拟键与修饰符。删除获焦控件时会同步拆掉原生 HWND，避免焦点回打重建崩溃。

### 最小示例

```xml
<HotKey name="hk_capture" width="180" height="28"
    border="1px solid #D9D9D9" background-color="#FFFFFFFF"
    native-background-color="#FFFFFFFF" />
```

```cpp
CHotKeyUI* p = static_cast<CHotKeyUI*>(m_pm.FindControl(_T("hk_capture")));
WORD vk = 0, mod = 0;
p->GetHotKey(vk, mod);          // HOTKEYF_CONTROL / ALT / SHIFT / WIN / EXT …
p->SetHotKey(VK_F5, HOTKEYF_CONTROL | HOTKEYF_WIN); // Win = DuiLib 扩展 0x10
```

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| （盒模型 / 颜色 / 字体） | 继承 Label / Control |

无标准 HTML 等价物（接近桌面「快捷键」编辑器）。

### 非标准

| 属性 | 说明 |
|------|------|
| `image` / `image-hover` / `image-focus` / `image-disabled` | 状态皮肤图（自绘底） |
| `native-background-color` | 原生 HOTKEY 子窗口背景色 |
| `readonly` / `read-only` | 只读展示：不创建原生子窗、箭头光标；由外部 `SetHotKey` 更新（[HotKeyBox](Modal.md#同步-hotkeybox快捷键对话框) 用此模式并在对话框级捕获按键） |

### C++ API

| 方法 | 说明 |
|------|------|
| `SetHotKey(vk, modifiers)` / `GetHotKey` | 设置 / 读取快捷键（不再强制创建原生窗；获焦时再创建） |
| `SetReadOnly` / `IsReadOnly` | 只读：不创建原生窗、箭头光标 |
| `FormatHotKeyName(vk, modifiers)` | 静态：显示名（如 `Ctrl + S`、`Win + E`）；均为 0 时返回空 |
| `HotKeyToRegisterMods(hotkeyf[, noRepeat])` | `HOTKEYF_*` → `RegisterHotKey` 的 `fsModifiers`（含 `HOTKEYF_WIN→MOD_WIN`） |
| `RegisterModsToHotKey(mods)` | 反向：`MOD_*` → `HOTKEYF_*` |
| `IsSameHotKey` / `HotKeyCompareMask` | 比较（忽略 `HOTKEYF_EXT`） |
| `IsLetterOrDigitKey` / `IsBareLetterOrDigit` | 字母数字；无修饰键的字母数字（RequireModifier 用） |
| `FindShortcutConflict(pm, vk, mod[, exclude, text])` | 扫描控件树占用（`SetShortcutKey` / Alt-`shortcut`） |
| `SetNativeBackgroundColor` | 同 `native-background-color` |

修饰键：系统 `HOTKEYF_SHIFT` / `CONTROL` / `ALT` / `EXT`，以及 DuiLib 扩展 **`HOTKEYF_WIN`（0x10）**。原生 `HOTKEY` 子窗不识别 Win；只读 / HotKeyBox 可完整存显。部分 `Win+…` 会被系统抢走（如 `Win+L`），对话框可能录不到。

聚焦时弹出原生控件；失焦后销毁子窗口，文字画在 Label 上。

---

## CHotKeyBinder（程序内 / 全局响应）

| | |
|--|--|
| 类 | `CHotKeyBinder` |
| 头文件 | `UIHotKeyBinder.h`（`UIlib.h` 已包含） |
| 源码 | `src/DuiLib/Control/UIHotKeyBinder.*` |

**作用域由调用方传入**：`HOTKEYBOX_SCOPE_APP`（程序内）或 `HOTKEYBOX_SCOPE_GLOBAL`（`RegisterHotKey`）。只提供绑定接口，不替你决定用哪种。

```cpp
CHotKeyBinder binder;
binder.SetHandler([](int id, WORD vk, WORD mod, int scope, LPVOID) {
    // 触发
}, NULL);
binder.Attach(&m_pm); // 程序快捷键需要

// 程序内
binder.Bind(m_hWnd, 1, 'S', HOTKEYF_CONTROL, HOTKEYBOX_SCOPE_APP);
// 全局（失败常见原因：被系统或其他程序占用）
binder.Bind(m_hWnd, 2, 'S', HOTKEYF_CONTROL | HOTKEYF_ALT, HOTKEYBOX_SCOPE_GLOBAL);
// 或从 Button 已存键+scope
binder.BindButton(pBtn, 3);

binder.Unbind(1);
binder.UnbindAll();
binder.Detach(); // 析构也会 Detach
```

| 方法 | 说明 |
|------|------|
| `Attach` / `Detach` | 挂到 `CPaintManagerUI`（APP 键走 PreMessageFilter；GLOBAL 子类化收 `WM_HOTKEY`） |
| `SetHandler` | 触发回调 `(id, vk, mod, scope, user)` |
| `SetRequireModifier` / `IsRequireModifier` | 默认 `true`：拒绝字母/数字裸键；功能键不受限 |
| `IsLetterOrDigitKey` / `IsBareLetterOrDigit` | 判定字母数字 / 裸字母数字 |
| `Bind(hWnd, id, vk, mod, scope)` | 绑定；同 id 覆盖；同组合键已存在则失败 |
| `BindButton(btn, id)` | 读 `GetShortcutKey` + scope |
| `Unbind` / `UnbindAll` / `IsBound` / `Find` | 管理查询 |

`HotKeyToRegisterMods` 仍可供自行 `RegisterHotKey`；一般优先用 Binder。