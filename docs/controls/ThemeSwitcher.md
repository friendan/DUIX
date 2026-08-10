# ThemeSwitcher

| | |
|--|--|
| 类 | `CThemeSwitcherUI` / `CThemePickerWnd` |
| XML | `<ThemeSwitcher>` / `themeswitcher` |
| 源码 | `src/DuiLib/Control/UIThemeSwitcher.*` |
| 预览窗皮肤 | 内嵌于 `UIThemeSwitcher.cpp`（C++11 `LR"dui(...)"dui` raw string），无需再附带 html |
| 继承 | Button |

点击后弹出**两列主题窗**：左主题名、右全量色值预览；可新建主题、编辑任意 token、导入/另存 `:root` 主题文件。正式切换经 `CThemeManager` 通知外部，便于持久化当前主题 id。

默认显示 **Lucide `palette` 调色板图标**（无字）。图标色默认随主题刷新（`tint-auto=true`）：标题栏内跟系统按钮一致（`titlebar-text` + 半透明悬停底，避免 primary 与标题栏同色时隐身）；其它位置跟 `color-text`，悬停用 `color-primary`。可用 `lucide` / `tabler-outline` 等覆盖图标；`tint-auto="false"` 时保留皮肤里的 `icon-tint*`。

### 最小示例

```xml
<!-- 标题栏：Spacer 顶到最小化左侧；height 与 TitleBar 同高则悬停底铺满（同系统按钮） -->
<!-- margin 四值是 CSS：top,right,bottom,left；右侧留缝写 margin="0,8,0,0" 或 margin-right="8" -->
<TitleBar height="40" ...>
  <Spacer />
  <ThemeSwitcher name="themeSwitch" width="36" height="40" margin="0,8,0,0" />
</TitleBar>

<!-- 固定着色，不跟主题 -->
<ThemeSwitcher tint-auto="false" icon-tint="#FFFFFFFF" icon-tint-hover="#91CAFFFF"
    lucide="palette" width="36" height="28" />
```

```cpp
// 进程级监听（推荐落盘当前 id）
class CThemePersist : public IThemeNotifyUI {
public:
	virtual void OnThemeChanged(LPCTSTR /*oldId*/, LPCTSTR newId, bool bPreview) {
		if( bPreview ) return;
		::WritePrivateProfileString(_T("ui"), _T("theme"), newId, m_ini);
	}
	CDuiString m_ini;
};

// 启动：tm->AddThemeNotify(&persist); tm->ApplyTheme(读出的 id);
// 窗口：selectchanged（sender=ThemeSwitcher）也可用 GetThemeId()
```

### 预览窗

| 区域 | 说明 |
|------|------|
| 新建主题 | 从当前主题拷贝色板并注册，进入编辑态 |
| 编辑颜色 | 点「编辑颜色」或**双击右侧 #色值**进入编辑；点选色值行用 ColorPalette，双击可键入 `#RRGGBB(AA)` |
| 导入… | `ApplyThemeFile` |
| 另存为… | `SaveThemeFile`（`:root { --token: #RRGGBBAA; }`） |
| 左列 | 已注册主题名；点选实时预览（`bPreview=true`） |
| 右列 | 迷你色条 + 分组色值表（标题栏 / 窗口 / 按钮 / 文本 / 输入控件 / 其它） |
| 确定 / 取消 | 提交或还原进入窗前主题 |
| 尺寸 | `UI_WNDSTYLE_FRAME`，可拖边调整；皮肤 `min-size: 720,440` |

### 属性

| 属性 | 说明 |
|------|------|
| `themes` | 逗号分隔 id 白名单；空=全部已注册 |
| `tint-auto` | `true`（默认）：图标色随主题自动刷新；`false`：不改 `icon-tint*`，由皮肤/代码自控 |

### 通知

| 类型 | 时机 |
|------|------|
| `selectchanged` | 预览窗确定后 |
| `IThemeNotifyUI::OnThemeChanged` | 任意正式 `ApplyTheme`（含本窗）；`bPreview` 区分预览 |

另见 [Theme.md](Theme.md)（`SaveThemeFile` / `ApplyThemeFile` / 内置 id）。
