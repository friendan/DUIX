# Segmented

| | |
|--|--|
| 类 | `CSegmentedUI` / `CSegmentItemUI` |
| XML | `<Segmented>`、`<Segment>` / `<SegmentItem>` |
| 源码 | `src/DuiLib/Control/UISegmented.*` |
| 继承 | Container |

工具栏常见的**互斥分段选择**（日/周/月、列表/卡片）。比一组 `Option`+`group` 更紧凑，视觉上是整条轨道 + 选中滑块。

通知：`selectchanged`（`wParam` = 选中下标）。

### 最小示例

```xml
<!-- 快捷：options -->
<Segmented name="range" options="日|周|月" selected="0" width="180" height="32" />

<!-- 子项：可带 value -->
<Segmented name="view" selected="list" width="200" height="32" block="true">
  <Segment text="列表" value="list" />
  <Segment text="卡片" value="card" />
  <Segment text="表格" value="table" />
</Segmented>
```

```cpp
CSegmentedUI* p = static_cast<CSegmentedUI*>(
    m_pm.FindControl(_T("view"))->GetInterface(DUI_CTR_SEGMENTED));
int i = p->GetSelected();
LPCTSTR v = p->GetSelectedValue();
p->SetSelected(1);
p->SetSelectedValue(_T("card"));
```

### 属性

| 属性 | 说明 | 默认 |
|------|------|------|
| `options` / `items` | `文案\|文案` 或 `文案:value\|…`（无子节点时生效） | 空 |
| `selected` / `current` / `active` | 初始选中：数字下标，或匹配 `value` | `0` |
| `block` | `true` 均分宽度；`false` 按文字自适应 | `true` |
| `item-padding` | 非 block 时文字左右留白（逻辑 px） | `12` |
| `inset` | 轨道内边距（选中块与外框间距） | `2` |
| `track-color` | 轨道底色 | 浅灰透明 |
| `selected-background-color` / `thumb-color` | 选中块背景 | 白 |
| `selected-color` | 选中文字色 | 近黑 |
| `color` | 未选中文字色 | 灰 |
| `hover-color` | 悬停文字色 | 近黑 |
| `border-radius` | 圆角 | `6` |

继承 Container 盒模型：`width` / `height` / `padding` / `margin` / `border` 等。

### 子项 Segment

| 属性 | 说明 |
|------|------|
| `text` / `title` | 显示文案 |
| `value` | 逻辑值；省略则等于 `text` |

### 交互

- 点击分段切换；悬停高亮
- 焦点下 ←→ / ↑↓ / Home / End
- 与 Option 组相同通知名 `selectchanged`，便于工具栏统一处理

### 与 Option / TabBar

| | Option 组 | Segmented | TabBar |
|--|-----------|-----------|--------|
| 场景 | 表单单选 | 工具栏视图/范围 | 多页标签 |
| 关闭/滚动 | 无 | 无 | 有 |
| 布局 | 分散 | 一条轨道 | 整行标签栏 |
