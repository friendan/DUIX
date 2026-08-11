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
p->SetHostMode(_T("window")); // 或 composition
p->NavigateUrl(_T("https://example.com/path"));
CDuiString url = p->GetLocationUrl(); // 当前地址缓存
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
| `host` / `host-mode` | WebView2：`window`（默认）/ `composition`；外接 CEF 离屏：`osr` / `offscreen`（别名）。composition 失败自动回退 window |
| `home-page` | 主页 URL；同时作为尚未导航时的 `GetLocationUrl` 回落 |
| `auto-navi` | `true` 时创建后自动导航到 `home-page` |
| `user-data-folder` | WebView2 用户数据目录（可选） |

### WebView2 双宿主

| `host` | 做法 | 适用 |
|--------|------|------|
| `window` | `CreateCoreWebView2Controller`，内核自带子 HWND | **默认**，实现简单、与 TabLayout 显隐配合稳 |
| `composition` | `CompositionController` + 独立 DComp 宿主窗 + `SendMouseInput` | 更好贴近合成路径；**不是**每帧拷像素进 DuiLib D2D 位图 |
| `osr` / `offscreen` | 无子 HWND；门面 `DoPaint`/`DoEvent` 转引擎 | **给外接 CEF 等用**；内置 WebView2/IE 忽略。**半透明 `opacity` 只能走此路径**（`BlitWebBrowserOsrBuffer(..., pOwner->ScaleImageFade())`） |

分层窗注意：composition 使用**独立子 HWND** 做 DComp Target，避免与 DuiLib 分层主窗的 DComp 根冲突；仍非「画进 `DoPaint`」。真·像素进控件树请用 `osr` + 外接引擎。`host=window` / `composition` 的网页内容在独立 HWND 内，**不受控件 `opacity` 影响**。

### C++ API（常用）

| 方法 | 说明 |
|------|------|
| `SetEngine` / `GetEngineName` | 选择 / 查询引擎 |
| `SetHostMode` / `GetHostMode` | WebView2 宿主模式 |
| `SetHomePage` / `GetHomePage` | 主页 |
| `SetLocationUrl` / `GetLocationUrl` | 当前地址缓存：`NavigateUrl` 写入；导航完成建议再用 `SetLocationUrl`；空则回落 `HomePage` |
| `Navigate2` / `NavigateUrl` / `NavigateHomePage` | 导航（会更新 `LocationUrl`） |
| `SetPos` | 布局；原生宿主 HWND 会避开窗口 `size-box`（LTRB）以及自身/祖先的 `window-resize` 热区，以免盖住缩放。Demo `browser.html`：窗口 `size-box: 0` + `TabLayout`/`root` 的 `window-resize`（仅热区数像素内缩，非整圈大留白） |
| `GoBack` / `GoForward` / `Refresh` / `Refresh2` | 历史与刷新（`Refresh2` 仅 IE） |
| `CanGoBack` / `CanGoForward` | 历史栈是否可退/可进 |
| `Stop` | 停止加载 |
| `OpenDevToolsWindow` | 打开开发者工具（WebView2；其它引擎可空） |
| `QueryUrl` | 向引擎查当前 URL，失败回落 `GetLocationUrl` |
| `ExecuteScript` | 执行脚本；结果经 `OnExecuteScriptResult` |
| `DoMessageLoopWork` | 转发给引擎（CEF 等在空闲时泵消息） |
| `GetNative` | 引擎原生指针（IE=`IWebBrowser2*`，WV2=`ICoreWebView2*`） |
| `GetWebBrowser2` / `GetHtmlWindow` | **仅 IE** |
| `SetHostEvents` | 引擎无关事件（`CWebBrowserHostEvents`） |
| `SetWebBrowserEventHandler` | **仅 IE** 旧回调（`CWebBrowserEventHandler`） |

### 宿主事件（`CWebBrowserHostEvents`）

| 回调 | 说明 |
|------|------|
| `OnNavigationStarting` | 可取消 |
| `OnNavigationCompleted` | 成功/失败；`url` 一般为当前 Source |
| `OnLoadError` | 加载失败（WebView2 `WebErrorStatus` / IE `NavigateError`） |
| `OnDocumentTitleChanged` | 标题 |
| `OnNewWindowRequested` | 新窗口 / `target=_blank`；设 `*handled=true` 可拦截。Browser 壳 Demo：改为 `AddNewTab(url)` |
| `OnFaviconChanged` | 站点图标（WebView2：PNG 字节；无图标时 `pData=NULL`） |
| `OnHistoryChanged` | 历史栈变化（用于同步后退/前进按钮 `CanGoBack`/`CanGoForward`） |
| `OnDownloadStarting` | 下载开始；可 `*pCancel=true`（WebView2） |
| `OnExecuteScriptResult` | `ExecuteScript` 异步结果 |

多标签地址栏：在 `OnNavigationCompleted` 里 `SetLocationUrl`，切换 Tab 时用活动页的 `GetLocationUrl()` 同步 `Edit`。
标题：在 `OnDocumentTitleChanged` 里对对应 `TabButton` 调用 **`SetTabTitle`**（或 `SetText`，已转发到标题 Label），不要只改基类 `m_sText`。
图标：在 `OnFaviconChanged` 里对对应 `TabButton` 调用 **`SetTabIcon(pData, dwSize)`**（WebView2 已接好；IE/CEF 暂无）。Browser 壳 Demo 已示例同步。
后退/前进：`CanGoBack`/`CanGoForward` + `OnHistoryChanged`；Demo 里禁用不可用的 Svg 按钮。

### 与窗口消息映射

