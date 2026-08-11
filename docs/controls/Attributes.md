# 属性与 HTML/CSS 符合度

控件 / 窗口属性的**唯一约定入口**（勿再往根目录 `AGENTS.md` 堆属性说明）。  
**按控件细节**见本目录各 `*.md`；构建与渲染硬约束见根目录 [AGENTS.md](../../AGENTS.md)。  
文档只描述**现行**属性；已删除/改名前的旧名不再收录。

---

## 盒模型与全局约定

全控件统一（含 Button 等叶子控件与 VBox/HBox 等容器）：

| 主题 | 约定 |
|------|------|
| `margin` | 外边距（`SetMargin` / `m_rcMargin`；根节点相对窗口） |
| `padding` | 内边距（`SetPadding` / `m_rcPadding`；内容区相对边框） |
| 单边 / 轴向 | `margin-top/right/bottom/left/x/y`、`padding-*`（值可为 `30` 或 `30px`） |
| `padding` 四值顺序 | **CSS** `top,right,bottom,left`（1～4 值简写同 CSS） |
| `margin` 四值顺序 | **CSS** `top,right,bottom,left`（同上） |
| C++ 存储 | `CDuiBox`（`SetMargin` / `SetPadding` / `m_rcMargin` / `m_rcPadding`）；构造与聚合初始化均为 TRBL |
| 文字缩进 | 用 `padding`；List/Combo/Menu 项用 `item-padding`（同 CSS 顺序） |
| 尺寸 | 控件：`width`/`height`/`min-*`/`max-*`（像素、`%`；另见下表 `auto`/`fit-content`）；窗口：`size`/`min-size`/`max-size`，亦可 `width`/`height`、`min-width`/`min-height`、`max-width`/`max-height`；树项 `item-min-width`；TabBar `tab-min-width`/`tab-max-width` |
| 子控件间距 | `gap` |
| 溢出 / 滚动 | `overflow` / `overflow-x` / `overflow-y`（映射 `EnableScrollBar`；旧名 `v-scrollbar`/`h-scrollbar` 仍可用） |
| 绝对定位 | `position="absolute"`；偏移用 `margin`；`position-align`；亦可写百分比重叠矩形（扩展语法） |
| 文字对齐 | `text-align` / `vertical-align`（可用 `middle`/`vcenter`）；列表项 `item-text-align` / `item-vertical-align` |
| 容器对齐 | `justify-content` / `align-items`（HBox 主轴水平、VBox 主轴竖直）；FlowLayout 的 `align` 仅为 `justify-content` 别名 |
| 边框 | `border="1px solid red"` 简写；细项 `border-width` / `border-color` / `border-style` / `border-radius`（CSS 半径：`12`/`12px` 或 `rx,ry`） |
| 颜色 | `ParseColorString` / C++ `DWORD`：CSS `#RRGGBBAA` / `0xRRGGBBAA`；`rgb()`/`rgba()`/`hsl()`/`hsla()`、命名色 |
| 背景 | `background-color`；`background` / `background-image` 可写 `linear-gradient`、`url(...)` 或位图 DSL |
| 状态图 | 优先 `background-image` + `:hover`/`:active`；`image-*` 留给 source 切割多态图 |
| 字体 | `font-family` / `font-size` / `font-weight` / `font-style` / `text-decoration`；列表项 `item-font-*` |
| 透明度 | 控件 `opacity`/`alpha`（默认乘祖先；`opacity-isolate`；父 `opacity-propagate=false`）；C++：`SetOpacityF` / `GetEffectiveOpacityF`；Edit/RichEdit/WebBrowser OSR 见各页；窗口 `opacity`/`alpha`；壁纸透出见 `wallpaper-bleed` |
| 命中测试 | `pointer-events`（`none`/`auto`）；旧名 `mouse` / `mouse-child` |
| 命名样式 | `class` / `style` → `<Default name>` |
| 窗口拖拽 | `html { action: title; }` 落到 root；控件仍可用 `action="title"`。子控件若 `PreferClientHit()`（已配 `*-hover`/`*-active`、SETCURSOR、cursor 等）保持客户区，悬停态才会生效 |
| 控件缩窗 | 任意控件（含 `TabLayout`）`window-resize` / `window-size-box`：按边启用宿主 HWND 缩放热区；与窗口 `size-box` 互补。`size-box` / `window-size-box` 四值均为 **左,上,右,下（LTRB）**，与 `margin`/`padding` 的 CSS 上右下左不同。若只要控件上的边、不要整窗其它边，把窗口 `size-box` 对应边设为 `0` |

