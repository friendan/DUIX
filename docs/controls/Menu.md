# Menu

| | |
|--|--|
| 类 | `CMenuUI、CMenuElementUI` |
| XML | `<Menu>` `<MenuElement>` |
| 源码 | `src/DuiLib/Control/UIMenu.*` |
| 继承属性 | 见 [List.md](List.md) |

> 本页聚焦 **属性与 HTML/CSS 的符合度**。盒模型全局约定见 [Attributes.md](Attributes.md)。

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| `height` | — |

### 非标准 / 无 HTML 等价

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `icon` | — | 无 |
| `icon-size` | — | 无 |
| `check-item` | — | 无 |
| `checked` | — | checked |
| `line-type` | — | 无 |
| `expand` | 显示下级箭头；Default 名 `ExpandIcon` | 无 |
| `line-color` | — | border-color |
| `line-padding` | CSS `top,right,bottom,left` | padding |

菜单项属性全部非标准（接近桌面菜单模型，非 HTML `<menu>`）。

### 主题

- 纯色菜单（`background-color` / `item-color*`，无 `background-image`）：可跟 `theme` token；duidemo `menu.html` 打开时由演示代码套当前主题色。
- **图片壳菜单**（`background-image` 九宫等）：不会自动跟主题，需自备多套皮或改纯色。
