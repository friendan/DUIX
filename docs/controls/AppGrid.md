# AppGrid

| | |
|--|--|
| 类 | `CAppGridUI` |
| XML | `<AppGrid>` / `appgrid` |
| 源码 | `src/DuiLib/Control/UIAppGrid.*` |
| 继承 | [Container](Container.md) → [Control](Control.md) |

自适应 **列 × 行** 的应用图标网格：按窗口客户区计算每页容量，底部分页圆点翻页，支持拖拽换位。子节点通常为 [AppIcon](AppIcon.md)，也可以是任意控件。

v1 **不做**：文件夹、长按菜单。分页模式：圆点 / 滚轮 / **拖拽时悬停圆点或方向键**跨页；`scroll` 模式：列表滚动。

### 布局模式

| 模式 | 属性 | 行为 |
|------|------|------|
| 分页（默认） | `scroll=false` | 按容量切页；可选底部分页圆点；滚轮翻页 |
| 连续滚动 | `scroll=true` | 全部可见项网格铺开 + 竖滚动条；无圆点、无按页切片 |

`scroll=true` 时自动启用竖滚动条；`EnsureItemVisible` 改为滚到该项。

### 最小示例

```xml
<AppGrid name="app_grid" item-size="72,88" gap="8" height="360"
  dot-size-min="6" dot-size-max="14" bkcolor="#F5F5F5FF">
  <AppIcon text="邮件" lucide="mail" />
  <AppIcon text="消息" lucide="message-circle" badge-count="3" />
  <AppIcon text="设置" iconpark="setting" />
</AppGrid>
```

### 布局

1. 内容区 = `pos` − `padding` −（多页时）底部分页条高度  
2. `cols = max(1, (contentW + gap) / (itemW + gap))`，行同理  
3. `perPage = cols × rows`；仅布局当前页**可见**子项，其它页 / 未通过过滤的项 `SetVisible(false)`  
4. 窗口 `SetPos` 变大/变小时自动重算；当前页越界则钳到最后一页  

`item-size` / `gap` / 圆点尺寸经 DPI Scale。

> **注意**：`EstimateSize` 故意**不**按子项取最大宽高（否则父布局会把网格压成单个 AppIcon 大小）。未写死宽高时由父容器撑满。

### 显示过滤（搜索）

不删子项，只影响分页布局：

| API | 说明 |
|-----|------|
| `filter` / `filter-text` / `SetFilterText` | 不区分大小写，匹配子项 `text` 或 `name` 子串；空串=全部显示 |
| `GetFilterText` / `ClearFilter` | 取当前文本；`ClearFilter` 同时清文本与自定义回调 |
| `SetItemFilter(fn, user)` / `ClearItemFilter` | 自定义谓词；与文本过滤同时生效时为 **AND** |
| `PassesFilter(p)` | 单项是否参与布局 |
| `GetVisibleItemCount` / `GetVisibleItemAt` / `GetVisibleIndexOf` | 过滤后的可见序列 |

搜索框示例：`textchanged` 时 `pGrid->SetFilterText(pEdit->GetText())`，监听 `filterchanged` 刷新「可见 N/M」。过滤变更会回到第 0 页。`GetGridItem*` 仍是完整顺序（持久化用）；`EnsureItemVisible` 在项被滤掉时返回 `false`。

### 属性 / API

| 属性 / API | 说明 | 默认 |
|------------|------|------|
| `item-size` | 格子逻辑像素 `w,h` | `72,88` |
| `gap` | 格子间距（Container） | `8`（本控件 ctor） |
| `page` / `SetPage` / `GetPage` | 当前页，0-based | `0` |
| `GetPageCount` | 总页数（按可见项） | |
| `NextPage` / `PrevPage` | 翻页（内部 `SetPage`） | |
| `EnsureItemVisible(i)` / `SetPageByItem(i)` | 翻到含该网格下标的页（项须可见） | |
| `GetColumns` / `GetRows` / `GetPerPage` | 当前布局容量 | |
| `GetGridItemCount` | 网格项数量（跳过 absolute，不过滤） | |
| `GetGridItemAt(i)` / `GetGridIndexOf(p)` | 按下标取项 / 取下标 | |
| `GetVisibleItemCount` / `GetVisibleItemAt` / `GetVisibleIndexOf` | 过滤后可见项 | |
| `SetFilterText` / `filter` | 显示过滤文本 | 空 |
| `SetItemFilter` | 自定义显示过滤回调 | 无 |
| `SwapItems(from,to)` | 互换两格；成功发 `itemmoved` | |
| `MoveItem(from,to)` | 将 from **插入**到下标 to；发 `itemmoved` | |
| `RemoveGridItemAt(i)` | 按网格下标删除 | |
| `HitTestItemIndex(pt)` | 当前页命中 → 全局网格下标，否则 `-1` | |
| `draggable` | 是否允许拖拽换位 | `true` |
| `scroll` / `SetScrollMode` | 连续滚动列表（关分页圆点与按页切片） | `false` |
| `action` | 默认 `title`：空白区 `IsCaptionDragHit` → 拖窗；图标/分页条保持客户区 | `title` |
| `show-page-dots` | 是否绘制分页圆点；仅一页时或 `scroll` 时不画 | `true` |
| `dot-size-min` / `dot-size-max` | 圆点直径范围（逻辑像素） | `6` / `14` |

