# Carousel / CarouselItem

轮播容器：多页显隐切换、可选自动播放、底部控制栏、可选 caption 条。对齐 EZUI Carousel（无滑动动画）。

| | |
|--|--|
| 类 | `CCarouselUI`、`CCarouselItemUI` |
| XML | `<Carousel>` / `<CarouselItem>`（亦可用 `carousel` / `carousel-item`） |
| 源码 | `src/DuiLib/Control/UICarousel.*` |
| 继承属性 | 见 [Container.md](Container.md)（纵向布局） |
| Demo | Accordion → Carousel；皮肤 `bin/skin/duidemo/carouseltest.html` |

---

## 最小示例

```xml
<Carousel height="220" interval="3000" ride="true" wrap="true" controls="true" controls-kind="info">
  <CarouselItem caption-title="标题" caption-text="说明" caption-kind="dark">
    <VBox background-color="#FF6F42C1" />
  </CarouselItem>
  <CarouselItem>
    <Label text="自定义页" text-align="center" vertical-align="vcenter" />
  </CarouselItem>
</Carousel>
```

---

## Carousel 属性

| 属性 | 说明 |
|------|------|
| `interval` | 自动播放间隔 ms；`≤0` 关闭 |
| `ride` | `true`：启动后自动播放 |
| `wrap` | 循环到首/末 |
| `pause` | 悬停时跳过自动翻页（默认 true） |
| `controls` | 是否显示底部 `|<< < n/m > >>|` |
| `controls-kind` | 控制按钮 `kind`（primary/info/…） |
| `controls-gap` | 首末钮与相邻钮间距 |
| `page-width` | 页码标签宽度 |
| `page-gap` | 页码与前后钮间距 |

### API

`Next` / `Prev` / `GoTo` / `Play` / `Pause` / `GetCurrentIndex` / `GetItemCount`

### 通知

`DUI_MSGTYPE_SLIDECHANGED`（`"slidechanged"`）：`wParam`=新索引，`lParam`=旧索引。

可选：子容器 `name` 含 `carouselIndicators` 时，切换后按当前页刷新其子项背景（白/灰）。

---

## CarouselItem 属性

| 属性 | 说明 |
|------|------|
| `caption-title` | 标题（懒创建底部 caption 条） |
| `caption-text` | 副标题 |
| `caption-align` | `left` / `center` / `right` |
| `caption-kind` | caption 背景用 ControlKind 色 |
| `caption-background` | 直接设 caption 背景色 |

无 caption 时 Item 内可任意布局子控件。

---

## 行为摘要

- 同时仅一项 `CarouselItem` 可见；Item 占满除控制栏外高度
- 自动播放：`CreateTimerQueueTimer` → `UIMSG_CAROUSEL_TICK`（**非** `SetTimer`，见 [Messages.md](Messages.md)）；光标在控件内且 `pause` 时本 tick 不翻页
- 手动切换会重启定时器计时
- 首版无横向滑动动画（可后续接 AnimationTabLayout 思路）
