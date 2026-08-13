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

```cpp
if( uMsg == UIMSG_TRAYICON ) { ... }
if( uMsg == UIMSG_MENUCLICK ) {
    MenuCmd* pCmd = (MenuCmd*)wParam;
    // ...
    m_pm.DeletePtr(pCmd);
}
```

库要加新 HWND 消息：在 `MSGTYPE_UI` 里 `UIMSG_ASYNC_NOTIFY` 之后追加，并改 `UIMSG__LIB_LAST`，且不得超过 `WM_DUILIB_MSG_LAST`。
