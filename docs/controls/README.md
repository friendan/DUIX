# 控件知识库

控件用法与属性文档放在本目录，**按控件一篇**，勿堆进根目录 `AGENTS.md`。

- Agent / 开发者：改或接入某控件时，先读对应 `docs/controls/<Name>.md`
- **属性总览 / 盒模型**：先读 [Attributes.md](Attributes.md)
- **窗口级属性**：[Window.md](Window.md)
- **HWND 自定义消息**：[Messages.md](Messages.md)
- `AGENTS.md`：只保留构建、渲染硬约束与本目录入口
- 属性说明以本目录为准（运行真相在各控件 `SetAttribute`）

## 开发指南

| 文档 | 说明 |
|------|------|
| [CustomControl.md](CustomControl.md) | 自定义控件：工厂 / 窗口回调 / 插件 / 并入库 |

	## 工具

	| 文档 | 说明 |
	|------|------|
	| [Zip.md](Zip.md) | ZIP 创建 / 解压 / 皮肤 `SetResourceZip` |
	| [TrayIcon.md](TrayIcon.md) | 系统托盘 `CTrayIcon` |
	| [Window.md](Window.md)#调试日志-cdulog | 通用日志 `CDuiLog`：开关 / 路径（用法详见 Window.md） |

## 属性符合度

| 文档 | 说明 |
|------|------|
| [Attributes.md](Attributes.md) | 盒模型、伪类、已对齐清单、索引 |
| [Control.md](Control.md) | 基类 `CControlUI` |
| [Container.md](Container.md) | 容器与布局 |
| [Window.md](Window.md) | 窗口 / `html` 根；**Per-Monitor DPI**；**任务栏悬停整栏闪白**排障 |
| [Messages.md](Messages.md) | HWND 自定义消息号段（库 / 业务）；**Shadow 下动画定时器硬约束** |

## 控件索引

