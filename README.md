# html / Window 属性说明

皮肤根节点可用 `<html>` 或 `<Window>`。属性可写在标签上，也可写在 `<style>` 的类型选择器里。

## 写法

**标签内联：**

```xml
<html size="800,600" caption="0,0,0,64" bktrans="false">
  ...
</html>
```

**CSS（推荐）：**

```xml
<html>
  ...
</html>
<style>
  html {
    size: 800,600;
    caption: 0,0,0,64;
  }
</style>
```

选择器 `html` 与 `window` 等价（不区分大小写）。

**优先级：** CSS 类型选择器 → 根节点内联属性（内联更高）。

属性名支持连字符（如 `header-align`）。布尔值用 `true` / `false`。颜色一般为 `#AARRGGBB`。

---

## 默认值

未写时使用下列默认（只需覆盖与默认不同的项）：

| 属性 | 默认 |
|------|------|
| `size` | 无（沿用 `Create(...)` 尺寸；写了会在创建后 `ResizeClient`） |
| `sizebox` | `4,4,6,6` |
| `caption` | `0,0,0,40` |
| `roundcorner` | `0,0` |
| `bktrans` / `layered` | `false` |
| `mininfo` | `320,240` |
| `maxinfo` | `0,0`（不限制） |
| `bkcolor` | `#FFF0F0F0`（落到 root 背景；分层且未显式设置时不强制） |
| `showshadow` | `true` |
| `shadowsize` | `6` |
| `shadowcorner` | `8,8,8,8` |
| `shadowsharpness` | `5` |
| `shadowdarkness` | `150` |
| `shadowposition` | `0,0` |
| `shadowcolor` | 黑色 |
| `shadowimage` | 无（无图片时用模糊阴影） |

精简示例：

```css
html {
  size: 800,600;
  caption: 0,0,0,64;
  mininfo: 600,480;
  shadowimage: main/shadow.png;
}
```

---

## 窗口几何

| 属性 | 格式 | 说明 |
|------|------|------|
| `size` | `宽,高` | 初始客户区大小（逻辑像素，会按 DPI 缩放） |
| `sizebox` | `左,上,右,下` | 无边框窗口边缘缩放热区 |
| `caption` | `左,上,右,下` | 标题拖拽区；常见为顶部一条，如 `0,0,0,64` |
| `roundcorner` | `cx,cy` | 窗口圆角椭圆直径 |
| `mininfo` | `宽,高` | 最小跟踪尺寸 |
| `maxinfo` | `宽,高` | 最大跟踪尺寸；`0,0` 表示不限制 |

---

## 透明 / 分层

| 属性 | 格式 | 说明 |
|------|------|------|
| `bktrans` | `true`/`false` | 同 `layered`，是否分层透明窗口 |
| `layered` | `true`/`false` | 同上 |
| `layeredopacity` | `0`–`255` | 分层整体透明度 |
| `layeredimage` | 图片路径 | 启用分层并设置分层图 |
| `opacity` / `alpha` | `0`–`255` | 窗口透明度（`SetLayeredWindowAttributes`） |
| `noactivate` | `true`/`false` | 不激活窗口 |

---

## 背景色

| 属性 | 格式 | 说明 |
|------|------|------|
| `bkcolor` / `bkcolor1` | `#AARRGGBB` | 窗口默认背景；root 未设 `bkcolor` 时自动套用 |

---

## 阴影

| 属性 | 格式 | 说明 |
|------|------|------|
| `showshadow` | `true`/`false` | 是否显示阴影 |
| `shadowsize` | 整数 | 阴影大小 |
| `shadowcorner` | `左,上,右,下` | 图片九宫格角 |
| `shadowimage` | 图片路径 | 图片阴影；不设则用算法模糊阴影 |
| `shadowsharpness` | 整数 | 模糊锐度 |
| `shadowdarkness` | 整数 | 暗度/透明度强度 |
| `shadowposition` | `x,y` | 阴影偏移 |
| `shadowcolor` | `#RRGGBB` 或 `#AARRGGBB` | 阴影颜色 |

---

## 字体与绘制杂项

| 属性 | 格式 | 说明 |
|------|------|------|
| `defaultfontcolor` | `#AARRGGBB` | 默认文字色 |
| `disabledfontcolor` | `#AARRGGBB` | 禁用文字色 |
| `linkfontcolor` | `#AARRGGBB` | 链接文字色 |
| `linkhoverfontcolor` | `#AARRGGBB` | 链接悬停色 |
| `selectedcolor` | `#AARRGGBB` | 默认选中背景色 |
| `showdirty` | `true`/`false` | 绘制更新脏区（调试） |
| `gdiplustext` | `true`/`false` | 使用 GDI+ 绘字 |
| `textrenderinghint` | 整数 | GDI+ 文字渲染提示 |
| `tooltiphovertime` | 毫秒 | Tooltip 悬停延迟 |

---

## 与控件属性的区别

- `html` / `Window` 属性作用在 **PaintManager / 窗口**，不是某个控件。
- 控件自身属性（如 `VBox` 的 `bkcolor`、`AccordionItem` 的 `header-align`）仍写在对应标签或 CSS 的类型/`#id` 选择器上。
