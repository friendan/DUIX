# Window / html 根

| | |
|--|--|
| 类 | 窗口属性（`UIDlgBuilder`） |
| XML | `<Window>` / 根 `html` |
| 源码 | `src/DuiLib/Core/UIDlgBuilder.cpp` |

> 全局盒模型 / 颜色 / 伪类见 [Attributes.md](Attributes.md)。本页只列**窗口级**属性。

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| `border-radius` | 窗口圆角（CSS 半径：`12`/`12px` 或 `rx,ry`）。控件上同名属性是控件自身圆角 |
| `opacity` / `alpha` | 窗口透明度（`0–1` / `%` / `0–255`） |
| `background-color` | 客户区背景色 |
| `default-font-color` / `disabled-font-color` / `link-font-color` / `link-hover-font-color` | 默认文字色（`ParseColorString`）；`color` 为 `default-font-color` 别名 |
| `font-family` / `font-size` / `font-weight` / `font-style` / `text-decoration` | 改写默认字体（同 `<Font default>`）。未指定时框架默认为 **微软雅黑 12** |
| `selected-color` | 默认选中**背景**色 |

### 部分接近

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `action` | `title`/`close`/`min`/`max`/`move`/`copy`；亦可用 `html { action: title; }` 落到 root。命中控件若 `PreferClientHit()`（SETCURSOR / cursor / 已配热态视觉）则不继承拖拽 | HTML form `action`（含义不同） |
| `min-size` / `max-size` | `w,h` 跟踪尺寸；亦可分写 `min-width`/`min-height`、`max-width`/`max-height` | min/max-width/height |
| `size` | 初始客户区 `w,h`；亦可分写 `width`/`height` | width/height |

### 非标准（桌面窗口模型）

| 属性 | 说明 |
|------|------|
| `size-box` | 可拖拽缩放边距 RECT |
| `caption` | 标题拖拽区 RECT |
| `layered` | 分层窗口 |
| `layered-opacity` | 分层整体透明度 `0`–`255` |
| `layered-image` | 启用分层并设置分层图 |
| `showshadow` / `shadowsize` / `shadowsharpness` / `shadowdarkness` / `shadowposition` / `shadowcolor` / `shadowcorner` / `shadowimage` | 阴影套件 |
| `default-font-color` / `disabled-font-color` / `link-font-color` / `link-hover-font-color` | 默认/链接字体色 |
| `selected-color` | 默认选中背景色（与 Option 的 `color-selected` / `background-color-selected` 不同） |
| `show-dirty` / `gdiplus-text` / `text-rendering-hint` / `tooltip-hover-time` / `no-activate` | 调试 / 文本渲染 / Tooltip / 无激活 |

分层 Present、DComp 等渲染约束见根目录 [AGENTS.md](../../AGENTS.md)，不在本页展开。
