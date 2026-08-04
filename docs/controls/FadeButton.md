# FadeButton

| | |
|--|--|
| 类 | `CFadeButtonUI` |
| XML | `<FadeButton>` |
| 源码 | `src/DuiLib/Control/UIFadeButton.*` |
| 继承属性 | 见 [Button.md](Button.md) |

带淡入淡出的按钮：鼠标进入 / 离开时在 `image` 与 `image-hover` 之间做 alpha 交叉（约 30 帧 × 10ms）。按下 / 禁用 / 焦点仍优先用 Button 对应状态图。

### 最小示例

```xml
<FadeButton width="80" height="32"
    image="btn_normal.png"
    image-hover="btn_hot.png"
    image-active="btn_pushed.png"
    image-disabled="btn_disabled.png" />
```

### 属性

无额外 XML 属性；皮肤图与通知均同 Button（`image` / `image-hover` / `image-active` / `image-disabled` / `image-focus`、`click` 等）。

### 动画（实现常量）

| 常量 | 值 | 说明 |
|------|-----|------|
| `FADE_ELLAPSE` | 10ms | 帧间隔 |
| `FADE_FRAME_COUNT` | 30 | 总帧数 |
| `FADE_IN_ID` / `FADE_OUT_ID` | 8 / 9 | 进入 / 离开动画 ID |

纯色 `kind=` 按钮无状态图时，淡入淡出不会生效（`PaintStatusImage` 只在有 `image` 时走交叉逻辑）。需要渐变悬停请配齐 `image` + `image-hover`。
