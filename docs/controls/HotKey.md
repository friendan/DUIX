# HotKey

| | |
|--|--|
| 类 | `CHotKeyUI` |
| XML | `<HotKey>` |
| 源码 | `src/DuiLib/Control/UIHotKey.*` |
| 继承属性 | 见 [Label.md](Label.md) |

快捷键录入框：获得焦点后创建系统 `HOTKEY` 子窗口，用户按下组合键后回写显示文本。可用 C++ `SetHotKey` / `GetHotKey` 读写虚拟键与修饰符。

### 最小示例

```xml
<HotKey name="hk_capture" width="180" height="28"
    border="1px solid #D9D9D9" background-color="#FFFFFFFF"
    native-background-color="#FFFFFFFF" />
```

```cpp
CHotKeyUI* p = static_cast<CHotKeyUI*>(m_pm.FindControl(_T("hk_capture")));
WORD vk = 0, mod = 0;
p->GetHotKey(vk, mod);          // HOTKEYF_CONTROL / HOTKEYF_ALT / HOTKEYF_SHIFT …
p->SetHotKey(VK_F5, HOTKEYF_CONTROL);
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

### C++ API

| 方法 | 说明 |
|------|------|
| `SetHotKey(vk, modifiers)` / `GetHotKey` | 设置 / 读取快捷键 |
| `SetNativeBackgroundColor` | 同 `native-background-color` |

聚焦时弹出原生控件；失焦后销毁子窗口，文字画在 Label 上。
