# Rate

| | |
|--|--|
| 类 | `CRateUI` |
| XML | `<Rate>` / `<rate>` |
| 源码 | `src/DuiLib/Control/UIRate.*` |
| 继承 | [Control](Control.md) |

星级评分。悬停预览，点击打分；可选半星循环、清空、只读展示。

通知：`valuechanged`。

### 最小示例

```xml
<Rate value="3" />
<Rate value="2.5" allow-half="true" />
<Rate value="4" readonly="true" />
<Rate value="3" character="♥" color="#FF4D4FFF" />
```

```cpp
CRateUI* p = static_cast<CRateUI*>(
    m_pm.FindControl(_T("rate1"))->GetInterface(DUI_CTR_RATE));
double v = p->GetValue();
p->SetValue(4.5);
```

### 属性

| 属性 | 说明 | 默认 |
|------|------|------|
| `value` | 当前分（`0` … `count`） | `0` |
| `count` / `max` | 星数 | `5` |
| `allow-half` | 允许半星（同星连点循环） | `false` |
| `allow-clear` | 允许清零（见交互） | `true` |
| `readonly` | 只展示，不可改 | `false` |
| `disabled` | 禁用（变淡） | `false` |
| `star-size` / `size` | 单星边长（逻辑 px） | `24` |
| `star-gap` | 星间距 | `8` |
| `character` / `char` | 字符（默认实心星） | `★` |
| `color` / `star-color` | 实心色 | `#FADB14` |
| `void-color` | 空星色 | 浅灰 |

继承 Control 盒模型：`width` / `height` / `padding` / `margin` 等。未设宽高时按星数自适应。

### 交互

- **整星**（默认）：点第 N 星 → 设为 N；`allow-clear` 时再点同分 → `0`
- **半星**（`allow-half`）：同星连点 **半星 → 全星 → 取消(0)**；点其他星从该星半星重新开始
- 悬停预览「下一次点击」的结果
- 焦点下 ←→ / ↑↓ 步进，Home / End 到两端
