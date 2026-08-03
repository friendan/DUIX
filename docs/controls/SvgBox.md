# SvgBox

| | |
|--|--|
| 类 | `CSvgBoxUI` |
| XML | `<SvgBox>` |
| 源码 | `src/DuiLib/Control/UISvgBox.*` |
| 继承属性 | 见 [Control.md](Control.md) |

> 本页聚焦 **属性与 HTML/CSS 的符合度**。盒模型全局约定见 [Attributes.md](Attributes.md)。

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| `src` | — |
| `color` | — |

### 部分接近（命名或伪类形式）

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `color-hover` | — | 无标准等价 |
| `color-active` | — | 无标准等价 |
| `color-disabled` | — | 无标准等价 |

### 非标准 / 无 HTML 等价

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `data` | — | 无（内联 SVG） |
| `bsicon` | — | 无（图标库） |
| `iconpark` | — | 无 |
| `lucide` | — | 无 |
| `tabler-filled` | — | 无 |
| `tabler-outline` | — | 无 |
| `remixicon` | — | 无 |
| `twicon` | — | 无 |

`src`/`color` 接近 SVG/CSS；图标库名属性与 `data` 内联均无 HTML 标准。伪类可用 `color-hover` 等或 CSS `:hover { color }` 改写。

### `color` 着色策略

按 SVG 内容自动选择，避免填充图标被强制加 `stroke` 后「变粗」：

| 判定 | 样式 | 典型来源 |
|------|------|----------|
| `fill="#…"` 且无 `currentColor` | 不着色（保留多色） | Twemoji |
| `stroke="currentColor"` 或 `fill="none"`+有 stroke | `fill:none; stroke:#rgb` | Lucide / Tabler Outline / IconPark |
| `fill="currentColor"` 或无 stroke | `fill:#rgb; stroke:none` | Bootstrap / Remix / Tabler Filled |
| 其它 | fill+stroke 同色 | 通用回退 |

`PreferClientHit`：配置了 `color-hover` / `color-active`（或基类热态）时，不继承 `html { action: title }` 的标题拖拽，悬停才能生效。新控件优先用基类 `background-color-hover` 等，不必再改命中测试。
