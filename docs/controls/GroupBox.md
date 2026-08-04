# GroupBox

| | |
|--|--|
| 类 | `CGroupBoxUI` |
| XML | `<GroupBox>` |
| 源码 | `src/DuiLib/Control/UIGroupBox.*` |
| 继承属性 | 见 [Container.md](Container.md)（基类为 `CVerticalLayoutUI`） |

带标题的分组框：边框内缩约 5px，标题画在上边框缺口处；默认 `padding` 为 CSS `25,20,20,20`，子控件纵向堆叠。

### 最小示例

```xml
<GroupBox text="网络" height="120" border="1px solid #D9D9D9"
    color="#333333FF" font-size="12">
  <HBox height="28" gap="8" align-items="vcenter">
    <Label text="地址" width="48" />
    <Edit name="addr" />
  </HBox>
</GroupBox>
```

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| `text` | 分组标题（≈ `<fieldset><legend>`） |
| `color` | 标题文字色 |
| `color-disabled` | 禁用时标题色 |
| `font-family` / `font-size` | 标题字体（经 `EnsureFont`） |
| `border` / `border-width` / `border-color` / `border-radius` | 边框（继承） |
| `padding` / `gap` / 布局属性 | 子项布局（VBox） |

### 说明

- 标题绘制在 `PaintBorder` 末尾，会盖住上边框一段。
- 焦点边框色仍可用基类 `border-color-focus`（有焦点且非 0 时）。
- 不是 HTML `<fieldset>` 语义容器，只是视觉分组 + VBox。
