# Modal

确认 / 提示对话框：独立弹出 HWND，kind 色标题栏，半透明遮罩，异步结果回调（不阻塞消息泵）。

| | |
|--|--|
| 类 | `CModal`、`CModalOptions` |
| 窗口 | `CModalWnd` + `CModalBackdropWnd`（内部） |
| 头文件 | `UIModal.h` |
| Demo | Accordion → Modal |

参考 EZUI `Modal.cpp`（异步 `Show` + callback；非阻塞 MessageBox）。

---

## 最小示例

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
| `OnResult(fn, user)` | null | `ok` + `UserData` |
| `UserData` | | 传给 OnResult |

---

## 行为说明

- 水平居中，垂直约在工作区上方 1/3（同 EZUI）
- 标题栏 / 正文区 `action="title"` 可拖动；Esc 取消，Enter 确定
- 半透明遮罩覆盖当前显示器工作区；关闭时恢复 Owner 启用与焦点
- 结果回调在窗口销毁前同步触发；勿在回调里再同步嵌套阻塞 UI
- 对话框：`WS_POPUP | WS_EX_TOOLWINDOW | WS_EX_TOPMOST` + 分层圆角
- 遮罩：`WS_EX_LAYERED`，**不用 TOPMOST**（避免盖住对话框的分层 Present）

---

## 外观与实现注意

### 圆角分层

- 根布局 `SetBorderRound` + `SetLayered(true)`，角外透明；**不要**对 Modal 用 `SetWindowRgn`（与分层 AA 圆角冲突）
- `CContainerUI::DoPaint` 在有 `BorderRound` 时用圆角裁剪包住子控件，否则子控件直角会盖住透明角
- 标题栏 `SetKind` 后会带上按钮级 `BorderRound`，须再 `SetBorderRound({0,0})`，由根节点统一裁剪

### 阴影与圆角

- `CShadowUI::MakeShadow`：优先 `GetWindowRgn`；无 RGN 时用 `PaintManager::GetBorderRadius()` 建圆角区域再模糊
- Modal：`SetBorderRadius` + 默认开阴影；**不要**再 `SetWindowRgn`（与分层 AA 圆角冲突）
- 新弹出窗若圆角且要阴影：设 `SetBorderRadius(cx,cy)`（或 `SetWindowRgn`），勿只靠矩形默认外形

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
| `src/DuiLib/Core/UIContainer.cpp` | 圆角裁剪含子控件 |
| `src/DuiLib/Utils/UIShadow.*` | 阴影；无 RGN 时跟 RoundCorner |
| `src/Demos/duidemo/MainWnd.cpp` | Demo 按钮 |
| `bin/skin/duidemo/main.html` | Accordion → Modal |
