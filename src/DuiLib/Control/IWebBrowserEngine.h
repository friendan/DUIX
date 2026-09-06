#ifndef __IWEBBROWSERENGINE_H__
#define __IWEBBROWSERENGINE_H__

#pragma once

#include <map>
#include <string>

namespace DuiLib
{
	class CControlUI;
	class CWebBrowserUI;
	class IRenderContext;
	struct tagTEventUI;
	typedef struct tagTEventUI TEventUI;

	/// 引擎无关的宿主事件（WebView2 / CEF 等外接引擎）
	class UILIB_API CWebBrowserHostEvents
	{
	public:
		virtual ~CWebBrowserHostEvents() {}
		virtual void OnNavigationStarting(CWebBrowserUI* pWeb, LPCTSTR url, bool* pCancel) {}
		virtual void OnNavigationCompleted(CWebBrowserUI* pWeb, LPCTSTR url, bool success) {}
		/// 加载失败（HTTP/网络/证书等）；errorCode 引擎相关，可为 0
		virtual void OnLoadError(CWebBrowserUI* pWeb, LPCTSTR url, int errorCode, LPCTSTR errorText) {}
		virtual void OnDocumentTitleChanged(CWebBrowserUI* pWeb, LPCTSTR title) {}
		virtual void OnNewWindowRequested(CWebBrowserUI* pWeb, LPCTSTR url, bool* pHandled) {}
		/// 网页图标（WebView2：PNG；无图标时 pData=NULL / dwSize=0）
		virtual void OnFaviconChanged(CWebBrowserUI* pWeb, const BYTE* pData, DWORD dwSize) {}
		/// 历史栈变化（后退/前进按钮）
		virtual void OnHistoryChanged(CWebBrowserUI* pWeb) {}
		/// 下载即将开始；*pCancel=true 可取消。suggestedPath 可为建议文件名或完整路径
		virtual void OnDownloadStarting(CWebBrowserUI* pWeb, LPCTSTR url, LPCTSTR suggestedPath, bool* pCancel) {}
		/// ExecuteScript 异步结果（通常为 JSON 文本；失败时可为 NULL）
		virtual void OnExecuteScriptResult(CWebBrowserUI* pWeb, LPCTSTR resultJson, bool success) {}
		/// 页面脚本向宿主回传（WebView2：add_WebMessageReceived；CEF 由应用侧自行桥接）。
		/// json 为页面 postMessage 传来的字符串（UTF-16，如观澜隐藏元素拾取结果）。
		/// 注意：事件可能不在 UI 线程触发，接收方须自行跨线程投递后再碰 UI。
		virtual void OnScriptMessage(CWebBrowserUI* pWeb, LPCTSTR json) {}
	};

	/// OSR 辅助：将 BGRA 缓冲贴到控件矩形（CEF OnPaint 缓冲可直接用；库不链 CEF）
	/// stride<=0 时按 width*4；topDown=true 表示首行在上（CEF PET_VIEW 默认）
	/// uFade：建议传 pOwner->ScaleImageFade()，以跟随控件/祖先 opacity
	UILIB_API bool BlitWebBrowserOsrBuffer(IRenderContext& ctx, const RECT& rcDest, const RECT& rcPaint,
		const BYTE* pBgra, int width, int height, int stride = 0, bool topDown = true, UINT uFade = 255);

	/// 可插拔浏览器引擎（内置 webview2/ie；cef 建议应用 Register 覆盖 stub）
	class UILIB_API IWebBrowserEngine
	{
	public:
		virtual ~IWebBrowserEngine() {}

