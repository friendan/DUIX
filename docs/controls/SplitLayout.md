# SplitLayout

| | |
|--|--|
| 类 | `CSplitLayoutUI` / `CHSplitLayoutUI` / `CVSplitLayoutUI` |
| XML | `<SplitLayout>` `<HSplit>` `<VSplit>` |
| 源码 | `src/DuiLib/Layout/UISplitLayout.*` |
| 继承 | [Container](Container.md) |

一维分割：子项沿主轴排开，**相邻两项之间**拖分隔条互抢空间（总量不变）。水平分栏、垂直分行；要行列组合就**嵌套**另一个 SplitLayout。不是二维网格：拖一条竖线不会同步其它行的列宽。

和 [HBox/VBox 的 `sep-width`](Container.md) 不同：那边拖的是**容器自己**的外边；这里拖的是**内部**分隔条。

### 最小示例

```xml
<HSplit sep-size="6">
  <VBox width="200" min-width="80" theme="panel">左栏</VBox>
  <VBox min-width="120" theme="panel">右栏（不写 width = 吃剩余）</VBox>
</HSplit>
```

嵌套成「左 | 上/下 | 右」：

```xml
<SplitLayout orientation="horizontal" sep-size="6">
  <VBox width="160" min-width="80">左</VBox>
  <VSplit min-width="120">
    <VBox height="100" min-height="48">上</VBox>
    <VBox min-height="48">下</VBox>
  </VSplit>
  <VBox width="140" min-width="80">右</VBox>
</SplitLayout>
```

duidemo：**布局 → SplitLayout**。

### 交互

| 操作 | 行为 |
|------|------|
| 悬停分隔条 | 线变粗，色跟主题 `color-primary`（可用 `sep-color-hover` 覆盖） |
| 拖分隔条 | 左边/上边 pane 变大，右边/下边变小（或相反），受各自 `min-*` / `max-*` 限制 |
| 松手 | 发 `valuechanged`，`wParam` = 分隔条下标（0 起） |
| 嵌套 | 内层 SplitLayout 自己的分隔条独立命中 |

首次拖某条分隔线时，涉及的两个 pane 会从弹性/`%` **固化成当前像素**，避免下次布局回弹。

### 属性

| 属性 | 说明 | 默认 |
|------|------|------|
| `orientation` | `horizontal` / `vertical`（亦可用 `h`/`v`、`横`/`纵`）。别名 `direction`。`<HSplit>` / `<VSplit>` 已定方向 | `horizontal`（`SplitLayout`） |
| `sep-size` | 分隔条热区厚度（逻辑像素，走 DPI）。别名 `sep-width` / `sep-height`。`0` 则贴死且不可拖 | `6` |
| `sep-imm` | `true` 拖拽中实时改布局；`false` 松手才应用 | `true` |
| `sep-color` | 常态分隔线。不写则主题 `color-border`。可写色值或 `var(--token)`（热切主题重解） | 主题边框 |
| `sep-color-hover` | 悬停。不写则主题 `color-primary`。CSS：`HSplit:hover { sep-color: ... }` | 主题主色 |
| `sep-color-active` | 拖拽中。不写则 `sep-color-hover`，再否则主题 `color-primary-active` | 主题主色按下 |

子项尺寸：

| 写法 | 含义 |
|------|------|
| `width="200"` / `height="120"` | 固定（水平看宽，垂直看高） |
| `width="30%"` | 按**扣除分隔条后**的可用主轴尺寸比例 |
| 不写主轴尺寸 | 弹性，均分剩余 |
| `min-width` / `max-width` 等 | 拖拽与布局下限/上限 |

`gap` **不参与** pane 间距（间距就是 `sep-size`）。

### C++ API

| 方法 | 说明 |
|------|------|
| `SetOrientation` / `GetOrientation` | `LAYOUT_HORIZONTAL` / `LAYOUT_VERTICAL` |
| `SetSepSize` / `GetSepSize` | 热区厚度（逻辑像素） |
| `SetSepImmMode` / `IsSepImmMode` | 是否即时布局 |
| `SetSepColor` / `GetSepColor` | 常态；`0` = 主题 `color-border` |
| `SetSepHoverColor` / `GetSepHoverColor` | 悬停；`0` = 主题 `color-primary` |
| `SetSepActiveColor` / `GetSepActiveColor` | 拖拽；`0` = hover 或主题 `color-primary-active` |

### 通知

| 类型 | `wParam` | 时机 |
|------|----------|------|
| `valuechanged` | 分隔条下标 | 鼠标松开 |

### 注意

- 子项建议都设 `min-width` / `min-height`，否则可被拖到 0。
- 绝对定位子项（`absolute`）不参与分割，仍按 [Container](Container.md) 自己摆。
- 不要用 `window-resize` 代替本控件：那是缩 **HWND**，不是改 pane。
