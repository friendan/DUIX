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

装饰色请用 `theme="none"`（如紫色 Switch、自定义 DateTime 日历色）。

Toast / Modal：新建时按当前 **kind / token** 建 UI；`ApplyToExistingManager` / `RefreshAllManagers` 均跳过 `toastRoot` / `modalRoot`。`SetWindowBackgroundColor` 不覆盖已 `SetKind` 的根底色（避免 Toast 白字打在主题白底上）。

未覆盖（刻意）：Slider 滑块图、Ring 位图、Svg 自动 tint（需皮肤写 `color`/`icon-tint`）。

### `var(--token)`

热切换重解；可走控件有效色板。

### 内置 id

`default` / `azure` / `emerald` / `graphite`（冷灰 slate：正文微灰、主色 `#334155`、标题栏同族） / `dark`

### 演示

装饰色 / CSS 伪类 / 彩色 Avatar·Img：`theme="none"`。
