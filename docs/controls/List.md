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
| `item-alternate-background-color` | 奇数行底色；设置非 0 色时自动开启斑马纹（Theme 会写入） | nth-child |

### ListLabelElement 图标（对齐 Button 子集）

| 属性 | 说明 |
|------|------|
| `bsicon` / `lucide` / `tabler-outline` / `tabler-filled` / `iconpark` / `remixicon` / `twicon` | SVG 图标库 |
| `icon` / `icon-src` | SVG 文件或 PNG/BMP/JPG |
| `icon-size` / `icon-gap` | 图标边长、与文字间距（逻辑像素） |
| `icon-position` / `icon-pos` | `left`（默认）/ `right` / `top` / `bottom`；上下排布时文字水平居中，未写死 `height` 时 `EstimateSize` 会计入图标高度 |
| `icon-tint` / `icon-color` | **SVG**：未设则跟 `item-color*`（悬停/选中/禁用）。**光栅**：默认原图；`#色` 强制；`auto` 跟文字色；`none`/`original` 原图。亦用于 [Combo](Combo.md) 下拉项；闭合态会复用选中项图标 |
| `icon-tint-hover` / `-selected`（`-active` 同 selected）/ `-disabled` | 状态覆盖 |

```xml
<ListLabelElement text="主页" lucide="home" icon-size="16" />
<ListLabelElement text="上传" lucide="upload" icon-position="top" height="56" />
<ListLabelElement text="PNG" icon="menu/icon.png" icon-tint="auto" />
```

CSS 伪类（解析期改写到 `item-*-hover` / `item-*-selected` 等）：

```css
#list_css_demo:hover { item-background-color: #E6F4FFFF; item-color: #1677FFFF; }
#list_css_demo:checked { item-background-color: #BAE0FFFF; item-color: #0958D9FF; }
```

### 空态（Empty）

0 项时在列表体上盖一层 [Empty](Empty.md)（不计入 `GetCount`，不进行数据）。

| 属性 / 写法 | 说明 |
|-------------|------|
| 嵌套 `<Empty>` | `List::Add` 识别后挂为绝对定位覆盖层 |
| `empty-text` / `empty-description` | 懒建 Empty，并设 `description` |
| `empty-image` | 懒建 Empty，并设自定义图 |

```xml
<List empty-text="暂无数据" header="hidden" height="160" />

<List header="hidden" height="160">
  <Empty description="还没有项">
    <Button text="添加" kind="primary" width="80" height="28" />
  </Empty>
</List>
```

### 其它非标准 / 无 HTML 等价（续）

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
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
