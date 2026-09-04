# Modal / MessageBox / InputBox

确认 / 提示 / 输入对话框：独立弹出 HWND。

| | |
|--|--|
| 异步确认 | `CModal`、`CModalOptions`（callback，不阻塞消息泵） |
| 同步确认 | `CMessageBox`（阻塞到关闭，**默认无需皮肤**） |
| 同步输入 | `CInputBox`、`CInputBoxOptions`（阻塞；确定后写回文本） |
| 同步快捷键 | `CHotKeyBox`、`CHotKeyBoxOptions`（阻塞；写出 vk/修饰键与显示名） |
| 窗口 | `CModalWnd` + `CModalBackdropWnd`（内部）；`CInputBoxWnd` / `CHotKeyBoxWnd`（内部） |
| 头文件 | `UIModal.h`、`UIMessageBox.h`、`UIInputBox.h`、`UIHotKeyBox.h` |
| Demo | Accordion → Modal（含 InputBox / HotKeyBox）；「铺满设置窗」测 `WindowImplBase` 同步；退出确认等走 `CMsgWnd` → `CMessageBox` |

参考 EZUI `Modal.cpp`（异步 `Show` + callback）。同步场景用 `CMessageBox::Show` / `CInputBox::Show`，不必再带皮肤文件。

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

## 同步 InputBox（输入对话框）

| | |
|--|--|
| 类 | `CInputBox` / `CInputBoxOptions` |
| 头文件 | `src/DuiLib/Control/UIInputBox.h`（`UIlib.h` 已包含） |
| 源码 | `src/DuiLib/Control/UIInputBox.cpp` |
| Demo | Accordion → Modal →「InputBox 改文本」「密码输入」 |

内置皮肤（TitleBar + 提示 Label + Edit + 确定/取消），**无需工程 html**。与 `CMessageBox` 一样是同步阻塞对话框。

### 行为

| 操作 | 返回值 | `outText` |
|------|--------|-----------|
| 确定 / 输入框回车 | `INPUTBOX_OK`（`1`） | 写入当前输入内容 |
| 取消 / Esc / 关窗 | `INPUTBOX_CANCEL`（`0`） | **不修改**（保持调用前内容） |

- 打开后自动聚焦 Edit；`SelectAll(true)`（默认）时全选初始文本
- Edit 使用控件默认 `padding="4,10,4,10"`（左右 10，文字不贴边）；勿在内嵌皮肤里写 `padding="…,0,…,0"` 把左右清零
- 窗体默认逻辑尺寸 `420×180`（随 DPI 缩放）

### 最小用法

```cpp
#include "UIlib.h"   // 已含 UIInputBox.h

CDuiString sText;
if( INPUTBOX_OK == CInputBox::Show(m_hWnd, _T("修改文本"), _T("请输入按钮文字："), sText) ) {
	// sText 为用户输入（本重载初始值为空）
}

// 带初始值（打开后默认全选）
CDuiString sName = _T("确定");
if( INPUTBOX_OK == CInputBox::Show(m_hWnd, _T("修改文本"), _T("请输入按钮文字："),
		sName.GetData(), sName) ) {
	btn->SetText(sName.GetData());
}
```

### 链式配置

```cpp
CDuiString pwd;
if( INPUTBOX_OK == CInputBox::Show(m_hWnd,
		CInputBoxOptions()
			.Title(_T("请输入密码"))
			.Prompt(_T("密码将用于登录验证"))
			.Placeholder(_T("至少 6 位"))
			.Password(true)
			.MaxLength(32)
			.OkText(_T("登录"))
			.CancelText(_T("取消"))
			.Width(420)
			.Height(180)
			.SelectAll(false),
		pwd) ) {
	// 使用 pwd
}

// 仅数字
CDuiString num;
CInputBox::Show(m_hWnd,
	CInputBoxOptions()
		.Title(_T("请输入数量"))
		.Prompt(_T("仅允许 0–9"))
		.Number(true)
		.MaxLength(9),
	num);
```

