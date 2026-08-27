# HWND 自定义消息

DuiLib 窗口上的自定义 `UINT` 消息号。定义在 `src/DuiLib/Core/UIManager.h`。

比较、投递时用宏名，**不要写死** `WM_USER+1`、`WM_APP+1` 这类数字。

---

## 范围

Windows 约定：`WM_USER` = `0x0400`，`WM_APP` = `0x8000`。

| 谁 | 宏 | 数值 | 用法 |
|----|----|------|------|
| **DuiLib 库** | `WM_DUILIB_MSG_FIRST` … `WM_DUILIB_MSG_LAST` | `0x8000` … `0x81FE`（`WM_APP+0` … `+0x1FE`） | 只用下面的 `UIMSG_*`，外部不要占用 |
| **应用程序** | `WM_DUILIB_USER` | `0x0400` 起，整段 `WM_USER` | `WM_DUILIB_USER + 0`、`+1`、`+2` … |

```
0x0400  WM_DUILIB_USER          ─┐
        +0, +1, +2 …             ├ 应用程序
0x7FFF  WM_USER 末               ─┘

0x8000  WM_DUILIB_MSG_FIRST     ─┐
        UIMSG_TRAYICON 等        ├ DuiLib 库（外部不要用 WM_APP+n）
0x81FE  WM_DUILIB_MSG_LAST      ─┘
```

`DUILIB_IS_LIB_MSG(uMsg)` 为真表示落在库号段。

---

## 应用程序怎么用

```cpp
#define WM_MY_SAVE   (WM_DUILIB_USER + 0)
#define WM_MY_RELOAD (WM_DUILIB_USER + 1)
#define WM_MY_FOO    (WM_DUILIB_USER + 2)

::PostMessage(hWnd, WM_MY_SAVE, 0, 0);

// 窗口里处理（HandleCustomMessage / HandleMessage）
LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if( uMsg == WM_MY_SAVE ) { /* ... */ return 0; }
    bHandled = FALSE;
    return 0;
}
```

不要：`#define WM_MY_FOO (WM_APP + 1)`（会撞库的 `UIMSG_SET_DPI`）。

---

## 库消息（只读，业务按名处理）

| 枚举 | 含义 |
|------|------|
| `UIMSG_TRAYICON` | 托盘回调（配合 [TrayIcon.md](TrayIcon.md) / `CTrayIcon`） |
| `UIMSG_SET_DPI` | DPI 已更新 |
| `UIMSG_MENUCLICK` | 菜单项点击（旧名 `WM_MENUCLICK` 仍可用） |
| `UIMSG_ASYNC_NOTIFY` | 异步 Notify / DelayedCleanup（内部） |
| `UIMSG_LOADING_TICK` | [Loading](Loading.md) 帧动画：`CreateTimerQueueTimer` → `PostMessage`，`wParam` = `CLoadingUI*` |
| `UIMSG_RING_TICK` | [Ring](Ring.md) 位图旋转：同上，`wParam` = `CRingUI*` |
| `UIMSG_SKELETON_TICK` | [Skeleton](Skeleton.md) 扫光：`wParam` = `CSkeletonUI*` |
| `UIMSG_GIFANIM_TICK` | [GifAnim](GifAnim.md) GIF 帧：`wParam` = `CGifAnimUI*` |
| `UIMSG_GIFANIMEX_TICK` | GifAnimEx（`USE_XIMAGE_EFFECT`）：`wParam` = `CGifAnimExUI*` |
| `UIMSG_SCROLLBAR_TICK` | [ScrollBar](ScrollBar.md) 拖滑/长按重复滚动：`wParam` = `CScrollBarUI*` |
| `UIMSG_CAROUSEL_TICK` | [Carousel](Carousel.md) 自动轮播：`wParam` = `CCarouselUI*` |
| `UIMSG_ANIMATION_TICK` | `CUIAnimation` mixin（[SidePanel](SidePanel.md) / FadeButton / AnimationTabLayout）：`wParam` = 宿主控件*，`lParam` = 动画 ID |
| `UIMSG_ROLLTEXT_TICK` | [RollText](RollText.md) 滚动帧 / 超时：`wParam` = `CRollTextUI*`，`lParam` = 定时 ID |
| `UIMSG_RICHEDIT_TICK` | [RichEdit](RichEdit.md) 插入符 / `TxSetTimer`：`wParam` = `CRichEditUI*`，`lParam` = 定时 ID |

```cpp
if( uMsg == UIMSG_TRAYICON ) { ... }
if( uMsg == UIMSG_MENUCLICK ) {
    MenuCmd* pCmd = (MenuCmd*)wParam;
    // ...
    m_pm.DeletePtr(pCmd);
}
```

库要加新 HWND 消息：在 `MSGTYPE_UI` 里 `UIMSG_CAROUSEL_TICK`（或当前 `UIMSG__LIB_LAST`）之后追加，并改 `UIMSG__LIB_LAST`，且不得超过 `WM_DUILIB_MSG_LAST`。

