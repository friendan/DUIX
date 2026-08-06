# ColorPalette

| | |
|--|--|
| 类 | `CColorPaletteUI` |
| XML | `<ColorPalette>` |
| 源码 | `src/DuiLib/Control/UIColorPalette.*` |
| 继承属性 | 见 [Control.md](Control.md) |

HSL 调色板：

- **上方色板**：横轴色相 H（0–360），纵轴明度 L（上亮下暗）
- **下方条**：饱和度 S（左低右高）

取色结果为 DuiLib `DWORD`（`RRGGBBAA`，可用 `ParseColorString` / `#RRGGBBAA`）。

### 最小示例

```xml
<ColorPalette name="Pallet" width="200" height="220"
    palette-height="180" bar-height="16"
    thumb-image="cur.png"
    select-color="#1677FFFF" />
```

```cpp
CColorPaletteUI* p = static_cast<CColorPaletteUI*>(
    m_pm.FindControl(_T("Pallet"))->GetInterface(DUI_CTR_COLORPALETTE));
DWORD c = p->GetSelectColor();
p->SetSelectColor(0x1677FFFF);
```

### 属性

| 属性 | 说明 |
|------|------|
| `palette-height` / `palletheight` | 上方色板高度（逻辑像素，按 DPI 缩放） |
| `bar-height` / `barheight` | 下方饱和度条高度（上限约 150） |
| `thumb-image` / `thumbimage` | 选中点图标；未设时画简易方框准星 |
| `select-color` / `color` | 初始选中色 |
| `disabled` | 禁用：灰罩、禁拖、禁键盘 |

历史拼写 `Pallet` 仍作演示控件名；API 另有 `SetPaletteHeight` 别名。

### C++ API

| 方法 | 说明 |
|------|------|
| `GetSelectColor` / `SetSelectColor` | 当前色；`SetSelectColor` 同步拇指并刷新色板/饱和度条 |
| `SetPalletHeight` / `SetPaletteHeight` / `SetBarHeight` / `SetThumbImage` | 同属性 |

内部 H/S/L 刻度：H `0–360`，S/L `0–200`（与经典 DuiLib 调色板一致）。

### 通知

| 类型 | 时机 | 说明 |
|------|------|------|
| `colorchanging`（`DUI_MSGTYPE_COLORCHANGING`） | 按下 / 拖动 | `wParam` 为当前色；适合实时预览 |
| `colorchanged`（`DUI_MSGTYPE_COLORCHANGED`） | 松手 / 键盘微调 | `wParam` 为当前色；适合提交结果 |

### 键盘（获焦后）

| 键 | 作用 |
|----|------|
| `←` / `→` | 色相 H |
| `↑` / `↓` | 明度 L |
| `Shift+←/→` 或 `PageUp/PageDown` | 饱和度 S |
| `Ctrl` 按住 | 步长 ×10 |

点击色板会 `SetFocus`；获焦时画蓝色焦点框。

### 演示

duidemo → **基础** → Accordion **ColorPalette**（`bin/skin/duidemo/main.html`）。
