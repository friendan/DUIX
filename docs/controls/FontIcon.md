# FontIcon

| | |
|--|--|
| 类 | `CFontIconUI` |
| XML | `<FontIcon>` / `fonticon` |
| 源码 | `src/DuiLib/Control/UIFontIcon.*` |
| 继承 | [Label](Label.md) → [Control](Control.md) |

圆形或圆角矩形色块 + **居中原文**（不缩写、不加载图）。`text` 可省略或为空：只画背景色块。适合单字 / 缩写固定文案 / iconfont 码点 / 纯色圆点。

与 [Avatar](Image.md) 区别：Avatar 可带头像图，无图时按姓名**缩写**；FontIcon 始终画指定 `text`（无字则不画字）。

### 最小示例

```xml
<FontIcon text="设" size="40" />
<FontIcon text="A" shape="rounded" size="32" kind="success" />
<FontIcon size="24" />  <!-- 无字：仅主题色背景；悬停跟 primary-hover -->
<FontIcon text="字" size="40" shape="rounded"
          background-color="#722ED1FF" color="#FFFFFFFF"
          background-color-hover="#B37FEBFF" />
```

### 属性

| 属性 | 说明 | 默认 |
|------|------|------|
| `text` | 居中原文（可含字体图标码点）；**空则只显示背景** | 空 |
| `shape` | `circle` / `rounded`（`roundrect`/`rect` 同 rounded） | `circle` |
| `size` | `small` 24 / `default`\|`medium` 32 / `large` 40 / `xlarge`\|`xl` 64，或像素 | `40` |
| `kind` | `primary` / `success` / …；未设则跟主题 primary | `none` |
| `background-color` | 底色；设置后不再跟主题默认 | — |
| `color` | 文字色；设置后不再跟主题默认 | — |
| `background-color-hover` / `background-color-active` | 悬停 / 按下底色 | 见下 |
| `color-hover` / `color-active` | 悬停 / 按下文字色 | 见下 |
| `clickable` | `true` 时点击发 `click` 通知；**默认手型光标**（仍可用 `cursor` 覆盖） | `false` |
| `border-radius` | 圆角；设置时自动按 `rounded`，且不再用默认 `size/4` | circle→`size/2`；rounded→`size/4` |
| `font` / `font-family` / `font-size` | 继承 Label，供 iconfont | — |

### 颜色

绘制时按状态解析（热切主题无需额外刷壳）：

1. 显式 `background-color-hover` / `color-hover`（及 active）→ 优先
2. 显式 `background-color` / `color` → 自定义常态（无 hover 属性则悬停仍用常态色）
3. 有 `kind` → kind Normal / Hover / Active
4. 否则 → 主题 `color-primary` / `color-primary-hover` / `color-primary-active`；字色 `color-primary-text`

默认开启悬停命中（`PreferClientHit`），避免被 `html{action:title}` 吃成拖窗。

`clickable="true"` 时与 [Label](Label.md) 相同发 `click`；`SetClickable(true)` 会把光标设为手型（`cursor` 已显式设置则不覆盖）。

### API

| 方法 | 说明 |
|------|------|
| `SetShape` / `GetShape` | `ShapeCircle` / `ShapeRounded` |
| `SetSizePreset` | 正方形边长 |
| `SetKind` | 套 kind 常态/悬停/按下色，并保持形状圆角（不加 kind 边框） |
| `SetClickable` | 可点；开启时默认 `DUI_HAND` |

Demo：Accordion → FontIcon（悬停看变色）。
