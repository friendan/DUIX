# IPAddress / IPAddressEx

| | |
|--|--|
| 类 | `CIPAddressUI`、`CIPAddressExUI` |
| XML | `<IPAddress>`、`<IPAddressEx>` |
| 源码 | `src/DuiLib/Control/UIIPAddress.*`、`UIIPAddressEx.*` |
| 继承 | IPAddress → [Label](Label.md)；IPAddressEx → [Edit](Edit.md) |

两个 IP 输入实现，按场景选用。

## IPAddress（系统控件）

聚焦时创建 Windows **SysIPAddress32**（`WC_IPADDRESS`）子窗口；失焦后销毁，文本画回 Label。

构造时若尚未设置文本，会尝试用本机 IP 初始化（`gethostname` / `gethostbyname`）。

聚焦态原生 HWND 通过 `native-background-color` / `native-color` 染色（`WM_CTLCOLOREDIT` / `WM_CTLCOLORSTATIC`）；主题 `chrome` 会写入这两项，打开中热切会 `SyncNativeShellColors`。

### 示例

```xml
<IPAddress name="ip" width="160" height="28"
    border="1px solid #D9D9D9" background-color="#FFFFFFFF"
    native-background-color="#FFFFFFFF" native-color="#000000E0" />
```

```cpp
CIPAddressUI* p = static_cast<CIPAddressUI*>(
    m_pm.FindControl(_T("ip"))->GetInterface(DUI_CTR_IPADDRESS));
DWORD dw = p->GetIP();   // 与 IPM_GETADDRESS 同布局的 DWORD
p->SetIP(MAKEIPADDRESS(192, 168, 1, 1));
p->SetReadOnly(true);
```

### 属性

| 属性 | 说明 |
|------|------|
| （Label 通用） | 盒模型、`background-color`、`color`、字体等 |
| `native-background-color` | 聚焦时 SysIPAddress32 底色 |
| `native-color` | 聚焦时原生文字色；未设则用 Label `color` |
| `readonly` | 只读 |

`SetAttribute` 支持上述项；另有 C++ `SetReadOnly` / `SyncNativeShellColors`。

---

## IPAddressEx（自绘分段）

基于 Edit 的四段数字输入（点分隔），不创建系统 IP 控件。构造时 `SetReadOnly(true)`，用按键在段内输入、方向键切段。

### 示例

```xml
<IPAddressEx name="ip_ex" width="160" height="28"
    border="1px solid #D9D9D9" background-color="#FFFFFFFF" color="#333333FF" />
```

```cpp
CIPAddressExUI* p = static_cast<CIPAddressExUI*>(m_pm.FindControl(_T("ip_ex")));
p->SetIP(_T("10.0.0.8"));
CDuiString s = p->GetIP();   // "10.0.0.8"
```

### 属性

无额外 SetAttribute；Edit / Label 属性可用。读写 IP 用 C++ `SetIP(LPCTSTR)` / `GetIP()`（字符串）。

### 交互要点

- 数字键写入当前段，满 3 位自动跳下一段；段值钳制 0–255。
- ↑/↓ 增减当前段；←/→、`.` 切换段。
- 适合不想依赖 `comctl32` IP 类、或要完全自绘皮肤时。

---

## 怎么选

| | IPAddress | IPAddressEx |
|--|-----------|-------------|
| 实现 | 系统 SysIPAddress32 | 自绘 + 键盘 |
| 读写 | `DWORD` `GetIP`/`SetIP` | 字符串 `GetIP`/`SetIP` |
| 皮肤 | 聚焦态可用 `native-*` 跟主题；关闭视觉样式以免系统白底 | 更易与 DuiLib 皮肤一致 |
| 依赖 | `ICC_INTERNET_CLASSES` | 无额外 common control |
