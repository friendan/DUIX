# ColorPalette

| | |
|--|--|
| 类 | `CColorPaletteUI` |
| XML | `<ColorPalette>` |
| 源码 | `src/DuiLib/Control/UIColorPalette.*` |
| 继承属性 | 见 [Control.md](Control.md) |

HSB 调色板：上方色相/饱和度面板 + 下方亮度条；拖动选色。取色结果为 DuiLib `DWORD` 颜色（可直接作背景色）。

### 最小示例

```xml
<ColorPalette name="palette" width="200" height="220"
    palette-height="180" bar-height="16"
    thumb-image="cur.png" />
```

```cpp
CColorPaletteUI* p = static_cast<CColorPaletteUI*>(
    m_pm.FindControl(_T("palette"))->GetInterface(DUI_CTR_COLORPALETTE));
DWORD c = p->GetSelectColor();
p->SetSelectColor(0x1677FFFF);
```

### 属性

| 属性 | 说明 |
|------|------|
| `palette-height` | 上方调色区高度 |
| `bar-height` | 下方亮度条高度（实现限制最大约 150，过大可能越界） |
| `thumb-image` | 选中点图标 |

### C++ API

| 方法 | 说明 |
|------|------|
| `GetSelectColor` / `SetSelectColor` | 当前色（DuiLib `DWORD`） |
| `SetPalletHeight` / `SetBarHeight` / `SetThumbImage` | 同属性 |

### 通知

| 类型 | 说明 |
|------|------|
| `colorchanged`（`DUI_MSGTYPE_COLORCHANGED`） | 用户选色变化；`wParam` 为颜色值 |

绘制走内存 DC 位图；与 D2D 主路径混用时注意脏区刷新（冒烟见 `AGENTS.md` ColorPalette 项）。
