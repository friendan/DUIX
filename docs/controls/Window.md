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
| `wallpaper-bleed` / `bg-bleed` | 壁纸透出：根有 `background-image` 时，控件**底色** alpha × 此系数（`0–1` / `%` / `0–255`）。`false`/`none`/`solid`=关。默认关（255） |
| `wallpaper-bleed-need-image` / `bg-bleed-need-image` | 默认 `true`：仅当根有背景图才透出；`false` 时只要设了 bleed 就对底色生效 |
| `background-color` | 客户区背景色（落到 root；root 已有底色则不覆盖） |
| `background-image` | 客户区背景图（落到 root；`url(...)` / 裸路径；root 已有背景图则不覆盖）。与 `wallpaper-bleed` 配合可透出壁纸 |

C++（窗口级，`CPaintManagerUI`）：

```cpp
m_pm.SetWindowBackgroundImage(_T("skin/bg.png"));
m_pm.SetWindowBackgroundImageFromMemory(pPng, cb);           // PNG/JPEG/BMP/GIF；亦识别 SVG 文本
m_pm.SetWindowBackgroundImageFromSvg(utf8Svg, nBytes, 1280, 800); // 显式 SVG 栅格
m_pm.SetWindowBackgroundImageFromSvg(_T("<svg .../>"), 0, 0);     // 固有尺寸
```

控件级（任意 `CControlUI`，需已挂 Manager）：

```cpp
pCtrl->SetBackgroundImageFromMemory(pData, cb);
pCtrl->SetBackgroundImageFromSvg(utf8, n, 256, 256);
```

SVG 栅格底层：`CSvgBoxUI::RasterizeToHBitmap`。 |
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
| `size-box` | 窗口客户区缩放热区厚度，四值顺序 **左,上,右,下（LTRB）**（例 `4,4,6,6`）；**不是** CSS 上右下左。某边 `0` 则该边不缩；可与控件 `window-resize` / `window-size-box` 分工 |
| `caption` | 标题拖拽区 RECT |
| `layered` | 分层窗口 |
| `layered-opacity` | 分层整体透明度 `0`–`255` |
| `layered-image` | 启用分层并设置分层图 |
| `shape-image` | 异形窗参考图。分层靠 alpha；未设 `action` 且 `shape-drag`（默认开）时自动 `move`。见 [Shape.md](Shape.md) |
| `shape-mask` | 非分层 RGN 用；省略则用 `shape-image` |
| `shape-alpha-threshold` | 异形 alpha 阈值，默认 `16` |
| `shape-drag` | `true`/`false`：是否自动 `move`（默认 true） |
| `showshadow` / `shadowsize` / `shadowsharpness` / `shadowdarkness` / `shadowposition` / `shadowcolor` / `shadowcorner` / `shadowimage` | 阴影套件 |
| `default-font-color` / `disabled-font-color` / `link-font-color` / `link-hover-font-color` | 默认/链接字体色 |
| `selected-color` | 默认选中背景色（与 Option 的 `color-selected` / `background-color-selected` 不同） |
| `show-dirty` / `gdiplus-text` / `text-rendering-hint` / `tooltip-hover-time` / `no-activate` | 调试 / 文本渲染 / Tooltip / 无激活 |

### 壁纸透出（`wallpaper-bleed`）

根控件设 `background-image`（或 **html/Window 上写 `background-image`**，Attach 后落到 root）后，中间面板不透明会挡住壁纸。窗口级：

```xml
<html theme="chrome" background-image="skin/bg.png" wallpaper-bleed="0.72">
  <VBox name="root">
    <TitleBar ... /> <!-- chrome 默认 solid，标题栏不透 -->
    <VBox>...</VBox> <!-- 底色按 0.72 乘 alpha，透出壁纸 -->
    <Button wallpaper-bleed="solid" ... /> <!-- 单控件强制不透 -->
  </VBox>
</html>
```

