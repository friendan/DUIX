# Control（基类）

| | |
|--|--|
| 类 | `CControlUI` |
| XML | 所有控件 |
| 源码 | `src/DuiLib/Core/UIControl.*` |

> 本页聚焦 **属性与 HTML/CSS 的符合度**。盒模型全局约定见 [Attributes.md](Attributes.md)。

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| `text-align` / `vertical-align` | 文字对齐（`middle`/`vcenter` 均可） |
| `margin` / `padding` | 盒模型；属性串与 `CDuiBox` 均为 **CSS** `top[,right[,bottom[,left]]]` |
| `margin-*` / `padding-*` | 单边与 `x`/`y`；值支持 `30` / `30px` |
| `background` / `background-color` | 纯色或 `linear-gradient(...)`；态色含 `-hover`/`-active`/`-disabled`/`-focus` |
| `background-image` | 渐变、`url(...)`、裸路径，或 `file='…'` DSL；态图：`-hover`/`-active`/`-disabled`/`-focus`/`-selected` |
| `border` / `border-width` / `border-*-width` / `border-style` / `border-color` | 边框 |
| `border-radius` | CSS 半径：`12`/`12px`→等轴；或 `rx,ry` |
| `width` / `height` / `min-*` / `max-*` | 尺寸；支持 `%` |
| `name` / `id` | 控件名（`id` 为 `name` 别名） |
| `text` / `tooltip` | 文案 / 提示 |
| `enabled` / `visible` | 启用 / 可见 |
| `visibility` / `display` | `hidden`/`collapse`/`none`→隐藏；其余→显示 |
| `disabled` | `true`→禁用（HTML 习惯；与 `enabled` 互补） |
| `cursor` | Win32 名 + CSS：`pointer`/`text`/`default`/`not-allowed`/`*-resize`/`move`/`crosshair`… |
| `class` / `style` | 引用 `<Default name=…>` 命名样式（`class` 为别名） |
| `pointer-events` | `none`/`auto`（旧名 `mouse` 仍可用） |
| `opacity` | `0–1` / `%` / `0–255`；经 `GetAdjustColor` 调制绘制色 alpha |
| `title` / `tooltip` | 提示（`title` 为别名；Accordion/Tab 等自有 `title` 仍为标题） |
| `draggable` / `drag` | 可拖拽 |
| `accesskey` / `shortcut` | 快捷键字符 |
| `contextmenu` / `menu` | 是否使用右键菜单 |

### 部分接近（命名或伪类形式）

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `position` | `absolute`/`static`；`true`→absolute；亦可写百分比重叠矩形；绝对偏移用 `margin` | 另有 relative/fixed/sticky；无 % 矩形写法 |
| `background-color-hover/active/disabled` | 状态背景色 | `:hover` / `:active` / `:disabled` |
| `border-color-hover/active/disabled` | 状态边框色 | 同上 |
| `border-color-focus` | 焦点边框色 | `:focus { border-color }` |
| `action` | `title`/`close`/`min`/`max`/`move`/`copy`；见 `PreferClientHit()` | HTML form `action`（含义不同） |
| `kind` | 主题预设 primary/success/danger… | 无 |
| `outline` | kind 描边模式 | CSS `outline`（含义不同） |

### 非标准 / 无 HTML 等价

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `inner-style` | 内联属性列表字符串 | HTML `style`（命名相反） |
| `position-align` | 绝对子控件相对父级对齐 | 无；接近绝对定位 + inset/transform 组合 |
| `fore-color` | — | 无（额外着色层） |
| `color-hsl` | — | filter: hue-rotate 等（不等价） |
| `foreground-image` | — | 无（前景图叠加） |
| `drag` | 布尔；优先 `draggable` | draggable |
| `drop` | — | drop 事件 / dropzone |
| `resource-text` | — | 无 |
| `rich-event` | — | 无 |
| `user-data` | — | data-* |
| `mouse` | 布尔；优先 `pointer-events` | pointer-events |
| `keyboard` | — | tabindex / 焦点 |
| `shortcut` | 优先 `accesskey` | accesskey |
| `menu` | 优先 `contextmenu` | contextmenu |
| `virtual-wnd` | — | 无 |

### 单边盒模型

`margin-top/right/bottom/left/x/y`、`padding-*` 同理；值可为 `30` 或 `30px`。

### 颜色

`ParseColorString`：`#RGB` / `#RRGGBB` / `#RRGGBBAA` / `#RGBA`，以及 `0x…`（同序）、`rgb()`/`rgba()`、`hsl()`/`hsla()`、命名色。内部 `DWORD` 与 C++ 字面量均为 **CSS `0xRRGGBBAA`**。

### 伪类（解析期）

`:hover` / `:active` / `:disabled` / `:focus` / `:checked`（及 `:selected`）在 `UIDlgBuilder` 中改写为 `*-hover` / `*-focus` / `*-selected` 等属性，**不是**运行时 CSS 级联。