| 控件 | 文档 | 源码 |
|------|------|------|
| Label | [Label.md](Label.md) | `src/DuiLib/Control/UILabel.*` |
| Button | [Button.md](Button.md) | `src/DuiLib/Control/UIButton.*` |
| ShapeButton / ShapeBox | [Shape.md](Shape.md) | `UIShapeButton.*` / `UIShape.*` |
| Option / CheckBox | [Option.md](Option.md) | `src/DuiLib/Control/UIOption.*` |
| Switch | [Switch.md](Switch.md) | `src/DuiLib/Control/UISwitch.*` |
| Spin / Number | [Spin.md](Spin.md) | `src/DuiLib/Control/UISpin.*` |
| Segmented | [Segmented.md](Segmented.md) | `src/DuiLib/Control/UISegmented.*` |
| Badge / Tag | [Badge.md](Badge.md) | `src/DuiLib/Control/UIBadge.*` |
| Transfer | [Transfer.md](Transfer.md) | `src/DuiLib/Control/UITransfer.*` |
| Rate | [Rate.md](Rate.md) | `src/DuiLib/Control/UIRate.*` |
| Empty | [Empty.md](Empty.md) | `src/DuiLib/Control/UIEmpty.*` |
| Skeleton | [Skeleton.md](Skeleton.md) | `src/DuiLib/Control/UISkeleton.*` |
| Image / Avatar | [Image.md](Image.md) | `src/DuiLib/Control/UIImage.*` |
| FontIcon | [FontIcon.md](FontIcon.md) | `src/DuiLib/Control/UIFontIcon.*` |
| AppIcon | [AppIcon.md](AppIcon.md) | `src/DuiLib/Control/UIAppIcon.*` |
| AppGrid | [AppGrid.md](AppGrid.md) | `src/DuiLib/Control/UIAppGrid.*` |
| Steps | [Steps.md](Steps.md) | `src/DuiLib/Control/UISteps.*` |
| Timeline | [Timeline.md](Timeline.md) | `src/DuiLib/Control/UITimeline.*` |
| Edit | [Edit.md](Edit.md) | `src/DuiLib/Control/UIEdit.*` |
| EditBox | [EditBox.md](EditBox.md) | `src/DuiLib/Control/UIEditBox.*`（左右 addon / clearable） |
| RichEdit | [RichEdit.md](RichEdit.md) | `src/DuiLib/Control/UIRichEdit.*` |
| List 族 | [List.md](List.md) | `src/DuiLib/Control/UIList.*`、`UIListEx.*` |
| VirtualList | [VirtualList.md](VirtualList.md) | `src/DuiLib/Control/UIVirtualList.*` |
| Combo / ComboBox | [Combo.md](Combo.md) | `src/DuiLib/Control/UICombo.*` |
| LookupEdit | [LookupEdit.md](LookupEdit.md) | `src/DuiLib/Control/UILookupEdit.*` |
| ScrollBar | [ScrollBar.md](ScrollBar.md) | `src/DuiLib/Control/UIScrollBar.*` |
| Progress / Slider | [Progress.md](Progress.md) | `UIProgress.*`、`UISlider.*` |
| SvgBox | [SvgBox.md](SvgBox.md) | `src/DuiLib/Control/UISvgBox.*` |
| Accordion | [Accordion.md](Accordion.md) | `src/DuiLib/Control/UIAccordion.*` |
| SplitLayout | [SplitLayout.md](SplitLayout.md) | `src/DuiLib/Layout/UISplitLayout.*` |
| Carousel | [Carousel.md](Carousel.md) | `src/DuiLib/Control/UICarousel.*` |
| TreeView | [TreeView.md](TreeView.md) | `src/DuiLib/Control/UITreeView.*` |
| Menu | [Menu.md](Menu.md) | `src/DuiLib/Control/UIMenu.*` |
| TabBar / TabButton | [TabBar.md](TabBar.md) | `UITabBar.*`、`UITabButton.*` |
| TitleBar | [TitleBar.md](TitleBar.md) | `src/DuiLib/Control/UITitleBar.*` |
| HotKey | [HotKey.md](HotKey.md) | `src/DuiLib/Control/UIHotKey.*`；绑定见同页 `CHotKeyBinder` |
| GroupBox | [GroupBox.md](GroupBox.md) | `src/DuiLib/Control/UIGroupBox.*` |
| Ring | [Ring.md](Ring.md) | `src/DuiLib/Control/UIRing.*` |
| FadeButton | [FadeButton.md](FadeButton.md) | `src/DuiLib/Control/UIFadeButton.*` |
| RollText | [RollText.md](RollText.md) | `src/DuiLib/Control/UIRollText.*` |
| IPAddress / IPAddressEx | [IPAddress.md](IPAddress.md) | `UIIPAddress.*`、`UIIPAddressEx.*` |
| DateTime | [DateTime.md](DateTime.md) | `src/DuiLib/Control/UIDateTime.*` |
| ActiveX | [ActiveX.md](ActiveX.md) | `src/DuiLib/Control/UIActiveX.*` |
| WebBrowser | [WebBrowser.md](WebBrowser.md) | `src/DuiLib/Control/UIWebBrowser.*` |
| GifAnim / GifAnimEx | [GifAnim.md](GifAnim.md) | `UIGifAnim.*`、`UIGifAnimEx.*` |
| Loading | [Loading.md](Loading.md) | `src/DuiLib/Control/UILoading.*` |
| ColorPalette | [ColorPalette.md](ColorPalette.md) | `src/DuiLib/Control/UIColorPalette.*` |
| ThemeSwitcher | [ThemeSwitcher.md](ThemeSwitcher.md) | `src/DuiLib/Control/UIThemeSwitcher.*` |
| Theme（全局） | [Theme.md](Theme.md) | `src/DuiLib/Core/UITheme.*` |
| PageControl | [PageControl.md](PageControl.md) | `src/DuiLib/Control/UIPageControl.*` |
| Toast | [Toast.md](Toast.md) | `src/DuiLib/Control/UIToast.*` |
| Modal / MessageBox / InputBox | [Modal.md](Modal.md) | `UIModal.*`、`UIMessageBox.*`、`UIInputBox.*` |
| SidePanel | [SidePanel.md](SidePanel.md) | `src/DuiLib/Control/UISidePanel.*` |

后续新控件：在此表追加一行，并新增同名 md（用法）或并入对应符合度页。