- `background-image` / `background-color` 均可写在 `<html>` / `<Window>` / `html { ... }` CSS 块
- 只改**背景色**绘制（`PaintBackgroundColor` → `GetAdjustColor`），文字/边框/背景图不受影响  
- 默认需根有背景图才生效；`wallpaper-bleed-need-image="false"` 可对纯色底也套 bleed  
- Demo：标题栏图片按钮 / Theme「选择背景图」；`main.html` 已开 bleed `0.72`

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
- **最大化**：几何同步仍跳过（`IsZoomed`），避免套用最大化异常矩形
- **最小化 / 还原**：开了 `SyncOwnerMove` 或 `SyncOwnerSize` 时，本窗最小化会一并最小化 Owner（避免露出后面的主窗）；还原本窗时还原 Owner 并重新对齐位置/尺寸
- **SyncOwnerSize 遵守 Owner 的 `min-size` / `max-size`**（本窗拖缩下限不低于 Owner 最小跟踪尺寸；`SetWindowPos` 同步时也会钳制）。铺满场景宽高差为 0 时，本窗与主窗同限
- 无 Owner（主窗）时为空操作；默认皆关
- Demo：Accordion → Modal →「铺满设置窗（同步主窗）」（`CSettingsSyncWnd` / `settings_sync.html`；标题栏含最小/最大化，便于测最小化联动）
- 轻量确认框仍用 [Modal.md](Modal.md) 的 `CModal::SyncOwnerMove`；业务模态窗用本基类

	HWND 自定义消息号段（库占用 `WM_APP` 低端，业务用 `WM_DUILIB_USER + n`）见 **[Messages.md](Messages.md)**。

---

## 空白右键菜单（`CPaintManagerUI`）

默认**关闭**，对库无影响；按需开启后在「无控件命中的空白处右键」给容器/根补发一次 `DUI_MSGTYPE_MENU`。

