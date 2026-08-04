# Skeleton

| | |
|--|--|
| 类 | `CSkeletonUI` |
| XML | `<Skeleton>` / `<skeleton>` |
| 源码 | `src/DuiLib/Control/UISkeleton.*` |
| 继承 | [Control](Control.md) |

加载骨架占位：灰块 + 可选扫光动画。用于列表/详情加载中。

### 最小示例

```xml
<!-- 默认：头像 + 标题 + 段落 -->
<Skeleton active="true" avatar="true" paragraph="3" width="280" height="120" />

<Skeleton type="button" width="80" height="32" active="true" />
<Skeleton type="avatar" width="40" height="40" />
<Skeleton type="input" width="200" height="32" />
<Skeleton type="paragraph" paragraph="4" width="240" height="100" />
```

```cpp
CSkeletonUI* p = static_cast<CSkeletonUI*>(
    m_pm.FindControl(_T("sk"))->GetInterface(DUI_CTR_SKELETON));
p->SetActive(false);
```

### 属性

| 属性 | 说明 | 默认 |
|------|------|------|
| `type` / `variant` | `default` / `avatar` / `button` / `input` / `paragraph` | `default` |
| `active` / `animated` | 扫光动画 | `true` |
| `avatar` | default 模式是否含头像 | `true` |
| `title` | default 模式是否含标题条 | `true` |
| `paragraph` / `rows` | 段落行数 | `3` |
| `round` / `border-radius` | 块圆角 | `4` |
| `block-color` / `color` | 底块色 | 浅灰透明 |
| `highlight-color` | 扫光高亮 | 半透明白 |

未设宽高时按类型自适应。
