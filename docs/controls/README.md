# 控件知识库

控件用法与属性文档放在本目录，**按控件一篇**，勿堆进根目录 `AGENTS.md`。

- Agent / 开发者：改或接入某控件时，先读对应 `docs/controls/<Name>.md`
- **属性总览 / 盒模型**：先读 [Attributes.md](Attributes.md)
- **窗口级属性**：[Window.md](Window.md)
- `AGENTS.md`：只保留构建、渲染硬约束与本目录入口
- 属性说明以本目录为准（运行真相在各控件 `SetAttribute`）

## 属性符合度

| 文档 | 说明 |
|------|------|
| [Attributes.md](Attributes.md) | 盒模型、伪类、已对齐清单、索引 |
| [Control.md](Control.md) | 基类 `CControlUI` |
| [Container.md](Container.md) | 容器与布局 |
| [Window.md](Window.md) | 窗口 / `html` 根 |

## 控件索引

| 控件 | 文档 | 源码 |
|------|------|------|
| Label | [Label.md](Label.md) | `src/DuiLib/Control/UILabel.*` |
| Button | [Button.md](Button.md) | `src/DuiLib/Control/UIButton.*` |
| Option / CheckBox | [Option.md](Option.md) | `src/DuiLib/Control/UIOption.*` |
| Edit | [Edit.md](Edit.md) | `src/DuiLib/Control/UIEdit.*` |
| RichEdit | [RichEdit.md](RichEdit.md) | `src/DuiLib/Control/UIRichEdit.*` |
| List 族 | [List.md](List.md) | `src/DuiLib/Control/UIList.*`、`UIListEx.*` |
| Combo / ComboBox | [Combo.md](Combo.md) | `src/DuiLib/Control/UICombo.*` |
| ScrollBar | [ScrollBar.md](ScrollBar.md) | `src/DuiLib/Control/UIScrollBar.*` |
| Progress / Slider | [Progress.md](Progress.md) | `UIProgress.*`、`UISlider.*` |
| SvgBox | [SvgBox.md](SvgBox.md) | `src/DuiLib/Control/UISvgBox.*` |
| Accordion | [Accordion.md](Accordion.md) | `src/DuiLib/Control/UIAccordion.*` |
| Carousel | [Carousel.md](Carousel.md) | `src/DuiLib/Control/UICarousel.*` |
| TreeView | [TreeView.md](TreeView.md) | `src/DuiLib/Control/UITreeView.*` |
| Menu | [Menu.md](Menu.md) | `src/DuiLib/Control/UIMenu.*` |
| TabBar / TabButton | [TabBar.md](TabBar.md) | `UITabBar.*`、`UITabButton.*` |
| TitleBar | [TitleBar.md](TitleBar.md) | `src/DuiLib/Control/UITitleBar.*` |
| PageControl | [PageControl.md](PageControl.md) | `src/DuiLib/Control/UIPageControl.*` |
| Toast | [Toast.md](Toast.md) | `src/DuiLib/Control/UIToast.*` |
| Modal | [Modal.md](Modal.md) | `src/DuiLib/Control/UIModal.*` |
| 其它（ActiveX / Web / Gif / Loading…） | [Misc.md](Misc.md) | `src/DuiLib/Control/*` |

后续新控件：在此表追加一行，并新增同名 md（用法）或并入对应符合度页。
