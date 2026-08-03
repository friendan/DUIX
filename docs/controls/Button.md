# Button

| | |
|--|--|
| 类 | `CButtonUI` |
| XML | `<Button>` |
| 源码 | `src/DuiLib/Control/UIButton.*` |
| 继承属性 | 见 [Label.md](Label.md) |

> 本页聚焦 **属性与 HTML/CSS 的符合度**。盒模型全局约定见 [Attributes.md](Attributes.md)。

### 部分接近（命名或伪类形式）

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `image-hover` | — | :hover { background-image } |
| `image-active` | — | :active { background-image } |
| `image-disabled` | — | :disabled { background-image } |
| `foreground-image-hover` | — | background-image / <img>（取值多为 DuiLib 图串） |
| `foreground-image-active` | — | background-image / <img>（取值多为 DuiLib 图串） |

### 非标准 / 无 HTML 等价

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `image` | 状态皮肤图（DuiLib file='…' 串） | background-image / <img> |
| `image-focus` | — | :focus { background-image } |
| `state-image` | — | 无 |
| `state-count` | — | 无 |
| `bind-tab-index` | — | 无 |
| `bind-tab-layout-name` | — | 无 |
| `color-focus` | 焦点文字色 | `:focus { color }` |

继承 Label（含 `color-hover` / `color-active`）与 Control（含 `background-color-*` / `border-color-*`）；Button 不再单独存一份状态背景/边框色。
