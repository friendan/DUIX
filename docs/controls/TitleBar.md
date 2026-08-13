# TitleBar

| | |
|--|--|
| 类 | `CTitleBarUI` |
| XML | `<TitleBar>` |
| 源码 | `src/DuiLib/Control/UITitleBar.*` |
| 继承属性 | 见 [Container.md](Container.md) / [Control.md](Control.md) |

> 本页聚焦 **属性与用法**。盒模型全局约定见 [Attributes.md](Attributes.md)。

无窗自定义标题栏：左侧自由放置内容，右侧固定最小化 / 最大化·还原 / 关闭。默认深色样式，可用 HTML/CSS 覆盖。

### 布局

```
[ left 弹性：使用者自由发挥 ][ min | max|restore | close ]
```

- HTML 直接子控件全部进入 **左侧**（可用 `Spacer` 把部分内容顶到中间）
- 系统按钮固定最右，不可由外部 `Add` 覆盖

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| `title` | 左侧内置标题 Label 文本 |
| `background-color` / `height` / `padding` / `action` | 基类；默认 `action=title` 可拖窗 |

### 非标准

| 属性 | 说明 | 默认 |
|------|------|------|
| `show-min` | 显示最小化 | true |
| `min-to-tray` / `minimize-to-tray` | 点最小化时 **藏窗并摘掉任务栏**（`CTrayIcon::HideWindowFromTaskbar`），需配合 [TrayIcon.md](TrayIcon.md)；默认仍是系统最小化 | false |
| `close-to-tray` | 点关闭 / Alt+F4 / `Close()` 时 **藏窗到托盘**（不销毁）；真正退出用 `WindowImplBase::ForceClose` | false |
| `show-max` | 显示最大化/还原 | true |
| `show-close` | 显示关闭 | true |
| `btn-width` | 系统按钮宽度 | 46 |

系统按钮 name 固定为 `minbtn` / `maxbtn` / `restorebtn` / `closebtn`，与 `WindowImplBase` 最大化/还原显隐切换兼容（`show-max="false"` 时 WinImplBase 不会再强制显示）。按钮无 `action=`，由 TitleBar 自行发系统命令，便于宿主拦截。

图标：启动时用 `EnumFontFamiliesEx` 检测本机是否有 **Segoe Fluent Icons**（优先）或 **Segoe MDL2 Assets**；有则用系统 Chrome 码点（`E921/E922/E923/E8BB`），否则回退到 `─ □ ❐ ✕`。

### 通知

| 类型 | 时机 |
|------|------|
| `titlebarmining` / `titlebarmaxing` / `titlebarclosing` | 执行前；处理里 `CancelNotify()` 可取消 |
| `titlebarmin` / `titlebarmax` / `titlebarclose` | 已执行系统命令后 |

示例（确认关闭）：

```cpp
if( msg.sType == DUI_MSGTYPE_TITLEBARCLOSING ) {
  CTitleBarUI* pBar = static_cast<CTitleBarUI*>(msg.pSender->GetInterface(DUI_CTR_TITLEBAR));
  if( IDCANCEL == MessageBox(...) && pBar ) pBar->CancelNotify();
}
```

### 示例

```xml
<TitleBar name="titlebar" title="My App" height="40"
    show-min="true" show-max="true" show-close="true"
    background-color="#333333FF" action="title">
  <Svg lucide="app-window" width="16" height="16" color="#B4B4BEFF" />
  <Spacer />
  <Edit width="220" height="28" />
</TitleBar>
```

左侧里用 `Spacer` 即可把后面的控件顶到中间或偏右（仍在系统按钮左侧）。

### 最小化 / 关闭到托盘

```xml
<TitleBar name="titlebar" min-to-tray="true" close-to-tray="true" show-min="true" show-close="true" ... />
```

```cpp
// 启动时创建托盘（TitleBar 不替你建）
m_tray.CreateTrayIcon(m_hWnd, IDR_MAINFRAME, _T("My App"));

// 托盘单击还原：普通 ShowWindow / ::ShowWindow 即可（有藏窗标记时自动恢复任务栏）
if( uMsg == UIMSG_TRAYICON ) {
  if( CTrayIcon::DecodeNotifyMsg(wParam, lParam, m_tray.GetNotifyVersion()) == WM_LBUTTONUP )
    ShowWindow(true); // 或 CTrayIcon::ShowWindowOnTaskbar(m_hWnd) 顺带抢前台
}

// 真正退出（绕过 close-to-tray）
ForceClose(0);
```

`min-to-tray` / `close-to-tray` 均走 `HideWindowFromTaskbar`。  
若窗口继承 `WindowImplBase` 且 **未** 自行 `GetTrayIcon().Create…`，库在 `InitWindow` 之后会 `EnsureAutoTray()`：自动创建托盘图标，左键显隐主窗，右键默认菜单「显示主窗口 / 退出」（退出走 `ForceClose`）。  
应用已 `Create` 托盘时不会覆盖，可继续自写右键菜单（见 duidemo 主窗）。  
还原：普通 `ShowWindow(true)` 即可自动恢复任务栏。

CSS 可用 `#titlebar`、`#closebtn:hover { background-color: ... }` 覆盖默认色。
