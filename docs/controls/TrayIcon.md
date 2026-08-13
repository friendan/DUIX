# TrayIcon

| | |
|--|--|
| 类 | `CTrayIcon` |
| 源码 | `src/DuiLib/Utils/TrayIcon.*` |
| 头文件 | `UIlib.h` → `Utils/TrayIcon.h` |
| 回调消息 | 默认 `UIMSG_TRAYICON`（见 [Messages.md](Messages.md)） |
| Demo | `src/Demos/duidemo/MainWnd.*` |

系统托盘封装（`Shell_NotifyIcon`）。**不是**皮肤控件，由窗口代码持有。

---

## 快速用法

```cpp
// 未指定图标：自动用程序/窗口 ICO
m_tray.Create(m_hWnd, 1, _T("我的应用"));
m_tray.CreateTrayIcon(m_hWnd, 0, _T("我的应用"));

// 指定资源（失败也会回退程序图标）
m_tray.CreateTrayIcon(m_hWnd, IDR_MAINFRAME, _T("我的应用"));
m_tray.Create(m_hWnd, 1, IDI_APPICON, _T("我的应用"));
m_tray.CreateFromFile(m_hWnd, 1, _T("skin\\app.ico"), _T("我的应用"));

// 可选：现代回调布局（改后须用 DecodeNotifyMsg 解析）
m_tray.SetNotifyVersion(NOTIFYICON_VERSION_4);
```

图标回退顺序：`WM_GETICON` → 窗口类图标 → `ExtractIconEx(exe)`。也可 `SetIconFromApplication()`。

窗口消息：

```cpp
LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if( uMsg == UIMSG_TRAYICON ) {
        UINT ev = CTrayIcon::DecodeNotifyMsg(wParam, lParam, m_tray.GetNotifyVersion());
        POINT pt = CTrayIcon::DecodeNotifyPos(wParam, lParam, m_tray.GetNotifyVersion());
        if( ev == WM_LBUTTONUP ) { /* 显隐主窗 */ }
        else if( ev == WM_RBUTTONUP ) { /* 在 pt 弹 CMenuWnd */ }
        else if( ev == NIN_BALLOONUSERCLICK ) { /* 点了气球 */ }
        bHandled = TRUE;
        return 0;
    }
    if( uMsg == CTrayIcon::GetTaskbarCreatedMsg() ) {
        m_tray.Recreate(); // explorer 重启后托盘会丢
        bHandled = TRUE;
        return 0;
    }
    return 0;
}
```

未调用 `SetNotifyVersion` 时保持**旧回调布局**（`lParam` 直接为 `WM_LBUTTONUP` 等），旧代码仍可 `(UINT)lParam` 判断；新代码建议一律走 `DecodeNotifyMsg`。

---

## 创建 / 销毁

| 方法 | 说明 |
|------|------|
| `CreateTrayIcon(hWnd, iconResId=0, tip, msg=0)` | 兼容旧接口；`iconResId=0` 或加载失败 → 程序图标 |
| `Create(hWnd, uId=1, tip, msg)` | **推荐无图标创建**：自动程序 ICO |
| `Create(hWnd, uId, hIcon, tip, msg, bOwnIcon)` | `hIcon=NULL` 时回退程序图标 |
| `Create(hWnd, uId, iconResId, tip, msg, hInst)` | 资源；失败回退程序图标 |
| `CreateFromFile(hWnd, uId, icoPath, tip, msg)` | `.ico`；失败回退程序图标 |
| `DeleteTrayIcon` / `RemoveIcon` | `NIM_DELETE` 并释放自持有图标 |
| `Recreate` | 任务栏重建后重新 `NIM_ADD` |

`msg=0` → `UIMSG_TRAYICON`。自定义消息须落在应用号段（`WM_DUILIB_USER + n`），见 [Messages.md](Messages.md)。

---

## 图标 / 提示 / 显隐

