# Accordion

| | |
|--|--|
| 类 | `CAccordionUI、CAccordionItemUI` |
| XML | `<Accordion>` `<AccordionItem>` |
| 源码 | `src/DuiLib/Control/UIAccordion.*` |
| 继承属性 | 见 [Container.md](Container.md) |

> 本页聚焦 **属性与 HTML/CSS 的符合度**。盒模型全局约定见 [Attributes.md](Attributes.md)。

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| `title` | — |
| `disabled` | — |

### 部分接近（命名或伪类形式）

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `header-background-color-hover` | — | 无标准等价（控件皮肤/列表项专用） |
| `header-background-color-active-hover` | — | 无标准等价（控件皮肤/列表项专用） |

### 非标准 / 无 HTML 等价

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `mode` | — | 无 |
| `header-height` | — | height |
| `active` | — | 无标准等价 |
| `content-padding` | 内容区内边距；与全局 `padding` 相同，**CSS** `top,right,bottom,left` | padding |
| `header-align` | — | text-align / justify-content |
| `header-color` | — | 无标准等价（控件皮肤/列表项专用） |
| `header-background-color` | — | 无标准等价（控件皮肤/列表项专用） |
| `header-background-color-active` | — | 无标准等价（控件皮肤/列表项专用） |

接近 HTML `<details>`/`<summary>` 的交互模型，但属性名与皮肤字段均为自定义。