---

## Shadow 子类化与控件动画定时器（硬约束）

### 现象

- 带 **窗口阴影** 的普通主窗（`WindowImplBase` + `AttachDialog` → `CShadowUI::Create`）上，控件调 `CPaintManagerUI::SetTimer` 返回成功，但 **`WM_TIMER` 到不了** `CPaintManagerUI::MessageHandler`，`DoEvent(UIEVENT_TIMER)` 永不触发。
- 表现：Loading / Ring 等 **角度恒为 0、画面静止**；`Start()` / `Paint` 正常，唯独没有定时 Tick。
- duidemo（Win10 `10.0.18363`）Accordion「Loading」面板曾完整复现；排查日志见根目录 AGENTS.md「调试日志」约定（仅写文件）。

### 根因

`AttachDialog` 会 `SetWindowLongPtr(GWLP_WNDPROC, ParentProc)` 子类化绘制 HWND（见 [Window.md](Window.md) 阴影行 / `UIShadow.cpp`）。在此环境下，Win32 **`SetTimer` / `WM_TIMER` 与 PaintManager 的派发链脱节**（Toast 源码注释：*Shadow 子类化会干扰宿主定时器投递*）。

`CPaintManagerUI::SetTimer` 按 `(pSender, nLocalID)` 区分控件本地 ID，**改本地 ID 不能修复**；问题在 HWND 消息到不了库，而非 ID 冲突。

### 正确做法（库内约定）

**帧动画 / 周期刷新不要依赖 `m_pManager->SetTimer` + `UIEVENT_TIMER`**。与 [Toast.md](Toast.md) 相同：

1. `CreateTimerQueueTimer(..., WT_EXECUTEDEFAULT)` 在线程池回调里 **`PostMessage(hWndPaint, UIMSG_*, (WPARAM)pControl, 0)`**（必须 Post 回 UI 线程，回调里不要直接改控件）。
2. 在 `CPaintManagerUI::MessageHandler` 增加 `case UIMSG_*`，调用控件 `OnAnimTick()`（内部 `TickFrame` / 改角度 + `Invalidate()`）。
3. 隐藏 / 析构 / 换图：`DeleteTimerQueueTimer(..., INVALID_HANDLE_VALUE)` 停表。
4. 新消息号：在本文件 `MSGTYPE_UI` 追加，`UIMSG__LIB_LAST` 随之更新。

| 控件 | 消息 | 间隔 | 源码 |
|------|------|------|------|
| Loading | `UIMSG_LOADING_TICK` | XML `time`（默认 16ms） | `UILoading.cpp` |
| Ring | `UIMSG_RING_TICK` | 100ms，+36°/帧 | `UIRing.cpp` |
| Skeleton | `UIMSG_SKELETON_TICK` | 40ms 扫光 | `UISkeleton.cpp` |
| GifAnim | `UIMSG_GIFANIM_TICK` | GIF 帧延迟（`WT_EXECUTEONLYONCE` 逐帧重排） | `UIGifAnim.cpp` |
| GifAnimEx | `UIMSG_GIFANIMEX_TICK` | 帧延迟（需 `USE_XIMAGE_EFFECT`） | `UIGifAnimEx.cpp` |
| ScrollBar | `UIMSG_SCROLLBAR_TICK` | 50ms 重复滚动 | `UIScrollBar.cpp` |
| Carousel | `UIMSG_CAROUSEL_TICK` | XML `interval`（默认 5000ms） | `UICarousel.cpp` |
| SidePanel / FadeButton / AnimationTabLayout | `UIMSG_ANIMATION_TICK` | 各动画 `elapse`（经 `CUIAnimation`） | `UIAnimation.cpp` |
| RollText | `UIMSG_ROLLTEXT_TICK` | `roll-interval` + 可选 `roll-duration` | `UIRollText.cpp` |
| RichEdit | `UIMSG_RICHEDIT_TICK` | `GetCaretBlinkTime()` + `TxSetTimer` | `UIRichEdit.cpp` |
| Toast | 私有 `WM_APP+…`（非 `UIMSG_*`） | 200ms 倒计时 | `UIToast.cpp` |

### 仍用 `SetTimer` 的控件（风险）

WebBrowser resize hook 等仍走 `SetTimer`。原生 `WC_EDIT`（[Edit](Edit.md)）插入符由系统 + Present 侧 `ExcludeClipRect`/`RedrawWindow` 处理，不走 RichEdit 定时器。

### 新做动画控件 checklist

- [ ] 不用 `SetTimer` 驱动每帧逻辑（除非明确只跑在无 Shadow 的子窗，且已实测）。
- [ ] TimerQueue → 新 `UIMSG_*` → `UIManager` 派发 → `OnAnimTick()`。
- [ ] `SetInternVisible(false)` / 析构里停表。
- [ ] 临时排查日志 **只写文件**（`bin/<topic>_debug.log`），见 [AGENTS.md](../../AGENTS.md)；定位完拆除。
