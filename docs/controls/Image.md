# Image / Avatar

| | |
|--|--|
| 类 | `CImageUI` / `CAvatarUI` |
| XML | `<Img>` / `<Picture>` / `<ImageBox>`；`<Avatar>` |
| 源码 | `src/DuiLib/Control/UIImage.*` |
| 继承 | [Label](Label.md) → [Control](Control.md) |

专用位图控件：`object-fit` 缩放、圆角裁剪（基类 `border-radius`）、失败占位。

**不要用 `<Image>` 作为控件标签**：构建器里 `<Image name="...">` 仍是皮肤资源预声明，会被跳过。

### 最小示例

```xml
<Img src="menu/icon.png" width="72" height="72" object-fit="cover" border-radius="8" />
<Img src="bad.png" width="72" height="72" placeholder-text="加载失败" background-color="#FAFAFAFF" />
<Avatar src="menu/icon.png" size="large" />
<Avatar text="Zhang San" size="large" />
```

### Img — 接近 HTML

| 属性 | 说明 |
|------|------|
| `src` / `url` | 图片路径（支持 `url(...)`） |
| `object-fit` | `fill`（默认）/ `contain` / `cover` / `none` / `scale-down` |
| `border-radius` | 圆角裁剪（基类 DoPaint round clip） |
| `padding` / `margin` | 盒模型（继承） |
| `background-color` | 无图或 contain 留白时的底色 |

### Img — 非标准

| 属性 | 说明 | 默认 |
|------|------|------|
| `placeholder` / `placeholder-image` | 无 `src` 时占位图 | 空 |
| `error` / `error-image` | `src` 加载失败时的图 | 空 |
| `placeholder-text` | 仍无图可显示时的居中文案 | 空 |

继承 Label 的 `color` / `font` 等作用于 `placeholder-text`。

### Avatar

默认圆形（`border-radius = size/2`）、`object-fit=cover`。无可用图时画底色 + 缩写文字。

| 属性 | 说明 | 默认 |
|------|------|------|
| `src` / `url` | 头像图 | 空 |
| `text` | 姓名；无图时取缩写（两词首字母，或中文前 1～2 字） | 空 |
| `alt` | 显式缩写（优先于 `text`） | 空 |
| `size` | `small` 24 / `default`\|`medium` 32 / `large` 40 / `xlarge`\|`xl` 64，或像素 | `40` |
| `circle` | `true`/`false`；`false` 时可自设 `border-radius` | `true` |
| `fallback-background-color` | 无图底色 | `#1677FFFF` |
| `fallback-color` | 缩写文字色 | `#FFFFFFFF` |
| `background-color` / `color` | 覆盖 fallback 色 | — |

失败且未配 `error-image` 时走缩写占位（不画破图）。
