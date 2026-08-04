# Loading

| | |
|--|--|
| 类 | `CLoadingUI` |
| XML | `<Loading>`（`DUI_CTR_LOADINGCIRCLE`） |
| 源码 | `src/DuiLib/Control/UILoading.*` |
| 继承属性 | 见 [Control.md](Control.md) |

自绘加载指示（Gdiplus），定时器驱动。位图旋转见 [Ring.md](Ring.md)；页面骨架见 [Skeleton.md](Skeleton.md)。

### 常见样式

```xml
<!-- 圆环类 -->
<Loading type="css" width="64" height="64" thickness="6"
    color="#0958D9FF" track-color="#D9D9D9FF" />
<Loading type="gap" width="64" height="64" thickness="6" color="#0958D9FF" />
<Loading type="fade" width="64" height="64" thickness="6" color="#0958D9FF" />
<Loading type="spoke" width="64" height="64" spoke="12" color="#0958D9FF" />

<!-- 三点依次亮 / 放大 -->
<Loading type="dots" width="80" height="36" color="#0958D9FF" duration="900" />

<!-- 波浪：曲线横移 / 柱状起伏 -->
<Loading type="wave" width="100" height="36" thickness="3" color="#0958D9FF" duration="1000" />
<Loading type="bars" width="100" height="40" spoke="4" color="#0958D9FF" duration="900" />

<!-- 水滴溅开：写实 / 几何卡通 -->
<Loading type="drop" width="64" height="64" color="#0958D9FF" duration="1100" />
<Loading type="drip" width="64" height="64" color="#0958D9FF" duration="1100" />

<!-- 星光：满天闪 / 单星呼吸 -->
<Loading type="stars" width="64" height="64" color="#0958D9FF" duration="1400" />
<Loading type="star" width="64" height="64" color="#0958D9FF" duration="1200" />
```

| `type` | 别名 | 效果 |
|--------|------|------|
| `css` | `border` | 浅灰全圈 + 彩色短头（CSS border-spinner） |
| `gap` | `c` | 硬缺口 C 形实心弧 |
| `fade` | `ring` / `spin` | 渐隐拖尾圆环 |
| `spoke` | （默认） | 辐条渐隐旋转 |
| `dots` | `dot` | 三点水平排列，依次放大/变亮 |
| `wave` | `sine` | 低振幅波形轨道 + 亮段沿线流动（非整条蛇形横爬） |
| `bars` | `eq` / `equalizer` | 实心信号柱（底对齐递增，依次点亮）；`spoke` 为柱数，建议 `4` |
| `drop` | `splash` / `water` | 水滴下落溅开（偏写实：涟漪+飞溅） |
| `drip` | `blob` / `drop-flat` | 水滴溅开（几何卡通） |
| `stars` | `twinkle` | 多星错相闪烁 |
| `star` | `sparkle` / `shine` | 单颗五角星呼吸缩放 |
| `dog` | `puppy` / `run` | Lucide/Tabler `dog` 图标蹦跳（与 SvgBox 同款） |
| `fish` | `fish-one` / `swim` | IconPark `fish-one` 游动（与 SvgBox `iconpark="fish-one"` 同款） |

其它：`arc` / `pulse` / `chase`。

```xml
<Loading type="dog" width="64" height="64" color="#0958D9FF" duration="700" />
<Loading type="fish" width="64" height="64" color="#0958D9FF" duration="1400" />
```

```cpp
CLoadingUI* p = static_cast<CLoadingUI*>(
    m_pm.FindControl(_T("ld"))->GetInterface(DUI_CTR_LOADINGCIRCLE));
p->SetLoadingType(LoadingCss);
p->Stop();
p->Start();
```

### 属性

| 属性 | 说明 | 默认 |
|------|------|------|
| `type` / `variant` | 见上表 | `spoke` |
| `spoke` | 辐条数 | `12` |
| `thickness` | 线宽；`0` 按控件尺寸自适应 | `0` |
| `outer-radius` / `inner-radius` | 辐条内外径；`0` 自适应 | `0` |
| `color` | 主色 | `#1677FF` |
| `track-color` | CSS 灰轨色 | `#f3f3f3` |
| `time` | 帧间隔 ms（流畅度） | `16` |
| `duration` | 转一圈时长 ms | `1200` |

圆环类（css/gap/fade/spoke/arc）先画好带缺口的静态图，再每帧只改旋转角（与 `Ring` 相同），避免 D2D 图缓存导致“闪一下/不转”。

`Init` 自动 `Start()`。控件建议 ≥ `48×48`，线宽 `4~6` 更接近常见网页效果。

### C++ API

| 方法 | 说明 |
|------|------|
| `Start()` / `Stop()` / `IsStopped()` | 启停 |
| `SetLoadingType` / `GetLoadingType` | 切换图形 |
