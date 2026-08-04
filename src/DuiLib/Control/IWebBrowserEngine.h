#ifndef __IWEBBROWSERENGINE_H__
#define __IWEBBROWSERENGINE_H__

#pragma once

#include <map>

namespace DuiLib
{
	class CControlUI;
	class CWebBrowserUI;

	/// 引擎无关的宿主事件（WebView2 / 未来 CEF 等）
	class UILIB_API CWebBrowserHostEvents
	{
	public:
		virtual ~CWebBrowserHostEvents() {}
		virtual void OnNavigationStarting(CWebBrowserUI* pWeb, LPCTSTR url, bool* pCancel) {}
		virtual void OnNavigationCompleted(CWebBrowserUI* pWeb, LPCTSTR url, bool success) {}
		virtual void OnDocumentTitleChanged(CWebBrowserUI* pWeb, LPCTSTR title) {}
		virtual void OnNewWindowRequested(CWebBrowserUI* pWeb, LPCTSTR url, bool* pHandled) {}
	};

	/// 可插拔浏览器引擎
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
		virtual HWND GetHostWindow() const = 0;
		/// IE: IWebBrowser2*；WebView2: ICoreWebView2*
		virtual void* GetNative() = 0;
		virtual void SetHostEvents(CWebBrowserHostEvents* pEvents) = 0;
		virtual void SetUserDataFolder(LPCTSTR /*path*/) {}
		/// WebView2: "window" | "composition"；其它引擎可忽略
		virtual void SetHostMode(LPCTSTR /*mode*/) {}
		virtual LPCTSTR GetHostMode() const { return _T("window"); }
	};

	typedef IWebBrowserEngine* (*WebBrowserEngineCreator)();

	class UILIB_API CWebBrowserEngineFactory
	{
	public:
		static CWebBrowserEngineFactory& Instance();
		void Register(LPCTSTR name, WebBrowserEngineCreator fn);
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