		virtual LPCTSTR GetName() const = 0;
		virtual bool Create(CControlUI* pOwner, HWND hParent, const RECT& rc) = 0;
		virtual void Destroy() = 0;
		virtual void SetPos(const RECT& rc) = 0;
		virtual void SetVisible(bool bVisible) = 0;
		virtual void Navigate(LPCTSTR url) = 0;
		virtual void GoBack() = 0;
		virtual void GoForward() = 0;
		virtual void Refresh() = 0;
		virtual void Stop() {}
		virtual bool CanGoBack() const { return false; }
		virtual bool CanGoForward() const { return false; }
		/// 当前地址；成功写入 out 并返回 true（门面可 Sync 到 LocationUrl）
		virtual bool GetUrl(CDuiString& /*out*/) const { return false; }
		/// 执行脚本；结果经 OnExecuteScriptResult（若引擎支持）
		virtual void ExecuteScript(LPCTSTR /*script*/) {}
		/// WebView2：打开开发者工具；其它引擎默认可空
		virtual void OpenDevToolsWindow() {}
		/// CEF 等需在 UI 消息循环空闲时泵送；WebView2/IE 默认可空实现
		virtual void DoMessageLoopWork() {}
		virtual HWND GetHostWindow() const = 0;
		/// IE: IWebBrowser2*；WebView2: ICoreWebView2*；CEF: 由外接实现定义
		virtual void* GetNative() = 0;
		virtual void SetHostEvents(CWebBrowserHostEvents* pEvents) = 0;
		virtual void SetUserDataFolder(LPCTSTR /*path*/) {}
		/// WebView2: "window" | "composition"；外接 CEF OSR 可用 "osr" / "offscreen"
		virtual void SetHostMode(LPCTSTR /*mode*/) {}
		virtual LPCTSTR GetHostMode() const { return _T("window"); }

		/// 离屏：无子 HWND，由 PaintOffScreen 画进 DuiLib；帧就绪后对 pOwner->Invalidate()
		virtual bool IsOffScreen() const { return false; }
		/// 返回 true 表示已绘制网页内容（可用 BlitWebBrowserOsrBuffer）
		virtual bool PaintOffScreen(IRenderContext& /*ctx*/, const RECT& /*rcPaint*/) { return false; }
		/// 处理鼠标/键盘等；返回 true 表示已消费（不再交给基类默认逻辑）
		virtual bool HandleEvent(TEventUI& /*event*/) { return false; }
	};

	typedef IWebBrowserEngine* (*WebBrowserEngineCreator)();

	class UILIB_API CWebBrowserEngineFactory
	{
	public:
		static CWebBrowserEngineFactory& Instance();
		/// 注册或覆盖（外接 CEF：EnsureBuiltinEngines 之后再 Register("cef", ...)）
		void Register(LPCTSTR name, WebBrowserEngineCreator fn);
		/// 仅当该名尚未注册时写入（内置引擎用，避免盖掉应用已注册的实现）
		bool RegisterIfAbsent(LPCTSTR name, WebBrowserEngineCreator fn);
		IWebBrowserEngine* Create(LPCTSTR name) const;
		bool IsRegistered(LPCTSTR name) const;
		void EnsureBuiltinEngines();

	private:
		CWebBrowserEngineFactory();
		std::map<CDuiString, WebBrowserEngineCreator> m_map;
		bool m_bBuiltins;
	};

	/// WebView2 子资源过滤回调（广告拦截注入，观澜与 CEF 引擎共用同一规则库）。
	/// topUrl：当前页面 URL；resourceUrl：被请求资源 URL（均 UTF-16）。
	/// 返回 true = 拦截该请求（引擎侧回 403 空响应）。主文档导航请求不会回调（引擎已放行）。
	typedef bool (*WebView2ResourceFilterFn)(LPCWSTR topUrl, LPCWSTR resourceUrl);

	/// 设置 / 清除（传 NULL）WebView2 引擎资源过滤回调。全进程一份，UI 线程调用；
	/// CWebView2Engine 在 AttachHandlers 时读取，之后每次子资源请求都会查询。
	UILIB_API void WebView2SetResourceFilter(WebView2ResourceFilterFn fn);
	UILIB_API WebView2ResourceFilterFn WebView2GetResourceFilter();

	/// WebView2 document-start 注入脚本提供者（页面早期广告规则 cosmetic 注入，观澜实现）。
	/// topUrl：即将开始的顶层导航 URL（UTF-16，含 scheme）。返回需在 document 创建后、
	/// 页面任何脚本之前执行的 JS（UTF-16）；空串 = 本次不注入。
	/// 引擎在 NavigationStarting（UI 线程、新 document 创建前）同步调用；同一 WebView
	/// 同一时刻只保留最新一份注入脚本（下次导航前自动移除旧的），规则增删即时生效。
	typedef std::wstring (*WebView2DocStartScriptFn)(LPCWSTR topUrl);

	/// 设置 / 清除（传 NULL）document-start 注入脚本提供者。全进程一份，UI 线程调用；
	/// CWebView2Engine 在每次顶层导航开始时查询。
	UILIB_API void WebView2SetDocStartScriptProvider(WebView2DocStartScriptFn fn);
	UILIB_API WebView2DocStartScriptFn WebView2GetDocStartScriptProvider();
}

#endif // __IWEBBROWSERENGINE_H__
