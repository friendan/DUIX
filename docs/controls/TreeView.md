# TreeView / TreeNode

| | |
|--|--|
| 类 | `CTreeViewUI、CTreeNodeUI` |
| XML | `<TreeView>` `<TreeNode>` |
| 源码 | `src/DuiLib/Control/UITreeView.*` |
| 继承属性 | 见 [List.md](List.md) |

> 本页聚焦 **属性与 HTML/CSS 的符合度**。盒模型全局约定见 [Attributes.md](Attributes.md)。

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| `text` | — |

### 部分接近（命名或伪类形式）

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `item-color-hover` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-color-selected-hover` | — | 无标准等价 |

### 非标准 / 无 HTML 等价

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `visible-folder-btn` | — | 无 |
| `visible-check-btn` | — | 无 |
| `item-min-width` | — | min-width |
| `item-color` | — | 无标准等价（控件皮肤/列表项专用） |
| `item-color-selected` | — | 无标准等价 |
| `horizattr` | — | 无（子控件属性串） |
| `dotlineattr` | — | 无 |
| `folderattr` | — | 无 |
| `checkboxattr` | — | 无 |
| `itemattr` | — | 无 |

`*attr` 系列为传给内部子控件的属性串，无 HTML 等价。
