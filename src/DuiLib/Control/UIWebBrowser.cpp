#include "StdAfx.h"
#include "UIWebBrowser.h"
#include "WebBrowserIeEngine.h"
#include <ExDisp.h>
#include <atlconv.h>
#include <atlcomcli.h>
#include <MsHTML.h>

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CWebBrowserUI)
	IMPLEMENT_DUICONTROL(CWebView2UI)

	bool BlitWebBrowserOsrBuffer(IRenderContext& ctx, const RECT& rcDest, const RECT& rcPaint,
		const BYTE* pBgra, int width, int height, int stride, bool topDown, UINT uFade)
	{
		if( pBgra == NULL || width <= 0 || height <= 0 ) return false;
		if( ::IsRectEmpty(&rcDest) ) return false;
		if( stride <= 0 ) stride = width * 4;

		BITMAPINFO bmi = { 0 };
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = width;
		bmi.bmiHeader.biHeight = topDown ? -height : height; // 负高度 = 自上而下，匹配 CEF PET_VIEW
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;

		void* pBits = NULL;
		HBITMAP hBmp = ::CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
		if( hBmp == NULL || pBits == NULL ) {
			if( hBmp ) ::DeleteObject(hBmp);
			return false;
		}

		const int dstStride = width * 4;
		BYTE* pDst = static_cast<BYTE*>(pBits);
		for( int y = 0; y < height; ++y ) {
			const BYTE* pSrc = pBgra + y * stride;
			::CopyMemory(pDst + y * dstStride, pSrc, (size_t)dstStride);
		}

		TImageInfo img;
		img.hBitmap = hBmp;
		img.pBits = static_cast<LPBYTE>(pBits);
		img.nX = width;
		img.nY = height;
		img.bAlpha = true;

		RECT rcBmp = { 0, 0, width, height };
		RECT rcCorners = { 0, 0, 0, 0 };
		ctx.DrawImage(&img, rcDest, rcPaint, rcBmp, rcCorners, uFade, false, false, false);

		// D2D 按 HBITMAP 键控 GPU 缓存：删 GDI 位图前必须失效，避免句柄复用脏缓存
		IRenderDevice* pDev = GetRenderDevice();
		if( pDev != NULL ) pDev->InvalidateImageGpu(&img);
		::DeleteObject(hBmp);
		return true;
	}

	CWebBrowserUI::CWebBrowserUI()
		: m_pEngine(NULL)
		, m_sHostMode(_T("window"))
		, m_bAutoNavi(false)
		, m_bFallback(true)
		, m_bForceEngine(false)
		, m_pHostEvents(NULL)
		, m_pWebBrowserEventHandler(NULL)
	{
		SetMouseEnabled(false);
	}

	CWebBrowserUI::~CWebBrowserUI()
	{
		DestroyEngine();
	}

	LPCTSTR CWebBrowserUI::GetClass() const
	{
		return _T("WebBrowserUI");
	}

	LPVOID CWebBrowserUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_WEBBROWSER) == 0 ) return static_cast<CWebBrowserUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	CDuiString CWebBrowserUI::ResolveDefaultEngine() const
	{
#ifdef DUILIB_HAS_WEBVIEW2
		if( CWebBrowserEngineFactory::Instance().IsRegistered(_T("webview2")) )
			return _T("webview2");
#endif
		return _T("ie");
	}

	void CWebBrowserUI::SetEngine(LPCTSTR name)
	{
		m_sWantedEngine = name ? name : _T("");
		m_bForceEngine = !m_sWantedEngine.IsEmpty();
		DestroyEngine();
		if( m_pManager ) EnsureEngine();
	}

	LPCTSTR CWebBrowserUI::GetEngineName() const
	{
		return m_sEngineName.GetData();
	}

	void CWebBrowserUI::SetEngineFallback(bool bFallback)
	{
		m_bFallback = bFallback;
	}

	bool CWebBrowserUI::IsEngineFallback() const
	{
		return m_bFallback;
	}

	void CWebBrowserUI::SetHomePage(LPCTSTR lpszUrl)
	{
		m_sHomePage = lpszUrl ? lpszUrl : _T("");
		if( m_sLocationUrl.IsEmpty() && !m_sHomePage.IsEmpty() )
			m_sLocationUrl = m_sHomePage;
	}

	LPCTSTR CWebBrowserUI::GetHomePage() const
	{
		return m_sHomePage.GetData();
	}

	void CWebBrowserUI::SetLocationUrl(LPCTSTR lpszUrl)
	{
		m_sLocationUrl = lpszUrl ? lpszUrl : _T("");
	}

	LPCTSTR CWebBrowserUI::GetLocationUrl() const
	{
		if( !m_sLocationUrl.IsEmpty() ) return m_sLocationUrl.GetData();
		return m_sHomePage.GetData();
	}

	void CWebBrowserUI::SetAutoNavigation(bool bAuto)
	{
		m_bAutoNavi = bAuto;
	}

	bool CWebBrowserUI::IsAutoNavigation() const
	{
		return m_bAutoNavi;
	}

	void CWebBrowserUI::SetUserDataFolder(LPCTSTR path)
	{
		m_sUserDataFolder = path ? path : _T("");
		if( m_pEngine ) m_pEngine->SetUserDataFolder(m_sUserDataFolder.GetData());
	}

	void CWebBrowserUI::SetHostMode(LPCTSTR mode)
	{
		CDuiString s = _T("window");
		if( mode && *mode ) {
			if( _tcsicmp(mode, _T("composition")) == 0 || _tcsicmp(mode, _T("compose")) == 0
				|| _tcsicmp(mode, _T("visual")) == 0 )
				s = _T("composition");
			else if( _tcsicmp(mode, _T("osr")) == 0 || _tcsicmp(mode, _T("offscreen")) == 0 )
				s = _T("osr");
			else if( _tcsicmp(mode, _T("window")) == 0 || _tcsicmp(mode, _T("hwnd")) == 0 )
				s = _T("window");
		}
		if( m_sHostMode == s ) return;
		m_sHostMode = s;
		DestroyEngine();
		SyncHostInteraction();
		if( m_pManager ) EnsureEngine();
	}

	LPCTSTR CWebBrowserUI::GetHostMode() const
	{
		return m_sHostMode.IsEmpty() ? _T("window") : m_sHostMode.GetData();
	}

	bool CWebBrowserUI::IsOffScreenHost() const
	{
		if( m_pEngine != NULL && m_pEngine->IsOffScreen() ) return true;
		if( _tcsicmp(GetHostMode(), _T("osr")) == 0 ) return true;
		if( _tcsicmp(GetHostMode(), _T("offscreen")) == 0 ) return true;
		return false;
	}

	void CWebBrowserUI::SyncHostInteraction()
	{
		const bool osr = IsOffScreenHost();
		SetMouseEnabled(osr);
		SetKeyboardEnabled(osr);
	}

	UINT CWebBrowserUI::GetControlFlags() const
	{
		if( !IsEnabled() ) return CControlUI::GetControlFlags();
		if( IsOffScreenHost() )
			return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
		return CControlUI::GetControlFlags();
	}

	void CWebBrowserUI::DoEvent(TEventUI& event)
	{
		if( m_pEngine != NULL && m_pEngine->IsOffScreen() ) {
			if( event.Type == UIEVENT_SETFOCUS || event.Type == UIEVENT_KILLFOCUS ) {
				CControlUI::DoEvent(event);
				m_pEngine->HandleEvent(event);
				return;
			}
			if( (event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK
				|| event.Type == UIEVENT_RBUTTONDOWN) && IsEnabled() && m_pManager != NULL ) {
				m_pManager->SetFocus(this);
			}
			if( m_pEngine->HandleEvent(event) )
				return;
		}
		CControlUI::DoEvent(event);
	}

	bool CWebBrowserUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		if( !CControlUI::DoPaint(ctx, rcPaint, pStopControl) )
			return false;
		if( m_pEngine != NULL && m_pEngine->IsOffScreen() )
			m_pEngine->PaintOffScreen(ctx, rcPaint);
		return true;
	}

	void CWebBrowserUI::SetHostEvents(CWebBrowserHostEvents* pEvents)
	{
		m_pHostEvents = pEvents;
		if( m_pEngine ) m_pEngine->SetHostEvents(pEvents);
	}

	void CWebBrowserUI::SetWebBrowserEventHandler(CWebBrowserEventHandler* pEventHandler)
	{
		m_pWebBrowserEventHandler = pEventHandler;
		if( m_pEngine && _tcsicmp(m_sEngineName.GetData(), _T("ie")) == 0 ) {
			CWebBrowserIeEngine* pIe = static_cast<CWebBrowserIeEngine*>(m_pEngine);
			pIe->SetIeEventHandler(pEventHandler);
		}
	}

	void CWebBrowserUI::DestroyEngine()
	{
		if( m_pEngine ) {
			m_pEngine->Destroy();
			delete m_pEngine;
			m_pEngine = NULL;
		}
		m_sEngineName.Empty();
	}

	void CWebBrowserUI::EnsureEngine()
	{
		if( m_pEngine != NULL ) return;
		if( m_pManager == NULL ) return;
		HWND hParent = m_pManager->GetPaintWindow();
		if( hParent == NULL ) return;

		CWebBrowserEngineFactory::Instance().EnsureBuiltinEngines();
		CDuiString want = m_bForceEngine ? m_sWantedEngine : ResolveDefaultEngine();
		if( want.IsEmpty() ) want = ResolveDefaultEngine();

		IWebBrowserEngine* pEng = CWebBrowserEngineFactory::Instance().Create(want.GetData());
		if( pEng == NULL && m_bFallback && _tcsicmp(want.GetData(), _T("ie")) != 0 )
			pEng = CWebBrowserEngineFactory::Instance().Create(_T("ie"));
		if( pEng == NULL ) return;

		if( !m_sUserDataFolder.IsEmpty() )
			pEng->SetUserDataFolder(m_sUserDataFolder.GetData());
		pEng->SetHostMode(GetHostMode());
		pEng->SetHostEvents(m_pHostEvents);

		RECT rc = m_rcItem;
		if( !pEng->Create(this, hParent, rc) ) {
			delete pEng;
			pEng = NULL;
			if( m_bFallback && _tcsicmp(want.GetData(), _T("ie")) != 0 ) {
				pEng = CWebBrowserEngineFactory::Instance().Create(_T("ie"));
				if( pEng ) {
					pEng->SetHostEvents(m_pHostEvents);
					if( !pEng->Create(this, hParent, rc) ) {
						delete pEng;
						pEng = NULL;
					}
				}
			}
		}
		if( pEng == NULL ) return;

		m_pEngine = pEng;
		m_sEngineName = pEng->GetName();
		if( _tcsicmp(m_sEngineName.GetData(), _T("ie")) == 0 && m_pWebBrowserEventHandler ) {
			static_cast<CWebBrowserIeEngine*>(m_pEngine)->SetIeEventHandler(m_pWebBrowserEventHandler);
		}

		SyncHostInteraction();
		m_pEngine->SetVisible(IsVisible());
		m_pEngine->SetPos(m_rcItem);

		if( !m_sPendingUrl.IsEmpty() ) {
			m_pEngine->Navigate(m_sPendingUrl.GetData());
			m_sPendingUrl.Empty();
		}
		else if( m_bAutoNavi && !m_sHomePage.IsEmpty() ) {
			m_pEngine->Navigate(m_sHomePage.GetData());
		}
	}

	void CWebBrowserUI::Init()
	{
		CControlUI::Init();
		EnsureEngine();
	}

	void CWebBrowserUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
	{
		CControlUI::SetManager(pManager, pParent, bInit);
		if( pManager && bInit ) EnsureEngine();
	}

	void CWebBrowserUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		EnsureEngine();
		if( m_pEngine == NULL ) return;

		// 原生宿主 HWND 勿盖住窗口 size-box / 祖先 window-resize 热区；OSR 无子窗，用完整控件矩形
		RECT rcHost = m_rcItem;
		if( !IsOffScreenHost() && m_pManager != NULL ) {
			HWND hWnd = m_pManager->GetPaintWindow();
			if( hWnd != NULL && !::IsZoomed(hWnd) ) {
				RECT rcClient = { 0 };
				::GetClientRect(hWnd, &rcClient);
				RECT sb = m_pManager->GetSizeBox();
				if( rcHost.right > rcClient.right - sb.right )
					rcHost.right = rcClient.right - sb.right;
				if( rcHost.bottom > rcClient.bottom - sb.bottom )
					rcHost.bottom = rcClient.bottom - sb.bottom;
				if( rcHost.left < rcClient.left + sb.left )
					rcHost.left = rcClient.left + sb.left;
				if( rcHost.top < rcClient.top + sb.top )
					rcHost.top = rcClient.top + sb.top;
				if( rcHost.right < rcHost.left ) rcHost.right = rcHost.left;
				if( rcHost.bottom < rcHost.top ) rcHost.bottom = rcHost.top;
				// size-box 为 0 时仍须避开 TabLayout 等 window-resize 边，否则子 HWND 吃掉右/下缩放
				ApplyAncestorWindowResizeHostInset(rcHost);
			}
		}
		m_pEngine->SetPos(rcHost);
	}

	void CWebBrowserUI::SetVisible(bool bVisible)
	{
		CControlUI::SetVisible(bVisible);
		if( m_pEngine ) m_pEngine->SetVisible(IsVisible());
	}

	void CWebBrowserUI::SetInternVisible(bool bVisible)
	{
		CControlUI::SetInternVisible(bVisible);
		if( m_pEngine ) m_pEngine->SetVisible(IsVisible());
	}

	void CWebBrowserUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("home-page")) == 0 || _tcsicmp(pstrName, _T("homepage")) == 0 ) {
			SetHomePage(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("auto-navi")) == 0 || _tcsicmp(pstrName, _T("autonavi")) == 0 ) {
			SetAutoNavigation(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("engine")) == 0 ) {
			SetEngine(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("engine-fallback")) == 0 || _tcsicmp(pstrName, _T("fallback")) == 0 ) {
			SetEngineFallback(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("ie")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("user-data-folder")) == 0 || _tcsicmp(pstrName, _T("userdata")) == 0 ) {
			SetUserDataFolder(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("host")) == 0 || _tcsicmp(pstrName, _T("host-mode")) == 0 ) {
			SetHostMode(pstrValue);
		}
		else {
			CControlUI::SetAttribute(pstrName, pstrValue);
		}
	}

	void CWebBrowserUI::Navigate2(LPCTSTR lpszUrl)
	{
		NavigateUrl(lpszUrl);
	}

	void CWebBrowserUI::NavigateUrl(LPCTSTR lpszUrl)
	{
		if( lpszUrl == NULL || *lpszUrl == _T('\0') ) return;
		m_sLocationUrl = lpszUrl;
		EnsureEngine();
		if( m_pEngine ) m_pEngine->Navigate(lpszUrl);
		else m_sPendingUrl = lpszUrl;
	}

	void CWebBrowserUI::NavigateHomePage()
	{
		if( !m_sHomePage.IsEmpty() )
			NavigateUrl(m_sHomePage.GetData());
	}

	void CWebBrowserUI::Refresh()
	{
		EnsureEngine();
		if( m_pEngine ) m_pEngine->Refresh();
	}

	void CWebBrowserUI::Refresh2(int Level)
	{
		EnsureEngine();
		if( m_pEngine && _tcsicmp(m_sEngineName.GetData(), _T("ie")) == 0 ) {
			CWebBrowserIeEngine* pIe = static_cast<CWebBrowserIeEngine*>(m_pEngine);
			if( pIe->GetHost() ) pIe->GetHost()->Refresh2(Level);
		}
		else if( m_pEngine ) {
			m_pEngine->Refresh();
		}
	}

	void CWebBrowserUI::GoBack()
	{
		EnsureEngine();
		if( m_pEngine ) m_pEngine->GoBack();
	}

	void CWebBrowserUI::GoForward()
	{
		EnsureEngine();
		if( m_pEngine ) m_pEngine->GoForward();
	}

	bool CWebBrowserUI::CanGoBack() const
	{
		return m_pEngine != NULL && m_pEngine->CanGoBack();
	}

	bool CWebBrowserUI::CanGoForward() const
	{
		return m_pEngine != NULL && m_pEngine->CanGoForward();
	}

	void CWebBrowserUI::Stop()
	{
		EnsureEngine();
		if( m_pEngine ) m_pEngine->Stop();
	}

	bool CWebBrowserUI::QueryUrl(CDuiString& out) const
	{
		out.Empty();
		if( m_pEngine != NULL && m_pEngine->GetUrl(out) && !out.IsEmpty() )
			return true;
		out = GetLocationUrl();
		return !out.IsEmpty();
	}

	void CWebBrowserUI::ExecuteScript(LPCTSTR script)
	{
		EnsureEngine();
		if( m_pEngine ) m_pEngine->ExecuteScript(script);
	}

	void CWebBrowserUI::OpenDevToolsWindow()
	{
		EnsureEngine();
		if( m_pEngine ) m_pEngine->OpenDevToolsWindow();
	}

	void CWebBrowserUI::DoMessageLoopWork()
	{
		if( m_pEngine ) m_pEngine->DoMessageLoopWork();
	}

	void* CWebBrowserUI::GetNative()
	{
		EnsureEngine();
		return m_pEngine ? m_pEngine->GetNative() : NULL;
	}

	IWebBrowser2* CWebBrowserUI::GetWebBrowser2()
	{
		if( m_pEngine == NULL || _tcsicmp(m_sEngineName.GetData(), _T("ie")) != 0 ) return NULL;
		return reinterpret_cast<IWebBrowser2*>(m_pEngine->GetNative());
	}

	IDispatch* CWebBrowserUI::GetHtmlWindow()
	{
		if( m_pEngine == NULL || _tcsicmp(m_sEngineName.GetData(), _T("ie")) != 0 ) return NULL;
		CWebBrowserIeEngine* pIe = static_cast<CWebBrowserIeEngine*>(m_pEngine);
		return pIe->GetHost() ? pIe->GetHost()->GetHtmlWindow() : NULL;
	}

	DISPID CWebBrowserUI::FindId(IDispatch *pObj, LPOLESTR pName)
	{
		return CWebBrowserIeHost::FindId(pObj, pName);
	}

	HRESULT CWebBrowserUI::InvokeMethod(IDispatch *pObj, LPOLESTR pMehtod, VARIANT *pVarResult, VARIANT *ps, int cArgs)
	{
		return CWebBrowserIeHost::InvokeMethod(pObj, pMehtod, pVarResult, ps, cArgs);
	}

	HRESULT CWebBrowserUI::GetProperty(IDispatch *pObj, LPOLESTR pName, VARIANT *pValue)
	{
		return CWebBrowserIeHost::GetProperty(pObj, pName, pValue);
	}

	HRESULT CWebBrowserUI::SetProperty(IDispatch *pObj, LPOLESTR pName, VARIANT *pValue)
	{
		return CWebBrowserIeHost::SetProperty(pObj, pName, pValue);
	}

	////////////////////////////////////////////////////////////////////////
	CWebView2UI::CWebView2UI()
	{
		SetEngine(_T("webview2"));
		SetEngineFallback(true);
	}

	LPCTSTR CWebView2UI::GetClass() const
	{
		return _T("WebView2UI");
	}

	LPVOID CWebView2UI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, _T("WebView2")) == 0 ) return static_cast<CWebView2UI*>(this);
		return CWebBrowserUI::GetInterface(pstrName);
	}
}
