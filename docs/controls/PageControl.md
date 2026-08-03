# PageControl

| | |
|--|--|
| 类 | `CPageControlUI` |
| XML | `<PageControl>` |
| 源码 | `src/DuiLib/Control/UIPageControl.*` |
| 继承属性 | 见 [Container.md](Container.md)（水平布局） |

> 本页聚焦 **属性与 HTML/CSS 的符合度**。盒模型全局约定见 [Attributes.md](Attributes.md)。

分页条：页码 Option、跳转 Edit、可选「…」省略。无 HTML `<nav>`/`pagination` 标准属性对等。

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| `font-family` / `font-size` | 页码文字字体 |

### 非标准 / 无 HTML 等价

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `max-page` | `总页数,可见页码数`，如 `20,6` | 无 |
| `page-color` | 页码文字色 | color |
| `page-color-hover` | 悬停文字色 | :hover { color } |
| `page-color-selected` | 选中文字色 | 无标准等价 |
| `page-background-color` | 页按钮背景 | background-color |
| `page-background-color-hover` | 悬停背景 | :hover |
| `page-background-color-selected` | 选中背景 | 无标准等价 |
| `goto-edit-border-color` | 跳转框边框色 | 无 |
| `goto-edit-border-width` | 跳转框边框宽 | 无 |
| `edit-maxlength` | 跳转框最大字符数 | maxlength |

通知：`DUI_MSGTYPE_PAGECHANED`（拼写沿用历史宏名）。
