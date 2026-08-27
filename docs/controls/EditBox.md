# EditBox

| | |
|--|--|
| 类 | `CEditBoxUI` / `CEditBoxLeftUI` / `CEditBoxRightUI` |
| XML | `<EditBox>` / `<EditBoxLeft>` / `<EditBoxRight>` |
| 源码 | `src/DuiLib/Control/UIEditBox.*` |
| 继承属性 | 见 [Container.md](Container.md) / [Control.md](Control.md)；文本属性转发见 [Edit.md](Edit.md) |

> 带左右功能区的输入框。统一边框/底色画在外壳；中间内嵌无边框 [`Edit`](Edit.md)。结构对齐 [TitleBar](TitleBar.md) 的左右容器思路。

### 布局

```
[ EditBoxLeft ][ 内嵌 Edit 弹性 ][ EditBoxRight + 内建钮 ]
```

- 用 **`<EditBoxLeft>`** / **`<EditBoxRight>`** 声明左右槽（别名 `EditBoxPrefix` / `EditBoxSuffix`），可直接写 `gap`、`padding` 等容器属性
- 未包容器的直接子控件仍默认进左侧；此时也可用外壳属性转发：`left-gap` / `left-padding` / `right-*`（别名 `prefix-*` / `suffix-*`），或 C++ `GetLeftSlot()` / `GetInterface(DUI_CTR_EDITBOXLEFT)`
- 内建清除 / 密码显隐钮始终在右侧最末
- **空槽隐藏**：左右槽没有自身可见子项时 `SetVisible(false)`（`IsSelfVisible`）
- 点装饰区空白会把焦点交给内嵌 Edit

### 示例

```xml
<!-- 直接子控件进左侧；用 left-* 设左槽 -->
<EditBox name="search" height="32" placeholder="搜索" clearable="true"
    left-gap="2" left-padding="0,0,0,2"
    border="1px solid" border-radius="4">
  <Button lucide="search" icon-size="14" width="28" height="28" kind="none"
      background-color="#00000000" mouse="false" />
</EditBox>

<EditBox name="pwd" type="password" password-toggle="true"
    placeholder="密码" height="32" border="1px solid" border-radius="4" />

<!-- 左选引擎、右搜索按钮 -->
<EditBox name="engine" height="32" placeholder="输入关键词…" clearable="true"
    border="1px solid" border-radius="4">
  <EditBoxLeft>
    <Combo name="engine_sel" auto-width="true" height="28" drop-box-size="90,100"
        padding="0,2,0,4" border="0" background-color="#00000000">
      <ListLabelElement text="百度" height="28" selected="true" />
      <ListLabelElement text="必应" height="28" />
      <ListLabelElement text="谷歌" height="28" />
    </Combo>
  </EditBoxLeft>
  <EditBoxRight>
    <Button text="搜索" width="52" height="28" kind="primary" />
  </EditBoxRight>
</EditBox>
```

### 非标准属性

| 属性 | 说明 | 默认 |
|------|------|------|
| `clearable` / `clear` | 有文本时显示清除钮（name=`editbox_clear`） | false |
| `password-toggle` | `type=password` 时显示显隐钮（name=`editbox_eye`） | false |
| `icon-btn-width` | 内建钮边长（逻辑像素） | 28 |
| `submit-on-enter` / `enter-submit` | 回车时激活 `submit-button` 指定的钮 | true |
| `submit-button` / `enter-button` | 提交钮 `name`：先子树再整窗查找；空=不点钮 | — |
| `history` / `history-enabled` / `show-history` | 聚焦时弹出历史下拉（内存） | false |
| `history-max` / `history-max-count` | 历史条数上限；超出丢掉最旧，重复提到最前 | 10 |
| `left-*` / `prefix-*` | 转发到左槽（如 `left-gap`、`left-padding`） | — |
| `right-*` / `suffix-*` | 转发到右槽 | — |

转发到内嵌 Edit：`text`/`value`、`placeholder*`、`readonly`、`password*`、`type`、`maxlength`、`select-on-focus`、`color`/`disabled-color`、`font*`、`text-align`、`native-*`、`image*`、`tip`/`tooltip`/`title` 等（见源码 `IsEditForwardAttr`）。`tip` 会映射为 `tooltip`。

`enabled` / `disabled` 走外壳 `SetEnabled`，级联到左右槽、内建钮与内嵌 Edit（勿只禁输入框）。

外壳负责：`border*`、`background-color`、`padding`、`height`、焦点边框（`IsFocused` 含内嵌 Edit 焦点）。  
失焦用普通 `border-color`，聚焦用 `border-color-focus`（主题默认已带）；边框在子控件之后再画一遍，避免被左右槽盖住。

