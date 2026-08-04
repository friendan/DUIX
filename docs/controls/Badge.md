# Badge / Tag

| | |
|--|--|
| 类 | `CBadgeUI`、`CTagUI` |
| XML | `<Badge>`、`<Tag>` |
| 源码 | `src/DuiLib/Control/UIBadge.*` |
| 继承 | Badge → Container；Tag → [Label](Label.md) |

**Tag**：圆角标签芯片（状态色 / 可关闭）。  
**Badge**：数字角标或小红点；可包裹子控件叠在右上角，也可单独使用。

### Tag 示例

```xml
<HBox height="28" gap="8" align-items="vcenter">
  <Tag text="默认" />
  <Tag text="成功" status="success" />
  <Tag text="进行中" status="processing" />
  <Tag text="错误" status="error" />
  <Tag text="警告" status="warning" />
  <Tag text="可关闭" status="processing" closable="true" />
</HBox>
```

关闭时发通知 `close`，并默认 `SetVisible(false)`。

### Badge 示例

```xml
<!-- 包一层：角标挂在子控件右上 -->
<Badge count="5" width="80" height="36">
  <Button text="消息" width="80" height="32" />
</Badge>

<Badge count="100" overflow-count="99" width="80" height="36">
  <Button text="通知" width="80" height="32" />
</Badge>

<Badge dot="true" width="80" height="36">
  <Button text="动态" width="80" height="32" />
</Badge>

<!-- 独立数字 -->
<Badge count="3" height="18" />
```

```cpp
CBadgeUI* b = static_cast<CBadgeUI*>(
    m_pm.FindControl(_T("msg_badge"))->GetInterface(DUI_CTR_BADGE));
b->SetCount(12);

CTagUI* t = static_cast<CTagUI*>(
    m_pm.FindControl(_T("tag1"))->GetInterface(DUI_CTR_TAG));
t->SetStatus(CTagUI::StatusSuccess);
```

### Tag 属性

| 属性 | 说明 | 默认 |
|------|------|------|
| `text` | 文案 | 空 |
| `status` | `default` / `success` / `processing` / `error` / `warning` | `default` |
| `closable` | 显示 ×，点击发 `close` 并隐藏 | `false` |
| `color` / `background-color` / `border-*` | 覆盖 status 配色 | status 预设 |
| `border-radius` | 圆角 | `4` |

继承 Label：`font`、`padding`、自动宽等。

### Badge 属性

| 属性 | 说明 | 默认 |
|------|------|------|
| `count` / `value` | 数字 | `0` |
| `overflow-count` | 超过显示 `N+` | `99` |
| `show-zero` | `count=0` 是否仍显示 | `false` |
| `dot` | 小红点（忽略数字文案） | `false` |
| `hang` | `true`：半悬右上角（自动上/右 padding，避免被父级裁切） | `true` |
| `offset` | `x,y` 自定义偏移（设置后不再用默认 hang 几何） | `0,0` |
| `badge-color` / `color` | 角标底色 | 红 |
| `badge-text-color` | 数字色 | 白 |

有子控件时：子控件按 Container 布局，角标画在**第一个子控件**右上角。无子控件时：控件自身即为角标尺寸。

### 通知

| 控件 | 类型 | 说明 |
|------|------|------|
| Tag | `close` | 点 × |
| Tag | `click` | 可点时继承 Label `clickable` |
