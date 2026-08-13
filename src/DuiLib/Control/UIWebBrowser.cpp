#include "StdAfx.h"
#include "UIWebBrowser.h"
#include "WebBrowserIeEngine.h"
#include <ExDisp.h>
#include <atlconv.h>
#include <atlcomcli.h>
#include <MsHTML.h>

namespace DuiLib
{
	namespace
	{
		const UINT kTimerNativeResizeHook = 0x57485201;
		const UINT kOverlayFollowMs = 200;
		const TCHAR kOverlayClass[] = _T("DuiLib_WbResizeOverlay");
		bool g_bOverlayClassReg = false;

		LRESULT CALLBACK OverlayWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			CWebBrowserUI* pWeb = reinterpret_cast<CWebBrowserUI*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
			switch( uMsg ) {
			case WM_MOUSEACTIVATE:
				return MA_NOACTIVATE;
			case WM_NCHITTEST:
				return HTCLIENT;
			case WM_SETCURSOR:
				if( pWeb != NULL ) {
					POINT pt = { 0, 0 };
					::GetCursorPos(&pt);
					LRESULT ht = pWeb->HitNativeHostResize(pt);
					LPCTSTR idc = IDC_ARROW;
					if( ht == HTLEFT || ht == HTRIGHT ) idc = IDC_SIZEWE;
					else if( ht == HTTOP || ht == HTBOTTOM ) idc = IDC_SIZENS;
					else if( ht == HTTOPLEFT || ht == HTBOTTOMRIGHT ) idc = IDC_SIZENWSE;
					else if( ht == HTTOPRIGHT || ht == HTBOTTOMLEFT ) idc = IDC_SIZENESW;
					::SetCursor(::LoadCursor(NULL, idc));
					return TRUE;
				}
				break;
			case WM_LBUTTONDOWN:
				if( pWeb != NULL && pWeb->GetManager() != NULL ) {
					POINT pt = { 0, 0 };
					::GetCursorPos(&pt);
					LRESULT ht = pWeb->HitNativeHostResize(pt);
					HWND hPaint = pWeb->GetManager()->GetPaintWindow();
					if( hPaint != NULL && ht != HTCLIENT ) {
						::ReleaseCapture();
						::PostMessage(hPaint, WM_NCLBUTTONDOWN, (WPARAM)ht,
							MAKELPARAM(pt.x, pt.y));
					}
					return 0;
				}
				break;
			case WM_ERASEBKGND:
				return 1;
			case WM_PAINT: {
				PAINTSTRUCT ps;
				::BeginPaint(hWnd, &ps);
				::EndPaint(hWnd, &ps);
				return 0;
			}
			case WM_NCDESTROY:
				::SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
				break;
			default:
				break;
			}
			return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		void RegisterOverlayClass()
		{
			if( g_bOverlayClassReg ) return;
			WNDCLASSEX wc = { sizeof(wc) };
			wc.lpfnWndProc = OverlayWndProc;
			wc.hInstance = (HINSTANCE)::GetModuleHandle(NULL);
			wc.lpszClassName = kOverlayClass;
			wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
			wc.hbrBackground = (HBRUSH)::GetStockObject(NULL_BRUSH);
			::RegisterClassEx(&wc);
			g_bOverlayClassReg = true;
		}

		HWND CreateResizeOverlay(HWND hPaint, CWebBrowserUI* pWeb)
		{
			RegisterOverlayClass();
			HWND hWnd = ::CreateWindowEx(
				WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
				kOverlayClass, NULL,
				WS_POPUP,
				0, 0, 0, 0, hPaint, NULL, (HINSTANCE)::GetModuleHandle(NULL), NULL);
			if( hWnd == NULL ) return NULL;
			::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pWeb);
			::SetLayeredWindowAttributes(hWnd, 0, 1, LWA_ALPHA);
			return hWnd;
		}

		void CombineEdgeRgn(HRGN rgn, int x1, int y1, int x2, int y2)
		{
			if( x2 <= x1 || y2 <= y1 ) return;
			HRGN edge = ::CreateRectRgn(x1, y1, x2, y2);
			::CombineRgn(rgn, rgn, edge, RGN_OR);
			::DeleteObject(edge);
		}

		RECT ControlRectToScreen(HWND hPaint, const RECT& rcItem)
		{
			POINT tl = { rcItem.left, rcItem.top };
			POINT br = { rcItem.right, rcItem.bottom };
			::ClientToScreen(hPaint, &tl);
			::ClientToScreen(hPaint, &br);
			RECT rc = { tl.x, tl.y, br.x, br.y };
			return rc;
		}

