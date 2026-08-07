# Empty

| | |
|--|--|
| 类 | `CEmptyUI` |
| XML | `<Empty>` / `<empty>` |
| 源码 | `src/DuiLib/Control/UIEmpty.*` |
| 继承 | VerticalLayout |

空状态占位：默认插画（或自定义图）+ 描述文案；子控件作为底部操作区。

嵌在 [List](List.md) 内时：0 项自动显示、有项隐藏；也可用 List 的 `empty-text` / `empty-image` 懒建。

### 最小示例

```xml
<Empty description="暂无数据" height="180" />

<Empty description="还没有消息" height="200">
  <Button text="刷新" kind="primary" width="80" height="32" />
</Empty>

<Empty description="自定义图" image="menu/menu_bk.png" image-size="72,72" height="200" />

<!-- List 空态 -->
<List empty-text="暂无数据" header="hidden" height="160" />
```

### 属性

| 属性 | 说明 | 默认 |
|------|------|------|
| `description` / `desc` / `text` | 描述 | `暂无数据` |
| `image` / `src` | 自定义图（皮肤图串）；空则画默认插画 | 空 |
| `image-size` | 图/插画区域 `宽,高` | `96,96` |
| `show-image` | 是否显示图区 | `true` |

继承 Layout：`width` / `height` / `padding` / `gap` / `align-items` 等。
