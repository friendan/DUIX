# GifAnim / GifAnimEx

| | |
|--|--|
| 类 | `CGifAnimUI`、`CGifAnimExUI` |
| XML | `<GifAnim>`（Ex 同名接口，见下） |
| 源码 | `UIGifAnim.*`、`UIGifAnimEx.*` |
| 继承 | GifAnim → [Control](Control.md)；GifAnimEx → [Label](Label.md) |

播放 GIF 动画。默认用 **GifAnim**（Gdiplus）；多实例 CPU 高时可编译 **GifAnimEx**（CxImage，需宏）。

## GifAnim

### 示例

```xml
<GifAnim width="64" height="64"
    background-image="loading.gif"
    auto-play="true"
    auto-size="false" />
```

```cpp
CGifAnimUI* p = static_cast<CGifAnimUI*>(
    m_pm.FindControl(_T("gif"))->GetInterface(DUI_CTR_GIFANIM));
p->PlayGif();
p->PauseGif();
p->StopGif();
```

### 属性

| 属性 | 说明 | 默认 |
|------|------|------|
| `background-image` | GIF 路径 / 资源 | — |
| `auto-play` | 加载后自动播放 | `true` |
| `auto-size` | 按首帧尺寸设控件宽高 | `false` |

可见时 `PlayGif`，隐藏时 `StopGif`。定时器用帧间隔属性（GIF 自带 delay）。

### C++ API

`PlayGif` / `PauseGif` / `StopGif` / `SetBackgroundImage` / `SetAutoPlay` / `SetAutoSize`。

---

## GifAnimEx

仅在定义 **`USE_XIMAGE_EFFECT`** 后编译注册（见 `UIGifAnimEx.h` 注释）：

```cpp
#define USE_XIMAGE_EFFECT
#include "UIlib.h"
```

用 CxImage 减负；属性主要是 `auto-play`，图源走 Label/基类图片属性。C++：`StartAnim` / `StopAnim`。

未开宏时工厂不注册，皮肤里写 `<GifAnim>` 仍走 Gdiplus 版。
