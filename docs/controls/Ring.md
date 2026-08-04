# Ring

| | |
|--|--|
| 类 | `CRingUI` |
| XML | `<Ring>` |
| 源码 | `src/DuiLib/Control/UIRing.*` |
| 继承属性 | 见 [Label.md](Label.md) |

旋转环 / 加载图：用 GDI+ 对 `background-image` 按角度旋转绘制。首次绘制成功加载图片后启动 100ms 定时器，每帧角度 `+36°`（约 10 FPS 转一圈）。

更现代的自绘加载指示见 [Loading.md](Loading.md)；本控件依赖一张可旋转的位图资源。

### 最小示例

```xml
<Ring width="48" height="48" background-image="loading_ring.png" />
```

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| `background-image` | 旋转源图（专用路径：加载为 GDI+ `Image`，不用普通皮肤九宫） |

其余尺寸 / 可见性等走 Label。

### 行为

| 项 | 说明 |
|------|------|
| 定时器 | `RING_TIMERID=100`，间隔 100ms |
| 绘制 | `DrawGdiplusImageRotated`，角度 `m_fCurAngle` |
| 析构 | 杀定时器并释放 GDI+ 图 |

无独立起停 XML 属性；隐藏控件不会自动停表（随控件生命周期）。需要更细控制时用 Loading 或自行改代码。
