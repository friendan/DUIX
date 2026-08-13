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
| `item-padding` | 菜单项文字区（同 List）；默认约 `0,14,0,32`（左为图标槽）。**二级菜单**会继承一级 `Menu` 的该值 | padding |
| `icon` / `icon-src` | SVG 文件或 PNG/BMP/JPG 路径 | 无 |
| `lucide` / `tabler-outline` / `bsicon` 等 | SVG 图标库（与 Button / ListLabel 同名） | 无 |
| `icon-size` | `16` 或 `16,16` | 无 |
| `icon-tint` / `icon-color` | **SVG**：未设则跟 `item-color*`。**光栅**：默认原图；`#色` 强制；`auto`/`true` 跟文字色；`none`/`original` 原图 | 无 |
| `check-item` | — | 无 |
| `checked` | — | checked |
| `line-type` | — | 无 |
| `expand` | 显示下级箭头；Default 名 `ExpandIcon` | 无 |
| `line-color` | — | border-color |
| `line-padding` | CSS `top,right,bottom,left` | padding |

菜单项属性全部非标准（接近桌面菜单模型，非 HTML `<menu>`）。

### 主题

- 纯色弹出菜单：`CMenuWnd` 在 `ResizeMenu` / `ResizeSubMenu` 时自动调用 `CThemeManager::ApplyMenuChrome`（list chrome + 分隔线 `line-color`）；热切主题时已打开菜单也会刷新。
- 退出自动套色：根 `Menu` 上 `theme="none"`，或使用 `background-image` 图片壳（九宫等不会自动跟色，需自备多套皮或改纯色）。
