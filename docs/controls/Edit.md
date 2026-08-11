# Edit

| | |
|--|--|
| 类 | `CEditUI` |
| XML | `<Edit>` |
| 源码 | `src/DuiLib/Control/UIEdit.*` |
| 继承属性 | 见 [Label.md](Label.md) |

> 本页聚焦 **属性与 HTML/CSS 的符合度**。盒模型全局约定见 [Attributes.md](Attributes.md)。

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| `readonly` | 只读 |
| `password` | 密码模式（亦可用 `type="password"`） |
| `type` | `text` / `number` / `password` |
| `maxlength` | 最大字符数 |
| `placeholder` | 占位提示 |
| `placeholder-color` | 占位文字色（`ParseColorString`） |
| `value` | 文本内容（`text` 别名，近似 `<input value>`） |

默认 `padding` 为 CSS `4,10,4,10`（左右 10，圆角时文字不贴边）。皮肤若写 `padding: 6,0,6,0` 会把左右清零，需自行保留水平内边距。

### 部分接近

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `select-on-focus` | 获得焦点时全选 | 无标准属性名（行为近似） |
| `native-color` / `native-background-color` | 原生 HWND 色 | 无 |

失焦自绘文字走 **GDI ClearType**（`GetDC`），与聚焦时原生 `WC_EDIT` 观感一致；D2D 预乘目标上无法 ClearType。

**`opacity`：** 默认继承父；有效透明度 < 255 时**不创建原生 `WC_EDIT`**。不跟父淡：`opacity-isolate="true"`。

需要 **上下步进 / 小数 / min·max** 时用 [Spin / Number](Spin.md)，不要只用 `type="number"`。

### 非标准

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `password-char` | 掩码字符 | 无 |
| `image` / `image-hover` / `image-focus` / `image-disabled` | 状态皮肤图 | background-image / 伪类 |
