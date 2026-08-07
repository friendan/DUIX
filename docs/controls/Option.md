# Option / CheckBox

| | |
|--|--|
| 类 | `COptionUI、CCheckBoxUI` |
| XML | `<Option>` `<CheckBox>` |
| 源码 | `src/DuiLib/Control/UIOption.*` |
| 继承属性 | 见 [Button.md](Button.md) |

> 本页聚焦 **属性与 HTML/CSS 的符合度**。盒模型全局约定见 [Attributes.md](Attributes.md)。

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| `group` | — |
| `selected` / `checked` | 选中（`checked` 为 HTML 别名） |

### 部分接近（命名或伪类形式）

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `image-selected-hover` | — | background-image / <img>（取值多为 DuiLib 图串） |
| `image-selected-active` | — | background-image / <img>（取值多为 DuiLib 图串） |
| `box-background-color-hover` | — | 无标准等价（控件皮肤/列表项专用） |
| `box-border-color-hover` | — | 无标准等价（控件皮肤/列表项专用） |

### 非标准 / 无 HTML 等价

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `group-type` | — | 无 |
| `image-selected` | — | :checked { background-image } |
| `foreground-image-selected` | — | background-image / <img>（取值多为 DuiLib 图串） |
| `selected-state-image` | — | background-image / <img>（取值多为 DuiLib 图串） |
| `selected-state-count` | — | 无标准等价 |
| `background-color-selected` | 选中背景色 | `:checked { background-color }` |
| `color-selected` | 选中文字色 | `:checked { color }` |
| `auto-check` | — | 无 |
| `box-size` | — | 无（checkbox 方框） |
| `box-gap` | — | gap |
| `box-border-width` | — | 无标准等价（控件皮肤/列表项专用） |
| `box-border-radius` | — | 无标准等价（控件皮肤/列表项专用） |
| `box-background-color` / `box-background-color-hover` | — | 无标准等价（控件皮肤/列表项专用） |
| `box-border-color` / `box-border-color-hover` | — | 无标准等价（控件皮肤/列表项专用） |
| `box-background-color-selected` | — | 无标准等价 |
| `box-border-color-selected` | — | 无标准等价 |
| `box-background-color-selected-hover` / `box-border-color-selected-hover` | 选中悬停（主题 `color-primary-active`） | 无 |
| `box-background-color-disabled` / `box-border-color-disabled` | 禁用方框 | 无 |
| `checkmark-color` / `accent-color` | 勾选标记色 | accent-color（近似） |

`CheckBox` 额外属性为方框/勾选绘制专用，全部无 HTML 标准对等物（最接近 `<input type=checkbox>` + `accent-color`）。主题 `chrome` 会写入方框/勾号/禁用色。
