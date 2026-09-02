# WindowImplBase / 消息映射

| | |
|--|--|
| 类 | `WindowImplBase`、`CNotifyPump` |
| 源码 | `src/DuiLib/Utils/WinImplBase.*`、`src/DuiLib/Core/UIBase.*`、`src/DuiLib/Core/UIDefine.h` |
| 相关 | 窗口属性见 [Window.md](Window.md)；HWND 消息号见 [Messages.md](Messages.md) |

面向**独立业务工程**（自有 `AGENTS.md`、链接本库）写窗体时的约定。本库用 clang-cl `/W3` 时，缺 `override` 会报 **`-Winconsistent-missing-override`**，每个包含该头的 TU 都会刷日志。

---

## 最小窗体骨架

```cpp
class CMyWnd : public WindowImplBase
{
public:
	CDuiString GetSkinFile() override;           // WindowImplBase 首次引入 → 子类用 override
	LPCTSTR GetWindowClassName() const override; // 覆写 CWindowWnd
	void InitWindow() override;                  // 覆写 WindowImplBase::InitWindow
	void OnFinalMessage(HWND hWnd) override;     // 覆写 CWindowWnd
	void Notify(TNotifyUI& msg) override;        // 覆写 INotifyUI

	DUI_DECLARE_MESSAGE_MAP()                    // 派生类宏；勿用 BASE 版
	void OnClick(TNotifyUI& msg) override;       // 覆写 WindowImplBase::OnClick
};
```

`.cpp`：

```cpp
DUI_BEGIN_MESSAGE_MAP(CMyWnd, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, CMyWnd::OnClick)
DUI_END_MESSAGE_MAP()
```

---

## 消息映射宏（必分清）

定义在 `UIDefine.h`：

| 宏 | 谁用 | 说明 |
|----|------|------|
| `DUI_DECLARE_MESSAGE_MAP_BASE()` | **仅** `CNotifyPump` | 首次 `virtual GetMessageMap()` |
| `DUI_DECLARE_MESSAGE_MAP()` | `WindowImplBase` 及所有派生窗 / `CNotifyPump` 子类 | 声明为 `GetMessageMap() const override` |
| `DUI_BEGIN_MESSAGE_MAP(子, 父)` / `DUI_END_MESSAGE_MAP` | `.cpp` | 表项实现 |
| `DUI_BASE_BEGIN_MESSAGE_MAP(CNotifyPump)` | 库内根类实现 | 业务勿用 |

**错误：** 派生窗再写一套「`virtual GetMessageMap()` 且无 `override`」——旧版单一宏会这样，现已拆开。业务工程请更新到含 `DUI_DECLARE_MESSAGE_MAP_BASE` 的头文件版本。

---

## 哪些声明必须 `override`

### 覆写基类 / 接口（必须 `override`）

| 基类 / 接口 | 典型函数 |
|-------------|----------|
| `CWindowWnd` | `OnFinalMessage`、`HandleMessage`、`GetWindowClassName`、`GetClassStyle` |
| `INotifyUI` | `Notify` |
| `IMessageFilterUI` | `MessageHandler` |
| `IDialogBuilderCallback` | `CreateControl` |
| `IQueryControlText` | `QueryControlText` |
| `WindowImplBase`（子类覆写） | `InitWindow`、`OnClick`、`GetSkinFile`、`OnDestroy`、`HandleCustomMessage`、`OnSysCommand` 等 |

### `WindowImplBase` 上首次引入（基类里写 `virtual`，子类覆写时再 `override`）

`InitResource`、`InitWindow`、`OnClick`、`GetSkinFile`、`GetSkinType`、`GetManagerName`、`IsInStaticControl`，以及 `OnClose` / `OnCreate` / `OnSize` 等以 `WindowImplBase` 为入口的虚钩子。

**禁止：** 在基类首次声明处写 `override`；**禁止**为压 warning 删基类 `virtual` 或 `#pragma` 关 `-Winconsistent-missing-override`。

---

## 可粘贴到业务工程 `AGENTS.md` 的摘要

独立工程若自有 Agent 规则，可复制：

```markdown
## DuiLib 窗口 / override（clang-cl）

- 派生窗：`DUI_DECLARE_MESSAGE_MAP()`；勿对业务窗使用 `DUI_DECLARE_MESSAGE_MAP_BASE()`（仅 CNotifyPump）。
- 凡覆写 `WindowImplBase` / `CWindowWnd` / `INotifyUI` / `IMessageFilterUI` 等虚函数，声明末尾写 `override`。
- 编译若出现 `-Winconsistent-missing-override`：只修报警指向的那一个声明；勿全库脚本批量替换。
- 详情：DuiLib 仓库 `docs/controls/WinImplBase.md`
```

---

## 对照示例

本库已按上述规则整理：`WinImplBase.h`、`UIThemeSwitcher` / `UIconPicker` 等内嵌窗，以及 `duidemo` 各 `*Wnd.h`。可参考 `src/Demos/duidemo/PopWnd.h`、`MainWnd.h`。
