# Modal / MessageBox

确认 / 提示对话框：独立弹出 HWND，kind 色标题栏，半透明遮罩。

| | |
|--|--|
| 异步 | `CModal`、`CModalOptions`（callback，不阻塞消息泵） |
| 同步 | `CMessageBox`（阻塞到关闭，**默认无需皮肤**） |
| 窗口 | `CModalWnd` + `CModalBackdropWnd`（内部） |
| 头文件 | `UIModal.h`、`UIMessageBox.h` |
| Demo | Accordion → Modal；「铺满设置窗」测 `WindowImplBase` 同步；退出确认等走 `CMsgWnd` → `CMessageBox` |

参考 EZUI `Modal.cpp`（异步 `Show` + callback）。同步场景用 `CMessageBox::Show`，不必再带 `msg.html`。

---

## 同步 MessageBox（推荐通用用法）

默认纯代码 UI，**不需要** html / `XML_MSG`：

```cpp
if( MESSAGEBOX_OK == CMessageBox::Show(m_hWnd, _T("提示"), _T("确定退出？")) )
    Close();

// 仅确定
CMessageBox::ShowInfo(m_hWnd, _T("提示"), _T("保存成功"));

// 可选：自定义皮肤（资源 id / 路径 / 内联 XML 以 '<' 开头）
CMessageBox::ShowSkin(m_hWnd, _T("提示"), _T("自定义外观"), _T("XML_MSG"));
CMessageBox::ShowSkin(m_hWnd, _T("提示"), _T("内置默认 XML"), NULL); // skin 空 = 内置模板
```

| 方法 | 说明 |
|------|------|
| `CMessageBox::Show(owner, title, text)` | 确定 + 取消；返回 `MESSAGEBOX_OK` / `MESSAGEBOX_CANCEL` |
| `CMessageBox::Show(..., opts)` | 可配 kind / 按钮文案 / 尺寸；仍阻塞 |
| `CMessageBox::ShowInfo(...)` | 仅确定 |
| `CMessageBox::ShowSkin(..., skin)` | 可选 html/xml 皮肤 |

Demo 里 `CMsgWnd::MessageBox` 已转发到 `CMessageBox::Show`；旧皮肤可用 `CMsgWnd::MessageBoxSkin`。

---

## 异步 Modal 示例

```cpp
static void CALLBACK OnModalResult(bool ok, LPCTSTR /*data*/, void* /*user*/)
{
    if( ok ) CToast::ShowSuccess(_T("已确认"));
    else     CToast::ShowInfo(_T("已取消"));
}

CModal::ShowInfo(_T("这是一条提示"));

CModal::ShowSuccess(_T("操作已成功完成。"), OnModalResult);

CModal::Confirm(_T("删除确认"), _T("确定删除该文件吗？此操作不可恢复。"),
    OnModalResult);

CModal::Show(_T("保存失败"), _T("磁盘空间不足，请清理后重试。"),
    CModalOptions()
        .Kind(CONTROLKIND_DANGER)
        .ShowCancel(true)
        .OkText(_T("重试"))
        .CancelText(_T("放弃"))
        .Owner(m_hWnd)
        .OnResult(OnModalResult));

// 点遮罩不关：只能确定 / Esc
CModal::Show(_T("点遮罩不关"), _T("只能点确定或 Esc。"),
    CModalOptions()
        .ClickBackdropToClose(false)
        .Owner(m_hWnd)
        .OnResult(OnModalResult));

// 不需要跟主窗：关掉同步
CModal::Show(_T("不同步"), _T("只动对话框，主窗留在原地。"),
    CModalOptions()
        .Owner(m_hWnd)
        .SyncOwnerMove(false)
        .OnResult(OnModalResult));
```

`OnResult`：`ok=true` 点确定 / Enter；`false` 点取消 / Esc / 关窗 / 点遮罩（若允许）/ `Dismiss`。

---

## API

| 方法 | 说明 |
|------|------|
| `CModal::Show(text, opts)` | 正文；标题默认「提示」 |
| `CModal::Show(title, text, opts)` | 标题 + 正文 |
| `CModal::ShowSuccess/Danger/Warning/Info` | 快捷 kind，仅确定按钮 |
| `CModal::Confirm(title, text, fn)` | 确定 + 取消，Primary |
| `CModal::Dismiss(h)` | 关闭（等同取消） |

`Show*` / `Confirm` 返回 Modal 的 `HWND`，可供 `Dismiss`。

### CModalOptions

