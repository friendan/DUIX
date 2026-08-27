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
| `native-color` / `native-background-color` | 原生 HWND 色；**未设置时分别跟 `color` / `background-color`（主题默认）** | 无 |

失焦自绘文字走 **GDI ClearType**（`GetDC`），与聚焦时原生 `WC_EDIT` 观感一致；D2D 预乘目标上无法 ClearType。

聚焦时原生窗高度仅为字体行高（垂直居中），底色若仍是硬编码白，深色/主题底上会像一条白带。未写 `native-background-color` 时已跟 `background-color`；主题热切会 `SyncNativeEditColors`。

**`opacity`：** 默认继承父；有效透明度 < 255 时**不创建原生 `WC_EDIT`**。不跟父淡：`opacity-isolate="true"`。

删除仍持有焦点的 Edit 是安全的：manager 会先 `ReapObjects` 清掉 `m_pFocus`，且 `SetFocus` 在同步 HWND 焦点前先置空焦点，避免 `DestroyWindow` 触发 paint `WM_SETFOCUS` 时在析构中途重建原生编辑框。

需要 **上下步进 / 小数 / min·max** 时用 [Spin / Number](Spin.md)，不要只用 `type="number"`。

需要 **左右功能图标 / 清除 / 密码显隐** 时用 [EditBox](EditBox.md)，不要在裸 Edit 上叠控件。

### 聚焦原生 WC_EDIT：插入符与中文输入法（排障）

现象常被误判成「没焦点 / CTLCOLOR 不对」：**能输入、系统已建 caret（`GetGUIThreadInfo` 里 `hwndCaret`/`rcCaret` 正常），但肉眼看不到闪烁竖线**；EditBox 上再叠加 **中文候选窗飘走或看不见**。根因在非分层 D2D Present 与子窗 XOR 光标，不在 EditBox 业务逻辑本身。

#### 根因（已用 `edit_caret_diag` 验证过）

1. **Present BitBlt 盖住 `WS_CHILD` Edit**  
   非分层离屏内容经 `Present` → `BitBlt` 到窗口 DC。即使用 `BeginPaint` 的 `ps.hdc` + 窗口 `WS_CLIPCHILDREN`，子窗上的**系统插入符仍是 XOR**，父窗一刷就没；侧栏等无关脏区 Present 也会间接影响。  
   日志特征：`focus==edit=1`、`GUI_CARETBLINKING=1`、`hwndCaret=edit`，但用户仍说「没光标」。
2. **CTLCOLOR / 视觉样式（次要加固）**  
   自定义刷子时需 `SetBkMode(OPAQUE)` + `SetBkColor`（对齐 [IPAddress](IPAddress.md)）；并 `SetWindowTheme(edit, L"", L"")`，避免 VisualStyle 盖绘。  
   **仅改 CTLCOLOR / Theme 不够**；没有 Present 侧处理时 caret 句柄在、画面仍无。
3. **EditBox IME**  
   Present 后若对焦点 Edit **无条件** `RedrawWindow`，会冲掉候选/组字窗；EditBox 外壳 `Invalidate` 更频，比裸 Edit 更明显。

#### 现行做法（勿轻易拆）

| 位置 | 行为 |
|------|------|
| [`UIManager.cpp`](../../src/DuiLib/Core/UIManager.cpp) 非分层 Present | 对可见 `WC_EDIT`/`EditWnd` 子窗 `ExcludeClipRect`；Present 后若焦点在 Edit 且**未组字**（`ImmGetCompositionString(GCS_COMPSTR)` 长度 ≤ 0）则 `RedrawWindow(..., RDW_INVALIDATE\|RDW_UPDATENOW)` |
| [`UIEdit.cpp`](../../src/DuiLib/Control/UIEdit.cpp) `CEditWnd` | CTLCOLOR：`OPAQUE` + `SetBkColor`；Init：`SetWindowTheme("", "")`；`WM_IME_STARTCOMPOSITION` / `WM_IME_COMPOSITION`：`ImmSetCompositionWindow` + `ImmSetCandidateWindow`（钉在 `GetCaretPos`） |
| EditBox | 内嵌 Edit 共用上述路径；勿在组字过程中额外整树狂刷 |

**组字判定**：用 `GCS_COMPSTR` 长度，不要用 `ImmGetOpenStatus`（中文模式常为开，会误跳过 caret 刷新）。

#### 改 Present 时注意

- **不要**去掉「未组字时的 `RedrawWindow`」——曾因此光标立刻消失。  
- **不要**在每次 Present 后无条件 `RedrawWindow`——会破坏 EditBox 中文输入法。  
- **不要**只靠 `HideCaret`/`ShowCaret` 替代 `RedrawWindow`（实测不够）。  
- **不要**再上「自绘 soft-caret / Present 里 HideCaret 循环」当产品方案。  
- 冒烟：`duidemo` → 表单 → 裸 Edit 与 EditBox：闪烁光标 + 中文候选在输入框附近。

### 非标准

| 属性 | 说明 | HTML/CSS 对照 |
|------|------|---------------|
| `password-char` | 掩码字符 | 无 |
| `image` / `image-hover` / `image-focus` / `image-disabled` | 状态皮肤图 | background-image / 伪类 |