圆点直径在 `[dot-size-min, dot-size-max]` 内按底栏可用宽度自动选取。颜色绘制时取有效主题：选中 `color-primary`，未选中 `color-text-secondary`（无则 `color-border`）。

### 交互

| 操作 | 行为 |
|------|------|
| 点击圆点 | 切页 |
| 滚轮 | 分页模式：上下翻页；`scroll`：滚内容 |
| 拖拽子项 | 超过阈值后源格隐藏、**拖影跟手**；与目标格**互换**（拖过不触发 click）；分页模式：悬停圆点切页，或 **←↑→↓ / PageUp·PageDown** 翻页后松手互换 |
| 拖空白区 | 默认 `action=title`：间隙 / 未铺满区域拖移窗口；图标、分页条、滚动条槽除外（`action="none"` 可关） |
| 单击子项 | 子控件 `click` → `itemclick` |
| 双击子项 | 系统 `DBLCLICK` → `itemdbclick`（此前第一次抬起仍会有一次 `itemclick`） |
| 右键子项 | 上抛网格 `itemrclick`（便于弹菜单） |
| 悬停 | 子 AppIcon 原生 hover |

### 通知

| 宏 | 字符串 | 参数 |
|----|--------|------|
| `DUI_MSGTYPE_PAGECHANGED` | `pagechanged` | `wParam`=新页，`lParam`=旧页 |
| `DUI_MSGTYPE_ITEMMOVED` | `itemmoved` | `wParam`=from，`lParam`=to（网格下标；`SwapItems` / `MoveItem` / 拖拽换位） |
| `DUI_MSGTYPE_ITEMCLICK` | `itemclick` | `wParam`=网格下标，`lParam`=`CControlUI*` |
| `DUI_MSGTYPE_ITEMDBCLICK` | `itemdbclick` | `wParam`=网格下标，`lParam`=`CControlUI*` |
| `DUI_MSGTYPE_ITEMRCLICK` | `itemrclick` | `wParam`=网格下标，`lParam`=`CControlUI*` |
| `DUI_MSGTYPE_FILTERCHANGED` | `filterchanged` | `wParam`=可见数，`lParam`=总网格数 |
| `DUI_MSGTYPE_DRAGBEGIN` | `dragbegin` | `wParam`=源下标，`lParam`=`CControlUI*` |
| `DUI_MSGTYPE_DRAGEND` | `dragend` | `wParam`=源下标，`lParam`=目标下标（取消为 `-1`） |

业务侧示例：

```cpp
if( msg.sType == DUI_MSGTYPE_ITEMCLICK && msg.pSender->GetName() == _T("app_grid") ) {
  CControlUI* pItem = (CControlUI*)msg.lParam;
  // 选中 / 预览
}
if( msg.sType == DUI_MSGTYPE_ITEMDBCLICK && msg.pSender->GetName() == _T("app_grid") ) {
  // 打开应用
}
if( msg.sType == DUI_MSGTYPE_ITEMRCLICK && msg.pSender->GetName() == _T("app_grid") ) {
  // 弹出右键菜单；坐标可用 msg.ptMouse / msg.ptScreen
}
// 恢复顺序：按配置名找到项后 MoveItem(GetGridIndexOf(p), targetIndex)
```

持久化顺序示例：拖拽后监听 `itemmoved`，再 `for (i=0; i<GetGridItemCount(); ++i)` 取 `GetGridItemAt(i)->GetName()` 写入配置；启动时按名 `FindSubControl` + `MoveItem` / `SwapItems` 恢复。

### 与 TileLayout / PageControl

| | AppGrid | TileLayout | PageControl |
|--|---------|------------|-------------|
| 分页 | 按容量切片 + 圆点 | 滚动/列数 | 数字页码条 |
| 子项 | 直接挂 AppIcon 等 | 瓦片排版 | 自身是页码控件 |
| 拖拽换位 | 有 | 无 | 无 |

### Demo

`duidemo` → Accordion「AppIcon / AppGrid」：前排为 [AppIcon](AppIcon.md) 功能样例（角标 / 文字图标 / `file=` EXE·WebP 等），其后 100 项翻页；「AppGrid·滚动」`scroll=true` 连续滑。
