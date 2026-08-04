# Switch

| | |
|--|--|
| 类 | `CSwitchUI` |
| XML | `<Switch>` / `<switch>` |
| 源码 | `src/DuiLib/Control/UISwitch.*` |
| 继承 | [Option](Option.md) → [Button](Button.md) |

自绘开关（胶囊轨道 + 圆形滑块）。无皮肤图时走原生样式；设了 `image` / `image-selected` 等则回退为 Option 图片态。

点击切换开/关，通知 `selectchanged`（与 CheckBox/Option 相同）。一般不要设 `group`。

### 最小示例

```xml
<Switch text="启用通知" />
<Switch checked="true" text="深色模式" track-color-checked="#722ED1FF" />
<Switch checked-text="开" unchecked-text="关" track-size="52,24" />
<Switch checked="true" disabled="true" text="已锁定" />
```

### 接近 HTML

| 属性 | 说明 |
|------|------|
| `checked` / `selected` | 开/关（继承 Option） |
| `disabled` | 禁用 |
| `text` | 轨道右侧标签 |

### 非标准

| 属性 | 说明 | 默认 |
|------|------|------|
| `track-size` / `switch-size` | 轨道 `宽,高` 或单值宽（高=宽/2） | `44,22` |
| `track-gap` | 轨道与标签间距 | `8` |
| `thumb-inset` | 滑块相对轨道内边距 | `2` |
| `checked-text` / `unchecked-text` | 轨道内开/关文案 | 空 |
| `track-color` | 关态轨道色 | `#00000040` |
| `track-color-checked` / `accent-color` / `checked-color` | 开态轨道色 | `#1677FFFF` |
| `track-color-hover` / `track-color-checked-hover` | 悬停 | 略深 |
| `track-color-disabled` / `track-color-checked-disabled` | 禁用 | 半透明 |
| `thumb-color` / `thumb-color-disabled` | 滑块 | 白 / 浅灰 |
| `inner-color` / `inner-color-unchecked` | 轨道内文字色 | 白 |

继承 Option 的 `group` 等仍可用，但开关通常单独使用。

### 通知

| 类型 | 说明 |
|------|------|
| `selectchanged` | 开/关变化（`wParam` 为选中态，与 CheckBox 一致） |
