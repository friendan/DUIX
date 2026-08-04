# ActiveX

| | |
|--|--|
| 类 | `CActiveXUI` |
| XML | `<ActiveX>` |
| 源码 | `src/DuiLib/Control/UIActiveX.*` |
| 继承属性 | 见 [Control.md](Control.md) |

ActiveX / OLE 控件宿主：按 CLSID（或 ProgID）创建控件，可窗口化或无窗口绘制。内嵌网页见 [WebBrowser.md](WebBrowser.md)（固定 `CLSID_WebBrowser`）。

### 最小示例

```xml
<ActiveX name="ax" width="400" height="300"
    clsid="{8856F961-340A-11D0-A96B-00C04FD705A2}"
    delay-create="true" />
```

```cpp
CActiveXUI* p = static_cast<CActiveXUI*>(m_pm.FindControl(_T("ax")));
IUnknown* punk = NULL;
p->GetControl(IID_IUnknown, (LPVOID*)&punk);
```

### 非标准属性

| 属性 | 说明 |
|------|------|
| `clsid` | CLSID 字符串（`{…}`）或可解析的 ProgID；设置时调用 `CreateControl` |
| `module-name` | 可选：从指定模块 `DllGetClassObject` 创建（不走系统注册表时） |
| `delay-create` | `true` 时延迟到需要时再创建（默认实现支持） |

其余尺寸、可见性等走 Control。

### C++ API

| 方法 | 说明 |
|------|------|
| `CreateControl(CLSID)` / `CreateControl(LPCTSTR)` | 创建控件 |
| `GetControl(iid, pp)` | QueryInterface |
| `GetHostWindow()` | 宿主 HWND（有窗口时） |
| `SetDelayCreate` / `SetModuleName` / `SetMFC` | 创建策略 |

分层窗口 / D2D 下嵌入 HWND 类 ActiveX 需注意与渲染路径的混绘；无窗口 ActiveX 走 `IViewObject::Draw`。
