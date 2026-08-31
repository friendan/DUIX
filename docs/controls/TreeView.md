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

### TreeNode 图标（对齐 Button / ListLabel）

节点文字在内部 `Option`（`CButtonUI`）上；下列属性直接转发，语义同 [Button.md](Button.md)。

| 属性 | 说明 |
|------|------|
| `bsicon` / `lucide` / `tabler-outline` / `tabler-filled` / `iconpark` / `remixicon` / `twicon` | SVG 图标库 |
| `icon` / `icon-src` | SVG 文件或 PNG/BMP/JPG |
| `icon-size` / `icon-gap` / `icon-position` | 尺寸、间距、方位（`left`/`right`/`top`/`bottom`） |
| `icon-tint` / `icon-color` | **SVG**：未设则跟节点 `item-color*`（悬停/选中时 TreeNode 会 `SetColor`）。**光栅**：默认原图；`#色` 强制；`auto` 跟文字色；`none`/`original` 原图 |

建议节点 `height` ≥ `icon-size`（默认行高 18 偏紧，demo 用 28）。

内部标签是 `Option`：库已设 `kind=none`、`padding=0`、`text-align=left`，并在 HBox 中撑满剩余宽度；勿再给节点 `itemattr` 加回大左右 padding，否则窄内容区会裁切图标。

```xml
<TreeView header="hidden" height="240" overflow="scroll"
    item-color="#333333FF" item-color-hover="#1677FFFF" item-color-selected="#0958D9FF">
  <TreeNode text="文档" lucide="folder" icon-size="16" height="28">
    <TreeNode text="说明.md" lucide="file-text" icon-size="16" height="28" />
    <TreeNode text="图标.png" icon="menu/icon.png" icon-tint="auto" icon-size="16" height="28" />
  </TreeNode>
  <TreeNode text="设置" lucide="settings" icon-size="16" height="28" />
</TreeView>
```

C++：`SetIconLib` / `SetIconSrc` / `ClearIcon` / `SetIconSize` / `SetIconTint` / `SetIconTintAuto`；或 `GetItemButton()->…`。

亦可用旧写法 `itemattr="lucide='home' icon-size='16'"`。

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
