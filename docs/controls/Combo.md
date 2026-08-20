# Combo / ComboBox

| | |
|--|--|
| 类 | `CComboUI、CComboBoxUI` |
| XML | `<Combo>` `<ComboBox>` |
| 源码 | `src/DuiLib/Control/UICombo.*`、`UIComboBox.*` |
| 继承属性 | 见 [Container.md](Container.md) |

> 本页聚焦 **属性与 HTML/CSS 的符合度**。盒模型全局约定见 [Attributes.md](Attributes.md)。
>
> 上万条、多列、按字段过滤请用 [LookupEdit](LookupEdit.md)，不要往 Combo 里塞虚拟行。

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| `text-align` | — |
| `vertical-align` | — |
| `font-family` | — |
| `font-size` | — |
| `color` | — |

### 部分接近（命名或伪类形式）

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `color-disabled` | — | 无标准等价 |
| `image-hover` | — | :hover { background-image } |
| `image-active` | — | :active { background-image } |
| `image-disabled` | — | :disabled { background-image } |
| `item-color-hover` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-background-color-hover` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-image-hover` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-color-disabled` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-background-color-disabled` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-image-disabled` | — | 无标准等价（控件皮肤/列表项专用） |

### 非标准 / 无 HTML 等价

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `text-overflow` | 省略号 | `text-overflow: ellipsis` |
| `word-break` | 换行 | `word-break` |
| `showhtml` | 迷你 HTML 标签绘制，非浏览器引擎 | innerHTML（语义不同，迷你标签） |
| `drop-shadow` | 下拉阴影（非窗口 `showshadow`） | box-shadow |
| `image` | 状态皮肤图（DuiLib file='…' 串） | background-image / <img> |
| `image-focus` | — | :focus { background-image } |
| `scroll-select` | — | 无 |
| `drop-box` | 下拉面板属性串 | 无 |
| `drop-box-size` | — | 无 |
| `drop-box-padding` | 下拉面板内边距（CSS 顺序） | padding |
| `item-font-family` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-font-size` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-font-weight` | `bold` / `700` / `true` | font-weight（项字体） |
| `item-text-align` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-vertical-align` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-text-overflow` | 项省略 | `text-overflow` |
| `item-padding` | 项内边距（CSS `top,right,bottom,left`） | padding |
| `item-color` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-background-color` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-background-image` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-alternate-background` | — | nth-child 斑马纹 |
| `item-alternate-background-color` | 奇数行底色；非 0 时自动开启 | nth-child |
| `item-color-selected` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-background-color-selected` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-image-selected` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-line-color` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-show-html` | — | 无 |
| `arrow-image` | — | 无 |

> **默认边距**：闭合态选中文字与下拉项默认左右各 `6px`（对应 `padding`/`item-padding` 的 `0,6,0,6`），开箱即用不贴边；需要更贴或更大间距再覆盖 `padding` / `item-padding`。

### 下拉项图标（ListLabelElement）

子项用 `<ListLabelElement>` 时，图标属性与 [List.md](List.md) 相同（`lucide` / `icon` / `icon-tint` 等）。  
**下拉列表**与**闭合态选中项**都会画图标（`CComboUI::PaintText` → `PaintIconAndText`）。

```xml
<Combo width="220" height="32" drop-box-size="0,180"
    padding="0,28,0,8" border="1px solid" border-radius="4"
    item-padding="0,10,0,10">
  <ListLabelElement text="主页" lucide="home" icon-size="16" height="28" selected="true" />
  <ListLabelElement text="PNG" icon="menu/icon.png" icon-tint="auto" icon-size="16" height="28" />
</Combo>
```

（`item-color*` / 壳底色可由主题 `chrome` 写入，勿写死浅色 hex。）

`ComboBox` 额外仅 `arrow-image`。下拉皮肤 `drop-box*` 无 HTML `<select>` 标准属性。
