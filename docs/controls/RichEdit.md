# RichEdit

| | |
|--|--|
| 类 | `CRichEditUI` |
| XML | `<RichEdit>` |
| 源码 | `src/DuiLib/Control/UIRichEdit.*` |
| 继承属性 | 见 [Control.md](Control.md) / Container |

> 本页聚焦 **属性与 HTML/CSS 的符合度**。盒模型全局约定见 [Attributes.md](Attributes.md)。

### 接近 HTML/CSS

| 属性 | 说明 |
|------|------|
| `multiline` / `readonly` / `password` | 编辑模式 |
| `maxlength` | 最大字符数 |
| `placeholder` / `placeholder-color` | 占位；`theme="chrome"` 时 `placeholder-color` 跟 `color-text-secondary` |
| `text-align` / `color` / `font-family` / `font-size` | 文字样式 |
| `overflow` / `overflow-x` / `overflow-y` | 映射启用滚动；优先于单独写 `v-scrollbar` |
| `v-scrollbar` / `h-scrollbar` | 布尔开关（兼容） |
| `auto-vscroll` / `auto-hscroll` | 随输入滚动 |
| `want-tab` / `want-return` / `want-ctrl-return` | 按键消费 |

### 非标准

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `transparent` / `rich` | 透明 / 富文本模式 | 部分接近 |
| `placeholder-align` | 占位对齐 | 无 |

文字缩进用 `padding`（盒模型内边距）。另可用 `text-padding` 再缩文字区；**不要**指望把 `padding` 写很大来代替——旧版曾把 `padding` 扣两遍导致矮控件文字区为空。

**撑满剩余高度：** 不写 `height`（固定高为 0）时 `EstimateSize` 返回 0，由父 VBox/手风琴 `fill` 等分摊剩余空间；再配 `overflow-y` / `v-scrollbar` 即可内部滚动。

**只读设全文：** `SetText` 走 `TxSetText`，`readonly` 下也可更新内容（勿依赖 `EM_REPLACESEL`）。

**绘制：** 离屏用 `CreatePixelBuffer`（正高度 bottom-up，GDI/`TxDraw` 可靠）→ 修 alpha → **垂直翻转扫描行**（供 D2D top-down 上传）→ `DrawImage` → `DestroyPixelBuffer`（会清 D2D 缓存，避免 HBITMAP 句柄复用串到 SvgBox）。勿在 `GetOrCreateBitmap` 里对所有 bottom-up 图全局翻转。

**`opacity`：** 内容贴图乘 `ScaleImageFade()`（默认含祖先；`opacity-isolate` 仅自身）。

**右键菜单：** 默认开启（`menu` / `contextmenu` 默认为真）：全选 / 复制 / 粘贴。只读时粘贴灰掉；无选区时复制灰掉。`menu="false"` 关闭。

**复制：** `Copy` 写 `CF_UNICODETEXT` 到系统剪贴板（不用 `WM_COPY` 的 OLE 延迟渲染）；析构前 `OleFlushClipboard`，避免关窗卡死。

**插入符闪烁：** 聚焦时 host 开 `CARET_BLINK_TIMERID` TimerQueue → `UIMSG_RICHEDIT_TICK` → `m_bDrawCaret` 相位；**DoPaint 自绘竖线**（内部坐标，**勿**在父 HWND `CreateCaret`，以免与 WC_EDIT 子窗争用）。引擎 `TxSetTimer` 仅应答。见 [Messages.md](Messages.md)。
