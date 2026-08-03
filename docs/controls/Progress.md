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
| `step` | — |

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