| 方法 | 说明 |
|------|------|
| `SetIcon(HICON, bOwnIcon)` / 文件 / 资源 ID | 修改托盘图标 |
| `SetIconFromApplication` | 换成程序/窗口图标 |
| `LoadApplicationIcon` | 静态：取出程序图标（调用方 `DestroyIcon`） |
| `HideWindowFromTaskbar` | 静态：藏窗并摘掉任务栏按钮 |
| `RestoreWindowToTaskbarIfNeeded` | 静态：有藏窗标记则恢复样式/任务栏（幂等；Show 时库会自动调） |
| `ShowWindowOnTaskbar` | 静态：Restore + Show + 可选前台 |
| `IsWindowHiddenFromTaskbar` | 静态：是否处于上述隐藏态 |
| `GetIcon` | 当前 `HICON` |
| `SetTooltipText` / `GetTooltipText` | 悬停提示 |
| `Show` / `Hide` | 显示/隐藏（`NIF_STATE`；失败则删再建） |
| `SetShowIcon` / `SetHideIcon` | 同上（旧名） |
| `IsEnabled` / `Enabled` | 是否已 `NIM_ADD` |
| `IsVisible` | 已启用且未 Hide |

---

## 气球通知

```cpp
m_tray.ShowBalloon(_T("完成"), _T("导出成功"), NIIF_INFO, 8000);
m_tray.ShowBalloon(_T("警告"), _T("磁盘不足"), NIIF_WARNING);
m_tray.HideBalloon();
```

| 参数 | 说明 |
|------|------|
| `dwInfoFlags` | `NIIF_INFO` / `WARNING` / `ERROR` / `NONE`；自定义图标加 `NIIF_USER`（可再加 `NIIF_LARGE_ICON`） |
| `uTimeoutMs` | 系统可能忽略，仅作提示 |
| `hBalloonIcon` | 可选；非空时自动带 `NIIF_USER` |

点击气球：`NIN_BALLOONUSERCLICK`（需正确解析回调；VERSION_4 更稳）。

---

## 回调版本

| 方法 | 说明 |
|------|------|
| `SetNotifyVersion(NOTIFYICON_VERSION_4)` | `NIM_SETVERSION` |
| `GetNotifyVersion` | 当前版本；`0` 表示未设置（按旧布局解码） |
| `DecodeNotifyMsg` | 取出 `WM_*` / `NIN_*` |
| `DecodeNotifyIconId` | 图标 `uId` |
| `DecodeNotifyPos` | VERSION_4 为点击屏幕坐标；旧版 `GetCursorPos` |

---

## 任务栏重启

```cpp
if( uMsg == CTrayIcon::GetTaskbarCreatedMsg() )
    m_tray.Recreate();
```

---

## 与 TitleBar to-tray 自动托盘

继承 `WindowImplBase` 时：皮肤里 `min-to-tray` / `close-to-tray` 为 true，且 **InitWindow 里没有** 自行 `Create`，则 `EnsureAutoTray()` 会：

1. `Create` 托盘（图标=程序 ICO，提示=窗口标题）
2. 左键：显隐主窗（`HideWindowFromTaskbar` / `ShowWindowOnTaskbar`）
3. 右键：默认菜单「显示主窗口」「退出」（`ForceClose`）
4. 处理 `TaskbarCreated` → `Recreate`

应用要自定义菜单时，在 `InitWindow` 里自己 `GetTrayIcon().Create…`（此时不会走自动逻辑），并在 `HandleCustomMessage` 里处理 `UIMSG_TRAYICON`（主窗 Demo 即此方式）。

---

## 注意

- 右键菜单库不内置自定义项：自动托盘仅有上述两项；完全自定义请自行 `CMenuWnd`。`ResizeMenu` 会按工作区钳制。
- 藏主窗请用 `HideWindowFromTaskbar`。还原用普通 `ShowWindow(true)` 即可。
- 托盘图标建议多尺寸 `.ico`；仅 256 大图在托盘可能糊。
- `Create`/`SetIcon` 传入外部 `HICON` 且 `bOwnIcon=false` 时，生命周期由调用方负责。
- 不要占用 `WM_APP+0…+0x1FE` 作自定义回调（库号段）。
