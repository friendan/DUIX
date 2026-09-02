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

| `readonly` | 只读（仍可拖选/复制；**不显示闪烁插入符**，见下文） |

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



### 聚焦原生 WC_EDIT：插入符与中文输入法



现象常被误判成「没焦点 / CTLCOLOR 不对」：**能输入、系统已建 caret（`GetGUIThreadInfo` 里 `hwndCaret`/`rcCaret` 正常），但肉眼看不到闪烁竖线**；EditBox 上再叠加 **中文候选窗飘走或看不见**。根因在非分层 D2D Present 与子窗 XOR 光标，不在 EditBox 业务逻辑本身。



#### 根因



1. **Present BitBlt 盖住 `WS_CHILD` Edit**  

   非分层离屏内容经 `Present` → `BitBlt` 到窗口 DC。子窗上的**系统插入符是 XOR**，父窗一刷就没；侧栏等无关脏区 Present 也会间接影响。  

   特征：`focus==edit=1`、`GUI_CARETBLINKING=1`、`hwndCaret=edit`，但画面仍无光标。

2. **CTLCOLOR / 视觉样式（次要加固）**  

   自定义刷子时需 `SetBkMode(OPAQUE)` + `SetBkColor`（对齐 [IPAddress](IPAddress.md)）；Init 时 `SetWindowTheme(edit, L"", L"")`，避免 VisualStyle 盖绘。  

   **仅改 CTLCOLOR / Theme 不够**。

3. **EditBox IME**  

   Present 后若对焦点 Edit **无条件** `RedrawWindow`，会冲掉候选/组字窗；EditBox 外壳 `Invalidate` 更频，比裸 Edit 更明显。



#### 排查过程（2026-08，D2D 非分层 + Shadow）



| 尝试 | 结果 |

|------|------|

| Present 对 Edit 子窗 `ExcludeClipRect` | 文字不被 BitBlt 盖住，**XOR 插入符仍不可见** |

| Present 后 `RedrawWindow(UPDATENOW)` 恢复 XOR | 能闪一下，很快变**常亮**（`GetGUIThreadInfo` 里 `blink=1` 卡死） |

| 仅靠 `HideCaret`/`ShowCaret` | 不够 |

| RichEdit 失焦时 `DestroyCaret()` | **勿用**——会动到 WC_EDIT 子窗正在用的系统 caret，且可能把插入点重置到开头 |



结论：**不能依赖系统 XOR 插入符**；需像 RichEdit 一样 **TimerQueue 自绘竖线（软光标）**。



#### 现行方案（勿轻易拆）



| 位置 | 行为 |

|------|------|

| [`UIManager.cpp`](../../src/DuiLib/Core/UIManager.cpp) 非分层 Present | 对可见 `WC_EDIT`/`EditWnd` 子窗 `ExcludeClipRect`；Dui 焦点切到原生 Edit 时对 RichEdit `StopAllQueueTimers`。**不再** Present 后 `RedrawWindow` 恢复 XOR |

| [`UIEdit.cpp`](../../src/DuiLib/Control/UIEdit.cpp) `CEditWnd` | `HideCaret`（**勿** `DestroyCaret`）+ TimerQueue（`UIMSG_EDIT_TICK` / `EDIT_CARET_BLINK_TIMERID`）切换 `m_bDrawCaret`；`WM_PAINT` 在 `DefWindowProc` 后画 1px 竖线 |

| 光标位置 | `CallWindowProc` + `EM_GETSEL` 取字符下标；前缀文本 `GetTextExtentPoint32` + 对齐/`SB_HORZ` 算坐标。**勿**用 `EM_POSFROMCHAR`（子类 `EditWnd` 上坐标易错） |

| 光标颜色 | `GetNativeEditColor()` → `GetAdjustColor()`，与原生文字色一致（跟 `native-color` / `color` / 主题默认字色） |

| 刷新时机 | `Init`、`WM_SETFOCUS`、按键/`EN_UPDATE`/`LBUTTONDOWN`、闪烁 tick 时 `RefreshSoftCaret`（union 旧/新 caret 区 `RedrawWindow`） |

| **`readonly`** | 不启 Timer、不画软光标、`HideCaret` + `Invalidate`；`SetReadOnly(true)` 或聚焦只读框时 `ApplyReadOnlyCaretPolicy`；切回可编辑且仍有焦点时 `RestartSoftCaretBlink` |

| CTLCOLOR / Theme | `OPAQUE` + `SetBkColor`；Init：`SetWindowTheme("", "")` |

| IME | `WM_IME_STARTCOMPOSITION` / `WM_IME_COMPOSITION`：`ImmSetCompositionWindow` + `ImmSetCandidateWindow`（钉在 caret 坐标）；组字判定用 **`GCS_COMPSTR` 长度**，勿用 `ImmGetOpenStatus` |

| EditBox | 内嵌 Edit 共用上述路径 |

| RichEdit | 独立 TimerQueue + `DoPaint` 画线（见 [`UIRichEdit.cpp`](../../src/DuiLib/Control/UIRichEdit.cpp)） |



#### 改 Present / Edit 时注意



- **不要**去掉 Present 的 `ExcludeClipRect`——BitBlt 仍会盖住子窗文字。  

- **不要**再靠 Present 后 `RedrawWindow` 恢复 XOR 插入符。  

- **不要**在 RichEdit 失焦路径 `DestroyCaret()`。  

- **不要**对软光标路径使用 `DestroyCaret()`——会把插入点打回行首。  

- **不要**在组字过程中对焦点 Edit 无条件 Present 后整窗 `RedrawWindow`（冲 IME）。  

- 冒烟：`duidemo` → 表单 → 裸 Edit 与 EditBox：可编辑框闪烁光标 + 位置随输入移动 + 中文候选在输入框附近；**只读 Edit 无光标**；RichEdit 光标仍正常。



### 非标准



| 属性 | 说明 | HTML/CSS 对照 |

|------|------|---------------|

| `password-char` | 掩码字符 | 无 |

| `image` / `image-hover` / `image-focus` / `image-disabled` | 状态皮肤图 | background-image / 伪类 |


