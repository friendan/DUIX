# ScrollBar

| | |
|--|--|
| 类 | `CScrollBarUI` |
| XML | `<ScrollBar>`（通常由 Container 的 `v-scrollbar` / `h-scrollbar` 创建） |
| 源码 | `src/DuiLib/Control/UIScrollBar.*` |
| 继承属性 | 见 [Control.md](Control.md) |

> 本页聚焦 **属性与 HTML/CSS 的符合度**。盒模型全局约定见 [Attributes.md](Attributes.md)。

### 与 `html { action: title }`

`CScrollBarUI::PreferClientHit()` 恒为可交互，命中滑块/轨道时保持 `HTCLIENT`，不继承窗口级标题拖拽。  
List / TreeView / VBox / HBox / Transfer / RichEdit / Combo 下拉等凡走 `CContainerUI` 滚动条的，都受益于此。  
`List` / `ListBody` 另有 `PreferClientHit`，且 `List::FindControl` 优先命中滚动条（避免 Empty 绝对层盖住）；`ListBody` / `VirtualList` 在 `DoEvent` 里把点在滚动条上的按下转给 `ScrollBar`。

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| `background-image` | 位图 DSL 或 linear-gradient |
| `value` | — |

### 部分接近（命名或伪类形式）

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `button-prev-image-hover` | — | 无标准等价（控件皮肤/列表项专用） |
| `button-prev-image-active` | — | 无标准等价（控件皮肤/列表项专用） |
| `button-prev-image-disabled` | — | 无标准等价（控件皮肤/列表项专用） |
| `button-next-image-hover` | — | 无标准等价（控件皮肤/列表项专用） |
| `button-next-image-active` | — | 无标准等价（控件皮肤/列表项专用） |
| `button-next-image-disabled` | — | 无标准等价（控件皮肤/列表项专用） |
| `thumb-image-hover` | — | 无标准等价（控件皮肤/列表项专用） |
| `thumb-image-active` | — | 无标准等价（控件皮肤/列表项专用） |
| `thumb-image-disabled` | — | 无标准等价（控件皮肤/列表项专用） |
| `rail-image-hover` | — | 无标准等价（控件皮肤/列表项专用） |
| `rail-image-active` | — | 无标准等价（控件皮肤/列表项专用） |
| `rail-image-disabled` | — | 无标准等价（控件皮肤/列表项专用） |
| `background-image-hover` | — | background-image / <img>（取值多为 DuiLib 图串） |
| `background-image-active` | — | background-image / <img>（取值多为 DuiLib 图串） |
| `background-image-disabled` | — | background-image / <img>（取值多为 DuiLib 图串） |

### 非标准 / 无 HTML 等价

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `button-prev-image` | 上/左端箭头图 | 无标准等价（控件皮肤/列表项专用） |
| `button-next-image` | 下/右端箭头图 | 无标准等价（控件皮肤/列表项专用） |
| `thumb-image` | — | 无标准等价（控件皮肤/列表项专用） |
| `rail-image` | — | 无标准等价（控件皮肤/列表项专用） |
| `horizontal` | — | 无（水平/垂直方向） |
| `line-size` | — | 无 |
| `range` | — | 无（用 min/max） |
| `show-button-prev` | 是否显示上/左端箭头 | 无 |
| `show-button-next` | 是否显示下/右端箭头 | 无 |
| `thumb-min-size` | — | 无 |

部件级 `*-image*` 全部非标准。CSS 仅有极有限的 `scrollbar-color` / `scrollbar-width`。
