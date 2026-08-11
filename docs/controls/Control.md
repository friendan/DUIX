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
| `width` / `height` / `min-*` / `max-*` | 尺寸；支持像素、`%`、`auto`/`fit-content`（见 [Attributes.md](Attributes.md)） |
| `name` / `id` | 控件名（`id` 为 `name` 别名） |
| `text` / `tooltip` | 文案 / 提示 |
| `enabled` / `visible` | 启用 / 可见 |
| `visibility` / `display` | `hidden`/`collapse`/`none`→隐藏；其余→显示 |
| `disabled` | `true`→禁用（HTML 习惯；与 `enabled` 互补） |
| `cursor` | Win32 名 + CSS：`pointer`/`text`/`default`/`not-allowed`/`*-resize`/`move`/`crosshair`… |
| `class` / `style` | 引用 `<Default name=…>` 命名样式（`class` 为别名） |
| `pointer-events` | `none`/`auto`（旧名 `mouse` 仍可用） |
| `opacity` / `alpha` | `0–1` / `%` / `0–255`。默认 **子乘祖先**（父设一次即可）。见下方用法 |
| `opacity-inherit` | 默认 `true`；`false`/`no`/`off`/`0` 不乘祖先 |
| `opacity-isolate` | `true` ≡ 不继承父（只看自身） |
| `opacity-propagate` / `child-opacity-inherit` | 默认 `true`；父设 `false`：自己可淡、子孙乘算时跳过本节点 |
| `wallpaper-bleed` / `bg-bleed` | 壁纸透出系数；`inherit`/`auto` 跟窗口；`solid`/`opaque`/`none`/`false` 本控件不透；数值同 `opacity`。仅影响**背景色**绘制 |
| API | `SetBackgroundImageFromMemory`（PNG/JPEG/BMP/GIF，可识别 SVG 文本）、`SetBackgroundImageFromSvg` |
| `title` / `tooltip` | 提示（`title` 为别名；Accordion/Tab 等自有 `title` 仍为标题） |
| `draggable` / `drag` | 可拖拽 |
| `accesskey` / `shortcut` | 快捷键字符 |
| `contextmenu` / `menu` | 是否使用右键菜单 |

**opacity 用法速查**

```xml
<!-- 整树一起淡 -->
<VBox opacity="0.5"> … </VBox>

<!-- 子控件不跟父 -->
<Button opacity-isolate="true" opacity="1" />

<!-- 只淡容器底，子保持不透明 -->
<VBox opacity="0.4" opacity-propagate="false"> … </VBox>
```

```cpp
p->SetOpacityF(0.5f);           // 跟父乘算（默认 inherit）
p->SetOpacityF(1.f, true);      // 同时 isolate，不受父影响
p->SetOpacityPropagate(false);  // 容器：不向下传
float a = p->GetEffectiveOpacityF();
```

**Edit**：有效透明度 <255 时禁用原生 `WC_EDIT`。**RichEdit** / **WebBrowser OSR**：贴图乘 fade。

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
| `window-resize` | 控件边缩放宿主 HWND：`true`/`all`；`false`/`none`；或边名列表 `left,top,right,bottom`（可 `l,t,r,b`，无顺序要求）。子控件命中沿父链（适合 TabLayout）。含原生子 HWND 的控件（如 WebBrowser）会按祖先热区内缩，避免盖住右/下边 | 无 |
| `window-size-box` | 四边热区厚度，顺序为 **左,上,右,下（LTRB）**，与窗口 `size-box` 相同；**不是** CSS `margin`/`padding` 的上右下左。例：`0,0,6,6` = 仅右、下各 6px。某边 `>0` 自动启用该边（若未写 `window-resize`）；已启用边厚度为 `0` 时回退窗口 `size-box`。最大化跳过 | 无 |
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
