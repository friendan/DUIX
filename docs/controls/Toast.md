# Toast

轻量通知：独立弹出 HWND，不抢焦点，自动倒计时关闭，支持多条堆叠。

| | |
|--|--|
| 类 | `CToast`、`CToastOptions` |
| 窗口 | `CToastWnd`（内部） |
| 头文件 | `UIToast.h` |
| Demo | Accordion → Toast（单行/双行/交互测试/四角） |

参考 EZUI `Toast.cpp` 的弹出层方案（非控件树内嵌）。

---

## 最小示例

```cpp
CToast::ShowSuccess(_T("保存成功"));          // 单行 + kind 图标
CToast::ShowDanger(_T("网络错误"), 5000);

CToast::Show(_T("同步完成"), _T("已上传 12 个文件"),
    CToastOptions()
        .Kind(CONTROLKIND_SUCCESS)
        .Duration(4000)
        .Align(ToastAlign_ScreenBottomRight)
        .Owner(m_hWnd));

CToast::DismissAll();

HWND h = CToast::ShowInfo(_T("可手动关闭"));
CToast::Dismiss(h);

// 整条点击跳详情（悬停默认暂停倒计时）
static void CALLBACK OnToastClick(HWND /*h*/, LPCTSTR data, void* /*user*/) {
    // 打开详情页，data 即 UserData
}
CToast::Show(_T("订单已支付"), _T("点击查看详情"),
    CToastOptions()
        .Kind(CONTROLKIND_PRIMARY)
        .Duration(8000)
        .UserData(_T("order:A1024"))
        .OnClick(OnToastClick)
        .OnDismiss(OnToastDismiss)
        .ClickDismiss(true));
```

`ToastDismissReason`：`Timeout`（倒计时）、`Manual`（× / 点击关闭 / `Dismiss`）、`Evicted`（超分组 `MaxCount` 被顶掉）。
---

## API

| 方法 | 说明 |
|------|------|
| `CToast::Show(text, opts)` | 单行正文 |
| `CToast::Show(title, text, opts)` | 标题 + 正文双行 |
| `CToast::ShowSuccess/Danger/Warning/Info` | 快捷 kind + 默认时长 |
| `CToast::Dismiss(hToast)` | 关闭单条（传入 `Show` 返回的 HWND） |
| `CToast::DismissAll` | 关闭全部 |
| `CToast::SetMaxCount(n)` / `GetMaxCount` | 每个 Align 组上限（Window* 再按 Owner）；`0`=无上限；超出关掉该组最旧的 |

### CToastOptions

| 方法 | 默认 | 说明 |
|------|------|------|
| `Title` / `Text` | | 同时有标题和正文时双行；仅正文则单行 |
| `Kind` | `INFO` | Bootstrap kind 配色 + 图标 |
| `Duration` | 4000 | 毫秒；`<=0` 不自动关 |
| `ShowClose` | true | 关闭按钮 |
| `ShowIcon` | true | 左侧图标 |
| `Icon(lib, name)` | | 覆盖 kind 默认图标；lib=`bsicon`/`lucide`/`tabler-filled`/`tabler-outline`/`iconpark`/`remixicon`/`twicon` |
| `Align` | `ScreenBottomRight` | 屏幕/窗口对齐 |
| `Owner` | 前台窗 | `Window*` 对齐基准（不设 HWND Owner，避免脏区裂开） |
| `MinWidth` / `MaxWidth` | 350 / 600 | 按正文测量自动加宽，夹在此区间；超 `MaxWidth` 则换行增高 |
| `Gap` | 16 | 边距 |
| `Height` | 0 | `0`=自动（单行 44 / 双行 68；换行时增高） |
| `PauseOnHover` | true | 鼠标悬停暂停倒计时 |
| `OnClick(fn, user)` | null | 正文 Label `clickable`；点文字回调，空白区仍可拖 |
| `OnDismiss(fn, user)` | null | 关闭回调；`Timeout` / `Manual` / `Evicted` |
| `UserData` | | 传给 OnClick / OnDismiss 的字符串 |
| `ClickDismiss` | true | 点击回调后是否自动关闭 |

### Kind 图标（默认 tabler-filled）

| Kind | 图标（tabler-filled） |
|------|------|
| Success / Primary | `circle-check` |
| Danger | `circle-x` |
| Warning | `alert-triangle` |
| Info / 其它 | `info-circle` |

可用 `Icon(lib, name)` 覆盖；整条 Toast 使用 `SetKind` 底色，矢量图标默认套 kind 前景色（`twicon` 除外）。

### ToastAlign

`ScreenTopLeft/Center/Right`、`ScreenBottomLeft/Center/Right`、`ScreenCenter`，以及对应的 `Window*` 系列。

---

## 行为说明

- `WS_POPUP | WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE`（不绑定 Owner）
- 显示时 `SW_SHOWNOACTIVATE`，不抢输入焦点
- 布局：图标 | 标题/正文 | 倒计时 | ×；背景 / 字色为当前主题的 **kind** 色（`g_kindColors`）
- `AttachDialog` 时主题窗口底不会盖掉 `toastRoot` 的 kind 底（否则 kind 前景字会对比失败）
- 宽度按正文自动加宽（Min~Max）；超出 Max 则 `word-break` 增高
- 根布局 `action="title"` 可拖动；有 `OnClick` 时正文 Label `clickable`（手形），点文字跳详情，空白区仍可拖
- `SetMaxCount(n)`：每个 Align 组最多 n 条（Window* 另按 Owner）；超出顶掉该组最旧；`0` 不限制
- `OnDismiss`：关闭时回调原因（超时 / 手动含×与 Dismiss / 被顶掉）
- 悬停默认暂停倒计时；× 仍只关闭
- 同屏多条按对齐方向堆叠；关闭后同组重排填补空位
- 用户拖走过的 Toast 脱离堆叠，之后重排/主窗跟随不再吸回原位
- `Window*` 对齐：主窗移动/缩放时 Toast 跟随（`SetWindowSubclass`）；已拖离的不跟随堆叠重排
- 圆角：`SetWindowRgn` + kind 背景色；阴影若开启会跟 `GetWindowRgn`（无 RGN 则跟 `SetBorderRadius`）
