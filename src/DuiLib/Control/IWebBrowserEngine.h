#ifndef __IWEBBROWSERENGINE_H__
#define __IWEBBROWSERENGINE_H__

#pragma once

#include <map>

namespace DuiLib
{
	class CControlUI;
	class CWebBrowserUI;

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
	};

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
		/// CEF 等需在 UI 消息循环空闲时泵送；WebView2/IE 默认可空实现
		virtual void DoMessageLoopWork() {}
		virtual HWND GetHostWindow() const = 0;
		/// IE: IWebBrowser2*；WebView2: ICoreWebView2*；CEF: 由外接实现定义
		virtual void* GetNative() = 0;
		virtual void SetHostEvents(CWebBrowserHostEvents* pEvents) = 0;
		virtual void SetUserDataFolder(LPCTSTR /*path*/) {}
		/// WebView2: "window" | "composition"；其它引擎可自定义或忽略
		virtual void SetHostMode(LPCTSTR /*mode*/) {}
		virtual LPCTSTR GetHostMode() const { return _T("window"); }
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
}

#endif // __IWEBBROWSERENGINE_H__
