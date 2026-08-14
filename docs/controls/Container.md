# Container / 布局

| | |
|--|--|
| 类 | `CContainerUI 及布局子类` |
| XML | `<Container>` `<HBox>`/`<HorizontalLayout>` `<VBox>`/`<VerticalLayout>` `<FlowLayout>` `<TileLayout>` `<TabLayout>` `<ChildLayout>` `<LinearLayout>` |
| 源码 | `src/DuiLib/Core/UIContainer.*`、`src/DuiLib/Layout/*` |
| 继承属性 | 见 [Control.md](Control.md) |

> 本页聚焦 **属性与 HTML/CSS 的符合度**。盒模型全局约定见 [Attributes.md](Attributes.md)。

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| `padding` | 内边距；**CSS** / `CDuiBox`（见 [Attributes.md](Attributes.md)） |
| `gap` | 子控件间距（近似 flex/grid gap） |
| `justify-content` | — |
| `align-items` | — |
| `overflow` / `overflow-x` / `overflow-y` | 映射到滚动条：`auto`/`scroll` 启用，`hidden`/`clip`/`visible` 关闭；简写 2 值为 x y |
| `pointer-events` | `none`→自身与子均不点；`auto` 恢复（旧名 `mouse` / `mouse-child`） |
| `flex-wrap` / `wrap` | FlowLayout 换行（`nowrap`/`wrap`） |
| `row-gap` / `line-spacing` | FlowLayout 行间距 |

### 部分接近（命名或伪类形式）

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `align` | FlowLayout 上作 `justify-content` 别名 | justify-content |
| `v-scrollbar` / `h-scrollbar` | 布尔开关；优先写 `overflow-*` | overflow-y / overflow-x |

### 非标准 / 无 HTML 等价

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `mouse-child` | 布尔；优先容器上写 `pointer-events` | pointer-events on children |
| `v-scrollbar-style` | — | scrollbar-*（极有限） |
| `h-scrollbar-style` | — | scrollbar-* |
| `scroll-step-size` | — | 无 |
| `fixed-scrollbar` | — | 无 |
| `show-scrollbar` | — | 无 |
| `sep-width` | — | resize handle（无标准） |
| `sep-height` | — | 同上 |
| `sep-size` | — | 同上 |
| `sep-imm` | — | 同上 |
| `line-spacing` | 优先 `row-gap` | row-gap |
| `item-size` | — | grid 单元格尺寸 |
| `columns` | — | grid-template-columns |
| `selected-id` | — | 无（Tab 选中索引） |
| `xml-file` | — | 无（子皮肤） |
| `animation-direction` | — | 无 |

### 布局专有

| 控件 | 非标准属性 |
|------|-----------|
| HBox / VBox | `sep-width` / `sep-height`（分隔条拖拽） |
| LinearLayout | `sep-size`、`sep-imm` |
| FlowLayout | `wrap`、`line-spacing`；`align` 仍作 `justify-content` 别名 |
| TileLayout | `item-size`、`columns` |
| TabLayout | `selected-id`；动态 `Add`/`SelectItem` 会恢复子项 `InternVisible`（见 [TabBar.md](TabBar.md)）。常见：页上放 `WebBrowser` 时在 **TabLayout**（或 root）写 `window-resize`，窗口 `size-box` 置 `0`，铺满仍能缩边（见 [Control.md](Control.md)、[WebBrowser.md](WebBrowser.md#原生-hwnd-与-window-resize)） |
| ChildLayout | `xml-file` |
| AnimationTabLayout | `animation-direction` |

### 动态 Add

运行时 `new` 子控件：先 `SetAttribute`（含 `theme` / `var(--token)`），再 `Add` / `AddAt`。详见 [Theme.md · 动态创建](Theme.md#动态创建)。