### API

| 方法 | 说明 |
|------|------|
| `CInputBox::Show(HWND owner, LPCTSTR title, LPCTSTR prompt, CDuiString& outText)` | 初始值为空；返回 `INPUTBOX_OK` / `INPUTBOX_CANCEL` |
| `CInputBox::Show(HWND owner, LPCTSTR title, LPCTSTR prompt, LPCTSTR defaultValue, CDuiString& outText)` | 带初始值 |
| `CInputBox::Show(HWND owner, const CInputBoxOptions& opts, CDuiString& outText)` | 完整配置；`owner` 优先于 opts 里的 `Owner` |

### CInputBoxOptions（链式，均可省略）

| 方法 | 默认 | 说明 |
|------|------|------|
| `Title(LPCTSTR)` | `输入` | 标题栏文字 |
| `Prompt(LPCTSTR)` | 空 | 输入框上方说明；空则隐藏该行 |
| `Value(LPCTSTR)` | 空 | 初始文本 |
| `Placeholder(LPCTSTR)` | 空 | Edit 占位提示 |
| `Password(bool)` | `false` | 密码模式 |
| `Number(bool)` | `false` | 仅允许数字（`ES_NUMBER`，不含小数点/负号；小数/步进请用 [Spin](Spin.md)） |
| `MaxLength(UINT)` | `0` | 最大字符数；`0`=不限制 |
| `OkText(LPCTSTR)` / `CancelText(LPCTSTR)` | `确定` / `取消` | 按钮文案 |
| `Width(int)` / `Height(int)` | `420` / `180` | 逻辑像素（随 DPI） |
| `SelectAll(bool)` | `true` | 打开后是否全选初始文本 |
| `Owner(HWND)` | `Show` 的 `owner` 参数 | 对齐/禁用基准窗；一般不必再设 |

### 返回常量

| 常量 | 值 | 含义 |
|------|----|------|
| `INPUTBOX_OK` | `1` | 确定 |
| `INPUTBOX_CANCEL` | `0` | 取消 |

---

## 同步 HotKeyBox（快捷键对话框）

| | |
|--|--|
| 类 | `CHotKeyBox` / `CHotKeyBoxOptions` |
| 头文件 | `src/DuiLib/Control/UIHotKeyBox.h`（`UIlib.h` 已包含） |
| 源码 | `src/DuiLib/Control/UIHotKeyBox.cpp` |
| 控件 | 内嵌 [HotKey](HotKey.md) |
| Demo | Accordion → Modal →「HotKeyBox」；Button `edit-hotkey` 右键「设置快捷键」 |

内置皮肤（TitleBar + 提示 + **只读** HotKey + 程序/全局 [Segmented](Segmented.md) + 确定/取消），**无需工程 html**。对话框打开后**全局捕获按键**（不必先点 HotKey）；Esc 取消、Enter 确定、Backspace/Delete 清除。对话框只负责选择；**不**调用 `RegisterHotKey` / 不注册加速键。

### 行为

| 操作 | 返回值 | 输出 |
|------|--------|------|
| 确定 | `HOTKEYBOX_OK` | 写入输出 |
| 确定但冲突 | `HOTKEYBOX_CANCEL` | 提示后**关闭**对话框，**不写出**（设置无效） |
| 取消 / Esc / 关窗 | `HOTKEYBOX_CANCEL` | **不修改**输出参数 |

冲突检测（默认开）：自定义回调 → `AddReserved` → 扫描 `ConflictManager` 控件树（`Button::SetShortcutKey`、控件 `shortcut`/Alt 字母）。编辑自身请 `.ExcludeControl(this)`。

| `HotKeyBoxScope` | 值 | 说明 |
|------------------|----|------|
| `HOTKEYBOX_SCOPE_APP` | 0 | 程序快捷键（默认） |
| `HOTKEYBOX_SCOPE_GLOBAL` | 1 | 全局快捷键 |

