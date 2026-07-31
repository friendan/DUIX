# TabBar / TabButton

水平标签栏：切换页、关闭、锁定钉左、溢出滚动、内置「+」、左键拖拽排序（幽灵预览）、右键菜单、双击/中键/Ctrl+W 关闭。

| | |
|--|--|
| 类 | `CTabBarUI`、`CTabButtonUI` |
| XML | `<TabBar>`、`<TabButton>` |
| 头文件 | `UITabBar.h`、`UITabButton.h` |
| Demo | `bin/skin/duidemo/tabbartest.xml`，入口 Accordion → TabBar 测试窗 |

当前仅支持**水平**方向；垂直 TabBar 未做（使用少、与现能力对称成本高）。

---

## 最小示例

```xml
<TabBar name="tabbar" height="36" flexible="true" tab-min-width="100" tab-max-width="300"
    showadd="true" bindtablayoutname="pages"
    tabbkcolor="#00000000" tabhotbkcolor="#FFD6EBFF" tabselectedbkcolor="#FFBAE0FF"
    tabtextcolor="#FF8C8C8C" tabhottextcolor="#FF1677FF" tabselectedtextcolor="#FF1677FF"
    tabselectedbordercolor="#FF1677FF" tabselectedbordersize="2">
  <TabButton text="概况" tabler-outline="home" />
  <TabButton text="详情" locked="true" tabler-outline="info-circle" />
  <TabButton text="设置" tabler-outline="settings" />
</TabBar>
<TabLayout name="pages" selectedid="0">
  <Label text="概况页" />
  <Label text="详情页" />
  <Label text="设置页" />
</TabLayout>
```

代码侧处理「+」与拦截关闭示例见 `src/Demos/duidemo/TabBarTestWnd.cpp`。

---

## TabBar 属性

| 属性 | 说明 |
|------|------|
| `bindtablayoutname` | 绑定的 `TabLayout` 名称；切换/关闭/移动时同步页 |
| `tab-width` / `tabwidth` | 固定宽度（非 flexible 时） |
| `flexible` / `flexiblewidth` | `true`：均分可视区 |
| `tab-min-width` | 弹性下限，默认 100；`0`=不限制 |
| `tab-max-width` | 弹性上限，默认 300；`0`=不限制 |
| `showadd` / `show-add` | 是否显示内置「+」 |
| `addbtnwidth` | 「+」宽度 |
| `scrollbtnwidth` | 溢出 ‹ › 宽度 |
| `contextmenu` / `menuxml` | 见下文「右键菜单」 |
| `tabbkcolor` / `tabhotbkcolor` / `tabselectedbkcolor` | 标签背景 |
| `tabtextcolor` / `tabhottextcolor` / `tabselectedtextcolor` | 文字色 |
| `tabbordercolor` / `tabbordersize` | 未选中边框 |
| `tabselectedbordercolor` / `tabselectedbordersize` | 选中底边指示条 |
| `showtabseparator` / `tabseparatorcolor` | 标签间竖线 |
| `closetextcolor` / `closehotbkcolor` / `closehottextcolor` | 关闭钮 ✕ |

颜色一般为 `#AARRGGBB`。

---

## TabButton 属性

| 属性 | 说明 |
|------|------|
| `text` / `title` | 标题 |
| `locked` | `true`：锁定（隐藏关闭、不可关；钉到左侧钉住区） |
| `active` | 初始选中（一般由 TabBar 管理） |
| `url` / `dir` | 业务自定义数据 |
| `bsicon` / `iconpark` / `lucide` / `tabler-outline` / `tabler-filled` / `remixicon` / `twicon` / `iconsrc` | 左侧图标（SvgBox） |

---

## 行为摘要

### 切换与关闭

- 单击标签主体：选中；点 ✕：关闭（锁定不可关）
- 双击主体：关闭；中键：关闭；Ctrl+W：关当前
- Ctrl+Tab / Ctrl+Shift+Tab：下一/上一标签（焦点在栏上或鼠标在栏上时；全局 Ctrl+Tab 亦由消息过滤处理）

### 宽度

- `flexible=false`：固定 `tab-width`
- `flexible=true`：按可视区均分，受 min/max 约束；溢出后仍可滚动

