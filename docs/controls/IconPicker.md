# IconPicker

| | |
|--|--|
| 类 | `CIconPickerUI` / `CIconPickerWnd` |
| XML | `<IconPicker>` / `IconPicker` |
| 源码 | `src/DuiLib/Control/UIconPicker.*` |
| 预览窗皮肤 | 内嵌于 `UIconPicker.cpp`（宽字面量 `LR"dui(...)dui"`；工程仅 Unicode），无需再附带 html |
| 继承 | Button |

点击后弹出**图标选择窗**：左列图标库、右侧图标网格，可搜索、拖大小、点选；确定后把 `{库名, 图标名}` 写回触发器并发 `selectchanged`，便于持久化所选图标名。

默认显示 **Lucide `grid-2x2`**（无字）。可用 `iconlib` / `icon` 覆盖初始选中图标。

### 最小示例

```xml
<ToolBar height="36" ...>

  <IconPicker name="pic" width="36" height="36" margin="0,8,0,0"
      libs="lucide,tabler-outline" icon="palette" />

</ToolBar>
```

```cpp
// 读取当前值
CIconPickerUI* p = ...; // FindControl("pic")
LPCTSTR lib  = p->GetLibrary();        // 如 "tabler-outline"
LPCTSTR name = p->GetSelectedIcon();   // 如 "settings"
// 存储：入库表时存 {lib, name} 两个字符串即可（不存下标）

// 还原：启动时设置
p->SetLibrary(_T("tabler-outline"));
p->SetSelectedIcon(_T("settings"));

// 用户确定选择后收到通知：pSender = picker
void OnIconPicked(TNotifyUI& msg) {
    if( msg.sType != DUI_MSGTYPE_SELECTCHANGED ) return;
    CIconPickerUI* p = static_cast<CIconPickerUI*>(msg.pSender);
    ::WritePrivateProfileString(_T("ui"), _T("iconlib"),  p->GetLibrary(), m_ini);
    ::WritePrivateProfileString(_T("ui"), _T("iconname"), p->GetSelectedIcon(), m_ini);
}
```

### 选择窗

| 区域 | 说明 |
|------|------|
| 左列 | 图标库列表（`libs` 白名单过滤；空=全部 7 个内置库） |
| 搜索框 | 实时按图标名子串过滤当前库 |
| 尺寸 | **预设/自定义两态切换**：默认显示 宽/高 两个下拉（预设 16/24/32/48/64/96/128，仅列[`size-min`,`size-max`]内的）；点「自定义宽高」→ 同一位置换成 宽/高 两个输入框可输入任意值；切回则显示「预设宽高」 |
| 尺寸校验 | 界于自定义 `size-min`~`size-max`（默认 8~256）；非法/过大自动钳制并回退 |
| 图标网格 | `CFlowLayoutUI`，每格一个按钮，点选高亮；悬停显示图标名 tooltip |
| 颜色 | 预设色块（无/绿/蓝/青/紫/橙/红）+「自定义」：点「自定义」展开折叠面板，内置 HSL 色板可拖选任意色，也可在 hex 输入框直接填 `#RRGGBB`（回车或点「应用」生效）；收起点「收起」。所选颜色实时作用于网格预览；默认**无**（用图标原色/主题默认）。`show-color="false"` 时整块颜色区隐藏，固定颜色 |
| 状态栏 | 窗口底部状态栏（主题标题栏色）：当前库图标总数 / 匹配数 / 已选 |
| 确定 / 取消 | 确定写回 `{库名, 图标名, 颜色}` 并发 `selectchanged` |

### 属性

| 属性 | 说明 |
|------|------|
| `libs` / `libraries` | 逗号分隔的**库白名单**（如 `lucide,tabler-outline`）；**空=全部内置库** |
| `iconlib` | 当前库名（如 `tabler-outline`） |
| `icon` / `icon-name` | 当前选中图标名 |
| `default-lib` / `default-library` | 打开选择窗时**默认选中的分类**（如 `lucide`）；**空=默认选第一个可见分类** |
| `size-min` | 尺寸下限（宽/高共用）；默认 `8` |
| `size-max` | 尺寸上限；默认 `256`（单次硬上限 1024） |
| `icon-tint-color` / `icon-color` | 图标颜色筛选（ARGB/`#RRGGBB`）；默认 `0` = 无 |
| `icon-size` / `icon-width` / `icon-height` | 图标显示尺寸（像素）：`icon-size` 为正方形；`icon-width` / `icon-height` 可分别设宽/高。默认 18 |
| `modal` | 默认 `false`（非模态可对照主窗）；`true` 则 `ShowModal` 禁用主窗 |
| `show-size` / `show-size-settings` | 选择窗是否显示“大小设置”区；默认 `true`；`false` 可固定宽高不给用户改 |
| `show-color` / `show-color-settings` | 选择窗是否显示“颜色自定义”区；默认 `true`；`false` 可固定颜色不给用户改 |

