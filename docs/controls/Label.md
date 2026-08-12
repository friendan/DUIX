# Label

| | |
|--|--|
| 类 | `CLabelUI` |
| XML | `<Label>`（`<Text>` 等同继承） |
| 源码 | `src/DuiLib/Control/UILabel.*` |
| 继承属性 | 见 [Control.md](Control.md) |

> 本页聚焦 **属性与 HTML/CSS 的符合度**。盒模型全局约定见 [Attributes.md](Attributes.md)。

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| `text-align` / `vertical-align` | 文字对齐（`middle`/`vcenter` 均可）；旧名 `align` / `valign` 仍可用 |
| `font-family` / `font-size` | 字体 |
| `font-weight` / `font-style` / `text-decoration` | `bold`/`700`；`italic`；`underline` / `line-through` |
| `color` / `color-disabled` | 文字色；支持 `var(--token)` 热切主题 |
| `color-hover` / `color-active` / `color-focus` | 悬停 / 按下 / 焦点文字色（同样支持 `var(--token)`） |
| `width="auto"` / `width="fit-content"` | 基类通用属性；Label 按**文字**自动算宽（改 `text` 会 `NeedParentUpdate`） |
| `height="auto"` / `height="fit-content"` | 基类通用属性；多行/非单行时可按文字算高 |
| `text-overflow="ellipsis"` | 尾部省略（`clip` 关闭） |
| `word-break="break-word"` | 自动换行（`normal` 单行） |
| `white-space` | `nowrap`→单行；`normal`（及 `pre-wrap`/`pre-line`）→换行 |

### 非标准 / 无 HTML 等价

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `no-prefix` | 关闭 `&` 加速键前缀 | Win DrawText 专用 |
| `showhtml` | 迷你 HTML 标签 | 非浏览器引擎 |
| `clickable` | 可点击 | pointer-events / 按钮 |

文字缩进请用基类 `padding`。
