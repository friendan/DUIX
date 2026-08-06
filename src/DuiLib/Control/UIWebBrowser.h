#ifndef __UIWEBBROWSER_H__
#define __UIWEBBROWSER_H__

#pragma once

#include "IWebBrowserEngine.h"
#include "Utils/WebBrowserEventHandler.h"

struct IWebBrowser2;
struct IDispatch;

namespace DuiLib
{
	class UILIB_API CWebBrowserUI : public CControlUI
	{
		DECLARE_DUICONTROL(CWebBrowserUI)
	public:
		CWebBrowserUI();
		virtual ~CWebBrowserUI();

		virtual LPCTSTR GetClass() const;
		virtual LPVOID GetInterface(LPCTSTR pstrName);
		virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		virtual void SetPos(RECT rc, bool bNeedInvalidate = true);
		virtual void SetVisible(bool bVisible = true);
		virtual void SetInternVisible(bool bVisible = true);
		virtual void Init();
		virtual void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);

		void SetEngine(LPCTSTR name);
		LPCTSTR GetEngineName() const;
		void SetEngineFallback(bool bFallback);
		bool IsEngineFallback() const;

		void SetHomePage(LPCTSTR lpszUrl);
		LPCTSTR GetHomePage() const;
		/// 当前页地址（导航过程中缓存；未导航过时回落 HomePage）
		void SetLocationUrl(LPCTSTR lpszUrl);
		LPCTSTR GetLocationUrl() const;
		void SetAutoNavigation(bool bAuto = true);
		bool IsAutoNavigation() const;
		void SetUserDataFolder(LPCTSTR path);

		/// WebView2 宿主：window（默认子 HWND）/ composition（DComp Visual）
		void SetHostMode(LPCTSTR mode);
		LPCTSTR GetHostMode() const;

		void SetHostEvents(CWebBrowserHostEvents* pEvents);
		void SetWebBrowserEventHandler(CWebBrowserEventHandler* pEventHandler);

		void Navigate2(LPCTSTR lpszUrl);
		void NavigateUrl(LPCTSTR lpszUrl);
		void NavigateHomePage();
		void Refresh();
		void Refresh2(int Level);
		void GoBack();
		void GoForward();
		bool CanGoBack() const;
		bool CanGoForward() const;
		void Stop();
		/// 向引擎查询当前 URL；失败则回落 GetLocationUrl()
		bool QueryUrl(CDuiString& out) const;
		void ExecuteScript(LPCTSTR script);
		/// 转发给当前引擎（CEF 等可在应用消息循环空闲时调用）
		void DoMessageLoopWork();

		void* GetNative();
		IWebBrowser2* GetWebBrowser2();
		IDispatch* GetHtmlWindow();

		static DISPID FindId(IDispatch *pObj, LPOLESTR pName);
		static HRESULT InvokeMethod(IDispatch *pObj, LPOLESTR pMehtod, VARIANT *pVarResult, VARIANT *ps, int cArgs);
		static HRESULT GetProperty(IDispatch *pObj, LPOLESTR pName, VARIANT *pValue);
		static HRESULT SetProperty(IDispatch *pObj, LPOLESTR pName, VARIANT *pValue);

	protected:
		void EnsureEngine();
		void DestroyEngine();
		CDuiString ResolveDefaultEngine() const;

		IWebBrowserEngine* m_pEngine;
		CDuiString m_sEngineName;
		CDuiString m_sWantedEngine;
		CDuiString m_sHomePage;
		CDuiString m_sLocationUrl;
		CDuiString m_sUserDataFolder;
		CDuiString m_sHostMode;
		bool m_bAutoNavi;
		bool m_bFallback;
		bool m_bForceEngine;
		CWebBrowserHostEvents* m_pHostEvents;
		CWebBrowserEventHandler* m_pWebBrowserEventHandler;
		CDuiString m_sPendingUrl;
	};

	/// XML `<WebView2>` 别名：强制 engine=webview2
	class UILIB_API CWebView2UI : public CWebBrowserUI
	{
		DECLARE_DUICONTROL(CWebView2UI)
	public:
		CWebView2UI();
		virtual LPCTSTR GetClass() const;
		virtual LPVOID GetInterface(LPCTSTR pstrName);
	};
}

#endif // __UIWEBBROWSER_H__