		bool IsPaintFamilyWindow(HWND hPaint, HWND hOverlay, HWND hWnd)
		{
			if( hWnd == NULL || hPaint == NULL ) return false;
			if( hWnd == hPaint || hWnd == hOverlay ) return true;
			if( ::IsChild(hPaint, hWnd) ) return true;
			HWND hRoot = ::GetAncestor(hWnd, GA_ROOT);
			if( hRoot == hPaint || hRoot == hOverlay ) return true;
			for( HWND hOwner = ::GetWindow(hWnd, GW_OWNER); hOwner != NULL;
				hOwner = ::GetWindow(hOwner, GW_OWNER) ) {
				if( hOwner == hPaint || hOwner == hOverlay ) return true;
			}
			return false;
		}

		bool ShouldHideResizeOverlay(HWND hPaint, HWND hOverlay)
		{
			HWND hFg = ::GetForegroundWindow();
			if( hFg == NULL || hFg == hPaint || hFg == hOverlay ) return false;
			if( IsPaintFamilyWindow(hPaint, hOverlay, hFg) ) return false;

			// 勿因主窗/其它无关窗在前台就藏 overlay（最小化还原后常见），
			// 否则要等用户点标题栏激活浏览器才“突然好了”。
			TCHAR cls[128] = { 0 };
			::GetClassName(hFg, cls, _countof(cls));
			if( _tcscmp(cls, _T("ThemePickerWnd")) == 0 ) return true;
			if( _tcscmp(cls, _T("DuiMessageBoxWnd")) == 0 ) return true;
			if( _tcsstr(cls, _T("Modal")) != NULL ) return true;
			return false;
		}
	}
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
		, m_hResizeOverlay(NULL)
		, m_bPaintWasIconic(false)
		, m_bHostSizeMove(false)
		, m_bNativeWindowResizeEnabled(true)
		, m_bNativeResizeOverlaySuspended(false)
	{
		SetMouseEnabled(false);
	}

	CWebBrowserUI::~CWebBrowserUI()
	{
		if( m_pManager != NULL )
			m_pManager->RemoveMessageFilter(this);
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

	void CWebBrowserUI::SetNativeWindowResizeEnabled(bool bEnable)
	{
		if( m_bNativeWindowResizeEnabled == bEnable ) return;
		m_bNativeWindowResizeEnabled = bEnable;
		if( !bEnable )
			DestroyResizeOverlay();
		else if( !m_bNativeResizeOverlaySuspended )
			UpdateResizeOverlay();
	}

	bool CWebBrowserUI::IsNativeWindowResizeEnabled() const
	{
		return m_bNativeWindowResizeEnabled;
	}

	void CWebBrowserUI::SuspendNativeResizeOverlay(bool bSuspend)
	{
		if( m_bNativeResizeOverlaySuspended == bSuspend ) return;
		m_bNativeResizeOverlaySuspended = bSuspend;
		if( bSuspend )
			DestroyResizeOverlay(); // KillTimer + 拆 popup
		else if( m_bNativeWindowResizeEnabled )
			UpdateResizeOverlay(true);
	}

	bool CWebBrowserUI::IsNativeResizeOverlaySuspended() const
	{
		return m_bNativeResizeOverlaySuspended;
	}

	LRESULT CWebBrowserUI::HitNativeHostResize(POINT ptScreen) const
	{
		if( !m_bNativeWindowResizeEnabled || m_bNativeResizeOverlaySuspended ) return HTCLIENT;
		if( m_pManager == NULL ) return HTCLIENT;
		HWND hPaint = m_pManager->GetPaintWindow();
		if( hPaint == NULL || ::IsZoomed(hPaint) ) return HTCLIENT;

		POINT pt = ptScreen;
		::ScreenToClient(hPaint, &pt);

		// 仅 window-resize / window-size-box（祖先链）；未配置则不缩窗
		for( const CControlUI* p = this; p != NULL; p = p->GetParent() ) {
			LRESULT ht = p->HitWindowResize(pt);
			if( ht != HTCLIENT ) return ht;
		}
		return HTCLIENT;
	}

	RECT CWebBrowserUI::GetNativeResizeGripInset() const
	{
		RECT inset = { 0, 0, 0, 0 };
		if( m_pManager == NULL ) return inset;
		HWND hPaint = m_pManager->GetPaintWindow();
		if( hPaint == NULL || ::IsZoomed(hPaint) ) return inset;

		const RECT& rc = m_rcItem;
		for( const CControlUI* p = this; p != NULL; p = p->GetParent() ) {
			if( p->GetWindowResizeEdges() == 0 ) continue;
			RECT sb = p->GetWindowResizeThickness();
			const RECT& a = p->GetPos();
			if( sb.right > 0 && rc.right >= a.right - 1 )
				inset.right = MAX(inset.right, sb.right);
			if( sb.bottom > 0 && rc.bottom >= a.bottom - 1 )
				inset.bottom = MAX(inset.bottom, sb.bottom);
			if( sb.left > 0 && rc.left <= a.left + 1 )
				inset.left = MAX(inset.left, sb.left);
			if( sb.top > 0 && rc.top <= a.top + 1 )
				inset.top = MAX(inset.top, sb.top);
		}
		return inset;
	}

	void CWebBrowserUI::DestroyResizeOverlay()
	{
		KillTimer(kTimerNativeResizeHook);
		if( m_hResizeOverlay != NULL && ::IsWindow(m_hResizeOverlay) )
			::DestroyWindow(m_hResizeOverlay);
		m_hResizeOverlay = NULL;
	}

	void CWebBrowserUI::ArmNativeResizeFollowTimer(UINT uElapse)
	{
		if( m_bNativeResizeOverlaySuspended || !m_bNativeWindowResizeEnabled )
			return;
		SetTimer(kTimerNativeResizeHook, uElapse);
	}

	void CWebBrowserUI::UpdateResizeOverlay(bool bForceRecreate)
	{
		if( m_bNativeResizeOverlaySuspended
			|| IsOffScreenHost() || m_pEngine == NULL || m_pManager == NULL || !IsVisible() ) {
			DestroyResizeOverlay();
			return;
		}
		HWND hPaint = m_pManager->GetPaintWindow();
		if( hPaint == NULL ) {
			DestroyResizeOverlay();
			return;
		}
		if( ::IsZoomed(hPaint) ) {
			DestroyResizeOverlay();
			return;
		}
		if( ::IsIconic(hPaint) ) {
			m_bPaintWasIconic = true;
			if( m_hResizeOverlay != NULL && ::IsWindow(m_hResizeOverlay) ) {
				::DestroyWindow(m_hResizeOverlay);
				m_hResizeOverlay = NULL;
			}
			ArmNativeResizeFollowTimer(kOverlayFollowMs);
			return;
		}
		if( m_bPaintWasIconic ) {
			bForceRecreate = true;
			m_bPaintWasIconic = false;
		}

		const RECT& rc = m_rcItem;
		const int wItem = rc.right - rc.left;
		const int hItem = rc.bottom - rc.top;
		RECT inset = GetNativeResizeGripInset();
		if( !m_bNativeWindowResizeEnabled || wItem < 1 || hItem < 1
			|| (inset.left < 1 && inset.top < 1 && inset.right < 1 && inset.bottom < 1) ) {
			if( m_hResizeOverlay != NULL )
				::ShowWindow(m_hResizeOverlay, SW_HIDE);
			if( m_bNativeWindowResizeEnabled )
				ArmNativeResizeFollowTimer(kOverlayFollowMs);
			else
				KillTimer(kTimerNativeResizeHook);
			return;
		}

		if( ShouldHideResizeOverlay(hPaint, m_hResizeOverlay) ) {
			if( m_hResizeOverlay != NULL )
				::ShowWindow(m_hResizeOverlay, SW_HIDE);
			ArmNativeResizeFollowTimer(kOverlayFollowMs);
			return;
		}

		if( bForceRecreate && m_hResizeOverlay != NULL && ::IsWindow(m_hResizeOverlay) ) {
			::DestroyWindow(m_hResizeOverlay);
			m_hResizeOverlay = NULL;
		}
		if( m_hResizeOverlay != NULL && !::IsWindow(m_hResizeOverlay) )
			m_hResizeOverlay = NULL;
		if( m_hResizeOverlay == NULL )
			m_hResizeOverlay = CreateResizeOverlay(hPaint, this);
		if( m_hResizeOverlay == NULL ) {
			ArmNativeResizeFollowTimer(kOverlayFollowMs);
			return;
		}

		const RECT rcScreen = ControlRectToScreen(hPaint, rc);
		const int w = rcScreen.right - rcScreen.left;
		const int h = rcScreen.bottom - rcScreen.top;

		HRGN rgn = ::CreateRectRgn(0, 0, 0, 0);
		CombineEdgeRgn(rgn, 0, 0, inset.left, h);
		CombineEdgeRgn(rgn, 0, 0, w, inset.top);
		CombineEdgeRgn(rgn, w - inset.right, 0, w, h);
		CombineEdgeRgn(rgn, 0, h - inset.bottom, w, h);
		::SetWindowRgn(m_hResizeOverlay, rgn, TRUE);

		// 拖拽改大小时勿每帧 HWND_TOP：会搅动 DWM，标题栏闪得很厉害。
		// 新建 / 强制重建 / 非拖拽再抬一次即可。
		UINT uFlags = SWP_NOACTIVATE | SWP_SHOWWINDOW;
		HWND hInsertAfter = HWND_TOP;
		if( m_bHostSizeMove && !bForceRecreate ) {
			uFlags |= SWP_NOZORDER;
			hInsertAfter = NULL;
		}
		::SetWindowPos(m_hResizeOverlay, hInsertAfter, rcScreen.left, rcScreen.top, w, h, uFlags);
		ArmNativeResizeFollowTimer(kOverlayFollowMs);
	}

	void CWebBrowserUI::ScheduleNativeResizeHook(bool bResetRetry)
	{
		if( m_bNativeResizeOverlaySuspended ) {
			DestroyResizeOverlay();
			return;
		}
		if( IsOffScreenHost() || m_pEngine == NULL || m_pManager == NULL ) {
			DestroyResizeOverlay();
			return;
		}
		// 交互缩放中：只跟 bounds，overlay 用定时器合并，避免每帧 Destroy/Create/TOP
		if( m_bHostSizeMove && !bResetRetry ) {
			ArmNativeResizeFollowTimer(50);
			return;
		}
		UpdateResizeOverlay(bResetRetry);
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
		if( event.Type == UIEVENT_TIMER && event.wParam == kTimerNativeResizeHook ) {
			UpdateResizeOverlay();
			return;
		}
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
		KillTimer(kTimerNativeResizeHook);
		DestroyResizeOverlay();
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
		ScheduleNativeResizeHook(true);

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
		if( m_pManager != NULL )
			m_pManager->RemoveMessageFilter(this);
		CControlUI::SetManager(pManager, pParent, bInit);
		if( m_pManager != NULL )
			m_pManager->AddMessageFilter(this);
		if( pManager && bInit ) EnsureEngine();
	}

	LRESULT CWebBrowserUI::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& bHandled)
	{
		bHandled = false;
		if( m_pEngine == NULL || IsOffScreenHost() )
			return 0;
		switch( uMsg ) {
		case WM_ENTERSIZEMOVE:
			m_bHostSizeMove = true;
			break;
		case WM_MOVE:
		case WM_MOVING:
			if( !m_bHostSizeMove )
				UpdateResizeOverlay();
			break;
		case WM_EXITSIZEMOVE:
			m_bHostSizeMove = false;
			UpdateResizeOverlay();
			break;
		case WM_ACTIVATE:
			if( wParam != WA_INACTIVE )
				UpdateResizeOverlay(m_bPaintWasIconic);
			break;
		case WM_ACTIVATEAPP:
			if( wParam )
				UpdateResizeOverlay(m_bPaintWasIconic);
			break;
		case WM_SHOWWINDOW:
			if( wParam )
				UpdateResizeOverlay(true);
			break;
		case WM_SIZE:
			if( wParam == SIZE_MINIMIZED ) {
				m_bPaintWasIconic = true;
				UpdateResizeOverlay();
			}
			else if( wParam == SIZE_MAXIMIZED ) {
				UpdateResizeOverlay();
			}
			else if( wParam == SIZE_RESTORED ) {
				// 拖拽中每个 WM_SIZE 都是 SIZE_RESTORED；旧逻辑 UpdateResizeOverlay(true)
				// 会每帧 Destroy+Create overlay → 整窗（含标题栏）狂闪。
				if( m_bPaintWasIconic )
					UpdateResizeOverlay(true);
				else if( !m_bHostSizeMove )
					UpdateResizeOverlay(false);
			}
			break;
		case WM_WINDOWPOSCHANGED:
			if( m_bHostSizeMove )
				break;
			UpdateResizeOverlay(m_bPaintWasIconic);
			break;
		default:
			break;
		}
		return 0;
	}

	void CWebBrowserUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		const bool bSame = (rc.left == m_rcItem.left && rc.top == m_rcItem.top
			&& rc.right == m_rcItem.right && rc.bottom == m_rcItem.bottom);
		CControlUI::SetPos(rc, bNeedInvalidate);
		EnsureEngine();
		if( m_pEngine == NULL ) return;

		if( !bSame )
			m_pEngine->SetPos(m_rcItem);
		ScheduleNativeResizeHook();
	}

	void CWebBrowserUI::SetVisible(bool bVisible)
	{
		CControlUI::SetVisible(bVisible);
		if( m_pEngine ) m_pEngine->SetVisible(IsVisible());
		UpdateResizeOverlay();
	}

	void CWebBrowserUI::SetInternVisible(bool bVisible)
	{
		CControlUI::SetInternVisible(bVisible);
		if( m_pEngine ) m_pEngine->SetVisible(IsVisible());
		UpdateResizeOverlay();
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
		else if( _tcsicmp(pstrName, _T("native-window-resize")) == 0 ) {
			SetNativeWindowResizeEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
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
