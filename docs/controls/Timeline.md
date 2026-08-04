# Timeline

| | |
|--|--|
| 类 | `CTimelineUI` / `CTimelineItemUI` |
| XML | `<Timeline>` + `<TimelineItem>` |
| 源码 | `src/DuiLib/Control/UITimeline.*` |
| 继承 | [Container](Container.md) |

垂直时间线：轴 + 圆点 + 时间 / 标题 / 描述。

### 最小示例

```xml
<Timeline height="180" pending="true"
    items="09:00 下单|10:30 仓库出库|14:00 派送中" />

<Timeline height="200" item-gap="36">
  <TimelineItem time="2026-08-01" title="创建订单" description="用户提交" />
  <TimelineItem time="2026-08-02" title="支付成功" status="finish" />
  <TimelineItem time="今天" title="运输中" status="process" color="#722ED1FF" />
</Timeline>
```

### Timeline 属性

| 属性 | 说明 | 默认 |
|------|------|------|
| `items` | `时间 标题\|…` 或 `标题\|…` | 空 |
| `pending` | 末项标为进行中（配合 `items`） | `false` |
| `dot-size` | 圆点 | `10` |
| `item-gap` | 行距 | `28` |
| `finish-color` / `process-color` / `wait-color` / `line-color` | 色 | — |
| `title-color` / `time-color` / `description-color` | 文案色 | — |

### TimelineItem 属性

| 属性 | 说明 |
|------|------|
| `title` / `text` | 标题 |
| `time` / `timestamp` | 时间文案 |
| `description` / `desc` / `content` | 描述 |
| `status` | `finish` / `process` / `wait` |
| `color` / `dot-color` | 圆点色（覆盖 status 色） |
