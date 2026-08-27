# Ring

| | |
|--|--|
| 类 | `CRingUI` |
| XML | `<Ring>` |
| 源码 | `src/DuiLib/Control/UIRing.*` |
| 继承属性 | 见 [Label.md](Label.md) |

旋转环 / 加载图：用 GDI+ 对 `background-image` 按角度旋转绘制。首次绘制成功加载图片后启动 **100ms** 队列定时器，每帧角度 `+36°`（约 1s 转一圈）。

更现代的自绘加载指示见 [Loading.md](Loading.md)；本控件依赖一张可旋转的位图资源。

### 最小示例

```xml
<Ring width="48" height="48" background-image="loading_ring.png" />
```

duidemo → 反馈 → Loading 旁有示例（资源 `skin/duidemo/loading_ring.png`）。

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| `background-image` | 旋转源图（专用路径：加载为 GDI+ `Image`，不用普通皮肤九宫） |

其余尺寸 / 可见性等走 Label。

### 行为

| 项 | 说明 |
|------|------|
| 定时 | `CreateTimerQueueTimer` → `PostMessage(UIMSG_RING_TICK)`（**非** `SetTimer` / `WM_TIMER`，见 [Messages.md](Messages.md)） |
| 间隔 | 100ms；每帧 `m_fCurAngle += 36°` |
| 绘制 | `DrawGdiplusImageRotated`，角度 `m_fCurAngle` |
| 可见性 | `SetVisible` / `SetInternVisible`：隐藏停表，再显示且已有位图则重启 |
| 析构 | `DeleteTimerQueueTimer` + 释放 GDI+ 图 |

无独立起停 XML 属性；需要 API 级控制时用 [Loading](Loading.md) 或自行改代码。
