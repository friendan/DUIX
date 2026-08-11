# 自定义控件

业务侧扩展控件：继承现有类 → 挂到皮肤标签 → 在 `SetAttribute` / 绘制 / 事件里补逻辑。不必改 DuiLib 源码也能用。

| | |
|--|--|
| 工厂 | `src/DuiLib/Core/ControlFactory.h`（`DECLARE` / `IMPLEMENT` / `REGIST_DUICONTROL`） |
| 解析 | `CDialogBuilder`：工厂 → 插件 → `IDialogBuilderCallback::CreateControl` |
| 窗口基类 | `WindowImplBase::CreateControl`（默认返回 `NULL`） |
| Demo | `src/Demos/duidemo/ControlEx.*`、`DuiDemo.cpp` 里 `REGIST_DUICONTROL` |

基类属性见 [Control.md](Control.md)、[Attributes.md](Attributes.md)。

---

## 选哪条挂接路径

| 方式 | 做法 | 适合 |
|------|------|------|
| **A. 全局工厂** | `IMPLEMENT_DUICONTROL` + 启动时 `REGIST_DUICONTROL` | 多窗口共用；皮肤里直接写标签（推荐业务默认） |
| **B. 窗口回调** | 重写 `WindowImplBase::CreateControl`，按标签 `new` | 仅某窗口用、或临时试验 |
| **C. 插件 DLL** | `CPaintManagerUI::LoadPlugin`，DLL 导出 `CreateControl` | 热插、独立分发控件包 |
| **D. 并入库** | 加源文件 + `ControlFactory.cpp` 里 `INNER_REGISTER_DUICONTROL` | 要做成官方内置控件时 |

解析顺序（`UIDlgBuilder`）：`C{标签}UI` 工厂 → 已加载插件 → 窗口 `CreateControl`。

---

## 命名约定

皮肤标签与类名对应关系：

```text
XML:  <CircleProgress ... />
工厂键: CCircleProgressUI   （DialogBuilder 自动加 C…UI）
回调参数: CircleProgress    （裸标签名，大小写不敏感比较即可）
```

- `REGIST_DUICONTROL(CCircleProgressUI)` 注册的就是 `CCircleProgressUI`（内部会 `MakeLower`）。
- `GetClass()` 建议返回短名（如 `CircleProgress`）或与库内控件一致的 `XxxUI`；查找接口时靠 `GetInterface`。
- `GetInterface(pstrName)`：对短名（如 `_T("CircleProgress")`）返回 `this`，否则交给基类。业务里 `FindControl` 后常用 `GetInterface` 再 `static_cast`。

---

## 最小可运行骨架（路径 A）

头文件：

```cpp
#pragma once
#include "UIlib.h"

class CHelloBadgeUI : public CLabelUI
{
	DECLARE_DUICONTROL(CHelloBadgeUI)
public:
	CHelloBadgeUI() {}

	LPCTSTR GetClass() const { return _T("HelloBadge"); }

	LPVOID GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, _T("HelloBadge")) == 0 )
			return static_cast<CHelloBadgeUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	void PaintText(IRenderContext& ctx); // 或 DoPaint / PaintForeColor 等

private:
	int m_nLevel = 0;
};
```

实现：

```cpp
#include "StdAfx.h"
#include "HelloBadge.h"

IMPLEMENT_DUICONTROL(CHelloBadgeUI)

void CHelloBadgeUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if( _tcscmp(pstrName, _T("level")) == 0 )
		m_nLevel = _ttoi(pstrValue);
	else
		CLabelUI::SetAttribute(pstrName, pstrValue);
}

void CHelloBadgeUI::PaintText(IRenderContext& ctx)
{
	// 先走基类文字，或完全自绘
	CLabelUI::PaintText(ctx);
}
```

启动时（与 `CoInitialize` / `SetInstance` 同级，建窗之前）：

```cpp
REGIST_DUICONTROL(CHelloBadgeUI);
```

皮肤：

```xml
<HelloBadge name="hb" text="Hi" level="2" width="80" height="24" />
```

路径 B 若未注册工厂，在窗口里：

```cpp
CControlUI* CMainWnd::CreateControl(LPCTSTR pstrClass)
{
	if( lstrcmpi(pstrClass, _T("HelloBadge")) == 0 )
		return new CHelloBadgeUI();
	return NULL;
}
```

