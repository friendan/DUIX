# Transfer

| | |
|--|--|
| 类 | `CTransferUI` / `CTransferItemUI` |
| XML | `<Transfer>`、`<TransferItem>` |
| 源码 | `src/DuiLib/Control/UITransfer.*` |
| 继承 | Transfer → HorizontalLayout |

左右穿梭框：左侧源列表、右侧目标列表，中间 `>` / `<` 移动勾选项。交互接近 Ant Design Transfer。

通知：`transferchange`（点中间按钮完成移动后）。

### 最小示例

```xml
<!-- 快捷：items + target -->
<Transfer name="roles" width="520" height="240"
    titles="可选角色|已选角色"
    items="管理员|编辑:editor|访客:guest|审计"
    target="editor|guest" />

<!-- 子项 -->
<Transfer name="fruits" width="520" height="240" titles="水果|购物车">
  <TransferItem text="苹果" value="apple" />
  <TransferItem text="香蕉" value="banana" target="true" />
  <TransferItem text="橙子" value="orange" />
</Transfer>
```

```cpp
CTransferUI* p = static_cast<CTransferUI*>(
    m_pm.FindControl(_T("roles"))->GetInterface(DUI_CTR_TRANSFER));
CDuiString keys = p->GetTargetKeys(); // "editor|guest"
p->MoveCheckedToTarget();
p->SetTargetKeys(_T("管理员"));
```

### 属性

| 属性 | 说明 | 默认 |
|------|------|------|
| `titles` / `title` | `源标题\|目标标题` | `可选\|已选` |
| `items` / `options` / `data` | `文案\|文案` 或 `文案:value\|…`（无子节点时生效） | 空 |
| `target` / `selected` / `target-keys` | 初始在右侧的 `value`（`\|` 分隔） | 空 |
| `item-height` | 行高（逻辑 px） | `32` |
| `show-select-all` | 表头全选勾选框 | `true` |

继承 Layout 盒模型：`width` / `height` / `padding` / `margin` / `border` / `gap` 等。

### 子项 TransferItem

| 属性 | 说明 |
|------|------|
| `text` / `title` | 显示文案 |
| `value` / `key` | 逻辑值；省略则等于 `text` |
| `target` / `selected` | `true` 初始在右侧 | `false` |

### API

| 方法 | 说明 |
|------|------|
| `GetTargetKeys()` / `GetSourceKeys()` | `\|` 拼接的 value |
| `SetItems` / `SetTargetKeys` / `AddItem` / `ClearItems` | 改数据并刷新 |
| `MoveCheckedToTarget` / `MoveCheckedToSource` | 移动当前勾选项 |

### 交互

- 勾选行或表头全选 → 中间按钮可用
- `>` 移到右侧，`<` 移回左侧；移动后勾选清除并发 `transferchange`
