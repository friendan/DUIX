# WebBrowser

| | |
|--|--|
| 类 | `CWebBrowserUI`（门面） / `CWebView2UI`（别名） |
| XML | `<WebBrowser>` / `<WebView2>` |
| 源码 | `src/DuiLib/Control/UIWebBrowser.*`、`IWebBrowserEngine.h`、`WebView2Engine.*`、`WebBrowserIeEngine.*`、`WebBrowserCefEngine.*` |
| 引擎 | **WebView2**（默认，需 SDK + Edge WebView2 Runtime）/ **IE**（ActiveX 兼容）/ **CEF**（stub，未链 SDK） |

多引擎可插拔：控件门面持有 `IWebBrowserEngine*`，由 `CWebBrowserEngineFactory` 创建。

### 最小示例

```xml
<!-- 默认：有 WebView2 编译则用 webview2，失败可回退 ie -->
<WebBrowser name="browser" home-page="https://example.com" auto-navi="true" />

<!-- 显式引擎 + 宿主模式 -->
<WebBrowser engine="webview2" host="window" home-page="https://example.com" auto-navi="true" />
<WebBrowser engine="webview2" host="composition" home-page="https://example.com" auto-navi="true" />
<WebBrowser engine="ie" home-page="https://example.com" auto-navi="true" />

<!-- 别名：强制 WebView2 -->
<WebView2 name="wv" home-page="https://example.com" auto-navi="true" />
```

```cpp
CWebBrowserUI* p = static_cast<CWebBrowserUI*>(
    m_pm.FindControl(_T("browser"))->GetInterface(DUI_CTR_WEBBROWSER));
p->SetHostMode(_T("composition")); // 或 window
p->Navigate2(_T("https://example.com/path"));
p->GoBack();
p->Refresh();
// IE only:
IWebBrowser2* pIe = p->GetWebBrowser2(); // 非 ie 引擎时为 NULL
```

### 属性

| 属性 | 说明 |
|------|------|
| `engine` | `webview2` / `ie` / `cef`；省略则优先 webview2（若编译启用）否则 ie |
| `engine-fallback` | `true`（默认）时创建失败回退 `ie` |
| `host` / `host-mode` | WebView2 宿主：`window`（默认，内核子 HWND）/ `composition`（DComp Visual + 合成宿主窗）；别名 `hwnd` / `compose` / `visual`。composition 失败自动回退 window |
| `home-page` | 主页 URL |
| `auto-navi` | `true` 时创建后自动导航到 `home-page` |
| `user-data-folder` | WebView2 用户数据目录（可选） |

### WebView2 双宿主

| `host` | 做法 | 适用 |
|--------|------|------|
| `window` | `CreateCoreWebView2Controller`，内核自带子 HWND | 默认，实现简单 |
| `composition` | `CompositionController` + 独立 DComp 宿主窗 + `SendMouseInput` | 更好贴近合成路径；**不是**每帧拷像素进 DuiLib D2D 位图 |

分层窗注意：composition 使用**独立子 HWND** 做 DComp Target，避免与 DuiLib 分层主窗的 DComp 根冲突；仍非「画进 `DoPaint`」。

### C++ API（常用）

| 方法 | 说明 |
|------|------|
| `SetEngine` / `GetEngineName` | 选择 / 查询引擎 |
| `SetHostMode` / `GetHostMode` | WebView2 宿主模式 |
| `Navigate2` / `NavigateUrl` / `NavigateHomePage` | 导航 |
| `GoBack` / `GoForward` / `Refresh` / `Refresh2` | 历史与刷新（`Refresh2` 仅 IE） |
| `GetNative` | 引擎原生指针（IE=`IWebBrowser2*`，WV2=`ICoreWebView2*`） |
| `GetWebBrowser2` / `GetHtmlWindow` | **仅 IE** |
| `SetHostEvents` | 引擎无关事件（`CWebBrowserHostEvents`） |
| `SetWebBrowserEventHandler` | **仅 IE** 旧回调（`CWebBrowserEventHandler`） |

### 构建

CMake 选项 `DUILIB_USE_WEBVIEW2`（默认 ON）。SDK 路径：

- 默认：`src/3rd/webview2/pkg/build/native`（仅保留 headers + `WebView2LoaderStatic.lib`）
- 或环境变量 `WEBVIEW2_SDK_PATH`

运行时需安装 [WebView2 Runtime](https://developer.microsoft.com/microsoft-edge/webview2/)（Evergreen）。Loader 使用 **静态库** `WebView2LoaderStatic.lib`，无需随包分发 `WebView2Loader.dll`。

### 扩展 CEF / 其它

实现 `IWebBrowserEngine`，在初始化时：

```cpp
CWebBrowserEngineFactory::Instance().Register(_T("cef"), MyCreateCefEngine);
```

本期 `engine="cef"` 为 stub（`Create` 返回 false）。

### Demo

`duidemo` →「浏览器壳」`CBrowserWnd`：首个标签 `host=composition`，后续新建标签 `host=window`；失败可回退 IE；地址栏 / 前进后退刷新已接线。