duidemo 同时演示了 A+B：`DuiDemo.cpp` 里 `REGIST_DUICONTROL(CCircleProgressUI)`，`MainWnd::CreateControl` 也可 `new`（工厂已命中时回调不会走到）。

---

## 该重写什么

| 需求 | 常见入口 |
|------|----------|
| XML 自定义属性 | `SetAttribute`：识别后处理，否则 `基类::SetAttribute` |
| 自绘外观 | `DoPaint`，或更细的 `PaintBackground*` / `PaintFore*` / `PaintText` / `PaintBorder` |
| 鼠标键盘 | `DoEvent`；需要定时器用 `m_pManager->SetTimer` / `KillTimer` |
| 布局尺寸 | `EstimateSize`；容器则继承 `CContainerUI` / 布局类并管子控件 |
| 通知业务 | `m_pManager->SendNotify(this, …)`（与现有 Button/List 同一套） |

优先**继承最接近的现成控件**（`CButtonUI`、`CProgressUI`、`CHorizontalLayoutUI`…），只改差异；从 `CControlUI` 白板开始成本最高。

---

## 绘制注意（D2D）

默认渲染后端是 Direct2D。自定义绘制请走 `IRenderContext`（`FillRect`、`DrawText`、`DrawImage` 等），与库内控件一致。

- 少用 `ctx.GetDC()` + GDI/Gdiplus：会触发 Flush / GdiInterop，易花屏或变慢；`ControlEx` 里部分示例仍是旧写法，新代码勿照抄。
- 改 Present / Flush / 分层路径前先读根目录 `AGENTS.md` 的 D2D 硬约束。
- HiDPI：尺寸与坐标一般已是布局后的像素矩形（`GetPos()`）；勿再按 96DPI 硬编码后忘缩放。

---

## 插件 DLL（路径 C）

```cpp
// 导出：CControlUI* CreateControl(LPCTSTR pstrType);
extern "C" __declspec(dllexport) CControlUI* CreateControl(LPCTSTR pstrType)
{
	if( lstrcmpi(pstrType, _T("HelloBadge")) == 0 )
		return new CHelloBadgeUI();
	return NULL;
}
```

主程序：

```cpp
CPaintManagerUI::LoadPlugin(_T("MyControls.dll"));
```

插件收到的同样是**裸标签名**（不是 `CHelloBadgeUI`）。

---

## 并入库内置（路径 D）

1. 在 `src/DuiLib/Control/`（或 Layout）加 `UIXxx.h/.cpp`，`namespace DuiLib`，`DECLARE`/`IMPLEMENT_DUICONTROL`。
2. `ControlFactory.cpp` 构造函数里加 `INNER_REGISTER_DUICONTROL(CXxxUI);`。
3. 若 CMake 用 `aux_source_directory`，新 cpp 后重新跑一次 `build_clang_ninja_*_init.bat`。
4. 在 `docs/controls/` 补同名 md，并在 [README.md](README.md) 索引表追加一行。
5. 需要主题色时对接 `kind` / `CTheme`（见 [Theme.md](Theme.md)）。

---

## 自检清单

- [ ] 皮肤标签能创建（Debug 下未知标签会 `DUITRACE`「未知控件」）
- [ ] 自有属性经 `SetAttribute` 生效；未识别的交给基类（宽高、背景等仍可用）
- [ ] `FindControl` + `GetInterface` 能转到具体类型
- [ ] 悬停 / 点击 / 禁用态绘制正常；D2D 下无整区空白
- [ ] （可选）`duidemo` 同页对照 `ControlEx` 行为

---

## 相关源码

| 文件 | 说明 |
|------|------|
| `src/DuiLib/Core/ControlFactory.h/.cpp` | 宏与内置注册表 |
| `src/DuiLib/Core/UIDlgBuilder.cpp` | 标签 → 控件实例 |
| `src/DuiLib/Utils/WinImplBase.h/.cpp` | `CreateControl` 回调 |
| `src/DuiLib/Core/UIManager.*` | `LoadPlugin` |
| `src/Demos/duidemo/ControlEx.*` | 圆形进度、图表、嵌 HWND 等示例 |