# RichEdit

| | |
|--|--|
| 类 | `CRichEditUI` |
| XML | `<RichEdit>` |
| 源码 | `src/DuiLib/Control/UIRichEdit.*` |
| 继承属性 | 见 [Control.md](Control.md) / Container |

> 本页聚焦 **属性与 HTML/CSS 的符合度**。盒模型全局约定见 [Attributes.md](Attributes.md)。

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| `multiline` / `readonly` / `password` | 编辑模式 |
| `maxlength` | 最大字符数 |
| `placeholder` / `placeholder-color` | 占位；`theme="chrome"` 时 `placeholder-color` 跟 `color-text-secondary` |
| `text-align` / `color` / `font-family` / `font-size` | 文字样式 |
| `overflow` / `overflow-x` / `overflow-y` | 映射启用滚动；优先于单独写 `v-scrollbar` |
| `v-scrollbar` / `h-scrollbar` | 布尔开关（兼容） |
| `auto-vscroll` / `auto-hscroll` | 随输入滚动 |
| `want-tab` / `want-return` / `want-ctrl-return` | 按键消费 |

### 非标准

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `transparent` / `rich` | 透明 / 富文本模式 | 部分接近 |
| `placeholder-align` | 占位对齐 | 无 |

文字缩进用 `padding`。

