# List / ListHeader / ListElement

| | |
|--|--|
| 类 | `CListUI 及相关` |
| XML | `<List>` `<ListHeader>` `<ListHeaderItem>` `<ListLabelElement>` `<ListContainerElement>` 等 |
| 源码 | `src/DuiLib/Control/UIList.*`、`UIListEx.*` |
| 继承属性 | 见 [Container.md](Container.md) |

> 本页聚焦 **属性与 HTML/CSS 的符合度**。盒模型全局约定见 [Attributes.md](Attributes.md)。

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| `selected` | — |
| `text-align` | — |
| `font-family` | — |
| `font-size` | — |
| `color` | — |

### 部分接近（命名或伪类形式）

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `item-foreground-image-hover` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-color-hover` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-background-color-hover` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-image-hover` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-color-disabled` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-background-color-disabled` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-image-disabled` | — | 无标准等价（控件皮肤/列表项专用） |
| `image-hover` | — | :hover { background-image } |
| `image-active` | — | :active { background-image } |
| `checkbox-image-hover` | — | 无标准等价（控件皮肤/列表项专用） |
| `checkbox-image-active` | — | 无标准等价（控件皮肤/列表项专用） |
| `checkbox-image-disabled` | — | 无标准等价（控件皮肤/列表项专用） |

### 非标准 / 无 HTML 等价

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `header` | — | 无（列表头开关） |
| `header-background-image` | — | 无标准等价（控件皮肤/列表项专用） |
| `scroll-select` | — | 无 |
| `multi-expanding` | — | 无 |
| `item-font-family` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-font-size` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-font-weight` | `bold` / `700` / `true` | font-weight（项字体） |
| `item-text-align` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-vertical-align` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-text-overflow` | 项文字省略 | `text-overflow` |
| `item-padding` | 列表项内边距（CSS `top,right,bottom,left`） | padding |
| `item-color` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-background-color` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-background-image` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-alternate-background` | — | nth-child 斑马纹 |
| `item-foreground-image` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-foreground-image-selected` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-color-selected` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-background-color-selected` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-image-selected` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-line-color` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-show-row-line` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-show-column-line` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-show-html` | — | 无 |
| `multi-select` | — | multiple（select/list） |
| `item-right-selected` | — | 无标准等价（控件皮肤/列表项专用） |
| `draggable` | — | 无 |
| `sep-width` | — | resize handle（无标准） |
| `text-overflow` | 省略号 | `text-overflow: ellipsis` |
| `showhtml` | 迷你 HTML 标签绘制，非浏览器引擎 | innerHTML（语义不同，迷你标签） |
| `image` | 状态皮肤图（DuiLib file='…' 串） | background-image / <img> |
| `image-focus` | — | :focus { background-image } |
| `sep-image` | — | background-image / <img>（取值多为 DuiLib 图串） |
| `scale` | — | 无（列宽比例） |
| `scale-header` | — | 无 |
| `editable` | — | 无标准等价 |
| `comboable` | — | 无标准等价 |
| `checkable` | — | 无标准等价 |
| `checkbox-width` | — | 无标准等价（控件皮肤/列表项专用） |
| `checkbox-height` | — | 无标准等价（控件皮肤/列表项专用） |
| `checkbox-image` | — | 无标准等价（控件皮肤/列表项专用） |
| `checkbox-image-focus` | — | 无标准等价（控件皮肤/列表项专用） |
| `checkbox-image-selected` | — | 无标准等价（控件皮肤/列表项专用） |
| `checkbox-foreground-image` | — | 无标准等价（控件皮肤/列表项专用） |

列表项/表头大量 `item*`、`checkbox*`、`sep*` 均为 **DuiLib 皮肤 DSL**，无 HTML `<table>`/`<ul>` 属性对应。选中态用 `selected` / `item-*-selected`，不是 CSS `:checked`。
