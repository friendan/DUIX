# VirtualList

| | |
|--|--|
| 类 | `CVirtualListUI` |
| XML | `<VirtualList>` |
| 源码 | `src/DuiLib/Control/UIVirtualList.*` |
| 继承 | [Container](Container.md) |

固定行高的**虚拟列表**：滚动范围 = `item-count × item-height`，只绘制可视行，不给每一行创建 `CControlUI`。适合十万级以上纯文本 / 自绘行。复杂模板行请继续用 [List](List.md)（或后续再做回收池模式）。

### 最小示例

```xml
<VirtualList name="vlist" item-count="100000" item-height="28"
    overflow="auto" item-alternate-background="true"
    item-background-color-hover="#E6F4FFFF"
    item-background-color-selected="#BAE0FFFF" />
```

```cpp
class CMyListCb : public IVirtualListCallback {
public:
  CDuiString m_s;
  LPCTSTR GetItemText(CControlUI* /*pList*/, int iIndex) override {
    m_s.Format(_T("行 %d"), iIndex + 1);
    return m_s;
  }
};

// InitWindow
CVirtualListUI* p = static_cast<CVirtualListUI*>(
  m_pm.FindControl(_T("vlist"))->GetInterface(DUI_CTR_VIRTUALLIST));
p->SetCallback(&m_cb);
p->SetItemCount(100000);
```

### 属性

| 属性 | 说明 | 默认 |
|------|------|------|
| `item-count` | 逻辑行数 | `0` |
| `item-height` | 行高（逻辑像素，走 DPI Scale） | `28` |
| `selected` | 当前选中索引 | `-1` |
| `item-padding` | 行内文字边距（CSS TRBL） | `0,12,0,12` |
| `item-color` / `item-background-color` | 常态 |
| `item-color-hover` / `item-background-color-hover` | 悬停 |
| `item-color-selected` / `item-background-color-selected` | 选中 |
| `item-color-disabled` / `item-background-color-disabled` | 禁用 |
| `item-line-color` / `item-show-row-line` | 行底部分割线 |
| `item-alternate-background` | 斑马纹开关 |
| `item-alternate-background-color` | 奇数行底色；非 0 时自动开启斑马纹 |
| `item-show-html` | 行文本按迷你 Html 绘制 |
| `item-text-align` | `left` / `center` / `right` |
| `item-font` / `item-font-family` / `item-font-size` | 行字体 |
| `overflow` | 建议 `auto` / `scroll` 开纵向滚动 |

### C++ API

| 方法 | 说明 |
|------|------|
| `SetItemCount` / `GetItemCount` | 逻辑行数 |
| `SetItemHeight` / `GetItemHeight` | 固定行高 |
| `SetCallback` | `IVirtualListCallback*` |
| `GetVisibleRange` | 当前首末可见索引 |
| `HitTestItem` | 坐标 → 行索引 |
| `GetCurSel` / `SelectItem` / `EnsureVisible` | 选中与滚入视口 |

`IVirtualListCallback::PaintItem` 返回 `true` 可完全自绘该行。

### 通知

| 类型 | `wParam` |
|------|----------|
| `itemselect` | 选中索引 |
| `itemclick` / `itemrclick` | 点击索引 |
| `itemactivate` | 双击 / Enter |

键盘：↑↓、PgUp/PgDn、Home/End；滚轮走容器滚动（步进 = 行高）。

### 限制（v1）

- 仅固定行高；可变高度 / 多列 / XML 模板行未做
- `Add` 子控件会被拒绝（不走传统 List 子项模型）
- 多选未做
