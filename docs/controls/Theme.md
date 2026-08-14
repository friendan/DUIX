# Theme

| | |
|--|--|
| 类 | `CTheme` / `CThemeManager` |
| 源码 | `src/DuiLib/Core/UITheme.*`、`UIThemeBuiltin.cpp` |

语义色主题：`g_kindColors`、PaintManager Default、窗口 chrome。与 Skin / HSL 正交。

### 推荐用法

```xml
<html theme="chrome">
  <VBox name="root">
    <VBox theme="panel">...</VBox>
    <VBox theme="secondary"><Label text="说明" /></VBox>
    <Switch theme="none" ... />
    <Edit border-color="var(--color-primary)" />
  </VBox>
</html>
```

也可用 CSS：`html { theme: chrome; }`（与 `action` 一样，窗口级属性落到控件树 root）。  
root（`body`/`VBox`）上仍可写 `theme`，会覆盖 html 级默认。

进程切换：`CThemeManager::GetInstance()->ApplyTheme(_T("azure"));`  
皮肤内切换：[`ThemeSwitcher`](ThemeSwitcher.md)（弹出预览窗；支持新建/编辑色值与另存主题文件）。

写出主题文件：`SaveThemeFile(pTheme, path)`（与 `ApplyThemeFile` 对称的 `:root` 格式）。  
`LoadThemeFile(path)`：只注册不切换（启动批量加载用户主题用）；`ApplyThemeFile` = Load + 正式 Apply。  
选择窗侧见 [`ThemeSwitcher`](ThemeSwitcher.md) 的 `AddThemeFile` / `themefilesaved`。
变更监听：`AddThemeNotify(IThemeNotifyUI*)`，`OnThemeChanged(old, new, bPreview)`——落盘当前 id 时忽略 `bPreview==true`。

### 继承规则

| 属性 | 行为 |
|------|------|
| `theme="chrome"` | 子树按控件类型套表面色 |
| `theme="panel"` | 仅本节点 elevated |
| `theme="secondary"` | typed 同 chrome；另将纯 Label/Text（`kind=none`）字色改为次要色 |
| `theme="none"` | 子树不套 chrome |
| `theme`/`theme-id` 已注册 id | 换色板 |
| 未知 `theme` | 忽略 |

### chrome 覆盖

TitleBar、ScrollBar（含无图箭头色跟 thumb）、Edit/HotKey/**IPAddress（含聚焦原生 HWND；打开中热切重刷）**/**RichEdit（placeholder-color 跟 Edit）**/Spin/Number、Combo（含下拉；**打开中热切会重刷壳**）、DateTime（字段 + 日历；**打开中热切重刷壳**）、**Switch**（轨道 / 滑块 / 禁用）、**CheckBox**（方框 / 选中悬停 / 禁用 / 勾号）、**Option**（含 `group` 作 Radio）、Accordion、TabBar（含内置右键菜单；**嵌在 TitleBar 内时底/字/图标按标题栏亮度适配**）、List/TreeView/**VirtualList 斑马纹**、**纯色弹出 Menu**（`ApplyMenuChrome`；禁用项透明底；`theme=none` / `background-image` 壳跳过）、**Transfer（含左右面板壳）**、GroupBox、PageControl、**Carousel / CarouselItem caption**、**SidePanel（面板底/边/标题；遮罩色保留皮肤）**、**Avatar / FontIcon 默认色跟 primary**、Tag/Badge/Rate/Steps/Timeline、Empty/Skeleton/Loading/ColorPalette、**Segmented（悬停/选中边按亮度自适应）** 等。

表单默认属性（`Edit` / `RichEdit` / `Spin` / `Number` / `Combo` / `ComboBox` / `DateTime`）：含 `border-width="1"` 与主题边框色；皮肤可用 `border="none"` 关掉。

列表默认属性（`List` / `VirtualList` / `ListHeader`）：`List`/`VirtualList` 含 `border-width="1"`、行线（`List` 另开内容列线 `item-show-column-line` 与表头列线 `header-show-column-line`）、斑马纹、整行 `item-background-color-hover` / `item-background-color-selected`（及 `item-color-selected`）；`ListHeader` 含底部分隔与 elevated 底。chrome 热切对非 Menu 的 List 同步上述边界与态色；可用 `border="none"` / `item-show-row-line="false"` / `item-show-column-line="false"` / `header-show-column-line="false"` / `item-alternate-background="false"` 关闭。

装饰色请用 `theme="none"`（如紫色 Switch、自定义 DateTime 日历色）。

Toast / Modal：新建时按当前 **kind / token** 建 UI；`ApplyToExistingManager` / `RefreshAllManagers` 均跳过 `toastRoot` / `modalRoot`。`SetWindowBackgroundColor` 不覆盖已 `SetKind` 的根底色（避免 Toast 白字打在主题白底上）。

未覆盖（刻意）：Slider 滑块图、Ring 位图、Svg 自动 tint（需皮肤写 `color`/`icon-tint`）。

### 动态创建

**推荐顺序：先 `SetAttribute`（含 `theme` / `var(--token)`），再 `parent->Add(p)`。**

```cpp
CEditUI* p = new CEditUI;
p->SetAttribute(_T("theme"), _T("chrome"));           // 或继承父级 chrome
p->SetAttribute(_T("border-color"), _T("var(--color-primary)"));
pParent->Add(p);  // Add 时套 chrome，再重解 var(--token)
```

`Add` / `AddAt`（已有 Manager）会调用 `CThemeManager::ApplyChromeToControl`：先 chrome，再重解已记录的 `_tvar:`。若 `Add` 之后才改 `theme` / `theme-id`，`SetAttribute` 也会触发同一套用。需要时也可手动调 `ApplyChromeToControl`。

说明：XML 路径在 `AttachDialog` 后整树套 chrome；勿依赖「只 new + SetAttribute、不 Add」来吃 chrome。

### `var(--token)`

热切换重解；可走控件有效色板。

悬停底色强度（小图标推荐 `medium`）：

| Token | 说明 |
|-------|------|
| `color-bg-hover` | 轻：列表/大面积 |
| `color-bg-hover-medium` | 中：图标按钮 |
| `color-bg-hover-primary` | 强：主色倾向底 |

### 内置 id

`default` / `azure` / `emerald` / `graphite`（冷灰 slate：正文微灰、主色 `#334155`、标题栏同族） / `dark`

### 演示

装饰色 / CSS 伪类 / 彩色 Avatar·Img：`theme="none"`。
