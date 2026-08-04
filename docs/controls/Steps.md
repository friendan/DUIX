# Steps

| | |
|--|--|
| 类 | `CStepsUI` / `CStepItemUI` |
| XML | `<Steps>` + `<Step>` / `<StepItem>` |
| 源码 | `src/DuiLib/Control/UISteps.*` |
| 继承 | [Container](Container.md) |

步骤条（水平 / 垂直）。`current` 之前为完成、当前为进行、之后为等待；单项可用 `status` 覆盖。

### 最小示例

```xml
<Steps current="1" items="填写信息|审核确认|完成" height="72" />

<Steps current="1" height="80" clickable="true">
  <Step title="提交" description="填写资料" />
  <Step title="审核" description="人工复核" />
  <Step title="完成" description="开通成功" />
</Steps>

<Steps direction="vertical" current="0" width="220" height="160"
    items="选配|支付|发货|签收" />
```

### Steps 属性

| 属性 | 说明 | 默认 |
|------|------|------|
| `current` | 当前步骤（0 起） | `0` |
| `direction` | `horizontal` / `vertical` | `horizontal` |
| `items` | `标题\|标题\|…`（无子节点时用） | 空 |
| `clickable` | 点击切换并通知 | `false` |
| `dot-size` | 圆点逻辑像素 | `24` |
| `finish-color` / `process-color` / `wait-color` / `error-color` | 状态色 | 蓝 / 蓝 / 灰 / 红 |
| `title-color` / `description-color` | 文案色 | — |

### Step 属性

| 属性 | 说明 |
|------|------|
| `title` / `text` | 标题 |
| `description` / `desc` | 副文案 |
| `status` | `auto`（默认）/ `wait` / `process` / `finish` / `error` |

### 通知

| 类型 | 说明 |
|------|------|
| `selectchanged` | `clickable` 时切换步骤，`wParam` 为新索引 |