### API

| 方法 | 说明 |
|------|------|
| `SetLibrary(LPCTSTR)` / `GetLibrary()` | 当前库名 |
| `SetSelectedIcon(LPCTSTR)` / `GetSelectedIcon()` | 当前选中图标名；写入即刷新触发按钮显示 |
| `SetDefaultLibrary(LPCTSTR)` / `GetDefaultLibrary()` | 打开选择窗时默认选中的分类；空=默认选第一个可见分类。设置的是“选择窗初始选中”，不影响触发按钮当前库 |
| `SetLibrariesFilter(LPCTSTR)` / `GetLibrariesFilter()` | 库白名单（空=全部） |
| `IsLibListed(lib)` | 该库是否出现在选择窗列表 |
| `SetModal(bool)` / `IsModal()` | 是否模态 |
| `SetSizeRange(min, max)` / `GetSizeMin()` / `GetSizeMax()` | 尺寸允许范围（宽/高共用）；默认 8~256 |
| `SetIconColor(DWORD)` / `GetIconColor()` | 图标颜色筛选（ARGB）；`0` = 无 |
| `SetIconSize(w,h)` / `GetIconWidth()` / `GetIconHeight()` | 图标显示尺寸（宽/高），即“返回给使用者的选择大小”；确定后写回并刷新触发按钮显示 |
| `SetShowSizeSettings(bool)` / `IsShowSizeSettings()` | 选择窗是否显示“大小设置”区（默认 true）；false 时隐藏，固定宽高 |
| `SetShowColorSettings(bool)` / `IsShowColorSettings()` | 选择窗是否显示“颜色自定义”区（默认 true）；false 时隐藏，固定颜色 |
| `OpenPicker()` | 打开选择窗（已开则前置） |

库白名单为空时显示全部 7 个内置库：`bsicon` / `iconpark` / `lucide` / `remixicon` / `tabler-outline` / `tabler-filled` / `twicon`。

### 通知

| 类型 | 时机 | 携带 |
|------|------|------|
| `selectchanged` | 确定后 | `pSender` = picker；`wParam`=库名，`lParam`=图标名（指针，回调内尽快读取/拷贝） |

**返回给使用者的完整选择**：通知里 `pSender` 就是 picker，回调内调用它的 getter 即可拿到**分类/颜色/大小**：`GetLibrary()`（分类）、`GetSelectedIcon()`（图标名）、`GetIconColor()`（颜色，`0`=无）、`GetIconWidth()` / `GetIconHeight()`（大小）。四点均已写回触发按钮并刷新显示（`GetLibrary`、`GetSelectedIcon`、`GetIconColor`、`SetIconSize` 在确定时被写入）。示例：

```cpp
void OnIconPicked(TNotifyUI& msg) {
    if( msg.sType != DUI_MSGTYPE_SELECTCHANGED ) return;
    CIconPickerUI* p = static_cast<CIconPickerUI*>(msg.pSender);
    const CString lib   = p->GetLibrary();        // 分类
    const CString name  = p->GetSelectedIcon();   // 图标名
    DWORD        color  = p->GetIconColor();      // 颜色（ARGB），0=无
    int          w      = p->GetIconWidth();      // 宽
    int          h      = p->GetIconHeight();     // 高
    // 持久化这五项即可
}

图标数据与跨库枚举见 [SvgBox.md](SvgBox.md)「枚举图标库 / 统一入口 CIconLibrary」。

### Demo(`duidemo`)

主窗 Accordion →「打开图标选择控件测试」→ `CIconPickerDemoWnd` / `iconpicker_wnd.html`，含三个例子：

| 例子 | 皮肤写法 | 左列行为 |
|------|------|------|
| 例 1（白名单） | `<IconPicker libs="lucide,tabler-outline" …>` | 只列出 `lucide` + `tabler-outline` 两个库 |
| 例 2（全部+自定义范围+默认分类） | `<IconPicker … default-lib="lucide" size-min="24" size-max="96">`（不写 `libs`） | 列出全部 7 个内置库；打开默认选中 `lucide`（不设 `default-lib` 则选中第一个 `bsicon`）；尺寸范围钳到 24~96 |
| 例 3（固定宽高/颜色） | `<IconPicker … icon-size="20" icon-tint-color="#FF8000" show-size="false" show-color="false">` | 隐藏“大小设置”和“颜色自定义”两区，只让用户选图标（宽高与颜色已固定） |

三个 picker 各自在其窗口右下方状态行显示选中的 `{库名, 图标名}`，便于对照 `libs` 白名单 / `show-*` 隐藏的效果。
