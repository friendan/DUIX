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

---

## WindowImplBase（业务窗基类）

自定义皮肤窗继承 `WindowImplBase`，`Create(owner, …)` + `ShowModal()` / `ShowModalFake()`。

| API | 默认 | 说明 |
|-----|------|------|
| `SetSyncOwnerMove(bool)` / `IsSyncOwnerMove()` | `false` | 拖/移本窗时同步移动 **HWND Owner**（屏幕坐标相对偏移） |
| `SetSyncOwnerSize(bool)` / `IsSyncOwnerSize()` | `false` | 缩放本窗时同步缩放 Owner（保留打开时宽高差；铺满时差为 0） |
| `ShowModal()` / `ShowModalFake()` | | 进入前抓取偏移/尺寸差；关闭后清除 |

铺满主窗的设置窗示例：

```cpp
pSettings->Create(m_hWnd, ...);
// 先把设置窗摆成与主窗同位置同大小，再：
pSettings->SetSyncOwnerMove(true);
pSettings->SetSyncOwnerSize(true);
pSettings->ShowModal();
```

行为细节：

- 仅 Move：纯移动 → Owner 跟移；仅右/下边缩放 → Owner 不动；左/上边缩放 → 只重算偏移
- 开了 Size：任意边缩放都同步 Owner 尺寸；同时开了 Move 时左/上边缩放会连位置一起跟
- **多屏幕**：屏幕物理像素（副屏可为负坐标）；跨屏 DPI 由 `WM_DPICHANGED` 跟系统建议矩形并同步 Owner；`WM_DISPLAYCHANGE` 重抓偏移
- Owner / 本窗最大化或最小化时跳过同步
- 无 Owner（主窗）时为空操作；默认皆关
- Demo：Accordion → Modal →「铺满设置窗（同步主窗）」（`CSettingsSyncWnd` / `settings_sync.html`）
- 轻量确认框仍用 [Modal.md](Modal.md) 的 `CModal::SyncOwnerMove`；业务模态窗用本基类
