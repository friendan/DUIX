# Progress / Slider

| | |
|--|--|
| 类 | `CProgressUI、CSliderUI` |
| XML | `<Progress>` `<Slider>` |
| 源码 | `src/DuiLib/Control/UIProgress.*`、`UISlider.*` |
| 继承属性 | 见 [Label.md](Label.md) |

> 本页聚焦 **属性与 HTML/CSS 的符合度**。盒模型全局约定见 [Attributes.md](Attributes.md)。

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| `min` | — |
| `max` | — |
| `value` | — |
| `step` / `change-step` | Slider 微调步长（箭头 / 滚轮）；**默认 `1`**；`<=0` 回落为 1 |

### 部分接近（命名或伪类形式）

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `thumb-image-hover` | — | 无标准等价（控件皮肤/列表项专用） |
| `thumb-image-active` | — | 无标准等价（控件皮肤/列表项专用） |

### 非标准 / 无 HTML 等价

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `horizontal` | — | 无（水平/垂直方向） |
| `stretch-foreground` | — | 无 |
| `show-text` | — | 无 |
| `thumb-image` | — | 无标准等价（控件皮肤/列表项专用） |
| `thumb-size` | — | 无 |
| `send-move` | — | 无 |

`min`/`max`/`value`/`step` 接近 HTML `<input type=range>` / `<progress>`；`horizontal`、`thumb*`、`stretch-foreground`、`send-move`、`show-text` 为非标准。

### Slider 交互

- 点击获得焦点（可 Tab 切入）；滚轮按 `step` 增减
- **左右箭头**（水平）/ **上下箭头**（竖直）按 `step` 微调；水平也可用上下，竖直也可用左右
- **Home** / **End** 跳到 `min` / `max`
- 变更发出 `valuechanged`（拖动中若 `send-move=true` 另有 `movevaluechanged`）

```xml
<Slider name="vol" min="0" max="100" value="50" step="5" send-move="true" />
```