| 方法 | 默认 | 说明 |
|------|------|------|
| `Title` / `Text` | 标题默认「提示」 | |
| `Kind` | `PRIMARY` | 标题栏与确定按钮配色 |
| `ShowCancel` | false | 是否显示取消按钮 |
| `OkText` / `CancelText` | 确定 / 取消 | |
| `Width` / `Height` | 420 / 200 | |
| `ClickBackdropToClose` | true | 点击半透明遮罩 → 取消 |
| `Owner` | 前台窗 | 对齐显示器 + 禁用该窗；**Create 不挂 HWND Owner** |
| `SyncOwnerMove` | true | 拖 Modal 时同步移动 Owner（屏幕坐标相对偏移，支持多显示器/负坐标副屏；不同步大小；最大化/最小化 Owner 时跳过）。不需要时 `.SyncOwnerMove(false)`。**带皮肤的业务模态窗请用 `WindowImplBase::SetSyncOwnerMove`，见 [Window.md](Window.md)** |
| `OnResult(fn, user)` | null | `ok` + `UserData` |
| `UserData` | | 传给 OnResult |

---

## 行为说明

- 水平居中，垂直约在工作区上方 1/3（同 EZUI）
- 标题栏 / 正文区 `action="title"` 可拖动；Esc 取消，Enter 确定
- `SyncOwnerMove` 默认开启：拖动时按打开时屏幕坐标偏移同步移动 Owner（含跨显示器）；不需要时 `.SyncOwnerMove(false)`
- 半透明遮罩覆盖 **Modal 当前所在显示器** 工作区；跨屏拖动或 `WM_DISPLAYCHANGE` 时会重铺；关闭时恢复 Owner 启用与焦点
- 结果回调在窗口销毁前同步触发；勿在回调里再同步嵌套阻塞 UI
- 对话框：`WS_POPUP | WS_EX_TOOLWINDOW | WS_EX_TOPMOST` + 分层圆角
- 遮罩：`WS_EX_LAYERED`，**不用 TOPMOST**（避免盖住对话框的分层 Present）

---

## 外观与实现注意

### 圆角分层

- Modal 关闭分层 DComp（`SetLayeredCompositionEnabled(false)`），走 BitmapRT + `UpdateLayeredWindow`
- Modal **关闭阴影**（`GetShadow()->ShowShadow(false)`）：Shadow 用 `CreateRoundRectRgn` 镂空，锯齿洞会叠在分层 AA 圆角透明边上，表现为「首帧圆角锯齿、拖一下才好」
- **窗口底色必须为 0**（`SetWindowBackgroundColor(0)`），且须在 `AttachDialog` **之前**设好 custom
- **根控件底色也须为 0**：正文/按钮行再铺 `color-modal-bg`
- 标题栏不要 `SetKind`；手动画 kind 色并保持直角
- Present 前 `ApplyRoundCornerMask` 对分层位图做 AA 圆角遮罩

### 阴影与圆角

- `CShadowUI::MakeShadow`：优先 `GetWindowRgn`；无 RGN 时跟 `BorderRadius`
- **分层 + BorderRadius**：不用 `CreateRoundRectRgn`（锯齿洞会叠在 AA 透明边上）；外沿数学圆角模糊，洞矩形外扩清干净
- Modal：`SetBorderRadius` + 默认开阴影；**不要** `SetWindowRgn`
- 新弹出窗若圆角且要阴影：设 `SetBorderRadius`（或 `SetWindowRgn`），勿只靠矩形默认外形

### Owner / 焦点 / 遮罩

- 勿把主窗设为 Create 的 HWND Owner（D2D 脏区裂开）；用 `Owner()` 仅作禁用 / 对齐基准
- `ClickBackdropToClose(false)` 时仍须保证对话框在遮罩之上；必要时 `RaiseAboveBackdrop`
- 关闭后 `EnableOwner(true)` 并抢回前台焦点，否则主窗可能一直无焦点

### 构建

- 新加 `UIModal.cpp` 后需重新跑一次 `build_clang_ninja_*_init.bat`（`aux_source_directory`）

---

## 相关源码

| 文件 | 说明 |
|------|------|
| `src/DuiLib/Control/UIModal.h/.cpp` | Modal API 与窗口 |
| `src/DuiLib/Control/UIMessageBox.h/.cpp` | 同步 MessageBox |
| `bin/skin/duidemo/msg.html` | 可选自定义皮肤示例（`ShowSkin` / `XML_MSG`） |
| `src/DuiLib/Core/UIContainer.cpp` | 圆角裁剪含子控件 |
| `src/DuiLib/Utils/UIShadow.*` | 阴影；无 RGN 时跟 RoundCorner |
| `src/Demos/duidemo/MainWnd.cpp` | Demo 按钮 |
| `bin/skin/duidemo/main.html` | Accordion → Modal |