注意：`padding` 是内边距；外边距用 `margin`。

### `width` / `height`：`auto` 与撑满

基类 [`CControlUI`](../../src/DuiLib/Core/UIControl.h) 解析：

| 写法 | 语义 |
|------|------|
| 不写 / `width` 未设（固定宽为 0） | HBox/VBox 主轴上 `EstimateSize` 为 0 → **撑满剩余**（可伸缩） |
| `width="120"` / `height="40"` | 固定像素 |
| `width="50%"` | 相对父级**可用**宽的百分比 |
| `width="auto"` / `width="fit-content"` | 固有尺寸：置 `AutoCalcWidth`，清固定宽与 `%`；由控件 `EstimateSize` 给出非 0 宽 |

说明：

- **Label / Text / Button 等**在 `auto` 下会按文字测量，得到固有宽（见 [Label.md](Label.md)）。
- **普通 Control / 多数容器**若 `EstimateSize` 仍返回 0，写 `auto` 后布局表现仍接近「撑满」，**不会**自动按子项汇总测宽。
- 与「不写 width」相比：`auto` 明确表示意图为 fit-content；真正生效依赖该控件是否实现内容测量。

### CSS 伪类（解析期）

`:hover` / `:active` / `:disabled` / `:focus` / `:checked`（及 `:selected`、`#id:hover`）在 `UIDlgBuilder` 改写为状态属性后合并到基选择器（**非**运行时级联）：

| 基属性 | 改写为 |
|--------|--------|
| `background-color` | `background-color-hover` / `-active` / `-disabled` / `-focus` / `-selected` |
| `color` | `color-hover` / `-active` / `-disabled` / `-focus` / `-selected` |
| `border-color` | `border-color-hover` / `-active` / `-disabled` / `-focus` |
| `image` | `image-hover` / `-active` / `-disabled` / `-focus` / `-selected` |
| `background-image` | `background-image-hover` / `-active` / `-disabled` / `-focus` / `-selected` |
| `foreground-image` | `foreground-image-hover` / `-active` / `-selected` |
| `icon-tint` / `icon-color` | `icon-tint-hover` / `-active` / `-disabled` / `-focus` / `-selected`；Button / ListLabel / TabButton / Menu / TreeNode |
| `item-color` | `item-color-hover` / `-selected`（含 `:active`）/ `-disabled`；List / VirtualList / Combo / Menu |
| `item-background-color` | `item-background-color-hover` / `-selected` / `-disabled` |
| `item-image` | `item-image-hover` / `-selected` / `-disabled` |
| `item-foreground-image` | `item-foreground-image-hover` / `-selected` |

基类已支持容器直接写 `VBox:hover { background-color: ... }`。

窗口级属性（`layered`、`shadow*`、`caption`、`min-size` 等）见 [Window.md](Window.md)。

---

## 必读差异（相对标准 HTML/CSS）

| 主题 | DuiLib Ultimate | HTML/CSS |
|------|-----------------|----------|
| `padding` / `margin` 四值 | **CSS** `top,right,bottom,left`（`CDuiBox`）；值可带 `px`（含多值 `10px 20px`） | 同左 |
| `position` | 仅 `absolute`/`static` + 扩展 | 另有 relative/fixed/sticky |
| `border-radius` | CSS 半径：`12`/`12px` 或 `cx,cy`（存储与绘制均为半径；GDI 椭圆直径在底层 ×2） | 另有 1–4 角 + `/` |
| 颜色 | `#` / `0x` / C++ `DWORD` 均为 **CSS** `RRGGBBAA`；`rgb()`/`rgba()`/`hsl()`/`hsla()`、命名色 | 像素缓冲仍为 Win32 `AARRGGBB` |
| 伪类 | 解析期改写（含 `:focus` / `:checked`） | 运行时级联；另有更多伪类 |
| 图片值 | `url(...)`、裸路径、`file='…' corner='…'` DSL | 标准多为 `url()` |

## 已对齐（优先使用）