显示名可用 `CHotKeyUI::FormatHotKeyName(vk, mod)`。冲突查找：`CHotKeyUI::FindShortcutConflict`。

### 最小用法

```cpp
WORD vk = 0, mod = 0;
int scope = HOTKEYBOX_SCOPE_APP;
CDuiString sDisp;
if( HOTKEYBOX_OK == CHotKeyBox::Show(m_hWnd,
		CHotKeyBoxOptions()
			.Title(_T("设置快捷键"))
			.Prompt(_T("请按下快捷键组合："))
			.ConflictManager(&m_pm)
			.ExcludeControl(pBtn)           // 编辑已有按钮时排除自身
			.AddReserved(VK_F4, HOTKEYF_ALT, _T("关闭窗口")),
		vk, mod, &sDisp, &scope) ) {
	// scope == HOTKEYBOX_SCOPE_GLOBAL 时：
	// ::RegisterHotKey(hWnd, id, CHotKeyUI::HotKeyToRegisterMods(mod), vk);
}
```

### CHotKeyBoxOptions

| 方法 | 默认 | 说明 |
|------|------|------|
| `Title` / `Prompt` | `快捷键` / `请按下快捷键组合：` | |
| `HotKey(vk, mod)` | `0,0` | 初始快捷键 |
| `Scope(int)` | `HOTKEYBOX_SCOPE_APP` | 初始选中「程序快捷键」或「全局快捷键」 |
| `ShowScope(bool)` | `true` | 是否显示程序/全局 Segmented |
| `AllowEmpty(bool)` | `true` | 允许确定时为空（清除） |
| `RequireModifier(bool)` | `true` | 仅限制**字母/数字**裸键；`F1~F12`、`Home`/`End` 等可单独设 |
| `CheckConflict(bool)` | `true` | 冲突则提示并关闭（返回 CANCEL，不写出） |
| `ConflictManager(pm)` | 按 Owner HWND 查找 | 扫描快捷键占用 |
| `ExcludeControl(p)` | `NULL` | 排除正在编辑的控件 |
| `AddReserved(vk, mod, name)` | 无 | 额外保留项（最多 32） |
| `ConflictCheck(fn, user)` | `NULL` | 自定义检测（返回 true=冲突） |
| `OkText` / `CancelText` | `确定` / `取消` | |
| `Width` / `Height` | `440` / `280` | 逻辑像素（隐藏 Scope / 缩短提示时可改矮） |
| `Owner(HWND)` | `Show` 的 owner | |

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
| `Kind` | `PRIMARY` | 标题栏配色；未设 `OkKind` 时确定按钮也跟它 |
| `OkKind` | `NONE`（跟随 Kind） | 仅确定按钮配色，例如标题 Primary、按钮 Danger |
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

- 相对 Owner（主窗）居中；无 Owner 时相对显示器工作区居中；位置夹在工作区内防出屏
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

- 新加 `UIModal.cpp` / `UIInputBox.cpp` 后需重新跑一次 `build_clang_ninja_*_init.bat`（`aux_source_directory`）

---

## 相关源码

| 文件 | 说明 |
|------|------|
| `src/DuiLib/Control/UIModal.h/.cpp` | Modal API 与窗口 |
| `src/DuiLib/Control/UIMessageBox.h/.cpp` | 同步 MessageBox |
| `src/DuiLib/Control/UIInputBox.h/.cpp` | 同步 InputBox |
| `bin/skin/duidemo/msg.html` | 可选自定义皮肤示例（`ShowSkin` / `XML_MSG`） |
| `src/DuiLib/Core/UIContainer.cpp` | 圆角裁剪含子控件 |
| `src/DuiLib/Utils/UIShadow.*` | 阴影；无 RGN 时跟 RoundCorner |
| `src/Demos/duidemo/MainWnd.cpp` | Demo 按钮 |
| `bin/skin/duidemo/main.html` | Accordion → Modal |
