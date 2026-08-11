# Button

| | |
|--|--|
| 类 | `CButtonUI` |
| XML | `<Button>` |
| 源码 | `src/DuiLib/Control/UIButton.*` |
| 继承属性 | 见 [Label.md](Label.md) |

> 本页聚焦 **属性与 HTML/CSS 的符合度**。盒模型全局约定见 [Attributes.md](Attributes.md)。

### 部分接近（命名或伪类形式）

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `image-hover` | — | :hover { background-image } |
| `image-active` | — | :active { background-image } |
| `image-disabled` | — | :disabled { background-image } |
| `foreground-image-hover` | — | background-image / \<img\>（取值多为 DuiLib 图串） |
| `foreground-image-active` | — | background-image / \<img\>（取值多为 DuiLib 图串） |

### 非标准 / 无 HTML 等价

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `image` | 状态皮肤图（DuiLib file='…' 串） | background-image / \<img\> |
| `image-focus` | — | :focus { background-image } |
| `state-image` | — | 无 |
| `state-count` | — | 无 |
| `bind-tab-index` | — | 无 |
| `bind-tab-layout-name` | — | 无 |
| `color-focus` | 焦点文字色 | `:focus { color }` |
| `bsicon` / `lucide` / `tabler-outline` / `tabler-filled` / `iconpark` / `remixicon` / `twicon` | 内嵌 [SvgBox](SvgBox.md) 图标库 | 无 |
| `icon` / `icon-src` | SVG 或 PNG/BMP/JPG 路径（光栅扩展名走位图槽） | 无 |
| `icon-size` / `icon-gap` | 图标边长、与文字间距（逻辑像素，随 DPI） | 无 |
| `icon-position` / `icon-pos` | `left`（默认）/ `right` / `top`（图标上）/ `bottom`（图标下、文字上）；有文字时整体居中；上下排布需足够 `height`；未写死宽高时 `EstimateSize` 会计入图标 | 无 |
| `icon-tint` / `icon-color`（及 `-hover`/`-active`/`-disabled`/`-focus`） | 图标着色。**SVG**：未设则跟随文字色 / kind。**光栅 PNG/BMP/JPG**：默认原图；`icon-tint="#…"` 强制色；`auto`/`true` 跟随文字色（同 SVG）；`none`/`false`/`original` 强制原图。CSS 伪类可改写 | 无 |
| `loading` / `loading-type` / `loading-disable` | 图标位显示 [Loading](Loading.md)；`loading-type` 同 Loading 的 `type`（默认 `css`）；`loading-disable` 默认 `true`（loading 时自动禁用，结束恢复原先 enabled） | 无 |

继承 Label（含 `color-hover` / `color-active`）与 Control（含 `background-color-*` / `border-color-*`）；Button 不再单独存一份状态背景/边框色。

默认悬停光标为手型（`cursor=hand`）；需要箭头时写 `cursor="arrow"`。Option / CheckBox 等继承 Button 的同样默认。

### 图标示例

```xml
<!-- 图标在左（默认）；图标+文字整体居中 -->
<Button text="图标左" kind="primary" lucide="chevron-left" icon-position="left" />
<!-- 图标在右 -->
<Button text="图标右" kind="success" lucide="chevron-right" icon-position="right" />
<!-- 纯图标 / 上下结构 -->
<Button kind="info" lucide="search" width="36" height="36" />
<Button text="上传" iconpark="upload" icon-position="top" />
<Button text="下载" lucide="download" icon-position="bottom" />
<!-- PNG：默认原图；auto / 显式色 才着色 -->
<Button text="原图" icon="menu/icon.png" icon-size="16" />
<Button text="跟随" kind="primary" icon="menu/icon.png" icon-tint="auto" icon-size="16" />
<Button text="着色" kind="danger" icon="menu/icon.png" icon-tint="#FFFFFF" icon-size="16" />
<!-- 加载中（图标位转圈；默认自动禁用） -->
<Button text="提交中" kind="primary" lucide="upload" loading="true" loading-type="css" />
<!-- 不自动禁用 -->
<Button text="可点转圈" lucide="upload" loading="true" loading-disable="false" />
```

CSS 伪类示例：

```css
Button:hover { icon-tint: #FF4D4FFF; }
#btn_save:active { icon-color: #0958D9FF; }
```
