#ifndef __WEBVIEW2ENGINE_H__
#define __WEBVIEW2ENGINE_H__

#pragma once

#include "IWebBrowserEngine.h"

#ifdef DUILIB_HAS_WEBVIEW2

#include <WebView2.h>

struct IDCompositionDevice;
struct IDCompositionTarget;
struct IDCompositionVisual;

namespace DuiLib
{
	class CWebBrowserUI;

	class CWebView2Engine : public IWebBrowserEngine
	{
	public:
		CWebView2Engine();
		virtual ~CWebView2Engine();

		virtual LPCTSTR GetName() const { return _T("webview2"); }
		virtual bool Create(CControlUI* pOwner, HWND hParent, const RECT& rc);
		virtual void Destroy();
		virtual void SetPos(const RECT& rc);
		virtual void SetVisible(bool bVisible);
		virtual void Navigate(LPCTSTR url);
		virtual void GoBack();
		virtual void GoForward();
		virtual void Refresh();
		virtual void Stop();
		virtual bool CanGoBack() const;
		virtual bool CanGoForward() const;
		virtual bool GetUrl(CDuiString& out) const;
		virtual void ExecuteScript(LPCTSTR script);
		virtual void OpenDevToolsWindow();
		virtual HWND GetHostWindow() const;
		virtual void* GetNative();
		virtual void SetHostEvents(CWebBrowserHostEvents* pEvents);
		virtual void SetUserDataFolder(LPCTSTR path);
		virtual void SetHostMode(LPCTSTR mode);
		virtual LPCTSTR GetHostMode() const;

		LRESULT HandleCompHostMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:
		bool WantComposition() const;
		bool EnsureCompHostWindow();
		bool EnsureCompVisual();
		void DestroyCompHost();
		void ApplyBounds();
		void FlushPendingNavigate();
		void AttachHandlers();
		void OnControllerReady(bool composition);
		void RequestFavicon();
		HRESULT StartWindowController(ICoreWebView2Environment* env);
		HRESULT StartCompositionController(ICoreWebView2Environment* env);
		void ForwardMouse(UINT uMsg, WPARAM wParam, LPARAM lParam);
		static void RegisterCompHostClass();
		static LRESULT CALLBACK CompHostWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		CControlUI* m_pOwner;
		CWebBrowserUI* m_pFacade;
		CWebBrowserHostEvents* m_pHostEvents;
		HWND m_hParent;
		HWND m_hCompHost;
		RECT m_rc;
		bool m_bVisible;
		bool m_bReady;
		bool m_bCompositionActive;
		bool m_bOwnDComp;
		bool m_bTrackingMouse;
		CDuiString m_sUserDataFolder;
		CDuiString m_sHostMode;
		CDuiString m_sPendingUrl;
		CDuiString m_sEffectiveHost;
		mutable CDuiString m_sCachedUrl;

		ICoreWebView2Controller* m_pController;
		ICoreWebView2CompositionController* m_pCompController;
		ICoreWebView2* m_pWebView;
		ICoreWebView2Environment* m_pEnv;   // 缓存环境（拦截时构造 403 响应用；Create 完成时 AddRef，Destroy Release）
		IDCompositionDevice* m_pDComp;
		IDCompositionTarget* m_pDCompTarget;
		IDCompositionVisual* m_pDCompVisual;

		EventRegistrationToken m_tokNavStarting;
		EventRegistrationToken m_tokNavCompleted;
		EventRegistrationToken m_tokTitleChanged;
		EventRegistrationToken m_tokNewWindow;
		EventRegistrationToken m_tokCursorChanged;
		EventRegistrationToken m_tokFaviconChanged;
		EventRegistrationToken m_tokHistoryChanged;
		EventRegistrationToken m_tokDownloadStarting;
		EventRegistrationToken m_tokWebResource;
	};
}

#endif // DUILIB_HAS_WEBVIEW2

#endif
