# Spin / Number

| | |
|--|--|
| 类 | `CSpinUI` |
| XML | `<Spin>` / `<Number>` |
| 源码 | `src/DuiLib/Control/UISpin.*` |
| 继承 | [Edit](Edit.md) |

数字步进：在 Edit 右侧画上下按钮；支持 `min` / `max` / `step` / 小数位。比 `Edit type="number"`（仅 `ES_NUMBER` 整数）完整。

### 最小示例

```xml
<!-- 主题默认已带 1px 边框色/宽；无主题或要改样式时再写 border -->
<Spin value="1" min="0" max="99" step="1" width="120" height="32" />
<Number value="1.5" min="0" max="10" step="0.5" precision="1" width="120" height="32" />
```

```cpp
CSpinUI* p = static_cast<CSpinUI*>(
    m_pm.FindControl(_T("qty"))->GetInterface(DUI_CTR_SPIN));
p->SetValue(3);
double v = p->GetValue();
```

### 属性

| 属性 | 说明 | 默认 |
|------|------|------|
| `value` | 当前值 | `0` |
| `min` / `max` | 范围 | 不限 |
| `step` | 步进 | `1` |
| `precision` / `digits` | 小数位（0=整数） | `0` |
| `controls` | 是否显示上下钮 | `true` |
| `button-width` | 按钮列宽（逻辑 px） | `22` |

继承 Edit：`readonly`、`padding`、边框、字体等。`precision>0` 或 `min<0` 时关闭 `ES_NUMBER`，以便输入小数点 / 负号。

### 交互

- 点 ▲▼ 或 ↑↓ 键、滚轮：步进
- 悬停 ▲▼：手型光标；文本区仍为 I 型
- 键盘输入后失焦或合法 EN_CHANGE：夹紧到 `[min,max]` 并规范化显示
- 通知：`textchanged`（Edit）+ `valuechanged`（数值变化）

### 与 Edit numberonly

| | Edit `type="number"` | Spin / Number |
|--|----------------------|---------------|
| 上下钮 | 无 | 有 |
| 小数 / 负号 | 否（ES_NUMBER） | 可 |
| min/max/step | 无 | 有 |