| API | 默认 | 说明 |
|-----|------|------|
| `SetBlankContextMenuEnabled(bool)` / `IsBlankContextMenuEnabled()` | `false` | 总开关。关闭时空白右键维持原行为（被吞掉） |
| `SetBlankContextMenuUseDeepestContainer(bool)` / `IsBlankContextMenuUseDeepestContainer()` | `true` | target：`true`=最内层覆盖该点的**可见容器**（忽略 mouse 开关，鼠标关闭的容器也会命中）；`false`=窗口根容器 |

	用法（**定向派发**）：开启后，空白右键会调用**命中目标容器自身的 `OnNotify` 回调**（`sType = DUI_MSGTYPE_MENU`），**不再广播给整个 manager 的所有 notifier**。想响应的容器必须在它上面挂 `OnNotify` 回调——即“谁处理谁设置”。

	```cpp
	// 1) 窗口里开启
	m_pm.SetBlankContextMenuEnabled(true);
	// m_pm.SetBlankContextMenuUseDeepestContainer(true); // 默认即 true，可不调

	// 2) 在需要响应空白右键的容器上挂 OnNotify 回调（例如站点页/根容器）。
	//    用 CEventSource 的 `+=` 挂 MakeDelegate，处理器签名为 bool(void*)：
	#include "UIlib.h"
	sitePage->OnNotify += DuiLib::MakeDelegate(this, &CMyPage::OnMenu);

	// 3) 处理器里按需弹菜单：
	bool CMyPage::OnMenu(void* p)   // 由 OnNotify 回调触发
	{
		if( p == NULL ) return false;
		DuiLib::TNotifyUI* msg = (DuiLib::TNotifyUI*)p;
		if( msg->sType != DuiLib::DUI_MSGTYPE_MENU ) return false;
		// 屏幕坐标用 msg->ptScreen；客户端坐标用 msg->ptMouse（同一鼠标点，均可信）
		ShowBlankContextMenu(msg->ptScreen, msg->pSender);
		return true;
	}
	```

	- `CEventSource::OnNotify` 用 `+=` 挂 `MakeDelegate(this, &Class::OnMenu)`；处理器签名 **`bool Class::OnMenu(void*)`**，把 `void*` 转成 `TNotifyUI*` 取字段。容器方法、窗口方法均可挂。
	- **就近冒泡（推荐挂树上任一祖先即可）**：空白命中某个最内层容器后，通知会从它沿父链**向上找第一个设置了 `OnNotify` 的容器**来派发。因此你**不必精确知道空白落在哪个深层容器**，在任一祖先（含根 / `sitePage` / `siteTabs` 等）上设一次回调，就能覆盖整棵子树。
	  - 例：容器 A 含 B、B 含 C，空白点在 C 内 —— **只需在 A（或 B）上挂 `OnNotify`**，通知会从 C 冒泡到第一个设过回调的祖先 A 触发；`msg.pSender` 仍指向命中的 C，回调可据此判断点在哪个容器。
	  - 就近原则：若 A、B 都设了回调，空白落在 C/B 时由**最内层设过回调的**（B）接收，不重复分发。
	- 通知字段：`sType = DUI_MSGTYPE_MENU`；`pSender` = 命中目标（最内层容器/根）；`ptScreen` = 屏幕坐标；`ptMouse` = 客户端坐标。两者是同一鼠标点，弹菜单直接用它俩。
	- **不要用 `GET_X/Y_LPARAM(msg.lParam)` 取坐标**：控件右键路径的 `lParam` 是控件指针，空白路径的 `lParam` 才透传原始 `WM_CONTEXTMENU` 屏幕坐标；统一用 `msg.ptScreen` 最稳。
	- **为何定向**：空白 MENU 只发给命中容器，避免广播给不相干窗口 notifier（早期版本会因窗口 notifier 对未注册 sType 空解引用而崩）。控件 MENU（分类/站点按钮，走 `SetContextMenuUsed` → `DUI_MSGTYPE_MENU` 广播）不受影响，仍走各自 `IsContextMenuUsed`。
	- 有控件命中时仍走原逻辑（`UIEVENT_CONTEXTMENU`），本开关不影响已有控件右键。
	- 背景：TabLayout 导航页内容未铺满时，空白区因 `action:title` 会被 `WM_NCHITTEST` 判成非客户区，右键变成 `WM_NCRBUTTONDOWN/UP`；本开关对应的 `WM_NCRBUTTONDOWN/UP` 处理会把右键转回客户区并触发这里。
	- Demo：主窗 Accordion「空白右键菜单」按钮 → `CBlankMenuWnd` / `blankmenu.html`（页面只占上部，下方空白右键弹菜单）。

	### 调试日志 `CDuiLog`

	极简文本日志（默认关，关闭时零开销）。定义在 `src/DuiLib/Utils/Utils.h`，导出给应用。

	```cpp
	#include <UIlib.h>
	DuiLib::CDuiLog::SetEnabled(true);              // 开
	DuiLib::CDuiLog::SetLogFile(_T("dui_log.txt")); // 显式指定；不设时默认 D:\\DUIX.log（无 D 盘则 C:\\DUIX.log）
	DuiLib::CDuiLog::Write(_T("msg=%d x=%s"), 1, _T("a"));

	DuiLib::CDuiLog::SetEnabled(false);             // 关回
	bool on = DuiLib::CDuiLog::IsEnabled();
	```

	- `SetEnabled(bool)` / `IsEnabled()`：总开关，默认 `false`。日志行自带时间戳并自动换行。
	- `SetLogFile(LPCTSTR)`：日志文件路径（工程固定 UNICODE，写 **UTF-16(LE)** 字节）。**不调或传 NULL** 时默认写到 `D:\DUIX.log`；无 D 盘则 `C:\DUIX.log`；写不了才退到 `OutputDebugString`。传 `_T("")` 清空 = 回退默认路径。
	- `Write(格式, …)`：printf 风格，无类别；仅当已启用才写，关闭时为空操作。
	- 库内已埋点示例：`WM_CONTEXTMENU` 分支会输出 `[ctxmenu]` 行，排查右键不弹菜单时：`evtRClick=…` 看命中对象、`blank=1/0` 看是否走空白兜底、`target=…` 看发给哪个容器、`SendNotify sent` 看广播是否真实执行。
	- 注意：诊断行不变慢库（关闭时 `Write` 立即返回），但发布版默认应保持关闭。