### API

| 方法 | 说明 |
|------|------|
| `GetEdit()` | 内嵌 `CEditUI*` |
| `GetLeftSlot()` / `GetRightSlot()` | 左右槽（`GetPrefixSlot` / `GetSuffixSlot` 同义）；`GetInterface(EditBoxLeft/Right)` 亦可 |
| `SetClearable` / `SetPasswordToggle` | 内建钮 |
| `SetSubmitOnEnter` / `SetSubmitButton` | 回车提交 |
| `SetHistoryEnabled` / `IsHistoryEnabled` / `SetHistoryMaxCount` / `GetHistoryMaxCount` | 历史开关与上限 |
| `AddHistory` / `RemoveHistory` / `RemoveHistoryAt` / `ClearHistory` / `SetHistory` / `GetHistoryCount` / `GetHistoryItem` | 读写历史（内存，不落盘）；`AddHistory` 空/纯空白返回 false；`SetHistory(items, n)` 整表替换（`items[0]` 最新） |
| `ShowHistoryPopup` / `CloseHistoryPopup` / `DismissHistoryPopup` / `IsHistoryPopupVisible` | 弹层：手动开/关；`Dismiss`=用户关掉并抑制本轮再弹；`Show` 会清抑制 |
| `SetEnabled` | 级联禁用/启用整棵子树 |
| `SyncInnerEditChrome()` | 主题换色后同步原生 Edit 底色 |

### 历史记录

默认关闭。`history="true"` 后：

- **聚焦 / 再点输入框**且已有记录 → 在 EditBox 下方弹出列表（仿 Combo；密码模式不弹）；主窗移动/缩放或控件重布局时弹层跟随
- **回车 / 回车提交 / 点「加入」** → `AddHistory(当前文本)`（空串或纯空白忽略并返回 false；已有则挪到最前；超过 `history-max` 丢掉最旧）
- 点列表文字 → 填入并关闭；右侧 **×** 删除该条；顶栏 **清空** / **关闭**；点弹层与输入框外侧、或 Esc 也可关闭
- **主动关闭**（顶栏 × / Esc / 点外侧）→ `DismissHistoryPopup`：输入框仍聚焦时再点不重开；失焦后再聚焦才会再弹。业务若要强制打开用 `ShowHistoryPopup()`
- 列表项之间有分割线；弹层打开时 `AddHistory` / `Remove*` / `SetHistory` 会即时刷新列表

不持久化；进程内有效。落盘：听 `historychanged`（`DUI_MSGTYPE_HISTORYCHANGED`），再 `GetHistoryCount` / `GetHistoryItem` 写出；启动时 `SetHistory(items, n)` 灌回（`items[0]` 为最新）。  
`wParam`：`DUI_HISTORYCHANGE_ADD` / `REMOVE` / `CLEAR` / `SET`；`REMOVE` 时 `lParam` 为原下标。启动灌回也会发 `SET`，可用标志跳过写盘。

### 回车提交

默认 `submit-on-enter="true"`：内嵌 Edit 回车后：

1. 再发一条 `RETURN`，`pSender` = 本 EditBox  
2. 若写了 `submit-button="xxx"`：先在 EditBox 子树按 name 找，找不到再在当前窗口 `FindControl`（钮可放在外侧同行）并 `Activate()`  
3. 未写 `submit-button` 或找不到 → 只发 `RETURN`

```xml
<EditBox name="engine" clearable="true"
    submit-on-enter="true" submit-button="go">
  <EditBoxRight>
    <Button name="go" text="搜索" kind="primary" />
  </EditBoxRight>
</EditBox>
```

业务可只听该钮的 `CLICK`。不想回车触发时设 `submit-on-enter="false"`。

### 通知

内嵌 Edit 的 `TEXTCHANGED` / `SETFOCUS` / `KILLFOCUS` / `RETURN` 会**再发一条**，`pSender` 为本 `EditBox`（可用 `GetName()` 或 `GetInterface(DUI_CTR_EDITBOX)`）。清除钮触发的改字也只发 EditBox 这条。  
原先内嵌 Edit 那条仍会先发一次（`pSender` 为内部 Edit，name 默认 `editbox_edit`）；业务请按 EditBox 的 name 判断。

历史变更：`historychanged`（`DUI_MSGTYPE_HISTORYCHANGED`），在 `AddHistory` / `RemoveHistory` / `RemoveHistoryAt` / `ClearHistory` / `SetHistory` 成功变更后发出。

原生 WC_EDIT 插入符 / IME 与 Present 相关说明见 [Edit.md](Edit.md)。