`WindowImplBase` 多继承 `CNotifyPump`。窗口类若再多继承其它接口（如直接继承 `CWebBrowserHostEvents`），消息映射成员指针的 `this` 可能错位，导致 `OnClick` 等失效。

推荐：

- 窗口只继承 `WindowImplBase`
- `CWebBrowserHostEvents` 用**嵌套类 / 成员组合**，`SetHostEvents(&m_hostEvents)`
- 导航命令优先在 `Notify()` 里按控件名处理（不依赖 `DUI_ON_MSGTYPE` 的 this 调整）

### 构建

CMake 选项 `DUILIB_USE_WEBVIEW2`（默认 ON）。SDK 路径：

- 默认：`src/3rd/webview2/pkg/build/native`（仅保留 headers + `WebView2LoaderStatic.lib`）
- 或环境变量 `WEBVIEW2_SDK_PATH`

运行时需安装 [WebView2 Runtime](https://developer.microsoft.com/microsoft-edge/webview2/)（Evergreen）。Loader 使用 **静态库** `WebView2LoaderStatic.lib`，无需随包分发 `WebView2Loader.dll`。

### 扩展 CEF / 其它引擎

内置 `engine="cef"` 仅为 **stub**（`Create` 失败）。正式 CEF 由应用实现 `IWebBrowserEngine` 并注册（**库不链 CEF**）：

```cpp
// 1) 进程级：CefInitialize / 多进程入口由应用负责（不在 DuiLib 内）
// 2) 注册（可盖掉 stub；EnsureBuiltinEngines 使用 RegisterIfAbsent，不会盖掉你已注册的 cef）
CWebBrowserEngineFactory::Instance().EnsureBuiltinEngines();
CWebBrowserEngineFactory::Instance().Register(_T("cef"), MyCreateCefEngine);
// 或启动极早时先 Register("cef", ...)，再任意 Create —— builtin 不会覆盖

// 3) 皮肤 / 代码
// 窗口化：
// <WebBrowser engine="cef" engine-fallback="false" host="window" ... />
// OSR：
// <WebBrowser engine="cef" engine-fallback="false" host="osr" ... />
```

外接引擎建议实现（接口均有默认空实现，可按需覆盖）：

| 方法 / 事件 | 用途 |
|-------------|------|
| `Stop` / `GetUrl` / `ExecuteScript` / `OpenDevToolsWindow` | 停止、读地址、脚本、开发者工具（WebView2） |
| `DoMessageLoopWork` | UI 空闲调用 `CefDoMessageLoopWork`（或等价泵） |
| `OnLoadError` / `OnDownloadStarting` / `OnFaviconChanged` | 与壳 Demo 对齐 |
| `GetNative` | 返回 `CefBrowser*` 等逃生舱 |
| `IsOffScreen` / `PaintOffScreen` / `HandleEvent` | **OSR**：见下节 |
| `BlitWebBrowserOsrBuffer` | 自由函数：BGRA → `IRenderContext`（CEF `OnPaint` 缓冲可直接贴） |

窗口化子 HWND：`Create(pOwner, hParent, rc)` + `SetPos`/`SetVisible` 即可。

`Register` = 覆盖；`RegisterIfAbsent` = 仅空位写入（内置引擎用）。

### CEF OSR（离屏）接入要点

库只提供桥，不实现 CEF：

1. `IsOffScreen()` 返回 `true`；`GetHostWindow()` 可 `NULL`
2. CEF `OnPaint` 拷贝 BGRA 后：`pOwner->Invalidate()`（`pOwner` 即 `Create` 传入的控件）
3. `PaintOffScreen` 里调用 `BlitWebBrowserOsrBuffer(..., stride, true, m_pOwner->ScaleImageFade())`（跟随控件 `opacity`）
4. `HandleEvent` 把 `TEventUI`（鼠标/滚轮/键盘/焦点）转成 `CefBrowserHost::Send*Event`；相对坐标 = `ptMouse - rcItem.left/top`
5. 皮肤 `host="osr"`：门面打开鼠标/键盘与 Tab 焦点；`SetPos` **不**再为 size-box 收缩矩形
6. 消息循环空闲：`pWeb->DoMessageLoopWork()` → 你的 `CefDoMessageLoopWork`

```cpp
class MyCefOsrEngine : public IWebBrowserEngine {
public:
	bool IsOffScreen() const override { return true; }
	HWND GetHostWindow() const override { return NULL; }
	bool PaintOffScreen(IRenderContext& ctx, const RECT& rcPaint) override {
		if( !m_pBuf || !m_pOwner ) return false;
		return BlitWebBrowserOsrBuffer(ctx, m_rc, rcPaint, m_pBuf, m_w, m_h, m_stride,
			true, m_pOwner->ScaleImageFade());
	}
	bool HandleEvent(TEventUI& e) override {
		// SendMouseClickEvent / SendMouseMoveEvent / SendKeyEvent …
		return true;
	}
	// CefRenderHandler::OnPaint → 写 m_pBuf 后 m_pOwner->Invalidate();
};
```

高频场景可自建缓存纹理，不必每帧走 `BlitWebBrowserOsrBuffer`（该辅助每次建临时 DIB，简单但非最优）。

### Demo

`duidemo` →「浏览器壳」`CBrowserWnd`（`browser.html`）：

- TabBar + TabLayout 多标签；工具栏后退/前进/刷新/主页/转到；地址栏回车导航
- 默认 `host=window`；导航与点击在 `Notify` / `urlBox->OnNotify` 处理
- 持有 `m_pActiveBrowser`；`tabselect` 时用 `GetLocationUrl()` 同步 `urlBox`
- 宿主事件用内部 `HostEvents` 组合，避免多继承破坏消息映射