### 溢出滚动

- 总宽超过可视区时出现 ‹ ›，可用按钮或滚轮
- 滚动对齐标签边缘；半截标签不绘制、不可点（避免误点）

### 锁定钉左

- `locked=true` 的标签固定在**可视区左侧钉住区**，**不参与横向滚动**
- 未锁定标签在钉住区右侧滚动
- 拖拽排序：钉住区与普通区互不跨越
- 锁定/解锁会自动归位（钉到左侧末 / 普通区头）

### 拖拽排序

- 左键按住拖过约 5px：半透明幽灵预览 + 放置竖线
- 松手落到目标标签：插入语义 `MoveTab(from, to)`
- 从 ✕ 按下不进入拖拽；点 ✕ 松手关闭

### 内置「+」

- 点击发送 `tabadd`；**不自动加标签**，由应用在 Notify 里 `AddTab`（并可同步 `TabLayout` 页）

---

## 右键菜单

默认**内置菜单**（代码生成，无需皮肤 XML）：

- 关闭 / 关闭其他 / 关闭左侧 / 关闭右侧 / 锁定（解锁）
- 锁定项、无可关项时对应菜单灰掉

| `contextmenu` 值 | 行为 |
|------------------|------|
| （默认空）/ `true` / `builtin` | 内置菜单 |
| `false` / `0` | 关闭右键菜单 |
| 文件路径 | 加载自定义菜单 XML |
| 以 `<` 开头的字符串 | 内联 XML |

自定义菜单项建议沿用内置 `name`（`tabbar_close`、`tabbar_close_others`、`tabbar_close_left`、`tabbar_close_right`、`tabbar_lock`），以便同一套点击处理；`UserData` 会写入标签下标。

点击由 TabBar 的 `MessageHandler` 处理 `WM_MENUCLICK`，应用窗一般**不必**再接菜单消息。

---

## 通知（Notify）

发送方均为 TabBar（`msg.pSender`）。可取消的通知里调用 `CancelNotify()`。

| `sType` | 宏 | wParam / lParam | 说明 |
|---------|-----|-----------------|------|
| `tabselecting` | `DUI_MSGTYPE_TABSELECTING` | 新索引 / 旧索引 | 切换前，可取消 |
| `tabselect` | `DUI_MSGTYPE_TABSELECT` | 新索引 / 旧索引 | 切换后 |
| `tabclosing` | `DUI_MSGTYPE_TABCLOSING` | 关闭索引 | 关闭前，可取消 |
| `tabclose` | `DUI_MSGTYPE_TABCLOSE` | 已关闭索引 | 关闭后（页已同步） |
| `tabmove` | `DUI_MSGTYPE_TABMOVE` | from / to | 拖拽或锁定归位移动后 |
| `tabadd` | `DUI_MSGTYPE_TABADD` | — | 点了「+」 |

绑定 `TabLayout` 时，关闭/移动会自动改页面顺序与选中项。

---

## 常用 API

```cpp
CTabButtonUI* AddTab(LPCTSTR title);
CTabButtonUI* InsertTab(int index, LPCTSTR title);
void RemoveTab(int index);           // 锁定则忽略
void RemoveOtherTabs(int keep);
void RemoveTabsToLeft(int index);
void RemoveTabsToRight(int index);
void RemoveUnlockedTabs();

bool SetActiveTab(int index, bool bCheckAllow = true);
void MoveTab(int iFrom, int iTo);    // 受钉住区约束

void SetFlexibleTabWidth(bool);
void SetContextMenuEnabled(bool);
void BindTabLayoutName(LPCTSTR);
void CancelNotify();                 // 仅在 selecting/closing 处理中
```

`CTabButtonUI::SetLocked(bool)` 会通知 TabBar 做钉左归位。

---

## 源码与测试

- 实现：`src/DuiLib/Control/UITabBar.cpp`、`UITabButton.cpp`
- 测试窗：`src/Demos/duidemo/TabBarTestWnd.*`，皮肤 `bin/skin/duidemo/tabbartest.xml`
- 编译：`build_clang_ninja_debug.bat` → `bin\duidemo_dbg.exe`
