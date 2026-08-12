#ifndef __UIWEBBROWSER_H__
#define __UIWEBBROWSER_H__

#pragma once

#include "IWebBrowserEngine.h"
#include "Utils/WebBrowserEventHandler.h"

struct IWebBrowser2;
struct IDispatch;

namespace DuiLib
{
	class UILIB_API CWebBrowserUI : public CControlUI, public IMessageFilterUI
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
		virtual UINT GetControlFlags() const;
		virtual void DoEvent(TEventUI& event);
		virtual bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl);
		virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

		/// host=osr / 引擎 IsOffScreen()
		bool IsOffScreenHost() const;

		/// 盖在引擎 HWND 上的挖空 popup：中间穿透给页面/滚动条，边缘缩窗
		void ScheduleNativeResizeHook(bool bResetRetry = false);
		/// 临时开关挖空缩窗层（默认 true）。弹自定义顶层窗前设 false，关闭后再 true，避免边条挡点击
		void SetNativeWindowResizeEnabled(bool bEnable);
		bool IsNativeWindowResizeEnabled() const;
		/// 屏幕坐标；落在 window-resize 热区返回 HTLEFT/…；未启用或已 Suspend 则 HTCLIENT
		LRESULT HitNativeHostResize(POINT ptScreen) const;

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

		/// WebView2：window / composition；外接 CEF 离屏：osr / offscreen
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
		void OpenDevToolsWindow();
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
		void SyncHostInteraction();
		CDuiString ResolveDefaultEngine() const;
		void UpdateResizeOverlay(bool bForceRecreate = false);
		void DestroyResizeOverlay();
		RECT GetNativeResizeGripInset() const;

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
		HWND m_hResizeOverlay;
		bool m_bPaintWasIconic;
		bool m_bNativeWindowResizeEnabled;
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
