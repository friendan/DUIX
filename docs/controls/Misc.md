# 其它控件（简表）

| | |
|--|--|
| 类 | `多项` |
| XML | 见各节 |
| 源码 | `src/DuiLib/Control/*` |
| 继承属性 | 见 [Control.md](Control.md) |

> 本页聚焦 **属性与 HTML/CSS 的符合度**。盒模型全局约定见 [Attributes.md](Attributes.md)。

### ActiveX — `CActiveXUI`

| 属性 | HTML? | 说明 |
|------|-------|------|
| `clsid` / `module-name` / `delay-create` | 否 | ActiveX 宿主 |

### WebBrowser — `CWebBrowserUI`

| 属性 | HTML? | 说明 |
|------|-------|------|
| `home-page` | 部分（≈ src） | 主页 URL |
| `auto-navi` | 否 | 自动导航 |

### GifAnim / GifAnimEx / Ring / Loading / ColorPalette / HotKey / GroupBox

| 控件 | 非标准属性 |
|------|-----------|
| GifAnim | `auto-play`、`auto-size`（`background-image` 名接近 CSS） |
| GifAnimEx | `auto-play` |
| Ring | （主要用基类 `background-image`） |
| Loading | `time`、`spoke`、`thickness`、`outer-radius`、`inner-radius`、`color`、`style` |
| ColorPalette | `palette-height`、`bar-height`、`thumb-image` |
| HotKey | `image*`、`native-background-color` |
| GroupBox | `color`、`color-disabled`、`font-family`、`font-size`（标题绘制） |
| DateTime / IPAddress | 无独立 SetAttribute（走基类/父类） |

### Toast / Modal

**无 XML 属性**；C++ API（`CToastOptions` / `CModalOptions`）。见 [Toast.md](Toast.md)、[Modal.md](Modal.md)。
