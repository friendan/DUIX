# LookupEdit

| | |
|--|--|
| 类 | `CLookupEditUI` / `CLookupColumnUI` |
| XML | `<LookupEdit>` / `<LookupColumn>` |
| 源码 | `src/DuiLib/Control/UILookupEdit.*` |
| 继承 | [Container](Container.md)（列元数据不参与闭合态排版） |

只读查找框：闭合态像 [Combo](Combo.md) / [Edit](Edit.md)，**不能在框里打字**。点击、空格、回车、`F4` / `↓` 打开**模态**挑选窗（非 Combo 分层下拉）。表体是 [VirtualList](VirtualList.md)，行由 C++ 回调提供，适合几千～十万行；**不要把行写进 XML**。

上万条、多列、按字段筛：用本控件。几十条单列仍用 Combo。

### 最小示例

```xml
<LookupEdit name="part" height="32" drop-box-size="520,300" drop-position="bottom"
    placeholder="点击选择零件..."
    padding="0,28,0,8" border="1px solid" border-radius="4">
  <LookupColumn name="code" text="编码" width="100" />
  <LookupColumn name="name" text="名称" width="180" />
  <LookupColumn name="spec" text="规格" width="120" />
</LookupEdit>
```

```cpp
class CPartLookupCb : public ILookupEditCallback {
public:
  int GetRowCount() override { return 10000; }
  LPCTSTR GetCellText(int nRow, int nCol) override {
    if( nCol == 0 ) m_s.Format(_T("P%05d"), nRow + 1);
    else if( nCol == 1 ) m_s.Format(_T("零件 %d"), nRow + 1);
    else m_s.Format(_T("规格-%d"), (nRow % 20) + 1);
    return m_s.GetData();
  }
private:
  CDuiString m_s;
};

// InitWindow
CLookupEditUI* p = static_cast<CLookupEditUI*>(
  m_pm.FindControl(_T("part"))->GetInterface(DUI_CTR_LOOKUPEDIT));
p->SetCallback(&m_cb);

// Notify itemselect：wParam = 全集行号（不是过滤后的可见下标）
if( msg.sType == DUI_MSGTYPE_ITEMSELECT && msg.pSender->GetName() == _T("part") ) {
  int row = (int)msg.wParam;
  p->SetText(m_cb.GetCellText(row, 1)); // 控件不自动填 text
}
```

### 交互

| 操作 | 行为 |
|------|------|
| 单击框 / 空格 / 回车 / F4 / ↓ | 打开挑选窗（模态，主窗暂时禁用） |
| 列筛选 Edit | 忽略大小写子串；多列 **AND**；空条件 = 该列不筛 |
| 单击行 | 仅高亮 |
| 双击行 / 回车（有高亮） / 确定 | 确认并关闭，发 `itemselect` |
| Esc / 取消 / 回车（无高亮） | 关闭，不改框内文字与 `GetCurSel` |
| 弹窗位置 | `drop-position`：相对编辑框 `bottom` / `top` / `left` / `right` / `center`。对侧空间不够则翻面，再夹进工作区 |
| 悬停 | 手型光标（只读可点）；可用 `cursor="arrow"` 覆盖 |

弹窗**非分层**，筛框是原生 `CEdit`，可正常使用 IME。

确认**不会**按某列自动 `SetText`。通知只带全集行号，业务自己回写显示文本并记下 id。

duidemo：**表单 → LookupEdit**（1 万行回调；可切弹出位置、大小与闭合态对齐）。

### 属性

| 属性 | 说明 | 默认 |
|------|------|------|
| `drop-box-size` | `宽,高`（逻辑像素，走 DPI）。**高**：弹窗高度，`0` 则 280；低于表头+筛栏+按钮+80 时抬到该下限。**宽**：`0`=按编辑框与列宽自适应；`>0`=弹窗宽度（小于列宽之和时多余列会被压窄/裁切，表体无横向滚动） | `0,280` |
| `drop-position` | 相对编辑框弹出：`bottom` / `top` / `left` / `right` / `center`（亦可用 `下/上/左/右/中`）。别名 `popup-position`、`placement` | `bottom` |
| `item-height` | 表体行高（逻辑像素，走 DPI Scale） | `28` |
| `placeholder` / `placeholder-color` | 闭合态无 text 时的提示 |
| `color` / `color-disabled` | 闭合态文字 |
| `text-padding` | 闭合态文字额外边距（CSS TRBL） | `0,6,0,6` 语义上左右 6 |
| `text-align` | 闭合态水平对齐：`left` / `center` / `right`（亦可用 `左` / `中` / `居中` / `右`）。别名 `align`。占位符同样生效。弹窗表体单元格仍左对齐 | `left` |
| `text-overflow` | 闭合态：`ellipsis` 省略号，其它值关闭 | `ellipsis` |
| `padding` | 建议右侧留箭头：`0,28,0,8` |

`LookupColumn`：`name`、`text`（表头）、`width`（逻辑像素）。运行时也可用 `AddColumn`。

### C++ API

| 方法 | 说明 |
|------|------|
| `SetCallback` | `ILookupEditCallback*`：`GetRowCount` / `GetCellText(row, col)` |
| `GetCurSel` / `SetCurSel` | 已确认的全集行号；`-1` 未选 |
| `AddColumn` / `GetColumn` / `GetColumnCount` | 列 |
| `Activate` | 打开挑选窗（与点击相同） |
| `SetDropPosition` / `GetDropPosition` | 弹窗相对编辑框：下/上/左/右/中 |
| `SetDropBoxSize` / `GetDropBoxSize` | 弹窗宽高；宽 `0` 为自适应 |
| `SetTextStyle` / `GetTextStyle` | 闭合态 DrawText 标志（含 DT_LEFT / DT_CENTER / DT_RIGHT） |

### 通知

| 类型 | `wParam` | 时机 |
|------|----------|------|
| `dropdown` | — | 即将打开 |
| `itemselect` | 全集行号 | 确认后（模态结束、主窗已恢复） |

过滤后的可见下标不会出现在通知里。