`margin` / `padding`（及单边）、`width`/`height`/`min-*`/`max-*`、`gap`、`justify-content`/`align-items`、`overflow`/`overflow-x`/`overflow-y`、`border`/`border-width`/`border-color`/`border-style`/`border-radius`、`background`/`background-color`/`background-image`、`color`、`font-family`/`font-size`/`font-weight`/`font-style`/`text-decoration`、`text-align`/`vertical-align`、`opacity`/`alpha`/`opacity-inherit`/`opacity-isolate`/`opacity-propagate`/`child-opacity-inherit`、`pointer-events`、`class`/`style`、`disabled`、`title`/`tooltip`、`draggable`、`accesskey`、`contextmenu`、`cursor`（含 CSS 关键字）、`id`、`enabled`/`visible`、`visibility`/`display`（映射可见性）。

窗口根上的 `selected-color` 是默认选中**背景**色（与 Option 的 `color-selected` 不同）。

## 状态图约定

| 场景 | 推荐 |
|------|------|
| 纯色 / 整图背景态 | `background-image` + `:hover`/`:active`/`:disabled`/`:focus`/`:checked`（或 `background-image-hover` / `-focus` / `-selected` 等） |
| 标题栏等 **source 切割** 多态图 | 继续用 `image` / `image-hover` / `image-active` |

## 非标准（控件 / 桌面模型）

| 属性 | 说明 |
|------|------|
| `showhtml` | 迷你标签，非浏览器 HTML；`<c …>` 色值走 `ParseColorString`（含命名色 / `rgb()`） |
| TabBar `tab*`、List 行线/多选、ScrollBar `thumb-image*` / `rail-image*` | 控件 DSL |
| `action=title`、`layered*`、`shadow*` | 桌面窗口模型 |

## 文档索引

| 文档 | 覆盖 |
|------|------|
| [Control.md](Control.md) | 基类 `CControlUI` |
| [Container.md](Container.md) | 容器与布局 |
| [Window.md](Window.md) | 窗口 / `html` 根 |
| [Label.md](Label.md) | Label |
| [Button.md](Button.md) | Button |
| [Option.md](Option.md) | Option / CheckBox |
| [Edit.md](Edit.md) | Edit |
| [RichEdit.md](RichEdit.md) | RichEdit |
| [List.md](List.md) | List 族 |
| [VirtualList.md](VirtualList.md) | VirtualList |
| [Combo.md](Combo.md) | Combo |
| [ScrollBar.md](ScrollBar.md) | ScrollBar |
| [Progress.md](Progress.md) | Progress / Slider |
| [SvgBox.md](SvgBox.md) | SvgBox |
| [Accordion.md](Accordion.md) | Accordion |
| [Carousel.md](Carousel.md) | Carousel |
| [TreeView.md](TreeView.md) | TreeView |
| [Menu.md](Menu.md) | Menu |
| [TabBar.md](TabBar.md) | TabBar / TabButton |
| [TitleBar.md](TitleBar.md) | TitleBar |
| [HotKey.md](HotKey.md) | HotKey |
| [GroupBox.md](GroupBox.md) | GroupBox |
| [Ring.md](Ring.md) | Ring |
| [FadeButton.md](FadeButton.md) | FadeButton |
| [RollText.md](RollText.md) | RollText |
| [IPAddress.md](IPAddress.md) | IPAddress |
| [DateTime.md](DateTime.md) | DateTime |
| [ActiveX.md](ActiveX.md) | ActiveX |
| [WebBrowser.md](WebBrowser.md) | WebBrowser |
| [GifAnim.md](GifAnim.md) | GifAnim |
| [Loading.md](Loading.md) | Loading |
| [ColorPalette.md](ColorPalette.md) | ColorPalette |
| [PageControl.md](PageControl.md) | PageControl |
| [Toast.md](Toast.md) / [Modal.md](Modal.md) | C++ API |
| [Switch.md](Switch.md) | Switch |
| [Spin.md](Spin.md) | Spin / Number |
| [Segmented.md](Segmented.md) | Segmented |
| [Badge.md](Badge.md) | Badge / Tag |
| [Transfer.md](Transfer.md) | Transfer |
| [Rate.md](Rate.md) | Rate |
| [Empty.md](Empty.md) | Empty |
| [Skeleton.md](Skeleton.md) | Skeleton |
| [Image.md](Image.md) | Img / Avatar |
| [FontIcon.md](FontIcon.md) | FontIcon |
| [Steps.md](Steps.md) | Steps |
| [Timeline.md](Timeline.md) | Timeline |

## 图例（各控件页）

- **接近 HTML/CSS**：名称与语义大体可用
- **部分接近**：伪类属性形式、或同名异义
- **非标准**：无 HTML 等价，或仅 DuiLib 皮肤/桌面 UI 模型
