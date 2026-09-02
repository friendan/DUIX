#include "StdAfx.h"
#include <zmouse.h>
#include <Imm.h>
#include "DuiExitTrace.h"
#pragma comment(lib, "Imm32.lib")

#pragma warning(push)
#pragma warning(disable:4838 4244)


namespace DuiLib {

namespace
{
	const TCHAR kTipPopupClass[] = _T("DuiLibTipPopup");
	const int kTipDefaultMaxTextWidth = 360;
	const int kTipPadX = 8;
	const int kTipPadY = 6;

	VOID CALLBACK ToolTipQueueTimerProc(PVOID lpParameter, BOOLEAN /*TimerOrWaitFired*/)
	{
		HWND hWnd = static_cast<HWND>(lpParameter);
		if( hWnd == NULL || !::IsWindow(hWnd) ) return;
		::PostMessage(hWnd, UIMSG_TOOLTIP_HOVER, 0, 0);
	}

	bool TipPopupEnsureClass(HINSTANCE hInst)
	{
		static bool s_registered = false;
		if( s_registered ) return true;
		WNDCLASSEX wc = { sizeof(wc) };
		if( ::GetClassInfoEx(hInst, kTipPopupClass, &wc) ) {
			s_registered = true;
			return true;
		}
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = CPaintManagerUI::TipPopupWndProc;
		wc.hInstance = hInst;
		wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszClassName = kTipPopupClass;
		if( ::RegisterClassEx(&wc) == 0 && ::GetLastError() != ERROR_CLASS_ALREADY_EXISTS )
			return false;
		s_registered = true;
		return true;
	}

	SIZE MeasureTipPopupWndSize(CPaintManagerUI* pManager, LPCTSTR text, int nMaxTextWidth)
	{
		SIZE szWnd = { 64, 28 };
		if( pManager == NULL || text == NULL || *text == _T('\0') ) return szWnd;

		HDC hdc = ::GetDC(NULL);
		if( hdc == NULL ) return szWnd;

		int maxTextCx = nMaxTextWidth;
		if( maxTextCx <= 0 )
			maxTextCx = pManager->GetDPIObj()->Scale(kTipDefaultMaxTextWidth);

		// 与 WM_PAINT 相同路径（GdiplusDrawText），避免 DWrite 测量与 GDI+ 绘制宽度不一致
		RECT rc = { 0, 0, 0, 0 };
		CRenderEngine::GdiplusDrawText(hdc, pManager, rc, text, pManager->GetDefaultFontColor(), 0,
			DT_LEFT | DT_TOP | DT_SINGLELINE | DT_CALCRECT | DT_NOPREFIX);
		int textCx = rc.right - rc.left;
		if( textCx > maxTextCx ) textCx = maxTextCx;

		rc = { 0, 0, textCx, 0 };
		CRenderEngine::GdiplusDrawText(hdc, pManager, rc, text, pManager->GetDefaultFontColor(), 0,
			DT_LEFT | DT_TOP | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX);
		::ReleaseDC(NULL, hdc);

		int cxClient = rc.right + kTipPadX * 2 + 2;
		int cyClient = rc.bottom + kTipPadY * 2 + 4;
		if( cxClient < 32 ) cxClient = 32;
		if( cyClient < 24 ) cyClient = 24;
		RECT rcWnd = { 0, 0, cxClient, cyClient };
		::AdjustWindowRectEx(&rcWnd, WS_POPUP | WS_BORDER, FALSE,
			WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE);
		szWnd.cx = rcWnd.right - rcWnd.left;
		szWnd.cy = rcWnd.bottom - rcWnd.top;
		return szWnd;
	}

	void ClampTipPopupPos(POINT& pt, SIZE sz)
	{
		HMONITOR hMon = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		if( hMon == NULL || !::GetMonitorInfo(hMon, &mi) ) return;
		const int margin = 4;
		if( pt.x + sz.cx > mi.rcWork.right - margin )
			pt.x = mi.rcWork.right - margin - sz.cx;
		if( pt.x < mi.rcWork.left + margin )
			pt.x = mi.rcWork.left + margin;
		if( pt.y + sz.cy > mi.rcWork.bottom - margin )
			pt.y = pt.y - sz.cy - 8;
		if( pt.y < mi.rcWork.top + margin )
			pt.y = mi.rcWork.top + margin;
	}
}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

	void GetChildWndRect(HWND hWnd, HWND hChildWnd, RECT& rcChildWnd)
	{
		::GetWindowRect(hChildWnd, &rcChildWnd);

		POINT pt;
		pt.x = rcChildWnd.left;
		pt.y = rcChildWnd.top;
		::ScreenToClient(hWnd, &pt);
		rcChildWnd.left = pt.x;
		rcChildWnd.top = pt.y;

		pt.x = rcChildWnd.right;
		pt.y = rcChildWnd.bottom;
		::ScreenToClient(hWnd, &pt);
		rcChildWnd.right = pt.x;
		rcChildWnd.bottom = pt.y;
	}

	static UINT MapKeyState()
	{
		UINT uState = 0;
		if( ::GetKeyState(VK_CONTROL) < 0 ) uState |= MK_CONTROL;
		if( ::GetKeyState(VK_LBUTTON) < 0 ) uState |= MK_LBUTTON;
		if( ::GetKeyState(VK_RBUTTON) < 0 ) uState |= MK_RBUTTON;
		if( ::GetKeyState(VK_SHIFT) < 0 ) uState |= MK_SHIFT;
		if( ::GetKeyState(VK_MENU) < 0 ) uState |= MK_ALT;
		return uState;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	typedef struct tagFINDTABINFO
	{
		CControlUI* pFocus;
		CControlUI* pLast;
		bool bForward;
		bool bNextIsIt;
	} FINDTABINFO;

	typedef struct tagFINDSHORTCUT
	{
		TCHAR ch;
		bool bPickNext;
	} FINDSHORTCUT;

	typedef struct tagTIMERINFO
	{
		CControlUI* pSender;
		UINT nLocalID;
		HWND hWnd;
		UINT uWinTimer;
		bool bKilled;
	} TIMERINFO;


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	tagTImageInfo::tagTImageInfo()
	{
		pImage = NULL;
		hBitmap = NULL;
		pBits = NULL;
		pSrcBits = NULL;
		nX = 0;
		nY = 0;
		bAlpha = false;
		bUseHSL = false;
		dwMask = 0;
		pBackend = NULL;
		nBackend = RENDER_BACKEND_GDI;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	tagTDrawInfo::tagTDrawInfo()
	{
		Clear();
	}

	void tagTDrawInfo::Parse(LPCTSTR pStrImage, LPCTSTR pStrModify,CPaintManagerUI *pManager)
	{
		// 1??aaa.jpg
		// 2??file='aaa.jpg' res='' restype='0' dest='0,0,0,0' source='0,0,0,0' corner='0,0,0,0' 
		// mask='#FF0000' fade='255' hole='false' xtiled='false' ytiled='false'
		sDrawString = pStrImage;
		sDrawModify = pStrModify;
		sImageName = pStrImage;

		CDuiString sItem;
		CDuiString sValue;
		LPTSTR pstr = NULL;
		for( int i = 0; i < 2; ++i ) {
			if( i == 1) pStrImage = pStrModify;
			if( !pStrImage ) continue;
			while( *pStrImage != _T('\0') ) {
				sItem.Empty();
				sValue.Empty();
				while( *pStrImage > _T('\0') && *pStrImage <= _T(' ') ) pStrImage = ::CharNext(pStrImage);
				while( *pStrImage != _T('\0') && *pStrImage != _T('=') && *pStrImage > _T(' ') ) {
					LPTSTR pstrTemp = ::CharNext(pStrImage);
					while( pStrImage < pstrTemp) {
						sItem += *pStrImage++;
					}
				}
				while( *pStrImage > _T('\0') && *pStrImage <= _T(' ') ) pStrImage = ::CharNext(pStrImage);
				if( *pStrImage++ != _T('=') ) break;
				while( *pStrImage > _T('\0') && *pStrImage <= _T(' ') ) pStrImage = ::CharNext(pStrImage);
				if( *pStrImage++ != _T('\'') ) break;
				while( *pStrImage != _T('\0') && *pStrImage != _T('\'') ) {
					LPTSTR pstrTemp = ::CharNext(pStrImage);
					while( pStrImage < pstrTemp) {
						sValue += *pStrImage++;
					}
				}
				if( *pStrImage++ != _T('\'') ) break;
				if( !sValue.IsEmpty() ) {
					if( sItem == _T("file") || sItem == _T("res") ) {
						sImageName = sValue;
					}
					else if( sItem == _T("restype") ) {
						sResType = sValue;
					}
					else if( sItem == _T("dest") ) {
						rcDest.left = _tcstol(sValue.GetData(), &pstr, 10);  ASSERT(pstr);    
						rcDest.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
						rcDest.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
						rcDest.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);  
						// XML / ?????? @scale ??????????????
						if(pManager != NULL) pManager->GetDPIObj()->Scale(&rcDest);
					}
					else if( sItem == _T("source") ) {
						rcSource.left = _tcstol(sValue.GetData(), &pstr, 10);  ASSERT(pstr);    
						rcSource.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
						rcSource.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
						rcSource.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
						if(pManager != NULL) pManager->GetDPIObj()->Scale(&rcSource);
					}
					else if( sItem == _T("corner") ) {
						rcCorner.left = _tcstol(sValue.GetData(), &pstr, 10);  ASSERT(pstr);    
						rcCorner.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
						rcCorner.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
						rcCorner.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
						if(pManager != NULL) pManager->GetDPIObj()->Scale(&rcCorner);
					}
					else if( sItem == _T("mask") ) {
						if( sValue[0] == _T('#')) dwMask = _tcstoul(sValue.GetData() + 1, &pstr, 16);
						else dwMask = _tcstoul(sValue.GetData(), &pstr, 16);
					}
					else if( sItem == _T("fade") ) {
						uFade = (UINT)_tcstoul(sValue.GetData(), &pstr, 10);
					}
					else if( sItem == _T("rotate") ) {
						uRotate = (UINT)_tcstoul(sValue.GetData(), &pstr, 10);
						bGdiplus = true;
					}
					else if( sItem == _T("gdiplus") ) {
						bGdiplus = (_tcsicmp(sValue.GetData(), _T("true")) == 0);
					}
					else if( sItem == _T("hole") ) {
						bHole = (_tcsicmp(sValue.GetData(), _T("true")) == 0);
					}
					else if( sItem == _T("xtiled") ) {
						bTiledX = (_tcsicmp(sValue.GetData(), _T("true")) == 0);
					}
					else if( sItem == _T("ytiled") ) {
						bTiledY = (_tcsicmp(sValue.GetData(), _T("true")) == 0);
					}
					else if( sItem == _T("hsl") ) {
						bHSL = (_tcsicmp(sValue.GetData(), _T("true")) == 0);
					}
					else if( sItem == _T("size") ) {
						szImage.cx = _tcstol(sValue.GetData(), &pstr, 10);  ASSERT(pstr);
						szImage.cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
					}
					else if( sItem == _T("align") ) {
						sAlign = sValue;
					}
					else if( sItem == _T("padding") ) {
						rcPadding.left = _tcstol(sValue.GetData(), &pstr, 10);  ASSERT(pstr);    
						rcPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
						rcPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
						rcPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);  
						//if(pManager != NULL) pManager->GetDPIObj()->Scale(&rcPadding);
					}
				}
				if( *pStrImage++ != _T(' ') ) break;
			}
		}

		// ????DPI???
		if (pManager != NULL && pManager->GetDPIObj()->GetScale() != 100) {
			CDuiString sScale;
			sScale.Format(_T("@%d."), pManager->GetDPIObj()->GetScale());
			sImageName.Replace(_T("."), sScale.GetData());
		}
	}
	void tagTDrawInfo::Clear()
	{
		sDrawString.Empty();
		sDrawModify.Empty();
		sImageName.Empty();

		memset(&rcDest, 0, sizeof(RECT));
		memset(&rcSource, 0, sizeof(RECT));
		memset(&rcCorner, 0, sizeof(RECT));
		dwMask = 0;
		uFade = 255;
		uRotate = 0;
		bHole = false;
		bTiledX = false;
		bTiledY = false;
		bHSL = false;
		bGdiplus = false;

		szImage.cx = szImage.cy = 0;
		sAlign.Empty();
		memset(&rcPadding, 0, sizeof(RECT));
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	typedef BOOL (__stdcall *PFUNCUPDATELAYEREDWINDOW)(HWND, HDC, POINT*, SIZE*, HDC, POINT*, COLORREF, BLENDFUNCTION*, DWORD);
	PFUNCUPDATELAYEREDWINDOW g_fUpdateLayeredWindow = NULL;

	HPEN m_hUpdateRectPen = NULL;

	HINSTANCE CPaintManagerUI::m_hResourceInstance = NULL;
	CDuiString CPaintManagerUI::m_pStrResourcePath;
	CDuiString CPaintManagerUI::m_pStrResourceZip;
	CDuiString CPaintManagerUI::m_pStrResourceZipPwd;  //Garfield 20160325 ??????zip??????
	CZipFile* CPaintManagerUI::m_pResourceZip = NULL;
	bool CPaintManagerUI::m_bCachedResourceZip = true;
	int CPaintManagerUI::m_nResType = UILIB_FILE;
	TResInfo CPaintManagerUI::m_SharedResInfo;
	HINSTANCE CPaintManagerUI::m_hInstance = NULL;
	bool CPaintManagerUI::m_bUseHSL = false;
	short CPaintManagerUI::m_H = 180;
	short CPaintManagerUI::m_S = 100;
	short CPaintManagerUI::m_L = 100;
	CStdPtrArray CPaintManagerUI::m_aPreMessages;
	CStdPtrArray CPaintManagerUI::m_aPlugins;

	CPaintManagerUI::CPaintManagerUI() :
		m_hWndPaint(NULL),
		m_hDcPaint(NULL),
		m_pRenderContext(NULL),
		m_pOffscreenSurface(NULL),
		m_pBackgroundSurface(NULL),
		m_hwndTipPopup(NULL),
		m_pTipPending(NULL),
		m_pTipShown(NULL),
		m_hTipQueueTimer(NULL),
		m_iHoverTime(400),
		m_bShowUpdateRect(false),
		m_pRoot(NULL),
		m_pFocus(NULL),
		m_pEventHover(NULL),
		m_pEventClick(NULL),
		m_pEventRClick(NULL),
		m_pEventKey(NULL),
		m_bBlankCtxMenu(false),
		m_bBlankCtxMenuDeepest(true),
		m_uTimerID(0x1000),
		m_bFirstLayout(true),
		m_bUpdateNeeded(false),
		m_bFocusNeeded(false),
		m_bOffscreenPaint(true),
		m_nOpacity(0xFF),
		m_nWallpaperBleed(0xFF),
		m_bWallpaperBleedNeedImage(true),
		m_dwWindowBackgroundColor(0xF0F0F0FF),
		m_bWindowBackgroundColorCustom(false),
		m_bWindowBackgroundImageCustom(false),
		m_windowAction(UIACTION_NONE),
		m_bLayered(false),
		m_bLayeredCompositionEnabled(true),
		m_bLayeredChanged(false),
		m_nShapeAlphaThreshold(16),
		m_bShapeDragEnabled(true),
		m_bWindowActionFromShape(false),
		m_bMouseTracking(false),
		m_bMouseCapture(false),
		m_bUsedVirtualWnd(false),
		m_bAsyncNotifyPosted(false),
		m_bForceUseSharedRes(false),
		m_pDPI(NULL),
		m_bUseGdiplusText(false),
		m_trh(0),
		m_bDragDrop(false),
		m_bDragMode(false),
		m_hDragBitmap(NULL)
	{
		if (m_SharedResInfo.m_DefaultFontInfo.sFontName.IsEmpty())
		{
			m_SharedResInfo.m_dwDefaultDisabledColor = 0xA7A6AAFF;
			m_SharedResInfo.m_dwDefaultFontColor = 0x000000FF;
			m_SharedResInfo.m_dwDefaultLinkFontColor = 0x0000FFFF;
			m_SharedResInfo.m_dwDefaultLinkHoverFontColor = 0xD3215FFF;
			m_SharedResInfo.m_dwDefaultSelectedBackgroundColor = 0xBAE4FFFF;

			LOGFONT lf = { 0 };
			::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
			lf.lfCharSet = DEFAULT_CHARSET;
			lf.lfHeight = -12;
			lf.lfWeight = FW_NORMAL;
			lf.lfQuality = CLEARTYPE_QUALITY;
			_tcsncpy(lf.lfFaceName, _T("微软雅黑"), LF_FACESIZE - 1);
			lf.lfFaceName[LF_FACESIZE - 1] = _T('\0');
			m_SharedResInfo.m_DefaultFontInfo.sFontName = lf.lfFaceName;
			m_SharedResInfo.m_DefaultFontInfo.iSize = -lf.lfHeight;
			m_SharedResInfo.m_DefaultFontInfo.bBold = false;
			m_SharedResInfo.m_DefaultFontInfo.bUnderline = false;
			m_SharedResInfo.m_DefaultFontInfo.bItalic = false;
			m_SharedResInfo.m_DefaultFontInfo.bStrikeout = false;
			GetRenderDevice()->CreateNativeFont(&m_SharedResInfo.m_DefaultFontInfo, lf.lfHeight, NULL);
		}

		m_ResInfo.m_dwDefaultDisabledColor = m_SharedResInfo.m_dwDefaultDisabledColor;
		m_ResInfo.m_dwDefaultFontColor = m_SharedResInfo.m_dwDefaultFontColor;
		m_ResInfo.m_dwDefaultLinkFontColor = m_SharedResInfo.m_dwDefaultLinkFontColor;
		m_ResInfo.m_dwDefaultLinkHoverFontColor = m_SharedResInfo.m_dwDefaultLinkHoverFontColor;
		m_ResInfo.m_dwDefaultSelectedBackgroundColor = m_SharedResInfo.m_dwDefaultSelectedBackgroundColor;

		if( m_hUpdateRectPen == NULL ) {
			m_hUpdateRectPen = ::CreatePen(PS_SOLID, 1, RGB(220, 0, 0));
			// Boot Windows Common Controls (for the ToolTip control)
			INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_WIN95_CLASSES };
			::InitCommonControlsEx(&icc);
			::LoadLibrary(_T("msimg32.dll"));
		}

		m_szMinWindow.cx = 320;
		m_szMinWindow.cy = 240;
		m_szMaxWindow.cx = 0;
		m_szMaxWindow.cy = 0;
		// size 默认 0：沿用 Create(...) 传入的窗口尺寸；皮肤里写 size 可覆盖
		m_szInitWindowSize.cx = 0;
		m_szInitWindowSize.cy = 0;
		m_szBorderRadius.cx = m_szBorderRadius.cy = 0;
		// 无边框窗口常见可拖拽边距
		m_rcSizeBox.left = 4;
		m_rcSizeBox.top = 4;
		m_rcSizeBox.right = 6;
		m_rcSizeBox.bottom = 6;
		// 顶部标题拖拽区（逻辑像素，与常见 40px 标题栏对齐）
		m_rcCaption.left = 0;
		m_rcCaption.top = 0;
		m_rcCaption.right = 0;
		m_rcCaption.bottom = 40;
		::ZeroMemory(&m_rcLayeredPadding, sizeof(m_rcLayeredPadding));
		::ZeroMemory(&m_rcLayeredUpdate, sizeof(m_rcLayeredUpdate));
		m_ptLastMousePos.x = m_ptLastMousePos.y = -1;

		m_pGdiplusStartupInput = new Gdiplus::GdiplusStartupInput;
		Gdiplus::GdiplusStartup( &m_gdiplusToken, m_pGdiplusStartupInput, NULL); // ????GDI???

		CShadowUI::Initialize(m_hInstance);

		m_pDragDrop = NULL;
	}

	CPaintManagerUI::~CPaintManagerUI()
	{
		DUI_EXIT_SCOPE(L"~CPaintManagerUI");
		// Delete the control-tree structures
		{
			DUI_EXIT_SCOPE(L"~PM delayed/async/root");
			for( int i = 0; i < m_aDelayedCleanup.GetSize(); i++ ) delete static_cast<CControlUI*>(m_aDelayedCleanup[i]);
			m_aDelayedCleanup.Resize(0);
			for( int i = 0; i < m_aAsyncNotify.GetSize(); i++ ) delete static_cast<TNotifyUI*>(m_aAsyncNotify[i]);
			m_aAsyncNotify.Resize(0);

			m_mNameHash.Resize(0);
			if( m_pRoot != NULL ) delete m_pRoot;
		}

		{
			DUI_EXIT_SCOPE(L"~PM fonts/images/styles");
			{
				DUI_EXIT_SCOPE(L"~PM DestroyNativeFont(default)");
				GetRenderDevice()->DestroyNativeFont(&m_ResInfo.m_DefaultFontInfo);
			}
			{
				DUI_EXIT_SCOPE(L"~PM RemoveAllFonts");
				RemoveAllFonts();
			}
			{
				DUI_EXIT_SCOPE(L"~PM RemoveAllImages");
				RemoveAllImages();
			}
			{
				DUI_EXIT_SCOPE(L"~PM RemoveAllStyle");
				RemoveAllStyle();
			}
			{
				DUI_EXIT_SCOPE(L"~PM RemoveAllCssRules");
				RemoveAllCssRules();
			}
			{
				DUI_EXIT_SCOPE(L"~PM RemoveAllDefaultAttributeList");
				RemoveAllDefaultAttributeList();
			}
			{
				DUI_EXIT_SCOPE(L"~PM RemoveAllWindowCustomAttribute");
				RemoveAllWindowCustomAttribute();
			}
			{
				DUI_EXIT_SCOPE(L"~PM RemoveAllOptionGroups");
				RemoveAllOptionGroups();
			}
			{
				DUI_EXIT_SCOPE(L"~PM RemoveAllTimers");
				RemoveAllTimers();
			}
			{
				DUI_EXIT_SCOPE(L"~PM RemoveAllDrawInfos");
				RemoveAllDrawInfos();
			}
		}

		KillTipQueueTimer();
		if( m_hwndTipPopup != NULL ) {
			::DestroyWindow(m_hwndTipPopup);
			m_hwndTipPopup = NULL;
		}
		if (!m_aFonts.IsEmpty()) {
			for (int i = 0; i < m_aFonts.GetSize();++i)
			{
				HANDLE handle = static_cast<HANDLE>(m_aFonts.GetAt(i));
				::RemoveFontMemResourceEx(handle);
			}
		}
		{
			DUI_EXIT_SCOPE(L"~PM DestroySurface(offscreen/bg)");
			if( m_pOffscreenSurface != NULL ) {
				GetRenderDevice()->DestroySurface(m_pOffscreenSurface);
				m_pOffscreenSurface = NULL;
			}
			if( m_pBackgroundSurface != NULL ) {
				GetRenderDevice()->DestroySurface(m_pBackgroundSurface);
				m_pBackgroundSurface = NULL;
			}
		}
		if( m_hDcPaint != NULL ) ::ReleaseDC(m_hWndPaint, m_hDcPaint);
		m_aPreMessages.Remove(m_aPreMessages.Find(this));
		// ?????????
		if( m_hDragBitmap != NULL ) ::DeleteObject(m_hDragBitmap);
		{
			DUI_EXIT_SCOPE(L"~PM GdiplusShutdown");
			Gdiplus::GdiplusShutdown(m_gdiplusToken);
			delete m_pGdiplusStartupInput;
		}
		// DPI????????
		if (m_pDPI != NULL) {
			delete m_pDPI;
			m_pDPI = NULL;
		}
	}

	void CPaintManagerUI::Init(HWND hWnd, LPCTSTR pstrName)
	{
		ASSERT(::IsWindow(hWnd));

		m_mNameHash.Resize();
		RemoveAllFonts();
		RemoveAllImages();
		RemoveAllStyle();
		RemoveAllCssRules();
		RemoveAllDefaultAttributeList();
		RemoveAllWindowCustomAttribute();
		RemoveAllOptionGroups();
		RemoveAllTimers();

		m_sName.Empty();
		if( pstrName != NULL ) m_sName = pstrName;

		if( m_hWndPaint != hWnd ) {
			m_hWndPaint = hWnd;
			m_hDcPaint = ::GetDC(hWnd);
			m_aPreMessages.Add(this);
		}
	}

	HINSTANCE CPaintManagerUI::GetInstance()
	{
		return m_hInstance;
	}

	CDuiString CPaintManagerUI::GetInstancePath()
	{
		if( m_hInstance == NULL ) return _T('\0');

		TCHAR tszModule[MAX_PATH + 1] = { 0 };
		::GetModuleFileName(m_hInstance, tszModule, MAX_PATH);
		CDuiString sInstancePath = tszModule;
		int pos = sInstancePath.ReverseFind(_T('\\'));
		if( pos >= 0 ) sInstancePath = sInstancePath.Left(pos + 1);
		return sInstancePath;
	}

	CDuiString CPaintManagerUI::GetCurrentPath()
	{
		TCHAR tszModule[MAX_PATH + 1] = { 0 };
		::GetCurrentDirectory(MAX_PATH, tszModule);
		return tszModule;
	}

	HINSTANCE CPaintManagerUI::GetResourceDll()
	{
		if( m_hResourceInstance == NULL ) return m_hInstance;
		return m_hResourceInstance;
	}

	const CDuiString& CPaintManagerUI::GetResourcePath()
	{
		return m_pStrResourcePath;
	}

	const CDuiString& CPaintManagerUI::GetResourceZip()
	{
		return m_pStrResourceZip;
	}

	const CDuiString& CPaintManagerUI::GetResourceZipPwd()
	{
		return m_pStrResourceZipPwd;
	}

	bool CPaintManagerUI::IsCachedResourceZip()
	{
		return m_bCachedResourceZip;
	}

	HANDLE CPaintManagerUI::GetResourceZipHandle()
	{
		return (HANDLE)m_pResourceZip;
	}

	void CPaintManagerUI::SetInstance(HINSTANCE hInst)
	{
		m_hInstance = hInst;
		// 须在任何 HWND 创建前：否则跨屏时系统位图拉伸 → 文字变大且模糊
		CDPI::EnableProcessDpiAwareness();
	}

	void CPaintManagerUI::SetCurrentPath(LPCTSTR pStrPath)
	{
		::SetCurrentDirectory(pStrPath);
	}

	void CPaintManagerUI::SetResourceDll(HINSTANCE hInst)
	{
		m_hResourceInstance = hInst;
	}

	void CPaintManagerUI::SetResourcePath(LPCTSTR pStrPath)
	{
		m_pStrResourcePath = pStrPath;
		if( m_pStrResourcePath.IsEmpty() ) return;
		TCHAR cEnd = m_pStrResourcePath.GetAt(m_pStrResourcePath.GetLength() - 1);
		if( cEnd != _T('\\') && cEnd != _T('/') ) m_pStrResourcePath += _T('\\');
	}

	void CPaintManagerUI::SetResourceZip(LPVOID pVoid, unsigned int len, LPCTSTR password)
	{
		if( m_pResourceZip != NULL ) {
			delete m_pResourceZip;
			m_pResourceZip = NULL;
		}
		m_pStrResourceZip = _T("membuffer");
		m_bCachedResourceZip = true;
		m_pStrResourceZipPwd = (password != NULL) ? password : _T("");
		if( pVoid == NULL || len == 0 ) return;

		m_pResourceZip = new CZipFile();
		m_pResourceZip->SetPassword(m_pStrResourceZipPwd.GetData());
		if( !m_pResourceZip->OpenMemory(pVoid, len) ) {
			delete m_pResourceZip;
			m_pResourceZip = NULL;
		}
	}

	void CPaintManagerUI::SetResourceZip(LPCTSTR pStrPath, bool bCachedResourceZip, LPCTSTR password)
	{
		CDuiString pwd = (password != NULL) ? password : _T("");
		if( m_pStrResourceZip == pStrPath && m_bCachedResourceZip == bCachedResourceZip
			&& m_pStrResourceZipPwd == pwd && m_pResourceZip != NULL )
			return;
		if( m_pResourceZip != NULL ) {
			delete m_pResourceZip;
			m_pResourceZip = NULL;
		}
		m_pStrResourceZip = pStrPath;
		m_bCachedResourceZip = bCachedResourceZip;
		m_pStrResourceZipPwd = pwd;
		if( m_bCachedResourceZip && pStrPath != NULL && *pStrPath != _T('\0') ) {
			CDuiString sFile = CPaintManagerUI::GetResourcePath();
			sFile += CPaintManagerUI::GetResourceZip();
			m_pResourceZip = new CZipFile();
			m_pResourceZip->SetPassword(pwd.GetData());
			if( !m_pResourceZip->Open(sFile.GetData()) ) {
				delete m_pResourceZip;
				m_pResourceZip = NULL;
			}
		}
	}

	static bool ReadFileToBuffer(LPCTSTR pstrPath, BYTE** ppData, DWORD* pdwSize)
	{
		if( ppData == NULL || pdwSize == NULL ) return false;
		*ppData = NULL;
		*pdwSize = 0;
		if( pstrPath == NULL || *pstrPath == _T('\0') ) return false;

		HANDLE hFile = ::CreateFile(pstrPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL);
		if( hFile == INVALID_HANDLE_VALUE ) return false;
		DWORD dwSize = ::GetFileSize(hFile, NULL);
		if( dwSize == 0 || dwSize == INVALID_FILE_SIZE ) {
			::CloseHandle(hFile);
			return false;
		}
		BYTE* pData = new BYTE[dwSize];
		DWORD dwRead = 0;
		BOOL bOk = ::ReadFile(hFile, pData, dwSize, &dwRead, NULL);
		::CloseHandle(hFile);
		if( !bOk || dwRead != dwSize ) {
			delete[] pData;
			return false;
		}
		*ppData = pData;
		*pdwSize = dwSize;
		return true;
	}

	bool CPaintManagerUI::LoadResourceData(LPCTSTR pstrRelativePath, BYTE** ppData, DWORD* pdwSize)
	{
		if( ppData == NULL || pdwSize == NULL ) return false;
		*ppData = NULL;
		*pdwSize = 0;
		if( pstrRelativePath == NULL || *pstrRelativePath == _T('\0') ) return false;

		// 1) ResourcePath（skin 目录优先：开发热更新 / 覆盖嵌入包）
		{
			CDuiString sFile = GetResourcePath();
			if( !sFile.IsEmpty() ) {
				sFile += pstrRelativePath;
				if( ReadFileToBuffer(sFile.GetData(), ppData, pdwSize) )
					return true;
			}
		}

		// 2) ZIP / ZIPRESOURCE（membuffer）— 磁盘没有时用打包资源
		if( !GetResourceZip().IsEmpty() ) {
			CZipFile* pZip = NULL;
			CZipFile localZip;
			bool bOwned = false;
			if( IsCachedResourceZip() ) {
				pZip = m_pResourceZip;
			}
			else {
				CDuiString sZip = GetResourcePath();
				sZip += GetResourceZip();
				localZip.SetPassword(GetResourceZipPwd().GetData());
				if( localZip.Open(sZip.GetData()) ) {
					pZip = &localZip;
					bOwned = true;
				}
			}
			if( pZip != NULL ) {
				CDuiString key = pstrRelativePath;
				key.Replace(_T("\\"), _T("/"));
				BYTE* pData = NULL;
				DWORD dwSize = 0;
				if( pZip->ExtractMemory(key.GetData(), &pData, &dwSize) && pData != NULL && dwSize > 0 ) {
					*ppData = pData;
					*pdwSize = dwSize;
					return true;
				}
				delete[] pData;
				(void)bOwned;
			}
		}

		// 3) 绝对路径 / 原样路径
		return ReadFileToBuffer(pstrRelativePath, ppData, pdwSize);
	}

	void CPaintManagerUI::SetResourceType(int nType)
	{
		m_nResType = nType;
	}

	int CPaintManagerUI::GetResourceType()
	{
		return m_nResType;
	}

	bool CPaintManagerUI::GetHSL(short* H, short* S, short* L)
	{
		*H = m_H;
		*S = m_S;
		*L = m_L;
		return m_bUseHSL;
	}

	void CPaintManagerUI::SetHSL(bool bUseHSL, short H, short S, short L)
	{
		if( m_bUseHSL || m_bUseHSL != bUseHSL ) {
			m_bUseHSL = bUseHSL;
			if( H == m_H && S == m_S && L == m_L ) return;
			m_H = CLAMP(H, 0, 360);
			m_S = CLAMP(S, 0, 200);
			m_L = CLAMP(L, 0, 200);
			AdjustSharedImagesHSL();
			for( int i = 0; i < m_aPreMessages.GetSize(); i++ ) {
				CPaintManagerUI* pManager = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);
				if( pManager != NULL ) pManager->AdjustImagesHSL();
			}
		}
	}

	void CPaintManagerUI::ReloadSkin()
	{
		ReloadSharedImages();
		for( int i = 0; i < m_aPreMessages.GetSize(); i++ ) {
			CPaintManagerUI* pManager = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);
			pManager->ReloadImages();
		}
	}

	CPaintManagerUI* CPaintManagerUI::GetPaintManager(LPCTSTR pstrName)
	{
		if( pstrName == NULL ) return NULL;
		CDuiString sName = pstrName;
		if( sName.IsEmpty() ) return NULL;
		for( int i = 0; i < m_aPreMessages.GetSize(); i++ ) {
			CPaintManagerUI* pManager = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);
			if( pManager != NULL && sName == pManager->GetName() ) return pManager;
		}
		return NULL;
	}

	CStdPtrArray* CPaintManagerUI::GetPaintManagers()
	{
		return &m_aPreMessages;
	}

	bool CPaintManagerUI::LoadPlugin(LPCTSTR pstrModuleName)
	{
		ASSERT( !::IsBadStringPtr(pstrModuleName,-1) || pstrModuleName == NULL );
		if( pstrModuleName == NULL ) return false;
		HMODULE hModule = ::LoadLibrary(pstrModuleName);
		if( hModule != NULL ) {
			LPCREATECONTROL lpCreateControl = (LPCREATECONTROL)::GetProcAddress(hModule, "CreateControl");
			if( lpCreateControl != NULL ) {
				LPVOID pFn = reinterpret_cast<LPVOID>(reinterpret_cast<uintptr_t>(lpCreateControl));
				if( m_aPlugins.Find(pFn) >= 0 ) return true;
				m_aPlugins.Add(pFn);
				return true;
			}
		}
		return false;
	}

	CStdPtrArray* CPaintManagerUI::GetPlugins()
	{
		return &m_aPlugins;
	}

	HWND CPaintManagerUI::GetPaintWindow() const
	{
		return m_hWndPaint;
	}

	HWND CPaintManagerUI::GetTooltipWindow() const
	{
		return m_hwndTipPopup;
	}

	int CPaintManagerUI::GetHoverTime() const
	{
		return m_iHoverTime;
	}

	void CPaintManagerUI::SetHoverTime(int iTime)
	{
		m_iHoverTime = iTime;
	}

	DWORD CPaintManagerUI::GetToolTipDelay() const
	{
		return (DWORD)((m_iHoverTime > 0) ? m_iHoverTime : 400);
	}

	LRESULT CALLBACK CPaintManagerUI::TipPopupWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// 穿透命中，避免 tip 盖住下方图标后挡点击（活动栏等纵向图标尤甚）
		if( uMsg == WM_NCHITTEST )
			return HTTRANSPARENT;

		CPaintManagerUI* pPm = reinterpret_cast<CPaintManagerUI*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if( uMsg == WM_PAINT ) {
			PAINTSTRUCT ps = { 0 };
			HDC hdc = ::BeginPaint(hWnd, &ps);
			RECT rc = { 0 };
			::GetClientRect(hWnd, &rc);
			if( pPm != NULL ) {
				DWORD dwBk = pPm->GetWindowBackgroundColor();
				DWORD dwText = pPm->GetDefaultFontColor();
				CRenderEngine::DrawColor(hdc, rc, dwBk);
				if( !pPm->m_sTipPopupText.IsEmpty() ) {
					RECT rcText = rc;
					rcText.left += kTipPadX;
					rcText.top += kTipPadY;
					rcText.right -= kTipPadX;
					rcText.bottom -= kTipPadY;
					CRenderEngine::GdiplusDrawText(hdc, pPm, rcText, pPm->m_sTipPopupText.GetData(),
						dwText, 0, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX);
				}
			}
			::EndPaint(hWnd, &ps);
			return 0;
		}
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	void CPaintManagerUI::EnsureTipPopup()
	{
		if( m_hwndTipPopup != NULL || m_hWndPaint == NULL ) return;
		if( !TipPopupEnsureClass(m_hInstance) ) return;
		m_hwndTipPopup = ::CreateWindowEx(
			WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
			kTipPopupClass, NULL,
			WS_POPUP | WS_BORDER,
			0, 0, 0, 0,
			m_hWndPaint, NULL, m_hInstance, NULL);
		if( m_hwndTipPopup == NULL ) return;
		::SetWindowLongPtr(m_hwndTipPopup, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	}

	void CPaintManagerUI::KillTipQueueTimer()
	{
		if( m_hTipQueueTimer != NULL ) {
			// 取消待发 tip 时用 NULL：勿 INVALID_HANDLE_VALUE 阻塞 UI 线程（拖动/移入按钮时会卡）
			::DeleteTimerQueueTimer(NULL, m_hTipQueueTimer, NULL);
			m_hTipQueueTimer = NULL;
		}
	}

	void CPaintManagerUI::ScheduleControlToolTip(CControlUI* pHover)
	{
		if( pHover == NULL || m_hWndPaint == NULL || pHover->GetToolTip().IsEmpty() ) return;
		KillTipQueueTimer();
		if( m_pTipShown != NULL && m_pTipShown != pHover )
			HideControlToolTip();
		m_pTipPending = pHover;
		HANDLE hTimer = NULL;
		DWORD dwDelay = GetToolTipDelay();
		if( ::CreateTimerQueueTimer(&hTimer, NULL, ToolTipQueueTimerProc,
			reinterpret_cast<PVOID>(m_hWndPaint), dwDelay, 0, WT_EXECUTEONLYONCE) ) {
			m_hTipQueueTimer = hTimer;
		}
	}

	void CPaintManagerUI::ShowControlToolTip(CControlUI* pHover)
	{
		if( pHover == NULL || m_hWndPaint == NULL || pHover != m_pEventHover ) return;
		m_sTipPopupText = pHover->GetToolTip();
		if( m_sTipPopupText.IsEmpty() ) return;

		EnsureTipPopup();
		if( m_hwndTipPopup == NULL ) return;

		SIZE szTip = MeasureTipPopupWndSize(this, m_sTipPopupText.GetData(), pHover->GetToolTipWidth());

		RECT rcCtrl = pHover->GetPos();
		RECT rcScr = rcCtrl;
		::MapWindowPoints(m_hWndPaint, NULL, reinterpret_cast<LPPOINT>(&rcScr), 2);

		// 优先控件右侧（侧栏图标不挡下一枚）；右侧不够再落到下方居中
		POINT pt = { rcScr.right + 6, (rcScr.top + rcScr.bottom - szTip.cy) / 2 };
		HMONITOR hMon = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		if( hMon != NULL && ::GetMonitorInfo(hMon, &mi) ) {
			if( pt.x + szTip.cx > mi.rcWork.right - 4 ) {
				pt.x = (rcScr.left + rcScr.right - szTip.cx) / 2;
				pt.y = rcScr.bottom + 4;
			}
		}
		ClampTipPopupPos(pt, szTip);

		::SetWindowPos(m_hwndTipPopup, HWND_TOPMOST, pt.x, pt.y, szTip.cx, szTip.cy,
			SWP_NOACTIVATE | SWP_SHOWWINDOW);
		::InvalidateRect(m_hwndTipPopup, NULL, TRUE);
		m_pTipShown = pHover;
		m_pTipPending = NULL;
		KillTipQueueTimer();
	}

	void CPaintManagerUI::HideControlToolTip()
	{
		KillTipQueueTimer();
		if( m_hwndTipPopup != NULL && ::IsWindow(m_hwndTipPopup) )
			::ShowWindow(m_hwndTipPopup, SW_HIDE);
		m_pTipShown = NULL;
		m_pTipPending = NULL;
	}

	void CPaintManagerUI::SyncToolTipWithHover(CControlUI* pHover)
	{
		if( m_pTipShown != NULL && pHover != m_pTipShown )
			HideControlToolTip();
		else if( m_pTipPending != NULL && pHover != m_pTipPending ) {
			KillTipQueueTimer();
			m_pTipPending = NULL;
		}
	}

	LRESULT CPaintManagerUI::HitTestCaptionDrag(bool bWouldDragCaption)
	{
		// tooltip 已不依赖客户区 WM_MOUSEHOVER；拖窗区可始终 HTCAPTION，由系统原生处理拖动。
		return bWouldDragCaption ? HTCAPTION : HTCLIENT;
	}

	void CPaintManagerUI::ArmMouseHoverTrack(bool bCancelFirst, POINT pt)
	{
		if( m_hWndPaint == NULL ) return;
		if( pt.x == -1 && pt.y == -1 ) return;
		if( bCancelFirst ) {
			TRACKMOUSEEVENT cancel = { sizeof(cancel), TME_CANCEL, m_hWndPaint, 0 };
			::TrackMouseEvent(&cancel);
			m_bMouseTracking = false;
		}
		if( m_bMouseTracking ) return;
		TRACKMOUSEEVENT tme = { sizeof(tme), TME_HOVER | TME_LEAVE, m_hWndPaint, GetToolTipDelay() };
		if( ::TrackMouseEvent(&tme) )
			m_bMouseTracking = true;
	}

	LPCTSTR CPaintManagerUI::GetName() const
	{
		return m_sName.GetData();
	}

	HDC CPaintManagerUI::GetPaintDC() const
	{
		return m_hDcPaint;
	}

	IRenderContext* CPaintManagerUI::GetRenderContext() const
	{
		return m_pRenderContext;
	}

	void CPaintManagerUI::SetRenderContext(IRenderContext* pRenderContext)
	{
		m_pRenderContext = pRenderContext;
	}

	POINT CPaintManagerUI::GetMousePos() const
	{
		return m_ptLastMousePos;
	}

	SIZE CPaintManagerUI::GetClientSize() const
	{
		RECT rcClient = { 0 };
		::GetClientRect(m_hWndPaint, &rcClient);
		return CDuiSize(rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
	}

	SIZE CPaintManagerUI::GetInitSize()
	{
		return m_szInitWindowSize;
	}

	void CPaintManagerUI::SetInitSize(int cx, int cy)
	{
		m_szInitWindowSize.cx = cx;
		m_szInitWindowSize.cy = cy;
		if( m_pRoot == NULL && m_hWndPaint != NULL ) {
			::SetWindowPos(m_hWndPaint, NULL, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		}
	}

	RECT CPaintManagerUI::GetSizeBox()
	{
		return GetDPIObj()->Scale(m_rcSizeBox);
	}

	void CPaintManagerUI::SetSizeBox(RECT& rcSizeBox)
	{
		m_rcSizeBox = rcSizeBox;
	}

	RECT CPaintManagerUI::GetCaptionRect()
	{
		return GetDPIObj()->Scale(m_rcCaption);
	}

	void CPaintManagerUI::SetCaptionRect(RECT& rcCaption)
	{
		m_rcCaption = rcCaption;
	}

	SIZE CPaintManagerUI::GetBorderRadius()
	{
		return GetDPIObj()->Scale(m_szBorderRadius);
	}

	void CPaintManagerUI::SetBorderRadius(int cx, int cy)
	{
		m_szBorderRadius.cx = cx;
		m_szBorderRadius.cy = cy;
	}

	SIZE CPaintManagerUI::GetMinSize()
	{
		return GetDPIObj()->Scale(m_szMinWindow);
	}

	void CPaintManagerUI::SetMinSize(int cx, int cy)
	{
		ASSERT(cx>=0 && cy>=0);
		m_szMinWindow.cx = cx;
		m_szMinWindow.cy = cy;
	}

	SIZE CPaintManagerUI::GetMaxSize()
	{
		return GetDPIObj()->Scale(m_szMaxWindow);
	}

	void CPaintManagerUI::SetMaxSize(int cx, int cy)
	{
		ASSERT(cx>=0 && cy>=0);
		m_szMaxWindow.cx = cx;
		m_szMaxWindow.cy = cy;
	}

	bool CPaintManagerUI::IsShowUpdateRect()
	{
		return m_bShowUpdateRect;
	}

	void CPaintManagerUI::SetShowUpdateRect(bool show)
	{
		m_bShowUpdateRect = show;
	}

	bool CPaintManagerUI::IsNoActivate()
	{
		return m_bNoActivate;
	}

	void CPaintManagerUI::SetNoActivate(bool bNoActivate)
	{
		m_bNoActivate = bNoActivate;
	}

	BYTE CPaintManagerUI::GetOpacity() const
	{
		return m_nOpacity;
	}

	void CPaintManagerUI::SetOpacity(BYTE nOpacity)
	{
		m_nOpacity = nOpacity;
		if( m_hWndPaint != NULL ) {
			typedef BOOL (__stdcall *PFUNCSETLAYEREDWINDOWATTR)(HWND, COLORREF, BYTE, DWORD);
			PFUNCSETLAYEREDWINDOWATTR fSetLayeredWindowAttributes = NULL;

			HMODULE hUser32 = ::GetModuleHandle(_T("User32.dll"));
			if (hUser32)
			{
				fSetLayeredWindowAttributes = 
					(PFUNCSETLAYEREDWINDOWATTR)::GetProcAddress(hUser32, "SetLayeredWindowAttributes");
				if( fSetLayeredWindowAttributes == NULL ) return;
			}

			DWORD dwStyle = ::GetWindowLong(m_hWndPaint, GWL_EXSTYLE);
			DWORD dwNewStyle = dwStyle;
			dwNewStyle |= WS_EX_LAYERED;
			if(dwStyle != dwNewStyle) ::SetWindowLong(m_hWndPaint, GWL_EXSTYLE, dwNewStyle);
			fSetLayeredWindowAttributes(m_hWndPaint, 0, nOpacity, LWA_ALPHA);
		}
	}

	BYTE CPaintManagerUI::GetWallpaperBleed() const
	{
		return m_nWallpaperBleed;
	}

	void CPaintManagerUI::SetWallpaperBleed(BYTE nBleed)
	{
		if( m_nWallpaperBleed == nBleed ) return;
		m_nWallpaperBleed = nBleed;
		NeedUpdate();
	}

	bool CPaintManagerUI::IsWallpaperBleedNeedImage() const
	{
		return m_bWallpaperBleedNeedImage;
	}

	void CPaintManagerUI::SetWallpaperBleedNeedImage(bool bNeed)
	{
		if( m_bWallpaperBleedNeedImage == bNeed ) return;
		m_bWallpaperBleedNeedImage = bNeed;
		NeedUpdate();
	}

	bool CPaintManagerUI::IsWallpaperBleedActive() const
	{
		if( m_nWallpaperBleed >= 255 ) return false;
		if( !m_bWallpaperBleedNeedImage ) return true;
		if( m_pRoot == NULL ) return false;
		LPCTSTR pImg = m_pRoot->GetBackgroundImage();
		return pImg != NULL && *pImg != _T('\0');
	}

	DWORD CPaintManagerUI::GetWindowBackgroundColor() const
	{
		return m_dwWindowBackgroundColor;
	}

	bool CPaintManagerUI::IsWindowBackgroundColorCustom() const
	{
		return m_bWindowBackgroundColorCustom;
	}

	void CPaintManagerUI::SetWindowBackgroundColor(DWORD dwColor)
	{
		m_dwWindowBackgroundColor = dwColor;
		m_bWindowBackgroundColorCustom = true;
		// Toast 等 kind 根在 Attach 前已 SetKind；主题 ApplyToManager 勿盖掉，
		// 否则 kind 前景字会打在 color-bg 上（常见白字白底 / 黑字深底）。
		if( m_pRoot != NULL && m_pRoot->GetKind() == CONTROLKIND_NONE )
			m_pRoot->SetBackgroundColor(dwColor);
		if( m_hWndPaint != NULL ) Invalidate();
	}

	void CPaintManagerUI::ApplyDefaultWindowBackgroundColor()
	{
		if( m_pRoot == NULL ) return;
		if( m_bLayered && !m_bWindowBackgroundColorCustom ) return;
		if( m_dwWindowBackgroundColor == 0 ) return;
		if( m_pRoot->GetBackgroundColor() != 0 ) return;
		m_pRoot->SetBackgroundColor(m_dwWindowBackgroundColor);
	}

	LPCTSTR CPaintManagerUI::GetWindowBackgroundImage() const
	{
		return m_sWindowBackgroundImage.GetData();
	}

	bool CPaintManagerUI::IsWindowBackgroundImageCustom() const
	{
		return m_bWindowBackgroundImageCustom;
	}

	void CPaintManagerUI::SetWindowBackgroundImage(LPCTSTR pStrImage)
	{
		CDuiString sUrl;
		if( pStrImage != NULL && ParseCssUrlImage(pStrImage, sUrl) )
			pStrImage = sUrl.GetData();
		CDuiString s = (pStrImage != NULL) ? pStrImage : _T("");
		if( m_bWindowBackgroundImageCustom && m_sWindowBackgroundImage == s ) return;
		// 换文件路径时丢掉内存图缓存；设内存图 key 时由 Install 先 Clear
		if( s != _T("_dui_window_background_memory") )
			RemoveImage(_T("_dui_window_background_memory"), false);
		m_sWindowBackgroundImage = s;
		m_bWindowBackgroundImageCustom = true;
		if( m_pRoot != NULL )
			m_pRoot->SetBackgroundImage(m_sWindowBackgroundImage.GetData());
		if( m_hWndPaint != NULL ) Invalidate();
	}

	void CPaintManagerUI::ApplyDefaultWindowBackgroundImage()
	{
		if( m_pRoot == NULL ) return;
		if( !m_bWindowBackgroundImageCustom ) return;
		if( m_sWindowBackgroundImage.IsEmpty() ) return;
		LPCTSTR pExist = m_pRoot->GetBackgroundImage();
		if( pExist != NULL && *pExist != _T('\0') ) return;
		m_pRoot->SetBackgroundImage(m_sWindowBackgroundImage.GetData());
	}

	namespace {
		static LPCTSTR kWindowBgMemKey = _T("_dui_window_background_memory");

		static bool LooksLikeSvgMemory(const BYTE* pData, DWORD dwSize)
		{
			if( pData == NULL || dwSize < 4 ) return false;
			DWORD i = 0;
			if( dwSize >= 3 && pData[0] == 0xEF && pData[1] == 0xBB && pData[2] == 0xBF ) i = 3;
			while( i < dwSize && (pData[i] == ' ' || pData[i] == '\t' || pData[i] == '\r' || pData[i] == '\n') )
				++i;
			if( i >= dwSize ) return false;
			if( pData[i] != '<' ) return false;
			if( i + 4 <= dwSize && _strnicmp((const char*)pData + i, "<svg", 4) == 0 ) return true;
			if( i + 5 <= dwSize && _strnicmp((const char*)pData + i, "<?xml", 5) == 0 ) return true;
			return false;
		}
	}

	void CPaintManagerUI::ClearWindowBackgroundMemoryImage()
	{
		RemoveImage(kWindowBgMemKey, false);
	}

	bool CPaintManagerUI::InstallWindowBackgroundHBitmap(HBITMAP hBmp, int w, int h, bool bAlpha)
	{
		if( hBmp == NULL || w <= 0 || h <= 0 ) {
			if( hBmp ) ::DeleteObject(hBmp);
			return false;
		}
		ClearWindowBackgroundMemoryImage();
		const TImageInfo* pInfo = AddImage(kWindowBgMemKey, hBmp, w, h, bAlpha, false);
		if( pInfo == NULL ) {
			::DeleteObject(hBmp);
			return false;
		}
		SetWindowBackgroundImage(kWindowBgMemKey);
		return true;
	}

	bool CPaintManagerUI::SetWindowBackgroundImageFromMemory(const BYTE* pData, DWORD dwSize, DWORD mask)
	{
		if( pData == NULL || dwSize == 0 ) return false;

		if( LooksLikeSvgMemory(pData, dwSize) )
			return SetWindowBackgroundImageFromSvg((const char*)pData, (size_t)dwSize, 0, 0, 0);

		TImageInfo* pInfo = CRenderEngine::LoadImageFromMemory(pData, dwSize, mask);
		if( pInfo == NULL || pInfo->hBitmap == NULL ) {
			if( pInfo ) CRenderEngine::FreeImage(pInfo);
			return false;
		}
		HBITMAP hBmp = pInfo->hBitmap;
		int w = pInfo->nX;
		int h = pInfo->nY;
		bool bA = pInfo->bAlpha;
		pInfo->hBitmap = NULL;
		CRenderEngine::FreeImage(pInfo);
		return InstallWindowBackgroundHBitmap(hBmp, w, h, bA);
	}

	bool CPaintManagerUI::SetWindowBackgroundImageFromSvg(const char* utf8Svg, size_t nBytes,
		int width, int height, DWORD dwTintColor)
	{
		int w = 0, h = 0;
		HBITMAP hBmp = CSvgBoxUI::RasterizeToHBitmap(utf8Svg, nBytes, width, height, dwTintColor, &w, &h);
		if( hBmp == NULL ) return false;
		return InstallWindowBackgroundHBitmap(hBmp, w, h, true);
	}

	bool CPaintManagerUI::SetWindowBackgroundImageFromSvg(LPCTSTR pstrSvg,
		int width, int height, DWORD dwTintColor)
	{
		int w = 0, h = 0;
		HBITMAP hBmp = CSvgBoxUI::RasterizeToHBitmap(pstrSvg, width, height, dwTintColor, &w, &h);
		if( hBmp == NULL ) return false;
		return InstallWindowBackgroundHBitmap(hBmp, w, h, true);
	}

	UIAction CPaintManagerUI::GetWindowAction() const
	{
		return m_windowAction;
	}

	void CPaintManagerUI::SetWindowAction(UIAction action)
	{
		m_windowAction = action;
		m_bWindowActionFromShape = false;
		ApplyDefaultWindowAction();
	}

	void CPaintManagerUI::ApplyDefaultWindowAction()
	{
		if( m_pRoot == NULL ) return;
		if( m_windowAction == UIACTION_NONE ) return;
		// body 内联/CSS 已设 action 时不覆盖
		if( m_pRoot->GetAction() != UIACTION_NONE ) return;
		m_pRoot->SetAction(m_windowAction);
	}

	LPCTSTR CPaintManagerUI::GetWindowTheme() const
	{
		return m_sWindowTheme.GetData();
	}

	void CPaintManagerUI::SetWindowTheme(LPCTSTR pstrTheme)
	{
		m_sWindowTheme = (pstrTheme != NULL) ? pstrTheme : _T("");
		ApplyDefaultWindowTheme();
	}

	LPCTSTR CPaintManagerUI::GetWindowThemeId() const
	{
		return m_sWindowThemeId.GetData();
	}

	void CPaintManagerUI::SetWindowThemeId(LPCTSTR pstrThemeId)
	{
		m_sWindowThemeId = (pstrThemeId != NULL) ? pstrThemeId : _T("");
		ApplyDefaultWindowTheme();
	}

	void CPaintManagerUI::ApplyDefaultWindowTheme()
	{
		if( m_pRoot == NULL ) return;
		// root 已有内联 theme / theme-id 时不覆盖（与 action 一致）
		if( !m_sWindowTheme.IsEmpty() ) {
			LPCTSTR t = m_pRoot->GetCustomAttribute(_T("theme"));
			if( t == NULL || *t == _T('\0') )
				m_pRoot->SetAttribute(_T("theme"), m_sWindowTheme.GetData());
		}
		if( !m_sWindowThemeId.IsEmpty() ) {
			LPCTSTR tid = m_pRoot->GetCustomAttribute(_T("theme-id"));
			if( tid == NULL || *tid == _T('\0') )
				m_pRoot->SetAttribute(_T("theme-id"), m_sWindowThemeId.GetData());
		}
	}

	bool CPaintManagerUI::IsLayered()
	{
		return m_bLayered;
	}

	void CPaintManagerUI::SetLayered(bool bLayered)
	{
		if( m_hWndPaint != NULL && bLayered != m_bLayered ) {
			UINT uStyle = GetWindowStyle(m_hWndPaint);
			if( (uStyle & WS_CHILD) != 0 ) return;
			if( g_fUpdateLayeredWindow == NULL ) {
				HMODULE hUser32 = ::GetModuleHandle(_T("User32.dll"));
				if (hUser32) {
					g_fUpdateLayeredWindow = 
						(PFUNCUPDATELAYEREDWINDOW)::GetProcAddress(hUser32, "UpdateLayeredWindow");
					if( g_fUpdateLayeredWindow == NULL ) return;
				}
			}
			m_bLayered = bLayered;
			if( m_pRoot != NULL ) m_pRoot->NeedUpdate();
			Invalidate();
		}
	}

	void CPaintManagerUI::SetLayeredCompositionEnabled(bool bEnable)
	{
		m_bLayeredCompositionEnabled = bEnable;
		if( m_pOffscreenSurface != NULL )
			m_pOffscreenSurface->SetLayeredCompositionEnabled(bEnable);
		if( m_hWndPaint != NULL ) Invalidate();
	}

	bool CPaintManagerUI::IsLayeredCompositionEnabled() const
	{
		return m_bLayeredCompositionEnabled;
	}

	RECT& CPaintManagerUI::GetLayeredPadding()
	{
		return m_rcLayeredPadding;
	}

	void CPaintManagerUI::SetLayeredPadding(RECT& rcLayeredPadding)
	{
		m_rcLayeredPadding = rcLayeredPadding;
		m_bLayeredChanged = true;
		Invalidate();
	}

	BYTE CPaintManagerUI::GetLayeredOpacity()
	{
		return m_nOpacity;
	}

	void CPaintManagerUI::SetLayeredOpacity(BYTE nOpacity)
	{
		m_nOpacity = nOpacity;
		m_bLayeredChanged = true;
		Invalidate();
	}

	LPCTSTR CPaintManagerUI::GetLayeredImage()
	{
		return m_diLayered.sDrawString.GetData();
	}

	void CPaintManagerUI::SetLayeredImage(LPCTSTR pstrImage)
	{
		m_diLayered.sDrawString = pstrImage;
		RECT rcNull = {0};
		CRenderEngine::DrawImageInfo(NULL, this, rcNull, rcNull, &m_diLayered);
		m_bLayeredChanged = true;
		Invalidate();
	}

	void CPaintManagerUI::SetShapeImage(LPCTSTR pstrImage)
	{
		m_sShapeImage = pstrImage ? pstrImage : _T("");
		if( !m_sShapeImage.IsEmpty() && m_bShapeDragEnabled && m_windowAction == UIACTION_NONE ) {
			m_windowAction = UIACTION_MOVEWINDOW;
			m_bWindowActionFromShape = true;
			ApplyDefaultWindowAction();
		}
	}

	LPCTSTR CPaintManagerUI::GetShapeImage() const
	{
		return m_sShapeImage.GetData();
	}

	void CPaintManagerUI::SetShapeMask(LPCTSTR pstrMask)
	{
		m_sShapeMask = pstrMask ? pstrMask : _T("");
	}

	LPCTSTR CPaintManagerUI::GetShapeMask() const
	{
		return m_sShapeMask.GetData();
	}

	LPCTSTR CPaintManagerUI::GetShapeHitImage() const
	{
		if( !m_sShapeMask.IsEmpty() ) return m_sShapeMask.GetData();
		return m_sShapeImage.GetData();
	}

	namespace {
		const TCHAR kShapeImageMemKey[] = _T("__dui_shape_image_mem");
		const TCHAR kShapeMaskMemKey[] = _T("__dui_shape_mask_mem");

		bool InstallShapeMemImage(CPaintManagerUI* pm, LPCTSTR key, const BYTE* pData, DWORD dwSize, DWORD mask)
		{
			if( pm == NULL || pData == NULL || dwSize == 0 || key == NULL ) return false;
			TImageInfo* pInfo = CRenderEngine::LoadImageFromMemory(pData, dwSize, mask);
			if( pInfo == NULL || pInfo->hBitmap == NULL ) {
				if( pInfo ) CRenderEngine::FreeImage(pInfo);
				return false;
			}
			const int w = pInfo->nX;
			const int h = pInfo->nY;
			const bool bA = pInfo->bAlpha;
			HBITMAP hBmp = pInfo->hBitmap;
			pInfo->hBitmap = NULL;
			CRenderEngine::FreeImage(pInfo);
			pm->RemoveImage(key, false);
			return pm->AddImage(key, hBmp, w, h, bA, false) != NULL;
		}
	}

	bool CPaintManagerUI::SetShapeImageFromMemory(const BYTE* pData, DWORD dwSize, DWORD mask)
	{
		if( !InstallShapeMemImage(this, kShapeImageMemKey, pData, dwSize, mask) )
			return false;
		SetShapeImage(kShapeImageMemKey);
		return true;
	}

	bool CPaintManagerUI::SetShapeMaskFromMemory(const BYTE* pData, DWORD dwSize, DWORD mask)
	{
		if( !InstallShapeMemImage(this, kShapeMaskMemKey, pData, dwSize, mask) )
			return false;
		SetShapeMask(kShapeMaskMemKey);
		return true;
	}

	void CPaintManagerUI::SetShapeAlphaThreshold(BYTE nThreshold)
	{
		m_nShapeAlphaThreshold = nThreshold;
	}

	BYTE CPaintManagerUI::GetShapeAlphaThreshold() const
	{
		return m_nShapeAlphaThreshold;
	}

	void CPaintManagerUI::SetShapeDragEnabled(bool bEnable)
	{
		m_bShapeDragEnabled = bEnable;
		if( !bEnable ) {
			if( m_bWindowActionFromShape && m_windowAction == UIACTION_MOVEWINDOW ) {
				m_windowAction = UIACTION_NONE;
				m_bWindowActionFromShape = false;
				if( m_pRoot != NULL && m_pRoot->GetAction() == UIACTION_MOVEWINDOW )
					m_pRoot->SetAction(UIACTION_NONE);
			}
			return;
		}
		if( !m_sShapeImage.IsEmpty() && m_windowAction == UIACTION_NONE ) {
			m_windowAction = UIACTION_MOVEWINDOW;
			m_bWindowActionFromShape = true;
			ApplyDefaultWindowAction();
		}
	}

	bool CPaintManagerUI::IsShapeDragEnabled() const
	{
		return m_bShapeDragEnabled;
	}

	bool CPaintManagerUI::CalcShapeWindowClientSize(SIZE& szOut, bool clampWorkArea, int workAreaPercent) const
	{
		szOut.cx = szOut.cy = 0;
		LPCTSTR pImg = GetShapeImage();
		if( pImg == NULL || *pImg == _T('\0') ) pImg = GetShapeMask();
		if( pImg == NULL || *pImg == _T('\0') ) return false;
		const TImageInfo* pInfo = const_cast<CPaintManagerUI*>(this)->GetImageEx(pImg);
		if( pInfo == NULL || pInfo->nX < 1 || pInfo->nY < 1 ) return false;
		int w = pInfo->nX;
		int h = pInfo->nY;
		if( clampWorkArea ) {
			if( workAreaPercent < 1 ) workAreaPercent = 1;
			if( workAreaPercent > 100 ) workAreaPercent = 100;
			RECT rcWork = { 0, 0, 1280, 720 };
			::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);
			const int maxW = (rcWork.right - rcWork.left) * workAreaPercent / 100;
			const int maxH = (rcWork.bottom - rcWork.top) * workAreaPercent / 100;
			if( w > maxW || h > maxH ) {
				const double sx = (double)maxW / (double)w;
				const double sy = (double)maxH / (double)h;
				const double s = (sx < sy) ? sx : sy;
				w = (int)(w * s + 0.5);
				h = (int)(h * s + 0.5);
			}
		}
		if( w < 1 ) w = 1;
		if( h < 1 ) h = 1;
		szOut.cx = w;
		szOut.cy = h;
		return true;
	}

	bool CPaintManagerUI::FitToShapeImage(HWND hWnd, bool clampWorkArea, int workAreaPercent)
	{
		if( hWnd == NULL || !::IsWindow(hWnd) ) return false;
		SIZE sz = { 0, 0 };
		if( !CalcShapeWindowClientSize(sz, clampWorkArea, workAreaPercent) ) return false;
		RECT rc = { 0, 0, sz.cx, sz.cy };
		::AdjustWindowRectEx(&rc, (DWORD)::GetWindowLong(hWnd, GWL_STYLE), FALSE,
			(DWORD)::GetWindowLong(hWnd, GWL_EXSTYLE));
		const int ww = rc.right - rc.left;
		const int wh = rc.bottom - rc.top;
		::SetWindowPos(hWnd, NULL, 0, 0, ww, wh, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		Invalidate();
		return true;
	}

	bool CPaintManagerUI::ApplyWindowShapeRgn(HWND hWnd)
	{
		LPCTSTR pHit = GetShapeHitImage();
		if( hWnd == NULL || pHit == NULL || *pHit == _T('\0') ) return false;

		// 分层窗外形靠 Present 的每像素 alpha；再 SetWindowRgn 会硬切抗锯齿边
		if( m_bLayered ) {
			::SetWindowRgn(hWnd, NULL, TRUE);
			return true;
		}

		const TImageInfo* pInfo = GetImageEx(pHit);
		if( pInfo == NULL || pInfo->hBitmap == NULL ) return false;

		RECT rcWnd = { 0 };
		::GetWindowRect(hWnd, &rcWnd);
		const int w = rcWnd.right - rcWnd.left;
		const int h = rcWnd.bottom - rcWnd.top;
		if( w < 1 || h < 1 ) return false;

		HRGN hRgn = CreateRegionFromAlphaImage(pInfo, m_nShapeAlphaThreshold, w, h);
		if( hRgn == NULL ) return false;
		::SetWindowRgn(hWnd, hRgn, TRUE);
		::DeleteObject(hRgn);
		return true;
	}

	CShadowUI* CPaintManagerUI::GetShadow()
	{
		return &m_shadow;
	}

	void CPaintManagerUI::SetUseGdiplusText(bool bUse)
	{
		m_bUseGdiplusText = bUse;
	}

	bool CPaintManagerUI::IsUseGdiplusText() const
	{
		return m_bUseGdiplusText;
	}

	void CPaintManagerUI::SetGdiplusTextRenderingHint(int trh)
	{
		m_trh = trh;
	}

	int CPaintManagerUI::GetGdiplusTextRenderingHint() const
	{
		return m_trh;
	}

	bool CPaintManagerUI::PreMessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lRes)
	{
		for( int i = 0; i < m_aPreMessageFilters.GetSize(); i++ ) 
		{
			bool bHandled = false;
			LRESULT lResult = static_cast<IMessageFilterUI*>(m_aPreMessageFilters[i])->MessageHandler(uMsg, wParam, lParam, bHandled);
			if( bHandled ) {
				lRes = lResult;
				return true;
			}
		}
		switch( uMsg ) {
			case WM_KEYDOWN:
			{
				// Tabbing between controls
				if( wParam == VK_TAB ) {
					if( m_pFocus && m_pFocus->IsVisible() && m_pFocus->IsEnabled() && _tcsstr(m_pFocus->GetClass(), _T("RichEditUI")) != NULL ) {
						if( static_cast<CRichEditUI*>(m_pFocus)->IsWantTab() ) return false;
					}
					if( m_pFocus && m_pFocus->IsVisible() && m_pFocus->IsEnabled() && _tcsstr(m_pFocus->GetClass(), _T("WkeWebkitUI")) != NULL ) {
						return false;
					}
					// 必须吞掉：SetNextTabControl 会同步 KillFocus 原生 Edit，
					// 若仍 Dispatch 到 CEditWnd，此时 m_pOwner 已空 → 崩溃
					SetNextTabControl(::GetKeyState(VK_SHIFT) >= 0);
					return true;
				}
			}
			break;
		case WM_SYSCHAR:
			{
				// Handle ALT-shortcut key-combinations
				FINDSHORTCUT fs = { 0 };
				fs.ch = toupper((int)wParam);
				CControlUI* pControl = m_pRoot->FindControl(__FindControlFromShortcut, &fs, UIFIND_ENABLED | UIFIND_ME_FIRST | UIFIND_TOP_FIRST);
				if( pControl != NULL ) {
					pControl->SetFocus();
					pControl->Activate();
					return true;
				}
			}
			break;
		case WM_SYSKEYDOWN:
			{
				if( m_pFocus != NULL ) {
					TEventUI event = { 0 };
					event.Type = UIEVENT_SYSKEY;
					event.chKey = (TCHAR)wParam;
					event.ptMouse = m_ptLastMousePos;
					event.wKeyState = MapKeyState();
					event.dwTimestamp = ::GetTickCount();
					m_pFocus->Event(event);
				}
			}
			break;
		}
		return false;
	}

	bool CPaintManagerUI::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lRes)
	{
		if( m_hWndPaint == NULL ) return false;
		// Cycle through listeners
		for( int i = 0; i < m_aMessageFilters.GetSize(); i++ ) 
		{
			bool bHandled = false;
			LRESULT lResult = static_cast<IMessageFilterUI*>(m_aMessageFilters[i])->MessageHandler(uMsg, wParam, lParam, bHandled);
			if( bHandled ) {
				lRes = lResult;
				switch( uMsg ) {
				case WM_MOUSEMOVE:
				case WM_LBUTTONDOWN:
				case WM_LBUTTONDBLCLK:
				case WM_LBUTTONUP:
					{
						POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
						m_ptLastMousePos = pt;
					}
					break;
				case WM_CONTEXTMENU:
				case WM_MOUSEWHEEL:
					{
						POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
						::ScreenToClient(m_hWndPaint, &pt);
						m_ptLastMousePos = pt;
					}
					break;
				}
				return true;
			}
		}

		if( m_bLayered ) {
			switch( uMsg ) {
			case WM_NCACTIVATE:
				if( !::IsIconic(m_hWndPaint) ) {
					lRes = (wParam == 0) ? TRUE : FALSE;
					return true;
				}
				break;
			case WM_NCCALCSIZE:
			case WM_NCPAINT:
				lRes = 0;
				return true;
			}
		}
		// Custom handling of events
		switch( uMsg ) {
		case UIMSG_ASYNC_NOTIFY:
			{
				for( int i = 0; i < m_aDelayedCleanup.GetSize(); i++ ) 
					delete static_cast<CControlUI*>(m_aDelayedCleanup[i]);
				m_aDelayedCleanup.Empty();
				
				m_bAsyncNotifyPosted = false;

				TNotifyUI* pMsg = NULL;
				while( (pMsg = static_cast<TNotifyUI*>(m_aAsyncNotify.GetAt(0))) ) {
					m_aAsyncNotify.Remove(0);
					if( pMsg->pSender != NULL ) {
						if( pMsg->pSender->OnNotify ) pMsg->pSender->OnNotify(pMsg);
					}
					for( int j = 0; j < m_aNotifiers.GetSize(); j++ ) {
						static_cast<INotifyUI*>(m_aNotifiers[j])->Notify(*pMsg);
					}
					delete pMsg;
				}
			}
			break;
		case WM_CLOSE:
			{
				// Make sure all matching "closing" events are sent
				TEventUI event = { 0 };
				event.ptMouse = m_ptLastMousePos;
				event.wKeyState = MapKeyState();
				event.dwTimestamp = ::GetTickCount();
				if( m_pEventHover != NULL ) {
					event.Type = UIEVENT_MOUSELEAVE;
					event.pSender = m_pEventHover;
					m_pEventHover->Event(event);
				}
				if( m_pEventClick != NULL ) {
					event.Type = UIEVENT_BUTTONUP;
					event.pSender = m_pEventClick;
					m_pEventClick->Event(event);
				}
                if (m_pEventRClick != NULL)
                {
                    event.Type = UIEVENT_RBUTTONUP;
                    event.pSender = m_pEventClick;
                    m_pEventRClick->Event(event);
                }

				SetFocus(NULL);

				if( ::GetActiveWindow() == m_hWndPaint ) {
					HWND hwndParent = GetWindowOwner(m_hWndPaint);
					if( hwndParent != NULL ) ::SetFocus(hwndParent);
				}

				if (m_hwndTipPopup != NULL) {
					::DestroyWindow(m_hwndTipPopup);
					m_hwndTipPopup = NULL;
				}
			}
			break;
		case WM_ERASEBKGND:
			{
				// We'll do the painting here...
				lRes = 1;
			}
			return true;
		case WM_PAINT:
			{
				if( m_pRoot == NULL ) {
					PAINTSTRUCT ps = { 0 };
					::BeginPaint(m_hWndPaint, &ps);
					COwnedRenderContextScope renderScope(this, m_hDcPaint);
					renderScope.GetContext().DrawColor(ps.rcPaint, 0x000000FF);
					::EndPaint(m_hWndPaint, &ps);
					return true;
				}

				RECT rcClient = { 0 };
				::GetClientRect(m_hWndPaint, &rcClient);

				RECT rcPaint = { 0 };
				if( !::GetUpdateRect(m_hWndPaint, &rcPaint, FALSE) ) return true;

				//if( m_bLayered ) {
				//	m_bOffscreenPaint = true;
				//	rcPaint = m_rcLayeredUpdate;
				//	if( ::IsRectEmpty(&m_rcLayeredUpdate) ) {
				//		PAINTSTRUCT ps = { 0 };
				//		::BeginPaint(m_hWndPaint, &ps);
				//		::EndPaint(m_hWndPaint, &ps);
				//		return true;
				//	}
				//	if( rcPaint.right > rcClient.right ) rcPaint.right = rcClient.right;
				//	if( rcPaint.bottom > rcClient.bottom ) rcPaint.bottom = rcClient.bottom;
				//	::ZeroMemory(&m_rcLayeredUpdate, sizeof(m_rcLayeredUpdate));
				//}
				//else {
				//	if( !::GetUpdateRect(m_hWndPaint, &rcPaint, FALSE) ) return true;
				//}

				// Set focus to first control?
				if( m_bFocusNeeded ) {
					SetNextTabControl();
				}

				SetPainting(true);

				bool bNeedSizeMsg = false;
				DWORD dwWidth = rcClient.right - rcClient.left;
				DWORD dwHeight = rcClient.bottom - rcClient.top;

				SetPainting(true);
				if( m_bUpdateNeeded ) {
					m_bUpdateNeeded = false;
					if( !::IsRectEmpty(&rcClient) && !::IsIconic(m_hWndPaint) ) {
						if( m_pRoot->IsUpdateNeeded() ) {
							RECT rcRoot = rcClient;
							// 尺寸变化交给下方 Ensure() 扩缩；勿每帧 DestroySurface，
							// 否则拖窗时整缓冲重建 → 标题栏等 chrome 闪烁严重。
							if( m_bLayered ) {
								rcRoot.left += m_rcLayeredPadding.left;
								rcRoot.top += m_rcLayeredPadding.top;
								rcRoot.right -= m_rcLayeredPadding.right;
								rcRoot.bottom -= m_rcLayeredPadding.bottom;
							}
							// 根节点 margin/padding：相对窗口缩进，露出 html(窗口) 背景
							{
								RECT rcPad = m_pRoot->GetMargin();
								rcRoot.left += rcPad.left;
								rcRoot.top += rcPad.top;
								rcRoot.right -= rcPad.right;
								rcRoot.bottom -= rcPad.bottom;
								if( rcRoot.right < rcRoot.left ) rcRoot.right = rcRoot.left;
								if( rcRoot.bottom < rcRoot.top ) rcRoot.bottom = rcRoot.top;
							}
							m_pRoot->SetPos(rcRoot, true);
							bNeedSizeMsg = true;
						}
						else {
							CControlUI* pControl = NULL;
							m_aFoundControls.Empty();
							m_pRoot->FindControl(__FindControlsFromUpdate, NULL, UIFIND_VISIBLE | UIFIND_ME_FIRST | UIFIND_UPDATETEST);
							for( int it = 0; it < m_aFoundControls.GetSize(); it++ ) {
								pControl = static_cast<CControlUI*>(m_aFoundControls[it]);
								pControl->SetPos(pControl->GetPos(), true);
							}
							bNeedSizeMsg = true;
						}
						// We'll want to notify the window when it is first initialized
						// with the correct layout. The window form would take the time
						// to submit swipes/animations.
						if( m_bFirstLayout ) {
							m_bFirstLayout = false;
							SendNotify(m_pRoot, DUI_MSGTYPE_WINDOWINIT,  0, 0, false);
							if( m_bLayered && m_bLayeredChanged ) {
								Invalidate();
								SetPainting(false);
								return true;
							}
							// ??????????????
							m_shadow.Update(m_hWndPaint);
						}
					}
				}
				else if( m_bLayered && m_bLayeredChanged ) {
					RECT rcRoot = rcClient;
					if( m_pOffscreenSurface != NULL )
						m_pOffscreenSurface->ClearAll();
					rcRoot.left += m_rcLayeredPadding.left;
					rcRoot.top += m_rcLayeredPadding.top;
					rcRoot.right -= m_rcLayeredPadding.right;
					rcRoot.bottom -= m_rcLayeredPadding.bottom;
					{
						RECT rcPad = m_pRoot->GetMargin();
						rcRoot.left += rcPad.left;
						rcRoot.top += rcPad.top;
						rcRoot.right -= rcPad.right;
						rcRoot.bottom -= rcPad.bottom;
						if( rcRoot.right < rcRoot.left ) rcRoot.right = rcRoot.left;
						if( rcRoot.bottom < rcRoot.top ) rcRoot.bottom = rcRoot.top;
					}
					m_pRoot->SetPos(rcRoot, true);
				}

				if( m_bLayered ) {
					// D2D ???????? Present?DComp / ULW?????????? WS_EX_LAYERED ?? Composition
					if( GetRenderDevice()->GetBackendKind() != DUILIB_RENDER_D2D ) {
						DWORD dwExStyle = ::GetWindowLong(m_hWndPaint, GWL_EXSTYLE);
						DWORD dwNewExStyle = dwExStyle | WS_EX_LAYERED;
						if(dwExStyle != dwNewExStyle) ::SetWindowLong(m_hWndPaint, GWL_EXSTYLE, dwNewExStyle);
					}
					m_bOffscreenPaint = true;
					UnionRect(&rcPaint, &rcPaint, &m_rcLayeredUpdate);
					if( rcPaint.right > rcClient.right ) rcPaint.right = rcClient.right;
					if( rcPaint.bottom > rcClient.bottom ) rcPaint.bottom = rcClient.bottom;
					::ZeroMemory(&m_rcLayeredUpdate, sizeof(m_rcLayeredUpdate));
				}

				//
				// Render screen
				//
				// Prepare offscreen surface / HWND ??
				bool bHwndDirect = false;
				if( m_bOffscreenPaint ) {
					if( m_pOffscreenSurface == NULL ) m_pOffscreenSurface = GetRenderDevice()->CreateSurface();
					// ?????? HWND RT?RichEdit ? GetDC ????? DisableWindowTarget?
					// Present ??? BitmapRT ??????????????? BitmapRT + Present?
					if( !bHwndDirect )
						m_pOffscreenSurface->Ensure((int)dwWidth, (int)dwHeight, m_hDcPaint);
					if( m_bLayered && GetRenderDevice()->GetBackendKind() == DUILIB_RENDER_D2D ) {
						m_pOffscreenSurface->SetLayeredCompositionEnabled(m_bLayeredCompositionEnabled);
						if( m_bLayeredCompositionEnabled )
							m_pOffscreenSurface->PrepareLayeredComposition(m_hWndPaint, (int)dwWidth, (int)dwHeight);
						// DComp Flip swapchain：脏区需整客户区，否则 Present1 会保留旧帧锯齿角
						if( m_pOffscreenSurface->IsLayeredComposition() )
							rcPaint = rcClient;
						m_pOffscreenSurface->SetDirtyRect(rcPaint);
					}
				}
				// Begin Windows paint
				PAINTSTRUCT ps = { 0 };
				::BeginPaint(m_hWndPaint, &ps);
				if( m_bOffscreenPaint && m_pOffscreenSurface != NULL && m_pOffscreenSurface->IsValid() ) {
					HDC hOffscreenDC = bHwndDirect ? m_hDcPaint : reinterpret_cast<HDC>(m_pOffscreenSurface->GetNativeTarget());
					int iSaveDC = ::SaveDC(hOffscreenDC);
					if( m_bLayered )
						m_pOffscreenSurface->ClearPaintRect(rcPaint, rcClient);
					{
						COwnedRenderContextScope renderScope(this, hOffscreenDC);
						IRenderContext& renderCtx = renderScope.GetContext();
						// 先铺 html/窗口背景，再画根控件（根有 margin 时四周可见）
						if( m_dwWindowBackgroundColor != 0 ) {
							RECT rcBk = { 0 };
							if( ::IntersectRect(&rcBk, &rcPaint, &rcClient) )
								renderCtx.DrawColor(rcBk, m_dwWindowBackgroundColor);
						}
						m_pRoot->Paint(renderCtx, rcPaint, NULL);

					if( m_bLayered ) {
						for( int i = 0; i < m_aNativeWindow.GetSize(); ) {
							HWND hChildWnd = static_cast<HWND>(m_aNativeWindow[i]);
							if (!::IsWindow(hChildWnd)) {
								m_aNativeWindow.Remove(i);
								m_aNativeWindowControl.Remove(i);
								continue;
							}
							++i;
							if (!::IsWindowVisible(hChildWnd)) continue;
							RECT rcChildWnd = GetNativeWindowRect(hChildWnd);
							RECT rcTemp = { 0 };
							if( !::IntersectRect(&rcTemp, &rcPaint, &rcChildWnd) ) continue;

							COLORREF* pChildBitmapBits = NULL;
							void* pChildNative = NULL;
							// GetDC??? D2D ?? GDI?? BitBlt??? EndFrame ?????
							HDC hPaintDC = renderCtx.GetDC();
							HDC hChildMemDC = ::CreateCompatibleDC(hPaintDC);
							int nChildW = rcChildWnd.right - rcChildWnd.left;
							int nChildH = rcChildWnd.bottom - rcChildWnd.top;
							if( !GetRenderDevice()->CreatePixelBuffer(nChildW, nChildH, (BYTE**)&pChildBitmapBits, &pChildNative) || pChildNative == NULL ) {
								::DeleteDC(hChildMemDC);
								continue;
							}
							HBITMAP hChildBitmap = reinterpret_cast<HBITMAP>(pChildNative);
							::ZeroMemory(pChildBitmapBits, nChildW * nChildH * 4);
							HBITMAP hOldChildBitmap = (HBITMAP) ::SelectObject(hChildMemDC, hChildBitmap);
							::SendMessage(hChildWnd, WM_PRINT, (WPARAM)hChildMemDC,(LPARAM)(PRF_CHECKVISIBLE|PRF_CHILDREN|PRF_CLIENT|PRF_OWNED));
							COLORREF* pChildBitmapBit;
							for( LONG y = 0; y < nChildH; y++ ) {
								for( LONG x = 0; x < nChildW; x++ ) {
									pChildBitmapBit = pChildBitmapBits + y * nChildW + x;
									if (*pChildBitmapBit != 0x00000000) *pChildBitmapBit |= 0xFF000000; // 像素 AARRGGBB
								}
							}
							::BitBlt(hPaintDC, rcChildWnd.left, rcChildWnd.top, nChildW, nChildH, hChildMemDC, 0, 0, SRCCOPY);
							::SelectObject(hChildMemDC, hOldChildBitmap);
							GetRenderDevice()->DestroyPixelBuffer(pChildNative);
							::DeleteDC(hChildMemDC);
						}
					}

					for( int i = 0; i < m_aPostPaintControls.GetSize(); i++ ) {
						CControlUI* pPostPaintControl = static_cast<CControlUI*>(m_aPostPaintControls[i]);
						pPostPaintControl->DoPostPaint(renderCtx, rcPaint);
					}
					} // EndFrame: sync D2D backend to GDI before layered mask / Present

					::RestoreDC(hOffscreenDC, iSaveDC);

					if( m_bLayered ) {
						if(!m_diLayered.sDrawString.IsEmpty()) {
							DWORD dwWidth = rcClient.right - rcClient.left;
							DWORD dwHeight = rcClient.bottom - rcClient.top;
							RECT rcLayeredClient = rcClient;
							rcLayeredClient.left += m_rcLayeredPadding.left;
							rcLayeredClient.top += m_rcLayeredPadding.top;
							rcLayeredClient.right -= m_rcLayeredPadding.right;
							rcLayeredClient.bottom -= m_rcLayeredPadding.bottom;

							if( m_pBackgroundSurface == NULL ) {
								m_pBackgroundSurface = GetRenderDevice()->CreateSurface();
								m_pBackgroundSurface->Ensure((int)dwWidth, (int)dwHeight, m_hDcPaint);
								BYTE* pBgBits = m_pBackgroundSurface->GetBits();
								::ZeroMemory(pBgBits, dwWidth * dwHeight * 4);
								HDC hBgDC = reinterpret_cast<HDC>(m_pBackgroundSurface->GetNativeTarget());
								COwnedRenderContextScope bgScope(this, hBgDC);
								CRenderClipScope clip(bgScope.GetContext(), rcLayeredClient);
								bgScope.GetContext().DrawImageInfo(rcLayeredClient, rcLayeredClient, &m_diLayered);
							}
							else if( m_bLayeredChanged ) {
								m_pBackgroundSurface->Ensure((int)dwWidth, (int)dwHeight, m_hDcPaint);
								BYTE* pBgBits = m_pBackgroundSurface->GetBits();
								::ZeroMemory(pBgBits, dwWidth * dwHeight * 4);
								HDC hBgDC = reinterpret_cast<HDC>(m_pBackgroundSurface->GetNativeTarget());
								COwnedRenderContextScope bgScope(this, hBgDC);
								CRenderClipScope clip(bgScope.GetContext(), rcLayeredClient);
								bgScope.GetContext().DrawImageInfo(rcLayeredClient, rcLayeredClient, &m_diLayered);
							}
							m_pOffscreenSurface->ApplyLayeredMask(m_pBackgroundSurface, rcPaint, rcClient);
						}
						else {
							m_pOffscreenSurface->FixLayeredAlpha(rcPaint, rcClient);
						}

						// 分层 + 窗口圆角：Present 前对位图做 AA 圆角遮罩（不依赖 RoundClip 是否被 Flush 破坏）
						{
							SIZE szRound = GetBorderRadius();
							if( szRound.cx > 0 || szRound.cy > 0 )
								m_pOffscreenSurface->ApplyRoundCornerMask(szRound.cx, szRound.cy);
						}

						RenderPresentParams presentParams;
						presentParams.hWnd = m_hWndPaint;
						presentParams.hWindowDC = m_hDcPaint;
						presentParams.rcPaint = rcPaint;
						presentParams.rcClient = rcClient;
						presentParams.bLayered = true;
						presentParams.nOpacity = m_nOpacity;
						m_pOffscreenSurface->Present(presentParams);
					}
					else {
						// 非分层 Present：显式 ExcludeClipRect 排除 WC_EDIT 子窗。
						// 日志已证明系统 caret 存在但看不见；若 BeginPaint 的 CLIPCHILDREN
						// 未生效（PtVisible(editCenter)=1），BitBlt 会盖掉 XOR 光标。
						int nPaintSave = ::SaveDC(ps.hdc);
						for( HWND hChild = ::GetWindow(m_hWndPaint, GW_CHILD);
							hChild != NULL;
							hChild = ::GetWindow(hChild, GW_HWNDNEXT) )
						{
							if( !::IsWindowVisible(hChild) ) continue;
							TCHAR cls[64] = {};
							::GetClassName(hChild, cls, _countof(cls));
							if( _tcsicmp(cls, WC_EDIT) != 0 && _tcsicmp(cls, _T("EditWnd")) != 0 )
								continue;
							RECT rcChild = {};
							::GetWindowRect(hChild, &rcChild);
							::MapWindowPoints(HWND_DESKTOP, m_hWndPaint, (LPPOINT)&rcChild, 2);
							::ExcludeClipRect(ps.hdc, rcChild.left, rcChild.top, rcChild.right, rcChild.bottom);
						}
						RenderPresentParams presentParams;
						presentParams.hWnd = m_hWndPaint;
						presentParams.hWindowDC = ps.hdc;
						presentParams.rcPaint = rcPaint;
						presentParams.rcClient = rcClient;
						presentParams.bLayered = false;
						presentParams.nOpacity = m_nOpacity;
						m_pOffscreenSurface->Present(presentParams);
						::RestoreDC(ps.hdc, nPaintSave);
						// 原生 Edit 插入符改由 CEditWnd TimerQueue 自绘；此处仅停 RichEdit 误留的 TimerQueue。
						{
							HWND hFocus = ::GetFocus();
							if( hFocus != NULL && ::IsChild(m_hWndPaint, hFocus) ) {
								TCHAR cls[64] = {};
								::GetClassName(hFocus, cls, _countof(cls));
								if( (_tcsicmp(cls, WC_EDIT) == 0 || _tcsicmp(cls, _T("EditWnd")) == 0)
									&& m_pFocus != NULL )
								{
									CRichEditUI* pRich = static_cast<CRichEditUI*>(m_pFocus->GetInterface(DUI_CTR_RICHEDIT));
									if( pRich != NULL )
										pRich->StopAllQueueTimers();
								}
							}
						}
					}

					if( m_bShowUpdateRect && !m_bLayered ) {
						HPEN hOldPen = (HPEN)::SelectObject(ps.hdc, m_hUpdateRectPen);
						::SelectObject(ps.hdc, ::GetStockObject(HOLLOW_BRUSH));
						::Rectangle(ps.hdc, rcPaint.left, rcPaint.top, rcPaint.right, rcPaint.bottom);
						::SelectObject(ps.hdc, hOldPen);
					}
				}
				else {
					// A standard paint job（同样走 ps.hdc，避免盖住子窗）
					int iSaveDC = ::SaveDC(ps.hdc);
					COwnedRenderContextScope renderScope(this, ps.hdc);
					IRenderContext& renderCtx = renderScope.GetContext();
					m_pRoot->Paint(renderCtx, rcPaint, NULL);
					for( int i = 0; i < m_aPostPaintControls.GetSize(); i++ ) {
						CControlUI* pPostPaintControl = static_cast<CControlUI*>(m_aPostPaintControls[i]);
						pPostPaintControl->DoPostPaint(renderCtx, rcPaint);
					}
					::RestoreDC(ps.hdc, iSaveDC);
				}
				// All Done!
				::EndPaint(m_hWndPaint, &ps);

				// ???????
				SetPainting(false);
				m_bLayeredChanged = false;
				if( m_bUpdateNeeded ) Invalidate();

				// ???????????????
				if(bNeedSizeMsg) {
					this->SendNotify(m_pRoot, DUI_MSGTYPE_WINDOWSIZE, 0, 0, true);
				}
				return true;
			}
		case WM_PRINTCLIENT:
			{
				if( m_pRoot == NULL ) break;
				RECT rcClient;
				::GetClientRect(m_hWndPaint, &rcClient);
				HDC hDC = (HDC) wParam;
				int save = ::SaveDC(hDC);
				COwnedRenderContextScope renderScope(this, hDC);
				IRenderContext& renderCtx = renderScope.GetContext();
				m_pRoot->Paint(renderCtx, rcClient, NULL);
				if( (lParam & PRF_CHILDREN) != 0 ) {
					HWND hWndChild = ::GetWindow(m_hWndPaint, GW_CHILD);
					while( hWndChild != NULL ) {
						RECT rcPos = { 0 };
						::GetWindowRect(hWndChild, &rcPos);
						::MapWindowPoints(HWND_DESKTOP, m_hWndPaint, reinterpret_cast<LPPOINT>(&rcPos), 2);
						::SetWindowOrgEx(hDC, -rcPos.left, -rcPos.top, NULL);
						::SendMessage(hWndChild, WM_PRINT, wParam, lParam | PRF_NONCLIENT);
						hWndChild = ::GetWindow(hWndChild, GW_HWNDNEXT);
					}
				}
				::RestoreDC(hDC, save);
			}
			break;
		case WM_GETMINMAXINFO:
			{
				MONITORINFO mi = {};
				mi.cbSize = sizeof(mi);
				::GetMonitorInfo(::MonitorFromWindow(m_hWndPaint, MONITOR_DEFAULTTONEAREST), &mi);
				const RECT& rcWork = mi.rcWork;
				const RECT& rcMon = mi.rcMonitor;
				const int cxWork = rcWork.right - rcWork.left;
				const int cyWork = rcWork.bottom - rcWork.top;

				LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
				lpMMI->ptMaxPosition.x = rcWork.left - rcMon.left;
				lpMMI->ptMaxPosition.y = rcWork.top - rcMon.top;
				lpMMI->ptMaxSize.x = cxWork;
				lpMMI->ptMaxSize.y = cyWork;
				if( m_szMinWindow.cx > 0 ) lpMMI->ptMinTrackSize.x = m_szMinWindow.cx;
				if( m_szMinWindow.cy > 0 ) lpMMI->ptMinTrackSize.y = m_szMinWindow.cy;
				if( m_szMaxWindow.cx > 0 ) lpMMI->ptMaxTrackSize.x = m_szMaxWindow.cx;
				else lpMMI->ptMaxTrackSize.x = cxWork;
				if( m_szMaxWindow.cy > 0 ) lpMMI->ptMaxTrackSize.y = m_szMaxWindow.cy;
				else lpMMI->ptMaxTrackSize.y = cyWork;
				if( m_szMaxWindow.cx > 0 ) lpMMI->ptMaxSize.x = m_szMaxWindow.cx;
				if( m_szMaxWindow.cy > 0 ) lpMMI->ptMaxSize.y = m_szMaxWindow.cy;
			}
			break;
		case WM_SIZE:
			{
				if( m_pFocus != NULL ) {
					TEventUI event = { 0 };
					event.Type = UIEVENT_WINDOWSIZE;
					event.pSender = m_pFocus;
					event.dwTimestamp = ::GetTickCount();
					m_pFocus->Event(event);
				}
				if( m_pRoot != NULL ) m_pRoot->NeedUpdate();
			}
			return true;
		case UIMSG_LOADING_TICK:
			DuiLib_LoadingOnQueueTick(reinterpret_cast<CLoadingUI*>(wParam));
			return true;
		case UIMSG_RING_TICK:
			DuiLib_RingOnQueueTick(reinterpret_cast<CRingUI*>(wParam));
			return true;
		case UIMSG_SKELETON_TICK:
			DuiLib_SkeletonOnQueueTick(reinterpret_cast<CSkeletonUI*>(wParam));
			return true;
		case UIMSG_GIFANIM_TICK:
			DuiLib_GifAnimOnQueueTick(reinterpret_cast<CGifAnimUI*>(wParam));
			return true;
#ifdef USE_XIMAGE_EFFECT
		case UIMSG_GIFANIMEX_TICK:
			DuiLib_GifAnimExOnQueueTick(reinterpret_cast<CGifAnimExUI*>(wParam));
			return true;
#endif
		case UIMSG_SCROLLBAR_TICK:
			DuiLib_ScrollBarOnQueueTick(reinterpret_cast<CScrollBarUI*>(wParam));
			return true;
		case UIMSG_CAROUSEL_TICK:
			DuiLib_CarouselOnQueueTick(reinterpret_cast<CCarouselUI*>(wParam));
			return true;
		case UIMSG_ANIMATION_TICK:
			{
				CControlUI* pControl = reinterpret_cast<CControlUI*>(wParam);
				if( pControl != NULL ) {
					TEventUI event = { 0 };
					event.Type = UIEVENT_TIMER;
					event.pSender = pControl;
					event.dwTimestamp = ::GetTickCount();
					event.wParam = lParam;
					event.lParam = lParam;
					pControl->Event(event);
				}
			}
			return true;
		case UIMSG_ROLLTEXT_TICK:
			{
				CControlUI* pControl = reinterpret_cast<CControlUI*>(wParam);
				if( pControl != NULL ) {
					TEventUI event = { 0 };
					event.Type = UIEVENT_TIMER;
					event.pSender = pControl;
					event.dwTimestamp = ::GetTickCount();
					event.wParam = lParam;
					event.lParam = lParam;
					pControl->Event(event);
				}
			}
			return true;
		case UIMSG_RICHEDIT_TICK:
			DuiLib_RichEditOnQueueTick(reinterpret_cast<CRichEditUI*>(wParam), (UINT)lParam);
			return true;
		case UIMSG_EDIT_TICK:
			DuiLib_EditOnQueueTick(reinterpret_cast<CEditUI*>(wParam), (UINT)lParam);
			return true;
		case UIMSG_TOOLTIP_HOVER:
			m_hTipQueueTimer = NULL;
			if( m_pTipPending != NULL && m_pEventHover == m_pTipPending ) {
				CControlUI* pAt = FindControl(m_ptLastMousePos);
				if( pAt == m_pTipPending )
					ShowControlToolTip(m_pTipPending);
				else
					m_pTipPending = NULL;
			}
			return true;
		case WM_TIMER:
			{
				for( int i = 0; i < m_aTimers.GetSize(); i++ ) {
					const TIMERINFO* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
					if(pTimer->hWnd == m_hWndPaint && 
						pTimer->uWinTimer == LOWORD(wParam) && 
						pTimer->bKilled == false)
					{
						TEventUI event = { 0 };
						event.Type = UIEVENT_TIMER;
						event.pSender = pTimer->pSender;
						event.dwTimestamp = ::GetTickCount();
						event.ptMouse = m_ptLastMousePos;
						event.wKeyState = MapKeyState();
						event.wParam = pTimer->nLocalID;
						event.lParam = lParam;
						pTimer->pSender->Event(event);
						break;
					}
				}
			}
			break;
		case WM_MOUSEHOVER:
			{
				m_bMouseTracking = false;
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				if( m_pEventHover != NULL ) {
					TEventUI event = { 0 };
					event.Type = UIEVENT_MOUSEHOVER;
					event.pSender = m_pEventHover;
					event.wParam = wParam;
					event.lParam = lParam;
					event.dwTimestamp = ::GetTickCount();
					event.ptMouse = pt;
					event.wKeyState = MapKeyState();
					m_pEventHover->Event(event);
				}
				ArmMouseHoverTrack(false, pt);
			}
			return true;
		case WM_MOUSELEAVE:
			{
				POINT pt = { 0 };
				RECT rcWnd = { 0 };
				::GetCursorPos(&pt);
				::GetWindowRect(m_hWndPaint, &rcWnd);
				// 客户区↔非客户区切换会误发 LEAVE；光标仍在窗内则忽略（勿 re-arm TME，避免与拖窗抢消息）
				if( !::IsIconic(m_hWndPaint) && ::PtInRect(&rcWnd, pt) ) {
					m_bMouseTracking = false;
					POINT ptClient = pt;
					::ScreenToClient(m_hWndPaint, &ptClient);
					SyncToolTipWithHover(FindControl(ptClient));
					break;
				}
				HideControlToolTip();
				m_bMouseTracking = false;
				::SendMessage(m_hWndPaint, WM_MOUSEMOVE, 0, (LPARAM)-1);
			}
			break;
		case WM_MOUSEMOVE:
			{
				POINT ptArm = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				if( (ptArm.x != -1 || ptArm.y != -1) && !m_bMouseTracking
					&& ::GetCapture() != m_hWndPaint )
					ArmMouseHoverTrack(false, ptArm);

				// Generate the appropriate mouse messages
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				// ??????
				bool bNeedDrag = true;
				if(m_ptLastMousePos.x == pt.x && m_ptLastMousePos.y == pt.y) {
					bNeedDrag = false;
				}
				// ?????????
				m_ptLastMousePos = pt;
				CControlUI* pNewHover = FindControl(pt);
				if( pNewHover != NULL && pNewHover->GetManager() != this ) break;
				if( !IsCaptured() )
					SyncToolTipWithHover(pNewHover);

				// ??????
				if(bNeedDrag && m_bDragMode && wParam == MK_LBUTTON)
				{
					// ???Capture
					::ReleaseCapture();
					// ???
					if(m_pDragDrop != NULL && m_pDragDrop->OnDragDrop(m_pEventClick)) {

						m_bDragMode = false;
						break;
					}

					CIDropSource* pdsrc = new CIDropSource;
					if(pdsrc == NULL) return 0;
					pdsrc->AddRef();

					CIDataObject* pdobj = new CIDataObject(pdsrc);
					if(pdobj == NULL) return 0;
					pdobj->AddRef();

					FORMATETC fmtetc = {0};
					STGMEDIUM medium = {0};
					fmtetc.dwAspect = DVASPECT_CONTENT;
					fmtetc.lindex = -1;
					fmtetc.cfFormat = CF_BITMAP;
					fmtetc.tymed = TYMED_GDI;

					//////////////////////////////////////
					HBITMAP hBitmap = (HBITMAP)OleDuplicateData(m_hDragBitmap, fmtetc.cfFormat, NULL);
					medium.hBitmap = hBitmap;
					pdobj->SetData(&fmtetc, &medium, FALSE);

					//////////////////////////////////////
					BITMAP bmap;
					GetObject(hBitmap, sizeof(BITMAP), &bmap);
					RECT rc={0, 0, bmap.bmWidth, bmap.bmHeight};
					fmtetc.cfFormat = CF_ENHMETAFILE;
					fmtetc.tymed = TYMED_ENHMF;
					HDC hMetaDC = CreateEnhMetaFile(m_hDcPaint, NULL, NULL, NULL);
					HDC hdcMem = CreateCompatibleDC(m_hDcPaint);
					HGDIOBJ hOldBmp = ::SelectObject(hdcMem, hBitmap);
					::BitBlt(hMetaDC, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);
					::SelectObject(hdcMem, hOldBmp);
					medium.hEnhMetaFile = CloseEnhMetaFile(hMetaDC);
					DeleteDC(hdcMem);
					medium.tymed = TYMED_ENHMF;
					pdobj->SetData(&fmtetc, &medium, TRUE);
					//////////////////////////////////////
					CDragSourceHelper dragSrcHelper;
					POINT ptDrag = {0};
					ptDrag.x = bmap.bmWidth / 2;
					ptDrag.y = bmap.bmHeight / 2;
					dragSrcHelper.InitializeFromBitmap(hBitmap, ptDrag, rc, pdobj); //will own the bmp
					DWORD dwEffect;
					::DoDragDrop(pdobj, pdsrc, DROPEFFECT_COPY | DROPEFFECT_MOVE, &dwEffect);
					if(dwEffect ) pdsrc->Release();
					else delete pdsrc;
					pdobj->Release();
					m_bDragMode = false;
					break;
				}

				TEventUI event = { 0 };
				event.ptMouse = pt;
				event.wParam = wParam;
				event.lParam = lParam;
				event.dwTimestamp = ::GetTickCount();
				event.wKeyState = MapKeyState();
				if( !IsCaptured() ) {
					pNewHover = FindControl(pt);
					if( pNewHover != NULL && pNewHover->GetManager() != this ) break;
					if( pNewHover != m_pEventHover && m_pEventHover != NULL ) {
						event.Type = UIEVENT_MOUSELEAVE;
						event.pSender = m_pEventHover;

						CStdPtrArray aNeedMouseLeaveNeeded(m_aNeedMouseLeaveNeeded.GetSize());
						aNeedMouseLeaveNeeded.Resize(m_aNeedMouseLeaveNeeded.GetSize());
						::CopyMemory(aNeedMouseLeaveNeeded.GetData(), m_aNeedMouseLeaveNeeded.GetData(), m_aNeedMouseLeaveNeeded.GetSize() * sizeof(LPVOID));
						for( int i = 0; i < aNeedMouseLeaveNeeded.GetSize(); i++ ) {
							static_cast<CControlUI*>(aNeedMouseLeaveNeeded[i])->Event(event);
						}

						m_pEventHover->Event(event);
						if( m_pTipPending == m_pEventHover || m_pTipShown == m_pEventHover )
							HideControlToolTip();
						m_pEventHover = NULL;
					}
					if( pNewHover != m_pEventHover && pNewHover != NULL ) {
						event.Type = UIEVENT_MOUSEENTER;
						event.pSender = pNewHover;
						pNewHover->Event(event);
						m_pEventHover = pNewHover;
						if( !pNewHover->GetToolTip().IsEmpty() )
							ScheduleControlToolTip(pNewHover);
					}
				}
				if( m_pEventClick != NULL ) {
					event.Type = UIEVENT_MOUSEMOVE;
					event.pSender = m_pEventClick;
					m_pEventClick->Event(event);
				}
				else if( pNewHover != NULL ) {
					event.Type = UIEVENT_MOUSEMOVE;
					event.pSender = pNewHover;
					pNewHover->Event(event);
				}
			}
			break;
		case WM_LBUTTONDOWN:
			{
				HideControlToolTip();
				// We alway set focus back to our app (this helps
				// when Win32 child windows are placed on the dialog
				// and we need to remove them on focus change).
				if (!m_bNoActivate) ::SetFocus(m_hWndPaint);
				if( m_pRoot == NULL ) break;
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				m_ptLastMousePos = pt;
				CControlUI* pControl = FindControl(pt);

				// 悬停策略下拖窗区保持 HTCLIENT，点击可能走到客户区 LBUTTONDOWN；
				// 与旧 HTCAPTION 行为对齐：转交 DefWindowProc 拖窗。
				bool bCaptionDrag = false;
				if( pControl != NULL ) {
					if( pControl->IsCaptionDragHit(pt) ) bCaptionDrag = true;
					else if( pControl->GetAction() == UIACTION_NONE && !pControl->PreferClientHit() ) {
						for( CControlUI* pWalk = pControl->GetParent(); pWalk != NULL; pWalk = pWalk->GetParent() ) {
							if( pWalk->IsCaptionDragHit(pt) ) { bCaptionDrag = true; break; }
							UIAction parentAct = pWalk->GetAction();
							if( parentAct == UIACTION_TITLE || parentAct == UIACTION_MOVEWINDOW ) break;
							if( parentAct != UIACTION_NONE ) break;
						}
						if( !bCaptionDrag ) {
							UIAction winAct = GetWindowAction();
							if( winAct == UIACTION_TITLE || winAct == UIACTION_MOVEWINDOW )
								bCaptionDrag = true;
						}
					}
				}
				else {
					UIAction winAct = GetWindowAction();
					if( winAct == UIACTION_TITLE || winAct == UIACTION_MOVEWINDOW )
						bCaptionDrag = true;
				}
				if( bCaptionDrag ) {
					::ReleaseCapture();
					POINT screen = pt;
					::ClientToScreen(m_hWndPaint, &screen);
					::SendMessage(m_hWndPaint, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(screen.x, screen.y));
					return true;
				}

				if( pControl == NULL ) break;
				if( pControl->GetManager() != this ) break;

				// ??????
				if(m_bDragDrop && pControl->IsDragEnabled()) {
					m_bDragMode = true;
					if( m_hDragBitmap != NULL ) {
						::DeleteObject(m_hDragBitmap);
						m_hDragBitmap = NULL;
					}
					m_hDragBitmap = CRenderEngine::GenerateBitmap(this, pControl, pControl->GetPos());
				}

				// ????????
				SetCapture();
				// ???????
				m_pEventClick = pControl;
				pControl->SetFocus();

				TEventUI event = { 0 };
				event.Type = UIEVENT_BUTTONDOWN;
				event.pSender = pControl;
				event.wParam = wParam;
				event.lParam = lParam;
				event.ptMouse = pt;
				event.wKeyState = (WORD)wParam;
				event.dwTimestamp = ::GetTickCount();
				pControl->Event(event);
			}
			break;
		case WM_LBUTTONDBLCLK:
			{
				if (!m_bNoActivate) ::SetFocus(m_hWndPaint);

				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				m_ptLastMousePos = pt;
				CControlUI* pControl = FindControl(pt);
				if( pControl == NULL ) break;
				if( pControl->GetManager() != this ) break;
				//SetCapture();
				TEventUI event = { 0 };
				event.Type = UIEVENT_DBLCLICK;
				event.pSender = pControl;
				event.ptMouse = pt;
				event.wParam = wParam;
				event.lParam = lParam;
				event.wKeyState = (WORD)wParam;
				event.dwTimestamp = ::GetTickCount();
				pControl->Event(event);
				m_pEventClick = pControl;
			}
			break;
		case WM_LBUTTONUP:
			{
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				m_ptLastMousePos = pt;
				if( m_pEventClick == NULL ) break;
				ReleaseCapture();
				TEventUI event = { 0 };
				event.Type = UIEVENT_BUTTONUP;
				event.pSender = m_pEventClick;
				event.wParam = wParam;
				event.lParam = lParam;
				event.ptMouse = pt;
				event.wKeyState = (WORD)wParam;
				event.dwTimestamp = ::GetTickCount();

				CControlUI* pClick = m_pEventClick;
				m_pEventClick = NULL;
				pClick->Event(event);
			}
			break;
		case WM_RBUTTONDOWN:
			{
				if (!m_bNoActivate) ::SetFocus(m_hWndPaint);
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				m_ptLastMousePos = pt;
				DUILOG(_T("[rbdn] enter pt=(%d,%d)"), pt.x, pt.y);
				CControlUI* pControl = FindControl(pt);
				if( pControl == NULL ) { DUILOG(_T("[rbdn] not found -> break")); break; }
				if( pControl->GetManager() != this ) { DUILOG(_T("[rbdn] manager mismatch -> break")); break; }
				DUILOG(_T("[rbdn] found %s(%s)"), pControl->GetClass(),
					pControl->GetName().IsEmpty() ? _T("(no-name)") : pControl->GetName().GetData());
				pControl->SetFocus();
				SetCapture();
				TEventUI event = { 0 };
				event.Type = UIEVENT_RBUTTONDOWN;
				event.pSender = pControl;
				event.wParam = wParam;
				event.lParam = lParam;
				event.ptMouse = pt;
				event.wKeyState = (WORD)wParam;
				event.dwTimestamp = ::GetTickCount();
				pControl->Event(event);
				m_pEventRClick = pControl;
			}
			break;
		case WM_RBUTTONUP:
			{
				if(m_bMouseCapture) ReleaseCapture();

				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				m_ptLastMousePos = pt;
				DUILOG(_T("[rbup] enter pt=(%d,%d) capture=%d"), pt.x, pt.y, m_bMouseCapture ? 1 : 0);
				CControlUI* pCtrl = FindControl(pt);
				m_pEventRClick = pCtrl;
				if(m_pEventRClick == NULL) { DUILOG(_T("[rbup] not found -> break")); break; }
				DUILOG(_T("[rbup] found %s(%s)"), m_pEventRClick->GetClass(),
					m_pEventRClick->GetName().IsEmpty() ? _T("(no-name)") : m_pEventRClick->GetName().GetData());

				TEventUI event = { 0 };
				event.Type = UIEVENT_RBUTTONUP;
				event.pSender = m_pEventRClick;
				event.wParam = wParam;
				event.lParam = lParam;
				event.ptMouse = pt;
				event.wKeyState = (WORD)wParam;
				event.dwTimestamp = ::GetTickCount();
				m_pEventRClick->Event(event);
			}
			break;
		case WM_MBUTTONDOWN:
			{
				if (!m_bNoActivate) ::SetFocus(m_hWndPaint);
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				m_ptLastMousePos = pt;
				CControlUI* pControl = FindControl(pt);
				if( pControl == NULL ) break;
				if( pControl->GetManager() != this ) break;
				pControl->SetFocus();
				SetCapture();
				TEventUI event = { 0 };
				event.Type = UIEVENT_MBUTTONDOWN;
				event.pSender = pControl;
				event.wParam = wParam;
				event.lParam = lParam;
				event.ptMouse = pt;
				event.wKeyState = (WORD)wParam;
				event.dwTimestamp = ::GetTickCount();
				pControl->Event(event);
				m_pEventClick = pControl;
			}
			break;
		case WM_MBUTTONUP:
			{
				if(m_bMouseCapture) ReleaseCapture();
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				m_ptLastMousePos = pt;
				m_pEventClick = FindControl(pt);
				if(m_pEventClick == NULL) break;

				TEventUI event = { 0 };
				event.Type = UIEVENT_MBUTTONUP;
				event.pSender = m_pEventClick;
				event.wParam = wParam;
				event.lParam = lParam;
				event.ptMouse = pt;
				event.wKeyState = (WORD)wParam;
				event.dwTimestamp = ::GetTickCount();
				m_pEventClick->Event(event);
			}
			break;
		case WM_CONTEXTMENU:
			{
				if( m_pRoot == NULL ) break;
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				::ScreenToClient(m_hWndPaint, &pt);
				m_ptLastMousePos = pt;
				// 视为“空白”的数种情形：
				//  1) m_pEventRClick == NULL（真无控件命中）；
				//  2) 命中的是未启用右键菜单的容器（纯布局容器原路径本就无菜单）。
				// 注意：CComboUI/CEditUI/CTextUI/ScrollBar 等自带原生右键菜单的控件不算空白。
				DUILOG(_T("[ctxmenu] enter pt=(%d,%d) switch=%d evtRClick=%s\n    name=%s used=%d"),
					pt.x, pt.y, m_bBlankCtxMenu ? 1 : 0,
					m_pEventRClick ? m_pEventRClick->GetClass() : _T("NULL"),
					m_pEventRClick ? (m_pEventRClick->GetName().IsEmpty() ? _T("(no-name)") : m_pEventRClick->GetName().GetData()) : _T("(no-name)"),
					m_pEventRClick ? (m_pEventRClick->IsContextMenuUsed() ? 1 : 0) : -1);
				if( m_bBlankCtxMenu ) {
					bool bBlank = ( m_pEventRClick == NULL );
					if( !bBlank && !m_pEventRClick->IsContextMenuUsed() ) {
						// 命中未启用右键菜单的容器 -> 视为空白兜底（纯布局容器原路径本就无菜单）
						IContainerUI* pC = static_cast<IContainerUI*>(m_pEventRClick->GetInterface(_T("IContainer")));
						bBlank = ( pC != NULL );
					}
					if( bBlank ) {
						ReleaseCapture();
						CControlUI* pTarget = m_bBlankCtxMenuDeepest ? FindDeepestContainerAt(pt) : m_pRoot;
						DUILOG(_T("[ctxmenu] blank -> target=%s(%s)"),
							pTarget ? pTarget->GetClass() : _T("NULL"),
							pTarget ? (pTarget->GetName().IsEmpty() ? _T("(no-name)") : pTarget->GetName().GetData()) : _T("(no-name)"));
						// 稳健：target 必须属于本 manager（避免误发到其它窗口/已被摘除的节点），否则回落窗口根
						if( pTarget != NULL && pTarget->GetManager() != this ) {
							DUILOG(_T("[ctxmenu] target not owned by this manager -> fallback to root"));
							pTarget = m_pRoot;
						}
						if( pTarget != NULL ) {
							// 定向：空白 MENU 只发“设置了 OnNotify 回调”的容器，不广播整个 manager。
							// 就近原则：从命中的最内层容器沿父链向上，找第一个 OnNotify 非空的容器来接收，
							// 从而“在祖先 A 设置一次，就能覆盖整棵子树（无论空白落在 B/C）”。
							CControlUI* pDispatch = NULL;
							for( CControlUI* p = pTarget; p != NULL; p = p->GetParent() ) {
								if( p->OnNotify ) { pDispatch = p; break; }
							}
							TNotifyUI Msg;
							Msg.sType = DUI_MSGTYPE_MENU;
							Msg.pSender = pTarget;   // 仍指向实际命中的最内层容器，便于回调知道点在哪个容器
							Msg.wParam = wParam;
							Msg.lParam = lParam;
							Msg.ptMouse = m_ptLastMousePos;
							Msg.ptScreen = m_ptLastMousePos;
							if( m_hWndPaint != NULL && ::IsWindow(m_hWndPaint) )
								::ClientToScreen(m_hWndPaint, &Msg.ptScreen);
							Msg.dwTimestamp = ::GetTickCount();
							if( m_bUsedVirtualWnd )
								Msg.sVirtualWnd = pTarget->GetVirtualWnd();
							if( pDispatch != NULL )
								pDispatch->OnNotify(&Msg);
							DUILOG(_T("[ctxmenu] blank hit=%s(%s) dispatchOnNotify=%s(%s)"),
								pTarget->GetClass(),
								!(pTarget->GetName().IsEmpty()) ? pTarget->GetName().GetData() : _T("(no-name)"),
								pDispatch ? pDispatch->GetClass() : _T("NONE"),
								(pDispatch && !pDispatch->GetName().IsEmpty()) ? pDispatch->GetName().GetData() : _T("(no-name)"));
						}
						break;
					}
					DUILOG(_T("[ctxmenu] blank=0 (hit non-blank control)"));
				}
				if( m_pEventRClick == NULL ) { DUILOG(_T("[ctxmenu] evtRClick NULL, switch off -> break (swallowed)")); break; }
				ReleaseCapture();
				DUILOG(_T("[ctxmenu] sending UIEVENT_CONTEXTMENU to %s"), m_pEventRClick->GetClass());
				TEventUI event = { 0 };
				event.Type = UIEVENT_CONTEXTMENU;
				event.pSender = m_pEventRClick;
				event.wParam = wParam;
				event.lParam = lParam;
				event.ptMouse = pt;
				event.wKeyState = (WORD)wParam;
				event.lParam = (LPARAM)m_pEventRClick;
				event.dwTimestamp = ::GetTickCount();
				m_pEventRClick->Event(event);
				m_pEventRClick = NULL;
			}
			break;
		case WM_MOUSEWHEEL:
			{
				if( m_pRoot == NULL ) break;
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				::ScreenToClient(m_hWndPaint, &pt);
				m_ptLastMousePos = pt;
				CControlUI* pControl = FindControl(pt);
				if( pControl == NULL ) break;
				if( pControl->GetManager() != this ) break;
				int zDelta = (int) (short) HIWORD(wParam);
				TEventUI event = { 0 };
				event.Type = UIEVENT_SCROLLWHEEL;
				event.pSender = pControl;
				event.wParam = MAKEWPARAM(zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, zDelta);
				event.lParam = lParam;
				event.ptMouse = pt;
				event.wKeyState = MapKeyState();
				event.dwTimestamp = ::GetTickCount();
				pControl->Event(event);

				// Let's make sure that the scroll item below the cursor is the same as before...
				::SendMessage(m_hWndPaint, WM_MOUSEMOVE, 0, (LPARAM) MAKELPARAM(m_ptLastMousePos.x, m_ptLastMousePos.y));
			}
			break;
		case WM_CHAR:
			{
				if( m_pRoot == NULL ) break;
				if( m_pFocus == NULL ) break;
				TEventUI event = { 0 };
				event.Type = UIEVENT_CHAR;
				event.pSender = m_pFocus;
				event.wParam = wParam;
				event.lParam = lParam;
				event.chKey = (TCHAR)wParam;
				event.ptMouse = m_ptLastMousePos;
				event.wKeyState = MapKeyState();
				event.dwTimestamp = ::GetTickCount();
				m_pFocus->Event(event);
			}
			break;
		case WM_KEYDOWN:
			{
				if( m_pRoot == NULL ) break;
				if( m_pFocus == NULL ) break;
				TEventUI event = { 0 };
				event.Type = UIEVENT_KEYDOWN;
				event.pSender = m_pFocus;
				event.wParam = wParam;
				event.lParam = lParam;
				event.chKey = (TCHAR)wParam;
				event.ptMouse = m_ptLastMousePos;
				event.wKeyState = MapKeyState();
				event.dwTimestamp = ::GetTickCount();
				m_pFocus->Event(event);
				m_pEventKey = m_pFocus;
			}
			break;
		case WM_KEYUP:
			{
				if( m_pRoot == NULL ) break;
				if( m_pEventKey == NULL ) break;
				TEventUI event = { 0 };
				event.Type = UIEVENT_KEYUP;
				event.pSender = m_pEventKey;
				event.wParam = wParam;
				event.lParam = lParam;
				event.chKey = (TCHAR)wParam;
				event.ptMouse = m_ptLastMousePos;
				event.wKeyState = MapKeyState();
				event.dwTimestamp = ::GetTickCount();
				m_pEventKey->Event(event);
				m_pEventKey = NULL;
			}
			break;
		case WM_SETCURSOR:
			{
				if( m_pRoot == NULL ) break;
				if( LOWORD(lParam) != HTCLIENT ) break;
				if( m_bMouseCapture ) return true;

				POINT pt = { 0 };
				::GetCursorPos(&pt);
				::ScreenToClient(m_hWndPaint, &pt);
				CControlUI* pControl = FindControl(pt);
				if( pControl == NULL ) break;
				if( (pControl->GetControlFlags() & UIFLAG_SETCURSOR) == 0 ) break;
				TEventUI event = { 0 };
				event.Type = UIEVENT_SETCURSOR;
				event.pSender = pControl;
				event.wParam = wParam;
				event.lParam = lParam;
				event.ptMouse = pt;
				event.wKeyState = MapKeyState();
				event.dwTimestamp = ::GetTickCount();
				pControl->Event(event);
			}
			return true;
		case WM_SETFOCUS:
			{
				if( m_pFocus != NULL ) {
					TEventUI event = { 0 };
					event.Type = UIEVENT_SETFOCUS;
					event.wParam = wParam;
					event.lParam = lParam;
					event.pSender = m_pFocus;
					event.dwTimestamp = ::GetTickCount();
					m_pFocus->Event(event);
				}
				break;
			}
		case WM_KILLFOCUS:
			{
				if(IsCaptured()) ReleaseCapture();
				break;
			}
		case WM_NOTIFY:
			{
				if( lParam == 0 ) break;
				LPNMHDR lpNMHDR = (LPNMHDR) lParam;
				if( lpNMHDR != NULL ) lRes = ::SendMessage(lpNMHDR->hwndFrom, OCM__BASE + uMsg, wParam, lParam);
				return true;
			}
			break;
		case WM_COMMAND:
			{
				if( lParam == 0 ) break;
				HWND hWndChild = (HWND) lParam;
				lRes = ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
				if(lRes != 0) return true;
			}
			break;
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORSTATIC:
			{
				// Refer To: http://msdn.microsoft.com/en-us/library/bb761691(v=vs.85).aspx
				// Read-only or disabled edit controls do not send the WM_CTLCOLOREDIT message; instead, they send the WM_CTLCOLORSTATIC message.
				if( lParam == 0 ) break;
				HWND hWndChild = (HWND) lParam;
				lRes = ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
				if(lRes != 0) return true;
			}
			break;
		default:
			break;
		}
		return false;
	}

	bool CPaintManagerUI::IsUpdateNeeded() const
	{
		return m_bUpdateNeeded;
	}

	void CPaintManagerUI::NeedUpdate()
	{
		m_bUpdateNeeded = true;
	}

	void CPaintManagerUI::Invalidate()
	{
		RECT rcClient = { 0 };
		::GetClientRect(m_hWndPaint, &rcClient);
		::UnionRect(&m_rcLayeredUpdate, &m_rcLayeredUpdate, &rcClient);
		::InvalidateRect(m_hWndPaint, NULL, FALSE);
	}

	void CPaintManagerUI::Invalidate(RECT& rcItem)
	{
		if( rcItem.left < 0 ) rcItem.left = 0;
		if( rcItem .top < 0 ) rcItem.top = 0;
		if( rcItem.right < rcItem.left ) rcItem.right = rcItem.left;
		if( rcItem.bottom < rcItem.top ) rcItem.bottom = rcItem.top;
		::UnionRect(&m_rcLayeredUpdate, &m_rcLayeredUpdate, &rcItem);
		::InvalidateRect(m_hWndPaint, &rcItem, FALSE);
	}

	bool CPaintManagerUI::IsValid()
	{
		return m_hWndPaint != NULL && m_pRoot != NULL;
	}

	bool CPaintManagerUI::AttachDialog(CControlUI* pControl)
	{
		ASSERT(::IsWindow(m_hWndPaint));
		// ???????????
		m_shadow.Create(this);

		// Reset any previous attachment
		SetFocus(NULL);
		m_pEventKey = NULL;
		m_pEventHover = NULL;
        m_pEventClick = NULL;
        m_pEventRClick = NULL;
		// Remove the existing control-tree. We might have gotten inside this function as
		// a result of an event fired or similar, so we cannot just delete the objects and
		// pull the internal memory of the calling code. We'll delay the cleanup.
		if( m_pRoot != NULL ) {
			m_aPostPaintControls.Empty();
			AddDelayedCleanup(m_pRoot);
		}
		// Set the dialog root element
		m_pRoot = pControl;
		// 先写主题 Default，再 InitControls，使新建控件吃到共享 Default
		CThemeManager::GetInstance()->ApplyManagerDefaults(this);
		ApplyDefaultWindowBackgroundColor();
		ApplyDefaultWindowBackgroundImage();
		ApplyDefaultWindowAction();
		ApplyDefaultWindowTheme();
		// Go ahead...
		m_bUpdateNeeded = true;
		m_bFirstLayout = true;
		m_bFocusNeeded = true;
		// Initiate all control
		bool bOk = InitControls(pControl);
		// root 就绪后再套 chrome（TitleBar 默认；表单需 theme=chrome）
		CThemeManager::GetInstance()->ApplyToExistingManager(this);
		return bOk;
	}

	bool CPaintManagerUI::InitControls(CControlUI* pControl, CControlUI* pParent /*= NULL*/)
	{
		ASSERT(pControl);
		if( pControl == NULL ) return false;
		pControl->SetManager(this, pParent != NULL ? pParent : pControl->GetParent(), true);
		pControl->FindControl(__FindControlFromNameHash, this, UIFIND_ALL);
		return true;
	}

	void CPaintManagerUI::ReapObjects(CControlUI* pControl)
	{
		if( pControl == m_pEventKey ) m_pEventKey = NULL;
		if( pControl == m_pEventHover ) m_pEventHover = NULL;
        if (pControl == m_pEventClick) m_pEventClick = NULL;
        if (pControl == m_pEventRClick) m_pEventRClick = NULL;
		if( pControl == m_pFocus ) m_pFocus = NULL;
		KillTimer(pControl);
		const CDuiString& sName = pControl->GetName();
		if( !sName.IsEmpty() ) {
			if( pControl == FindControl(sName.GetData()) ) m_mNameHash.Remove(sName);
		}
		for( int i = 0; i < m_aAsyncNotify.GetSize(); i++ ) {
			TNotifyUI* pMsg = static_cast<TNotifyUI*>(m_aAsyncNotify[i]);
			if( pMsg->pSender == pControl ) pMsg->pSender = NULL;
		}    
	}

	bool CPaintManagerUI::AddOptionGroup(LPCTSTR pStrGroupName, CControlUI* pControl)
	{
		LPVOID lp = m_mOptionGroup.Find(pStrGroupName);
		if( lp ) {
			CStdPtrArray* aOptionGroup = static_cast<CStdPtrArray*>(lp);
			for( int i = 0; i < aOptionGroup->GetSize(); i++ ) {
				if( static_cast<CControlUI*>(aOptionGroup->GetAt(i)) == pControl ) {
					return false;
				}
			}
			aOptionGroup->Add(pControl);
		}
		else {
			CStdPtrArray* aOptionGroup = new CStdPtrArray(6);
			aOptionGroup->Add(pControl);
			m_mOptionGroup.Insert(pStrGroupName, aOptionGroup);
		}
		return true;
	}

	CStdPtrArray* CPaintManagerUI::GetOptionGroup(LPCTSTR pStrGroupName)
	{
		LPVOID lp = m_mOptionGroup.Find(pStrGroupName);
		if( lp ) return static_cast<CStdPtrArray*>(lp);
		return NULL;
	}

	void CPaintManagerUI::RemoveOptionGroup(LPCTSTR pStrGroupName, CControlUI* pControl)
	{
		LPVOID lp = m_mOptionGroup.Find(pStrGroupName);
		if( lp ) {
			CStdPtrArray* aOptionGroup = static_cast<CStdPtrArray*>(lp);
			if( aOptionGroup == NULL ) return;
			for( int i = 0; i < aOptionGroup->GetSize(); i++ ) {
				if( static_cast<CControlUI*>(aOptionGroup->GetAt(i)) == pControl ) {
					aOptionGroup->Remove(i);
					break;
				}
			}
			if( aOptionGroup->IsEmpty() ) {
				delete aOptionGroup;
				m_mOptionGroup.Remove(pStrGroupName);
			}
		}
	}

	void CPaintManagerUI::RemoveAllOptionGroups()
	{
		CStdPtrArray* aOptionGroup;
		for( int i = 0; i< m_mOptionGroup.GetSize(); i++ ) {
			if(LPCTSTR key = m_mOptionGroup.GetAt(i)) {
				aOptionGroup = static_cast<CStdPtrArray*>(m_mOptionGroup.Find(key));
				delete aOptionGroup;
			}
		}
		m_mOptionGroup.RemoveAll();
	}

	void CPaintManagerUI::MessageLoop()
	{
		MSG msg = { 0 };
		while( ::GetMessage(&msg, NULL, 0, 0) ) {
			// 消息派发点日志：看右键消息被投给了哪个 hwnd（区分本窗还是其它窗口）
			if( (msg.message == WM_RBUTTONDOWN || msg.message == WM_RBUTTONUP || msg.message == WM_CONTEXTMENU)
				&& CDuiLog::IsEnabled() ) {
				POINT mpt = { GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) };
				HWND mh = ::WindowFromPoint(mpt);
				DUILOG(_T("[dispatch] uMsg=0x%X hwnd=0x%p windowFromPoint=0x%p pt=(%d,%d)"),
					msg.message, (void*)msg.hwnd, (void*)mh, mpt.x, mpt.y);
			}
			if( !CPaintManagerUI::TranslateMessage(&msg) ) {
				::TranslateMessage(&msg);
				try{
					::DispatchMessage(&msg);
				} catch(...) {
					DUITRACE(_T("EXCEPTION: %s(%d)\n"), __FILET__, __LINE__);
#ifdef _DEBUG
					throw "CPaintManagerUI::MessageLoop";
#endif
				}
			}
		}
	}

	void CPaintManagerUI::Term()
	{
		DUI_EXIT_SCOPE(L"Term()");
		{
			DUI_EXIT_SCOPE(L"Term ResourceManager/Factory");
			CResourceManager::GetInstance()->Release();
			CControlFactory::GetInstance()->Release();
		}

		{
			DUI_EXIT_SCOPE(L"Term shared images");
			TImageInfo* data;
			for( int i = 0; i< m_SharedResInfo.m_ImageHash.GetSize(); i++ ) {
				if(LPCTSTR key = m_SharedResInfo.m_ImageHash.GetAt(i)) {
					data = static_cast<TImageInfo*>(m_SharedResInfo.m_ImageHash.Find(key, false));
					if (data) {
						GetRenderDevice()->FreeImage(data);
						data = NULL;
					}
				}
			}
			m_SharedResInfo.m_ImageHash.RemoveAll();
		}
		{
			DUI_EXIT_SCOPE(L"Term shared fonts");
			TFontInfo* pFontInfo;
			for( int i = 0; i< m_SharedResInfo.m_CustomFonts.GetSize(); i++ ) {
				if(LPCTSTR key = m_SharedResInfo.m_CustomFonts.GetAt(i)) {
					pFontInfo = static_cast<TFontInfo*>(m_SharedResInfo.m_CustomFonts.Find(key, false));
					if (pFontInfo) {
						GetRenderDevice()->DestroyNativeFont(pFontInfo);
						delete pFontInfo;
						pFontInfo = NULL;
					}
				}
			}
			m_SharedResInfo.m_CustomFonts.RemoveAll();
			GetRenderDevice()->DestroyNativeFont(&m_SharedResInfo.m_DefaultFontInfo);
		}
		CDuiString* pStyle;
		for( int i = 0; i< m_SharedResInfo.m_StyleHash.GetSize(); i++ ) {
			if(LPCTSTR key = m_SharedResInfo.m_StyleHash.GetAt(i)) {
				pStyle = static_cast<CDuiString*>(m_SharedResInfo.m_StyleHash.Find(key, false));
				if (pStyle) {
					delete pStyle;
					pStyle = NULL;
				}
			}
		}
		m_SharedResInfo.m_StyleHash.RemoveAll();

		// ???
		CDuiString* pAttr;
		for( int i = 0; i< m_SharedResInfo.m_AttrHash.GetSize(); i++ ) {
			if(LPCTSTR key = m_SharedResInfo.m_AttrHash.GetAt(i)) {
				pAttr = static_cast<CDuiString*>(m_SharedResInfo.m_AttrHash.Find(key, false));
				if (pAttr) {
					delete pAttr;
					pAttr = NULL;
				}
			}
		}
		m_SharedResInfo.m_AttrHash.RemoveAll();

		{
			DUI_EXIT_SCOPE(L"Term CloseZip");
			if( m_pResourceZip != NULL ) {
				delete m_pResourceZip;
				m_pResourceZip = NULL;
			}
		}
	}

	CDPI * DuiLib::CPaintManagerUI::GetDPIObj()
	{
		if (m_pDPI == NULL) {
			m_pDPI = new CDPI;
		}
		return m_pDPI;
	}

	void DuiLib::CPaintManagerUI::SetDPI(int iDPI, bool bResizeWindow)
	{
		int scale1 = GetDPIObj()->GetScale();
		GetDPIObj()->SetScale(iDPI);
		int scale2 = GetDPIObj()->GetScale();
		ResetDPIAssets();
		HWND hWnd = GetPaintWindow();
		if( bResizeWindow && hWnd != NULL && scale1 > 0 && scale2 != scale1 && !::IsZoomed(hWnd) ) {
			RECT rcWnd = {0};
			::GetWindowRect(hWnd, &rcWnd);
			RECT rc = rcWnd;
			rc.right = rcWnd.left + (rcWnd.right - rcWnd.left) * scale2 / scale1;
			rc.bottom = rcWnd.top + (rcWnd.bottom - rcWnd.top) * scale2 / scale1;
			::SetWindowPos(hWnd, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);
		}
		// AttachDialog 前可调 SetDPI（OnCreate）；勿用 GetRoot()（会 ASSERT）
		if( GetRootPtr() != NULL ) GetRootPtr()->NeedUpdate();
		// 根未挂上时不 Post UIMSG_SET_DPI，避免应用在 HandleCustomMessage 里 FindControl 踩空根
		if( hWnd != NULL && GetRootPtr() != NULL )
			::PostMessage(hWnd, UIMSG_SET_DPI, 0, 0);
	}

	void DuiLib::CPaintManagerUI::SetAllDPI(int iDPI)
	{
		for (int i = 0; i < m_aPreMessages.GetSize(); i++) {
			CPaintManagerUI* pManager = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);
			pManager->SetDPI(iDPI);
		}
	}

	void DuiLib::CPaintManagerUI::ResetDPIAssets()
	{
		RemoveAllDrawInfos();
		RemoveAllImages();

		for (int it = 0; it < m_ResInfo.m_CustomFonts.GetSize(); it++) {
			TFontInfo* pFontInfo = static_cast<TFontInfo*>(m_ResInfo.m_CustomFonts.Find(m_ResInfo.m_CustomFonts[it]));
			RebuildFont(pFontInfo);
		}
		RebuildFont(&m_ResInfo.m_DefaultFontInfo);

		for (int it = 0; it < m_SharedResInfo.m_CustomFonts.GetSize(); it++) {
			TFontInfo* pFontInfo = static_cast<TFontInfo*>(m_SharedResInfo.m_CustomFonts.Find(m_SharedResInfo.m_CustomFonts[it]));
			RebuildFont(pFontInfo);
		}
		RebuildFont(&m_SharedResInfo.m_DefaultFontInfo);

		// mem 图（AppIcon file=/EXE 等）已随 RemoveAllImages 失效，通知控件重建
		if( GetRootPtr() != NULL ) {
			CStdPtrArray aStack;
			aStack.Add(GetRootPtr());
			while( aStack.GetSize() > 0 ) {
				CControlUI* p = static_cast<CControlUI*>(aStack.GetAt(aStack.GetSize() - 1));
				aStack.Remove(aStack.GetSize() - 1);
				if( p == NULL ) continue;
				p->OnResetDpiAssets();
				CContainerUI* pContainer = static_cast<CContainerUI*>(p->GetInterface(DUI_CTR_CONTAINER));
				if( pContainer == NULL ) continue;
				const int n = pContainer->GetCount();
				for( int i = 0; i < n; ++i )
					aStack.Add(pContainer->GetItemAt(i));
			}
		}

		CControlUI* pRoot = GetRootPtr();
		if( pRoot == NULL ) return;
		CStdPtrArray *richEditList = FindSubControlsByClass(pRoot, _T("RichEditUI"));
		if( richEditList == NULL ) return;
		for (int i = 0; i < richEditList->GetSize(); i++)
		{
			CRichEditUI* pT = static_cast<CRichEditUI*>((*richEditList)[i]);
			if( pT != NULL )
				pT->SetFont(pT->GetFont());
		}
	}

	void DuiLib::CPaintManagerUI::RebuildFont(TFontInfo * pFontInfo)
	{
		if( pFontInfo == NULL ) return;
		GetRenderDevice()->CreateNativeFont(pFontInfo, -GetDPIObj()->Scale(pFontInfo->iSize), m_hDcPaint);
	}

	CControlUI* CPaintManagerUI::GetFocus() const
	{
		return m_pFocus;
	}

	void CPaintManagerUI::SetFocus(CControlUI* pControl)
	{
		// Already has focus?
		if( pControl == m_pFocus ) return;

		// 先清空 m_pFocus，再发 KillFocus / 同步 HWND 焦点。
		// 否则原生 Edit DestroyWindow → paint 收到 WM_SETFOCUS 时仍指向旧控件，
		// DoEvent(SETFOCUS) 会在析构中途重新 new CEditWnd 导致崩溃。
		CControlUI* pOldFocus = m_pFocus;
		m_pFocus = NULL;
		if( pOldFocus != NULL )
		{
			TEventUI event = { 0 };
			event.Type = UIEVENT_KILLFOCUS;
			event.pSender = pControl;
			event.dwTimestamp = ::GetTickCount();
			pOldFocus->Event(event);
			SendNotify(pOldFocus, DUI_MSGTYPE_KILLFOCUS);
		}

		HWND hFocusWnd = ::GetFocus();
		if( hFocusWnd != m_hWndPaint )
			::SetFocus(m_hWndPaint);

		if( pControl == NULL ) {
			return;
		}
		if( pControl->GetManager() == this
			&& pControl->IsVisible()
			&& pControl->IsEnabled() )
		{
			m_pFocus = pControl;
			TEventUI event = { 0 };
			event.Type = UIEVENT_SETFOCUS;
			event.pSender = pControl;
			event.dwTimestamp = ::GetTickCount();
			m_pFocus->Event(event);
			SendNotify(m_pFocus, DUI_MSGTYPE_SETFOCUS);
		}
	}

	void CPaintManagerUI::SetFocusNeeded(CControlUI* pControl)
	{
		CControlUI* pOldFocus = m_pFocus;
		m_pFocus = NULL;
		if( pOldFocus != NULL ) {
			TEventUI event = { 0 };
			event.Type = UIEVENT_KILLFOCUS;
			event.pSender = pControl;
			event.dwTimestamp = ::GetTickCount();
			pOldFocus->Event(event);
			SendNotify(pOldFocus, DUI_MSGTYPE_KILLFOCUS);
		}
		::SetFocus(m_hWndPaint);
		if( pControl == NULL ) return;
		FINDTABINFO info = { 0 };
		info.pFocus = pControl;
		info.bForward = false;
		m_pFocus = m_pRoot->FindControl(__FindControlFromTab, &info, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_ME_FIRST);
		m_bFocusNeeded = true;
		if( m_pRoot != NULL ) m_pRoot->NeedUpdate();
	}

	bool CPaintManagerUI::SetTimer(CControlUI* pControl, UINT nTimerID, UINT uElapse)
	{
		ASSERT(pControl!=NULL);
		ASSERT(uElapse>0);
		for( int i = 0; i< m_aTimers.GetSize(); i++ ) {
			TIMERINFO* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
			if( pTimer->pSender == pControl
				&& pTimer->hWnd == m_hWndPaint
				&& pTimer->nLocalID == nTimerID ) {
					if( pTimer->bKilled == true ) {
						if( ::SetTimer(m_hWndPaint, pTimer->uWinTimer, uElapse, NULL) ) {
							pTimer->bKilled = false;
							return true;
						}
						return false;
					}
					return false;
			}
		}

		m_uTimerID = (++m_uTimerID) % 0xF0; //0xf1-0xfe???????
		if( !::SetTimer(m_hWndPaint, m_uTimerID, uElapse, NULL) ) return FALSE;
		TIMERINFO* pTimer = new TIMERINFO;
		if( pTimer == NULL ) return FALSE;
		pTimer->hWnd = m_hWndPaint;
		pTimer->pSender = pControl;
		pTimer->nLocalID = nTimerID;
		pTimer->uWinTimer = m_uTimerID;
		pTimer->bKilled = false;
		return m_aTimers.Add(pTimer);
	}

	bool CPaintManagerUI::KillTimer(CControlUI* pControl, UINT nTimerID)
	{
		ASSERT(pControl!=NULL);
		for( int i = 0; i< m_aTimers.GetSize(); i++ ) {
			TIMERINFO* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
			if( pTimer->pSender == pControl
				&& pTimer->hWnd == m_hWndPaint
				&& pTimer->nLocalID == nTimerID )
			{
				if( pTimer->bKilled == false ) {
					if( ::IsWindow(m_hWndPaint) ) ::KillTimer(pTimer->hWnd, pTimer->uWinTimer);
					pTimer->bKilled = true;
					return true;
				}
			}
		}
		return false;
	}

	void CPaintManagerUI::KillTimer(CControlUI* pControl)
	{
		ASSERT(pControl!=NULL);
		int count = m_aTimers.GetSize();
		for( int i = 0, j = 0; i < count; i++ ) {
			TIMERINFO* pTimer = static_cast<TIMERINFO*>(m_aTimers[i - j]);
			if( pTimer->pSender == pControl && pTimer->hWnd == m_hWndPaint ) {
				if( pTimer->bKilled == false ) ::KillTimer(pTimer->hWnd, pTimer->uWinTimer);
				delete pTimer;
				m_aTimers.Remove(i - j);
				j++;
			}
		}
	}

	void CPaintManagerUI::RemoveAllTimers()
	{
		for( int i = 0; i < m_aTimers.GetSize(); i++ ) {
			TIMERINFO* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
			if( pTimer->hWnd == m_hWndPaint ) {
				if( pTimer->bKilled == false ) {
					if( ::IsWindow(m_hWndPaint) ) ::KillTimer(m_hWndPaint, pTimer->uWinTimer);
				}
				delete pTimer;
			}
		}

		m_aTimers.Empty();
	}

	void CPaintManagerUI::SetCapture()
	{
		::SetCapture(m_hWndPaint);
		m_bMouseCapture = true;
	}

	void CPaintManagerUI::ReleaseCapture()
	{
		::ReleaseCapture();
		m_bMouseCapture = false;
		m_bDragMode = false;
	}

	bool CPaintManagerUI::IsCaptured()
	{
		return m_bMouseCapture;
	}

	bool CPaintManagerUI::IsPainting()
	{
		return m_bIsPainting;
	}

	void CPaintManagerUI::SetPainting(bool bIsPainting)
	{
		m_bIsPainting = bIsPainting;
	}

	bool CPaintManagerUI::SetNextTabControl(bool bForward)
	{
		// If we're in the process of restructuring the layout we can delay the
		// focus calulation until the next repaint.
		if( m_bUpdateNeeded && bForward ) {
			m_bFocusNeeded = true;
			::InvalidateRect(m_hWndPaint, NULL, FALSE);
			return true;
		}
		// Find next/previous tabbable control
		FINDTABINFO info1 = { 0 };
		info1.pFocus = m_pFocus;
		info1.bForward = bForward;
		CControlUI* pControl = m_pRoot->FindControl(__FindControlFromTab, &info1, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_ME_FIRST);
		if( pControl == NULL ) {  
			if( bForward ) {
				// Wrap around
				FINDTABINFO info2 = { 0 };
				info2.pFocus = bForward ? NULL : info1.pLast;
				info2.bForward = bForward;
				pControl = m_pRoot->FindControl(__FindControlFromTab, &info2, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_ME_FIRST);
			}
			else {
				pControl = info1.pLast;
			}
		}
		if( pControl != NULL ) SetFocus(pControl);
		m_bFocusNeeded = false;
		return true;
	}

	bool CPaintManagerUI::AddNotifier(INotifyUI* pNotifier)
	{
		ASSERT(m_aNotifiers.Find(pNotifier)<0);
		return m_aNotifiers.Add(pNotifier);
	}

	bool CPaintManagerUI::RemoveNotifier(INotifyUI* pNotifier)
	{
		for( int i = 0; i < m_aNotifiers.GetSize(); i++ ) {
			if( static_cast<INotifyUI*>(m_aNotifiers[i]) == pNotifier ) {
				return m_aNotifiers.Remove(i);
			}
		}
		return false;
	}

	bool CPaintManagerUI::AddPreMessageFilter(IMessageFilterUI* pFilter)
	{
		ASSERT(m_aPreMessageFilters.Find(pFilter)<0);
		return m_aPreMessageFilters.Add(pFilter);
	}

	bool CPaintManagerUI::RemovePreMessageFilter(IMessageFilterUI* pFilter)
	{
		for( int i = 0; i < m_aPreMessageFilters.GetSize(); i++ ) {
			if( static_cast<IMessageFilterUI*>(m_aPreMessageFilters[i]) == pFilter ) {
				return m_aPreMessageFilters.Remove(i);
			}
		}
		return false;
	}

	bool CPaintManagerUI::AddMessageFilter(IMessageFilterUI* pFilter)
	{
		ASSERT(m_aMessageFilters.Find(pFilter)<0);
		return m_aMessageFilters.Add(pFilter);
	}

	bool CPaintManagerUI::RemoveMessageFilter(IMessageFilterUI* pFilter)
	{
		for( int i = 0; i < m_aMessageFilters.GetSize(); i++ ) {
			if( static_cast<IMessageFilterUI*>(m_aMessageFilters[i]) == pFilter ) {
				return m_aMessageFilters.Remove(i);
			}
		}
		return false;
	}

	int CPaintManagerUI::GetPostPaintCount() const
	{
		return m_aPostPaintControls.GetSize();
	}

	bool CPaintManagerUI::IsPostPaint(CControlUI* pControl)
	{
		return m_aPostPaintControls.Find(pControl) >= 0;
	}

	bool CPaintManagerUI::AddPostPaint(CControlUI* pControl)
	{
		ASSERT(m_aPostPaintControls.Find(pControl) < 0);
		return m_aPostPaintControls.Add(pControl);
	}

	bool CPaintManagerUI::RemovePostPaint(CControlUI* pControl)
	{
		for( int i = 0; i < m_aPostPaintControls.GetSize(); i++ ) {
			if( static_cast<CControlUI*>(m_aPostPaintControls[i]) == pControl ) {
				return m_aPostPaintControls.Remove(i);
			}
		}
		return false;
	}

	bool CPaintManagerUI::SetPostPaintIndex(CControlUI* pControl, int iIndex)
	{
		RemovePostPaint(pControl);
		return m_aPostPaintControls.InsertAt(iIndex, pControl);
	}

	int CPaintManagerUI::GetNativeWindowCount() const
	{
		return m_aNativeWindow.GetSize();
	}

	bool CPaintManagerUI::AddNativeWindow(CControlUI* pControl, HWND hChildWnd)
	{
		if (pControl == NULL || hChildWnd == NULL) return false;

		RECT rcChildWnd = GetNativeWindowRect(hChildWnd);
		Invalidate(rcChildWnd);

		if (m_aNativeWindow.Find(hChildWnd) >= 0) return false;
		if (m_aNativeWindow.Add(hChildWnd)) {
			m_aNativeWindowControl.Add(pControl);
			return true;
		}
		return false;
	}

	bool CPaintManagerUI::RemoveNativeWindow(HWND hChildWnd)
	{
		for( int i = 0; i < m_aNativeWindow.GetSize(); i++ ) {
			if( static_cast<HWND>(m_aNativeWindow[i]) == hChildWnd ) {
				if( m_aNativeWindow.Remove(i) ) {
					m_aNativeWindowControl.Remove(i);
					return true;
				}
				return false;
			}
		}
		return false;
	}

	RECT CPaintManagerUI::GetNativeWindowRect(HWND hChildWnd)
	{
		RECT rcChildWnd = { 0 };
		GetChildWndRect(m_hWndPaint, hChildWnd, rcChildWnd);
		return rcChildWnd;
	}

	void CPaintManagerUI::AddDelayedCleanup(CControlUI* pControl)
	{
		if (pControl == NULL) return;
		// 入队前清掉 focus/hover 等引用，避免延迟删除窗口期 m_pFocus 仍指向已摘树控件。
		ReapObjects(pControl);
		pControl->SetManager(this, NULL, false);
		m_aDelayedCleanup.Add(pControl);
		PostAsyncNotify();
	}

	void CPaintManagerUI::AddMouseLeaveNeeded(CControlUI* pControl)
	{
		if (pControl == NULL) return;
		for( int i = 0; i < m_aNeedMouseLeaveNeeded.GetSize(); i++ ) {
			if( static_cast<CControlUI*>(m_aNeedMouseLeaveNeeded[i]) == pControl ) {
				return;
			}
		}
		m_aNeedMouseLeaveNeeded.Add(pControl);
	}

	bool CPaintManagerUI::RemoveMouseLeaveNeeded(CControlUI* pControl)
	{
		if (pControl == NULL) return false;
		for( int i = 0; i < m_aNeedMouseLeaveNeeded.GetSize(); i++ ) {
			if( static_cast<CControlUI*>(m_aNeedMouseLeaveNeeded[i]) == pControl ) {
				return m_aNeedMouseLeaveNeeded.Remove(i);
			}
		}
		return false;
	}

	void CPaintManagerUI::SendNotify(CControlUI* pControl, LPCTSTR pstrMessage, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/, bool bAsync /*= false*/)
	{
		TNotifyUI Msg;
		Msg.pSender = pControl;
		Msg.sType = pstrMessage;
		Msg.wParam = wParam;
		Msg.lParam = lParam;
		SendNotify(Msg, bAsync);
	}

	void CPaintManagerUI::SendNotify(TNotifyUI& Msg, bool bAsync /*= false*/)
	{
		Msg.ptMouse = m_ptLastMousePos;
		// ptScreen 统一填 ptMouse 的屏幕坐标版：任意通知都可信，控件/空白菜单弹菜单用同一坐标。
		Msg.ptScreen = m_ptLastMousePos;
		if( m_hWndPaint != NULL && ::IsWindow(m_hWndPaint) )
			::ClientToScreen(m_hWndPaint, &Msg.ptScreen);
		Msg.dwTimestamp = ::GetTickCount();
		if( m_bUsedVirtualWnd )
		{
			Msg.sVirtualWnd = Msg.pSender->GetVirtualWnd();
		}

		if( !bAsync ) {
			// Send to all listeners
			if( Msg.pSender != NULL && !::IsBadReadPtr(Msg.pSender, sizeof(void*)) ) {
				DUILOG(_T("[notify] before OnNotify sender=%s(%s)"),
					Msg.pSender->GetClass(),
					!(Msg.pSender->GetName().IsEmpty()) ? Msg.pSender->GetName().GetData() : _T("(no-name)"));
				if( Msg.pSender->OnNotify ) {
					Msg.pSender->OnNotify(&Msg);
					DUILOG(_T("[notify] after OnNotify"));
				}
				else {
					DUILOG(_T("[notify] OnNotify is null, skip"));
				}
			}
			int nNotifiers = m_aNotifiers.GetSize();
			DUILOG(_T("[notify] broadcasting to %d notifiers"), nNotifiers);
			for( int i = 0; i < nNotifiers; i++ ) {
				INotifyUI* pN = static_cast<INotifyUI*>(m_aNotifiers[i]);
				// 粗查指针可读性，明显非法（垂悬/错位）则跳过，避免拖垮整个消息派发
				if( pN == NULL || ::IsBadReadPtr(pN, sizeof(void*)) ) {
					DUILOG(_T("[notify] notifier[%d]=0x%p invalid, skip"), i, (void*)pN);
					continue;
				}
				DUILOG(_T("[notify] notifier[%d]=0x%p -> Notify"), i, (void*)pN);
				pN->Notify(Msg);
				DUILOG(_T("[notify] notifier[%d] done"), i);
			}
			DUILOG(_T("[notify] broadcast done"));
		}
		else {
			TNotifyUI *pMsg = new TNotifyUI;
			pMsg->sVirtualWnd = Msg.sVirtualWnd;
			pMsg->pSender = Msg.pSender;
			pMsg->sType = Msg.sType;
			pMsg->wParam = Msg.wParam;
			pMsg->lParam = Msg.lParam;
			pMsg->ptMouse = Msg.ptMouse;
			pMsg->dwTimestamp = Msg.dwTimestamp;
			m_aAsyncNotify.Add(pMsg);

			PostAsyncNotify();
		}
	}

	bool CPaintManagerUI::IsForceUseSharedRes() const
	{
		return m_bForceUseSharedRes;
	}

	void CPaintManagerUI::SetForceUseSharedRes(bool bForce)
	{
		m_bForceUseSharedRes = bForce;
	}

	DWORD CPaintManagerUI::GetDefaultDisabledColor() const
	{
		return m_ResInfo.m_dwDefaultDisabledColor;
	}

	void CPaintManagerUI::SetDefaultDisabledColor(DWORD dwColor, bool bShared)
	{
		if (bShared)
		{
			if (m_ResInfo.m_dwDefaultDisabledColor == m_SharedResInfo.m_dwDefaultDisabledColor)
				m_ResInfo.m_dwDefaultDisabledColor = dwColor;
			m_SharedResInfo.m_dwDefaultDisabledColor = dwColor;
		}
		else
		{
			m_ResInfo.m_dwDefaultDisabledColor = dwColor;
		}
	}

	DWORD CPaintManagerUI::GetDefaultFontColor() const
	{
		return m_ResInfo.m_dwDefaultFontColor;
	}

	void CPaintManagerUI::SetDefaultFontColor(DWORD dwColor, bool bShared)
	{
		if (bShared)
		{
			if (m_ResInfo.m_dwDefaultFontColor == m_SharedResInfo.m_dwDefaultFontColor)
				m_ResInfo.m_dwDefaultFontColor = dwColor;
			m_SharedResInfo.m_dwDefaultFontColor = dwColor;
		}
		else
		{
			m_ResInfo.m_dwDefaultFontColor = dwColor;
		}
	}

	DWORD CPaintManagerUI::GetDefaultLinkFontColor() const
	{
		return m_ResInfo.m_dwDefaultLinkFontColor;
	}

	void CPaintManagerUI::SetDefaultLinkFontColor(DWORD dwColor, bool bShared)
	{
		if (bShared)
		{
			if (m_ResInfo.m_dwDefaultLinkFontColor == m_SharedResInfo.m_dwDefaultLinkFontColor)
				m_ResInfo.m_dwDefaultLinkFontColor = dwColor;
			m_SharedResInfo.m_dwDefaultLinkFontColor = dwColor;
		}
		else
		{
			m_ResInfo.m_dwDefaultLinkFontColor = dwColor;
		}
	}

	DWORD CPaintManagerUI::GetDefaultLinkHoverFontColor() const
	{
		return m_ResInfo.m_dwDefaultLinkHoverFontColor;
	}

	void CPaintManagerUI::SetDefaultLinkHoverFontColor(DWORD dwColor, bool bShared)
	{
		if (bShared)
		{
			if (m_ResInfo.m_dwDefaultLinkHoverFontColor == m_SharedResInfo.m_dwDefaultLinkHoverFontColor)
				m_ResInfo.m_dwDefaultLinkHoverFontColor = dwColor;
			m_SharedResInfo.m_dwDefaultLinkHoverFontColor = dwColor;
		}
		else
		{
			m_ResInfo.m_dwDefaultLinkHoverFontColor = dwColor;
		}
	}

	DWORD CPaintManagerUI::GetDefaultSelectedBackgroundColor() const
	{
		return m_ResInfo.m_dwDefaultSelectedBackgroundColor;
	}

	void CPaintManagerUI::SetDefaultSelectedBackgroundColor(DWORD dwColor, bool bShared)
	{
		if (bShared)
		{
			if (m_ResInfo.m_dwDefaultSelectedBackgroundColor == m_SharedResInfo.m_dwDefaultSelectedBackgroundColor)
				m_ResInfo.m_dwDefaultSelectedBackgroundColor = dwColor;
			m_SharedResInfo.m_dwDefaultSelectedBackgroundColor = dwColor;
		}
		else
		{
			m_ResInfo.m_dwDefaultSelectedBackgroundColor = dwColor;
		}
	}

	TFontInfo* CPaintManagerUI::GetDefaultFontInfo()
	{
		if (m_ResInfo.m_DefaultFontInfo.sFontName.IsEmpty())
		{
			if( m_SharedResInfo.m_DefaultFontInfo.tm.tmHeight == 0 ) 
			{
				HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, m_SharedResInfo.m_DefaultFontInfo.hFont);
				::GetTextMetrics(m_hDcPaint, &m_SharedResInfo.m_DefaultFontInfo.tm);
				::SelectObject(m_hDcPaint, hOldFont);
			}
			return &m_SharedResInfo.m_DefaultFontInfo;
		}
		else
		{
			if( m_ResInfo.m_DefaultFontInfo.tm.tmHeight == 0 ) 
			{
				HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, m_ResInfo.m_DefaultFontInfo.hFont);
				::GetTextMetrics(m_hDcPaint, &m_ResInfo.m_DefaultFontInfo.tm);
				::SelectObject(m_hDcPaint, hOldFont);
			}
			return &m_ResInfo.m_DefaultFontInfo;
		}
	}

	void CPaintManagerUI::SetDefaultFont(LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bStrikeout, bool bShared)
	{
		TFontInfo* pDefault = bShared ? &m_SharedResInfo.m_DefaultFontInfo : &m_ResInfo.m_DefaultFontInfo;
		pDefault->sFontName = (pStrFontName != NULL) ? pStrFontName : _T("");
		pDefault->iSize = nSize;
		pDefault->bBold = bBold;
		pDefault->bUnderline = bUnderline;
		pDefault->bItalic = bItalic;
		pDefault->bStrikeout = bStrikeout;
		GetRenderDevice()->CreateNativeFont(pDefault, -GetDPIObj()->Scale(nSize), m_hDcPaint);
	}

	DWORD CPaintManagerUI::GetCustomFontCount(bool bShared) const
	{
		if (bShared)
			return m_SharedResInfo.m_CustomFonts.GetSize();
		else
			return m_ResInfo.m_CustomFonts.GetSize();
	}

	HFONT CPaintManagerUI::AddFont(int id, LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bStrikeout, bool bShared)
	{
		TFontInfo* pFontInfo = new TFontInfo;
		if( !pFontInfo ) return NULL;
		pFontInfo->hFont = NULL;
		pFontInfo->pBackend = NULL;
		pFontInfo->nBackend = RENDER_BACKEND_GDI;
		pFontInfo->sFontName = (pStrFontName != NULL) ? pStrFontName : _T("");
		pFontInfo->iSize = nSize;
		pFontInfo->bBold = bBold;
		pFontInfo->bUnderline = bUnderline;
		pFontInfo->bItalic = bItalic;
		pFontInfo->bStrikeout = bStrikeout;
		::ZeroMemory(&pFontInfo->tm, sizeof(pFontInfo->tm));

		if( !GetRenderDevice()->CreateNativeFont(pFontInfo, -GetDPIObj()->Scale(nSize), m_hDcPaint) ) {
			delete pFontInfo;
			return NULL;
		}
		HFONT hFont = pFontInfo->hFont;

		TCHAR idBuffer[16];
		::ZeroMemory(idBuffer, sizeof(idBuffer));
		_itot(id, idBuffer, 10);
		if (bShared || m_bForceUseSharedRes)
		{
			TFontInfo* pOldFontInfo = static_cast<TFontInfo*>(m_SharedResInfo.m_CustomFonts.Find(idBuffer));
			if (pOldFontInfo)
			{
				GetRenderDevice()->DestroyNativeFont(pOldFontInfo);
				delete pOldFontInfo;
				m_SharedResInfo.m_CustomFonts.Remove(idBuffer);
			}

			if( !m_SharedResInfo.m_CustomFonts.Insert(idBuffer, pFontInfo) ) 
			{
				GetRenderDevice()->DestroyNativeFont(pFontInfo);
				delete pFontInfo;
				return NULL;
			}
		}
		else
		{
			TFontInfo* pOldFontInfo = static_cast<TFontInfo*>(m_ResInfo.m_CustomFonts.Find(idBuffer));
			if (pOldFontInfo)
			{
				GetRenderDevice()->DestroyNativeFont(pOldFontInfo);
				delete pOldFontInfo;
				m_ResInfo.m_CustomFonts.Remove(idBuffer);
			}

			if( !m_ResInfo.m_CustomFonts.Insert(idBuffer, pFontInfo) ) 
			{
				GetRenderDevice()->DestroyNativeFont(pFontInfo);
				delete pFontInfo;
				return NULL;
			}
		}

		return hFont;
	}

	int CPaintManagerUI::EnsureFont(LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bStrikeout, bool bShared)
	{
		if( pStrFontName == NULL || *pStrFontName == _T('\0') || nSize <= 0 ) return -1;
		int idx = GetFontIndex(pStrFontName, nSize, bBold, bUnderline, bItalic, bStrikeout, false);
		if( idx < 0 ) idx = GetFontIndex(pStrFontName, nSize, bBold, bUnderline, bItalic, bStrikeout, true);
		if( idx >= 0 ) return idx;

		int newId = 10000;
		for( int i = 0; i < m_ResInfo.m_CustomFonts.GetSize(); ++i ) {
			if( LPCTSTR key = m_ResInfo.m_CustomFonts.GetAt(i) ) {
				int id = _ttoi(key);
				if( id >= newId ) newId = id + 1;
			}
		}
		for( int i = 0; i < m_SharedResInfo.m_CustomFonts.GetSize(); ++i ) {
			if( LPCTSTR key = m_SharedResInfo.m_CustomFonts.GetAt(i) ) {
				int id = _ttoi(key);
				if( id >= newId ) newId = id + 1;
			}
		}
		if( AddFont(newId, pStrFontName, nSize, bBold, bUnderline, bItalic, bStrikeout, bShared) == NULL )
			return -1;
		return newId;
	}

	void CPaintManagerUI::AddFontArray(LPCTSTR pstrPath) {
		LPBYTE pData = NULL;
		DWORD dwSize = 0;
		if( !CPaintManagerUI::LoadResourceData(pstrPath, &pData, &dwSize) || pData == NULL || dwSize == 0 )
			return;

		DWORD nFonts;
		HANDLE hFont = ::AddFontMemResourceEx(pData, dwSize, NULL, &nFonts);
		delete[] pData;
		pData = NULL;
		m_aFonts.Add(hFont);
	}
	HFONT CPaintManagerUI::GetFont(int id)
	{
		if (id < 0) return GetDefaultFontInfo()->hFont;

		TCHAR idBuffer[16];
		::ZeroMemory(idBuffer, sizeof(idBuffer));
		_itot(id, idBuffer, 10);
		TFontInfo* pFontInfo = static_cast<TFontInfo*>(m_ResInfo.m_CustomFonts.Find(idBuffer));
		if( !pFontInfo ) pFontInfo = static_cast<TFontInfo*>(m_SharedResInfo.m_CustomFonts.Find(idBuffer));
		if (!pFontInfo) return GetDefaultFontInfo()->hFont;
		return pFontInfo->hFont;
	}

	HFONT CPaintManagerUI::GetFont(LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bStrikeout)
	{
		TFontInfo* pFontInfo = NULL;
		for( int i = 0; i< m_ResInfo.m_CustomFonts.GetSize(); i++ ) {
			if(LPCTSTR key = m_ResInfo.m_CustomFonts.GetAt(i)) {
				pFontInfo = static_cast<TFontInfo*>(m_ResInfo.m_CustomFonts.Find(key));
				if (pFontInfo && pFontInfo->sFontName == pStrFontName && pFontInfo->iSize == nSize && 
					pFontInfo->bBold == bBold && pFontInfo->bUnderline == bUnderline && pFontInfo->bItalic == bItalic
					&& pFontInfo->bStrikeout == bStrikeout)
					return pFontInfo->hFont;
			}
		}
		for( int i = 0; i< m_SharedResInfo.m_CustomFonts.GetSize(); i++ ) {
			if(LPCTSTR key = m_SharedResInfo.m_CustomFonts.GetAt(i)) {
				pFontInfo = static_cast<TFontInfo*>(m_SharedResInfo.m_CustomFonts.Find(key));
				if (pFontInfo && pFontInfo->sFontName == pStrFontName && pFontInfo->iSize == nSize && 
					pFontInfo->bBold == bBold && pFontInfo->bUnderline == bUnderline && pFontInfo->bItalic == bItalic
					&& pFontInfo->bStrikeout == bStrikeout)
					return pFontInfo->hFont;
			}
		}

		return NULL;
	}

	int CPaintManagerUI::GetFontIndex(HFONT hFont, bool bShared)
	{
		TFontInfo* pFontInfo = NULL;
		if (bShared)
		{
			for( int i = 0; i< m_SharedResInfo.m_CustomFonts.GetSize(); i++ ) {
				if(LPCTSTR key = m_SharedResInfo.m_CustomFonts.GetAt(i)) {
					pFontInfo = static_cast<TFontInfo*>(m_SharedResInfo.m_CustomFonts.Find(key));
					if (pFontInfo && pFontInfo->hFont == hFont) return _ttoi(key);
				}
			}
		}
		else
		{
			for( int i = 0; i< m_ResInfo.m_CustomFonts.GetSize(); i++ ) {
				if(LPCTSTR key = m_ResInfo.m_CustomFonts.GetAt(i)) {
					pFontInfo = static_cast<TFontInfo*>(m_ResInfo.m_CustomFonts.Find(key));
					if (pFontInfo && pFontInfo->hFont == hFont) return _ttoi(key);
				}
			}
		}

		return -1;
	}

	int CPaintManagerUI::GetFontIndex(LPCTSTR pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bStrikeout, bool bShared)
	{
		TFontInfo* pFontInfo = NULL;
		if (bShared)
		{
			for( int i = 0; i< m_SharedResInfo.m_CustomFonts.GetSize(); i++ ) {
				if(LPCTSTR key = m_SharedResInfo.m_CustomFonts.GetAt(i)) {
					pFontInfo = static_cast<TFontInfo*>(m_SharedResInfo.m_CustomFonts.Find(key));
					if (pFontInfo && pFontInfo->sFontName == pStrFontName && pFontInfo->iSize == nSize && 
						pFontInfo->bBold == bBold && pFontInfo->bUnderline == bUnderline && pFontInfo->bItalic == bItalic
						&& pFontInfo->bStrikeout == bStrikeout)
						return _ttoi(key);
				}
			}
		}
		else
		{
			for( int i = 0; i< m_ResInfo.m_CustomFonts.GetSize(); i++ ) {
				if(LPCTSTR key = m_ResInfo.m_CustomFonts.GetAt(i)) {
					pFontInfo = static_cast<TFontInfo*>(m_ResInfo.m_CustomFonts.Find(key));
					if (pFontInfo && pFontInfo->sFontName == pStrFontName && pFontInfo->iSize == nSize && 
						pFontInfo->bBold == bBold && pFontInfo->bUnderline == bUnderline && pFontInfo->bItalic == bItalic
						&& pFontInfo->bStrikeout == bStrikeout)
						return _ttoi(key);
				}
			}
		}

		return -1;
	}

	void CPaintManagerUI::RemoveFont(HFONT hFont, bool bShared)
	{
		TFontInfo* pFontInfo = NULL;
		if (bShared)
		{
			for( int i = 0; i < m_SharedResInfo.m_CustomFonts.GetSize(); i++ ) 
			{
				if(LPCTSTR key = m_SharedResInfo.m_CustomFonts.GetAt(i)) 
				{
					pFontInfo = static_cast<TFontInfo*>(m_SharedResInfo.m_CustomFonts.Find(key));
					if (pFontInfo && pFontInfo->hFont == hFont) 
					{
						GetRenderDevice()->DestroyNativeFont(pFontInfo);
						delete pFontInfo;
						m_SharedResInfo.m_CustomFonts.Remove(key);
						return;
					}
				}
			}
		}
		else
		{
			for( int i = 0; i < m_ResInfo.m_CustomFonts.GetSize(); i++ ) 
			{
				if(LPCTSTR key = m_ResInfo.m_CustomFonts.GetAt(i)) 
				{
					pFontInfo = static_cast<TFontInfo*>(m_ResInfo.m_CustomFonts.Find(key));
					if (pFontInfo && pFontInfo->hFont == hFont) 
					{
						GetRenderDevice()->DestroyNativeFont(pFontInfo);
						delete pFontInfo;
						m_ResInfo.m_CustomFonts.Remove(key);
						return;
					}
				}
			}
		}
	}

	void CPaintManagerUI::RemoveFont(int id, bool bShared)
	{
		TCHAR idBuffer[16];
		::ZeroMemory(idBuffer, sizeof(idBuffer));
		_itot(id, idBuffer, 10);

		TFontInfo* pFontInfo = NULL;
		if (bShared)
		{
			pFontInfo = static_cast<TFontInfo*>(m_SharedResInfo.m_CustomFonts.Find(idBuffer));
			if (pFontInfo)
			{
				GetRenderDevice()->DestroyNativeFont(pFontInfo);
				delete pFontInfo;
				m_SharedResInfo.m_CustomFonts.Remove(idBuffer);
			}
		}
		else
		{
			pFontInfo = static_cast<TFontInfo*>(m_ResInfo.m_CustomFonts.Find(idBuffer));
			if (pFontInfo)
			{
				GetRenderDevice()->DestroyNativeFont(pFontInfo);
				delete pFontInfo;
				m_ResInfo.m_CustomFonts.Remove(idBuffer);
			}
		}
	}

	void CPaintManagerUI::RemoveAllFonts(bool bShared)
	{
		TFontInfo* pFontInfo;
		if (bShared)
		{
			for( int i = 0; i< m_SharedResInfo.m_CustomFonts.GetSize(); i++ ) {
				if(LPCTSTR key = m_SharedResInfo.m_CustomFonts.GetAt(i)) {
					pFontInfo = static_cast<TFontInfo*>(m_SharedResInfo.m_CustomFonts.Find(key, false));
					if (pFontInfo) {
						GetRenderDevice()->DestroyNativeFont(pFontInfo);
						delete pFontInfo;
					}
				}
			}
			m_SharedResInfo.m_CustomFonts.RemoveAll();
		}
		else
		{
			for( int i = 0; i< m_ResInfo.m_CustomFonts.GetSize(); i++ ) {
				if(LPCTSTR key = m_ResInfo.m_CustomFonts.GetAt(i)) {
					pFontInfo = static_cast<TFontInfo*>(m_ResInfo.m_CustomFonts.Find(key, false));
					if (pFontInfo) {
						GetRenderDevice()->DestroyNativeFont(pFontInfo);
						delete pFontInfo;
					}
				}
			}
			m_ResInfo.m_CustomFonts.RemoveAll();
		}
	}

	TFontInfo* CPaintManagerUI::GetFontInfo(int id)
	{
		if (id < 0) return GetDefaultFontInfo();

		TCHAR idBuffer[16];
		::ZeroMemory(idBuffer, sizeof(idBuffer));
		_itot(id, idBuffer, 10);
		TFontInfo* pFontInfo = static_cast<TFontInfo*>(m_ResInfo.m_CustomFonts.Find(idBuffer));
		if (!pFontInfo) pFontInfo = static_cast<TFontInfo*>(m_SharedResInfo.m_CustomFonts.Find(idBuffer));
		if (!pFontInfo) pFontInfo = GetDefaultFontInfo();
		if (pFontInfo->tm.tmHeight == 0) 
		{
			HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, pFontInfo->hFont);
			::GetTextMetrics(m_hDcPaint, &pFontInfo->tm);
			::SelectObject(m_hDcPaint, hOldFont);
		}
		return pFontInfo;
	}

	TFontInfo* CPaintManagerUI::GetFontInfo(HFONT hFont)
	{
		TFontInfo* pFontInfo = NULL;
		for( int i = 0; i< m_ResInfo.m_CustomFonts.GetSize(); i++ ) 
		{
			if(LPCTSTR key = m_ResInfo.m_CustomFonts.GetAt(i)) 
			{
				pFontInfo = static_cast<TFontInfo*>(m_ResInfo.m_CustomFonts.Find(key));
				if (pFontInfo && pFontInfo->hFont == hFont) break;
			}
		}
		if (!pFontInfo)
		{
			for( int i = 0; i< m_SharedResInfo.m_CustomFonts.GetSize(); i++ ) 
			{
				if(LPCTSTR key = m_SharedResInfo.m_CustomFonts.GetAt(i)) 
				{
					pFontInfo = static_cast<TFontInfo*>(m_SharedResInfo.m_CustomFonts.Find(key));
					if (pFontInfo && pFontInfo->hFont == hFont) break;
				}
			}
		}
		if (!pFontInfo) pFontInfo = GetDefaultFontInfo();
		if( pFontInfo->tm.tmHeight == 0 ) {
			HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, pFontInfo->hFont);
			::GetTextMetrics(m_hDcPaint, &pFontInfo->tm);
			::SelectObject(m_hDcPaint, hOldFont);
		}
		return pFontInfo;
	}

	const TImageInfo* CPaintManagerUI::GetImage(LPCTSTR bitmap)
	{
		TImageInfo* data = static_cast<TImageInfo*>(m_ResInfo.m_ImageHash.Find(bitmap));
		if( !data ) data = static_cast<TImageInfo*>(m_SharedResInfo.m_ImageHash.Find(bitmap));
		return data;
	}

	const TImageInfo* CPaintManagerUI::GetImageEx(LPCTSTR bitmap, LPCTSTR type, DWORD mask, bool bUseHSL, bool bGdiplus, HINSTANCE instance)
	{
		const TImageInfo* data = GetImage(bitmap);
		if( !data ) {
			if( AddImage(bitmap, type, mask, bUseHSL, bGdiplus, false, instance) ) {
				if (m_bForceUseSharedRes) data = static_cast<TImageInfo*>(m_SharedResInfo.m_ImageHash.Find(bitmap));
				else data = static_cast<TImageInfo*>(m_ResInfo.m_ImageHash.Find(bitmap)); 
			}
		}

		return data;
	}

	const TImageInfo* CPaintManagerUI::AddImage(LPCTSTR bitmap, LPCTSTR type, DWORD mask, bool bUseHSL, bool bGdiplus, bool bShared, HINSTANCE instance)
	{
		if( bitmap == NULL || bitmap[0] == _T('\0') ) return NULL;

		TImageInfo* data = NULL;
		if( type != NULL && lstrlen(type) > 0) {
			if( isdigit(*bitmap) ) {
				LPTSTR pstr = NULL;
				int iIndex = _tcstol(bitmap, &pstr, 10);
				
				data = bGdiplus ? GetRenderDevice()->GdiplusLoadImage(iIndex, type, mask, instance) : GetRenderDevice()->LoadImage(iIndex, type, mask, instance);
			}
		}
		else {
			data = bGdiplus ? GetRenderDevice()->GdiplusLoadImage(bitmap, NULL, mask, instance) : GetRenderDevice()->LoadImage(bitmap, NULL, mask, instance);
			if(!data) {
				CDuiString sImageName = bitmap;
				int iAtIdx = sImageName.ReverseFind(_T('@'));
				int iDotIdx = sImageName.ReverseFind(_T('.'));
				if(iAtIdx != -1 && iDotIdx != -1) {
					CDuiString sExe = sImageName.Mid(iDotIdx);
					sImageName = sImageName.Left(iAtIdx) + sExe;
					data = bGdiplus ? GetRenderDevice()->GdiplusLoadImage(sImageName.GetData(), NULL, mask, instance) : GetRenderDevice()->LoadImage(sImageName.GetData(), NULL, mask, instance);
				}
			}

		}

		if( data == NULL ) {
			return NULL;
		}
		data->bUseHSL = bUseHSL;
		if( type != NULL ) data->sResType = type;
		data->dwMask = mask;
		if( data->bUseHSL ) {
			data->pSrcBits = new BYTE[data->nX * data->nY * 4];
			::CopyMemory(data->pSrcBits, data->pBits, data->nX * data->nY * 4);
		}
		else data->pSrcBits = NULL;
		if( m_bUseHSL ) CRenderEngine::AdjustImage(true, data, m_H, m_S, m_L);
		if (data)
		{
			if (bShared || m_bForceUseSharedRes)
			{
				TImageInfo* pOldImageInfo = static_cast<TImageInfo*>(m_SharedResInfo.m_ImageHash.Find(bitmap));
				if (pOldImageInfo)
				{
					GetRenderDevice()->FreeImage(pOldImageInfo);
					m_SharedResInfo.m_ImageHash.Remove(bitmap);
				}

				if( !m_SharedResInfo.m_ImageHash.Insert(bitmap, data) ) {
					GetRenderDevice()->FreeImage(data);
					data = NULL;
				}
			}
			else
			{
				TImageInfo* pOldImageInfo = static_cast<TImageInfo*>(m_ResInfo.m_ImageHash.Find(bitmap));
				if (pOldImageInfo)
				{
					GetRenderDevice()->FreeImage(pOldImageInfo);
					m_ResInfo.m_ImageHash.Remove(bitmap);
				}

				if( !m_ResInfo.m_ImageHash.Insert(bitmap, data) ) {
					GetRenderDevice()->FreeImage(data);
					data = NULL;
				}
			}
		}

		return data;
	}

	const TImageInfo* CPaintManagerUI::AddImage(LPCTSTR bitmap, HBITMAP hBitmap, int iWidth, int iHeight, bool bAlpha, bool bShared)
	{
		// ??????????HBITMAP????????????hsl????
		if( bitmap == NULL || bitmap[0] == _T('\0') ) return NULL;
		if( hBitmap == NULL || iWidth <= 0 || iHeight <= 0 ) return NULL;

		TImageInfo* data = new TImageInfo;
		data->pBits = NULL;
		data->pSrcBits = NULL;
		data->hBitmap = hBitmap;
		data->pImage = NULL;
		data->pBackend = NULL;
		data->nBackend = RENDER_BACKEND_GDI;
		data->pBits = NULL;
		data->nX = iWidth;
		data->nY = iHeight;
		data->bAlpha = bAlpha;
		data->bUseHSL = false;
		data->pSrcBits = NULL;
		data->dwMask = 0;

		if (bShared || m_bForceUseSharedRes)
		{
			if( !m_SharedResInfo.m_ImageHash.Insert(bitmap, data) ) {
				GetRenderDevice()->FreeImage(data);
				data = NULL;
			}
		}
		else
		{
			if( !m_ResInfo.m_ImageHash.Insert(bitmap, data) ) {
				GetRenderDevice()->FreeImage(data);
				data = NULL;
			}
		}

		return data;
	}

	void CPaintManagerUI::RemoveImage(LPCTSTR bitmap, bool bShared)
	{
		TImageInfo* data = NULL;
		if (bShared) 
		{
			data = static_cast<TImageInfo*>(m_SharedResInfo.m_ImageHash.Find(bitmap));
			if (data)
			{
				GetRenderDevice()->FreeImage(data) ;
				m_SharedResInfo.m_ImageHash.Remove(bitmap);
			}
		}
		else
		{
			data = static_cast<TImageInfo*>(m_ResInfo.m_ImageHash.Find(bitmap));
			if (data)
			{
				GetRenderDevice()->FreeImage(data) ;
				m_ResInfo.m_ImageHash.Remove(bitmap);
			}
		}
	}

	void CPaintManagerUI::RemoveAllImages(bool bShared)
	{
		if (bShared)
		{
			TImageInfo* data;
			for( int i = 0; i< m_SharedResInfo.m_ImageHash.GetSize(); i++ ) {
				if(LPCTSTR key = m_SharedResInfo.m_ImageHash.GetAt(i)) {
					data = static_cast<TImageInfo*>(m_SharedResInfo.m_ImageHash.Find(key, false));
					if (data) {
						GetRenderDevice()->FreeImage(data);
					}
				}
			}
			m_SharedResInfo.m_ImageHash.RemoveAll();
		}
		else
		{
			TImageInfo* data;
			for( int i = 0; i< m_ResInfo.m_ImageHash.GetSize(); i++ ) {
				if(LPCTSTR key = m_ResInfo.m_ImageHash.GetAt(i)) {
					data = static_cast<TImageInfo*>(m_ResInfo.m_ImageHash.Find(key, false));
					if (data) {
						GetRenderDevice()->FreeImage(data);
					}
				}
			}
			m_ResInfo.m_ImageHash.RemoveAll();
		}
	}

	void CPaintManagerUI::AdjustSharedImagesHSL()
	{
		TImageInfo* data;
		for( int i = 0; i< m_SharedResInfo.m_ImageHash.GetSize(); i++ ) {
			if(LPCTSTR key = m_SharedResInfo.m_ImageHash.GetAt(i)) {
				data = static_cast<TImageInfo*>(m_SharedResInfo.m_ImageHash.Find(key));
				if( data && data->bUseHSL ) {
					CRenderEngine::AdjustImage(m_bUseHSL, data, m_H, m_S, m_L);
				}
			}
		}
	}

	void CPaintManagerUI::AdjustImagesHSL()
	{
		TImageInfo* data;
		for( int i = 0; i< m_ResInfo.m_ImageHash.GetSize(); i++ ) {
			if(LPCTSTR key = m_ResInfo.m_ImageHash.GetAt(i)) {
				data = static_cast<TImageInfo*>(m_ResInfo.m_ImageHash.Find(key));
				if( data && data->bUseHSL ) {
					CRenderEngine::AdjustImage(m_bUseHSL, data, m_H, m_S, m_L);
				}
			}
		}
		Invalidate();
	}

	void CPaintManagerUI::PostAsyncNotify()
	{
		if (!m_bAsyncNotifyPosted) {
			::PostMessage(m_hWndPaint, UIMSG_ASYNC_NOTIFY, 0, 0L);
			m_bAsyncNotifyPosted = true;
		}
	}
	void CPaintManagerUI::ReloadSharedImages()
	{
		TImageInfo* data = NULL;
		TImageInfo* pNewData = NULL;
		for( int i = 0; i< m_SharedResInfo.m_ImageHash.GetSize(); i++ ) {
			if(LPCTSTR bitmap = m_SharedResInfo.m_ImageHash.GetAt(i)) {
				data = static_cast<TImageInfo*>(m_SharedResInfo.m_ImageHash.Find(bitmap));
				if( data != NULL ) {
					if( !data->sResType.IsEmpty() ) {
						if( isdigit(*bitmap) ) {
							LPTSTR pstr = NULL;
							int iIndex = _tcstol(bitmap, &pstr, 10);
							pNewData = GetRenderDevice()->LoadImage(iIndex, data->sResType.GetData(), data->dwMask);
						}
					}
					else {
						pNewData = GetRenderDevice()->LoadImage(bitmap, NULL, data->dwMask);
					}
					if( pNewData == NULL ) continue;

					GetRenderDevice()->FreeImage(data, false);
					data->hBitmap = pNewData->hBitmap;
					data->pImage = pNewData->pImage;
					data->pBits = pNewData->pBits;
					data->nX = pNewData->nX;
					data->nY = pNewData->nY;
					data->bAlpha = pNewData->bAlpha;
					data->pSrcBits = NULL;
					if( data->bUseHSL ) {
						data->pSrcBits = new BYTE[data->nX * data->nY * 4];
						::CopyMemory(data->pSrcBits, data->pBits, data->nX * data->nY * 4);
					}
					else data->pSrcBits = NULL;
					if( m_bUseHSL ) CRenderEngine::AdjustImage(true, data, m_H, m_S, m_L);

					delete pNewData;
				}
			}
		}
	}

	void CPaintManagerUI::ReloadImages()
	{
		RemoveAllDrawInfos();

		TImageInfo* data = NULL;
		TImageInfo* pNewData = NULL;
		for( int i = 0; i< m_ResInfo.m_ImageHash.GetSize(); i++ ) {
			if(LPCTSTR bitmap = m_ResInfo.m_ImageHash.GetAt(i)) {
				data = static_cast<TImageInfo*>(m_ResInfo.m_ImageHash.Find(bitmap));
				if( data != NULL ) {
					if( !data->sResType.IsEmpty() ) {
						if( isdigit(*bitmap) ) {
							LPTSTR pstr = NULL;
							int iIndex = _tcstol(bitmap, &pstr, 10);
							pNewData = GetRenderDevice()->LoadImage(iIndex, data->sResType.GetData(), data->dwMask);
						}
					}
					else {
						pNewData = GetRenderDevice()->LoadImage(bitmap, NULL, data->dwMask);
					}

					GetRenderDevice()->FreeImage(data, false);
					if( pNewData == NULL ) {
						m_ResInfo.m_ImageHash.Remove(bitmap);
						continue;
					}
					data->hBitmap = pNewData->hBitmap;
					data->pBits = pNewData->pBits;
					data->nX = pNewData->nX;
					data->nY = pNewData->nY;
					data->bAlpha = pNewData->bAlpha;
					data->pSrcBits = NULL;
					if( data->bUseHSL ) {
						data->pSrcBits = new BYTE[data->nX * data->nY * 4];
						::CopyMemory(data->pSrcBits, data->pBits, data->nX * data->nY * 4);
					}
					else data->pSrcBits = NULL;
					if( m_bUseHSL ) CRenderEngine::AdjustImage(true, data, m_H, m_S, m_L);

					delete pNewData;
				}
			}
		}

		if( m_pRoot ) m_pRoot->Invalidate();
	}

	const TDrawInfo* CPaintManagerUI::GetDrawInfo(LPCTSTR pStrImage, LPCTSTR pStrModify)
	{
		CDuiString sKey;
		sKey.Format(_T("%s%s"), pStrImage == NULL ? _T("") : pStrImage, pStrModify == NULL ? _T("") : pStrModify);
		TDrawInfo* pDrawInfo = static_cast<TDrawInfo*>(m_ResInfo.m_DrawInfoHash.Find(sKey));
		if(pDrawInfo == NULL && !sKey.IsEmpty()) {
			pDrawInfo = new TDrawInfo();
			pDrawInfo->Parse(pStrImage, pStrModify,this);
			m_ResInfo.m_DrawInfoHash.Insert(sKey, pDrawInfo);
		}
		return pDrawInfo;
	}

	void CPaintManagerUI::RemoveDrawInfo(LPCTSTR pStrImage, LPCTSTR pStrModify)
	{
		CDuiString sKey;
		sKey.Format(_T("%s%s"), pStrImage == NULL ? _T("") : pStrImage, pStrModify == NULL ? _T("") : pStrModify);
		TDrawInfo* pDrawInfo = static_cast<TDrawInfo*>(m_ResInfo.m_DrawInfoHash.Find(sKey));
		if(pDrawInfo != NULL) {
			m_ResInfo.m_DrawInfoHash.Remove(sKey);
			delete pDrawInfo;
			pDrawInfo = NULL;
		}
	}

	void CPaintManagerUI::RemoveAllDrawInfos()
	{
		TDrawInfo* pDrawInfo = NULL;
		for( int i = 0; i< m_ResInfo.m_DrawInfoHash.GetSize(); i++ ) {
			LPCTSTR key = m_ResInfo.m_DrawInfoHash.GetAt(i);
			if(key != NULL) {
				pDrawInfo = static_cast<TDrawInfo*>(m_ResInfo.m_DrawInfoHash.Find(key, false));
				if (pDrawInfo) {
					delete pDrawInfo;
					pDrawInfo = NULL;
				}
			}
		}
		m_ResInfo.m_DrawInfoHash.RemoveAll();
	}

	void CPaintManagerUI::AddDefaultAttributeList(LPCTSTR pStrControlName, LPCTSTR pStrControlAttrList, bool bShared)
	{
		if (bShared || m_bForceUseSharedRes)
		{
			CDuiString* pDefaultAttr = new CDuiString(pStrControlAttrList);
			if (pDefaultAttr != NULL)
			{
				CDuiString* pOldDefaultAttr = static_cast<CDuiString*>(m_SharedResInfo.m_AttrHash.Set(pStrControlName, (LPVOID)pDefaultAttr));
				if (pOldDefaultAttr) delete pOldDefaultAttr;
			}
		}
		else
		{
			CDuiString* pDefaultAttr = new CDuiString(pStrControlAttrList);
			if (pDefaultAttr != NULL)
			{
				CDuiString* pOldDefaultAttr = static_cast<CDuiString*>(m_ResInfo.m_AttrHash.Set(pStrControlName, (LPVOID)pDefaultAttr));
				if (pOldDefaultAttr) delete pOldDefaultAttr;
			}
		}
	}

	LPCTSTR CPaintManagerUI::GetDefaultAttributeList(LPCTSTR pStrControlName) const
	{
		CDuiString* pDefaultAttr = static_cast<CDuiString*>(m_ResInfo.m_AttrHash.Find(pStrControlName));
		if( !pDefaultAttr ) pDefaultAttr = static_cast<CDuiString*>(m_SharedResInfo.m_AttrHash.Find(pStrControlName));
		if (pDefaultAttr) return pDefaultAttr->GetData();
		return NULL;
	}

	bool CPaintManagerUI::RemoveDefaultAttributeList(LPCTSTR pStrControlName, bool bShared)
	{
		if (bShared)
		{
			CDuiString* pDefaultAttr = static_cast<CDuiString*>(m_SharedResInfo.m_AttrHash.Find(pStrControlName));
			if( !pDefaultAttr ) return false;

			delete pDefaultAttr;
			return m_SharedResInfo.m_AttrHash.Remove(pStrControlName);
		}
		else
		{
			CDuiString* pDefaultAttr = static_cast<CDuiString*>(m_ResInfo.m_AttrHash.Find(pStrControlName));
			if( !pDefaultAttr ) return false;

			delete pDefaultAttr;
			return m_ResInfo.m_AttrHash.Remove(pStrControlName);
		}
	}

	void CPaintManagerUI::RemoveAllDefaultAttributeList(bool bShared)
	{
		if (bShared)
		{
			CDuiString* pDefaultAttr;
			for( int i = 0; i< m_SharedResInfo.m_AttrHash.GetSize(); i++ ) {
				if(LPCTSTR key = m_SharedResInfo.m_AttrHash.GetAt(i)) {
					pDefaultAttr = static_cast<CDuiString*>(m_SharedResInfo.m_AttrHash.Find(key));
					if (pDefaultAttr) delete pDefaultAttr;
				}
			}
			m_SharedResInfo.m_AttrHash.RemoveAll();
		}
		else
		{
			CDuiString* pDefaultAttr;
			for( int i = 0; i< m_ResInfo.m_AttrHash.GetSize(); i++ ) {
				if(LPCTSTR key = m_ResInfo.m_AttrHash.GetAt(i)) {
					pDefaultAttr = static_cast<CDuiString*>(m_ResInfo.m_AttrHash.Find(key));
					if (pDefaultAttr) delete pDefaultAttr;
				}
			}
			m_ResInfo.m_AttrHash.RemoveAll();
		}
	}

	void CPaintManagerUI::AddWindowCustomAttribute(LPCTSTR pstrName, LPCTSTR pstrAttr)
	{
		if( pstrName == NULL || pstrName[0] == _T('\0') || pstrAttr == NULL || pstrAttr[0] == _T('\0') ) return;
		CDuiString* pCostomAttr = new CDuiString(pstrAttr);
		if (pCostomAttr != NULL) {
			if (m_mWindowCustomAttrHash.Find(pstrName) == NULL)
				m_mWindowCustomAttrHash.Set(pstrName, (LPVOID)pCostomAttr);
			else
				delete pCostomAttr;
		}
	}

	LPCTSTR CPaintManagerUI::GetWindowCustomAttribute(LPCTSTR pstrName) const
	{
		if( pstrName == NULL || pstrName[0] == _T('\0') ) return NULL;
		CDuiString* pCostomAttr = static_cast<CDuiString*>(m_mWindowCustomAttrHash.Find(pstrName));
		if( pCostomAttr ) return pCostomAttr->GetData();
		return NULL;
	}

	bool CPaintManagerUI::RemoveWindowCustomAttribute(LPCTSTR pstrName)
	{
		if( pstrName == NULL || pstrName[0] == _T('\0') ) return NULL;
		CDuiString* pCostomAttr = static_cast<CDuiString*>(m_mWindowCustomAttrHash.Find(pstrName));
		if( !pCostomAttr ) return false;

		delete pCostomAttr;
		return m_mWindowCustomAttrHash.Remove(pstrName);
	}

	void CPaintManagerUI::RemoveAllWindowCustomAttribute()
	{
		CDuiString* pCostomAttr;
		for( int i = 0; i< m_mWindowCustomAttrHash.GetSize(); i++ ) {
			if(LPCTSTR key = m_mWindowCustomAttrHash.GetAt(i)) {
				pCostomAttr = static_cast<CDuiString*>(m_mWindowCustomAttrHash.Find(key));
				delete pCostomAttr;
			}
		}
		m_mWindowCustomAttrHash.Resize();
	}

	CControlUI* CPaintManagerUI::GetRoot() const
	{
		ASSERT(m_pRoot);
		return m_pRoot;
	}

	void CPaintManagerUI::SetBlankContextMenuEnabled(bool bEnable)
	{
		m_bBlankCtxMenu = bEnable;
	}

	bool CPaintManagerUI::IsBlankContextMenuEnabled() const
	{
		return m_bBlankCtxMenu;
	}

	void CPaintManagerUI::SetBlankContextMenuUseDeepestContainer(bool bUse)
	{
		m_bBlankCtxMenuDeepest = bUse;
	}

	bool CPaintManagerUI::IsBlankContextMenuUseDeepestContainer() const
	{
		return m_bBlankCtxMenuDeepest;
	}

	CControlUI* CPaintManagerUI::FindControl(POINT pt) const
	{
		ASSERT(m_pRoot);
		return m_pRoot->FindControl(__FindControlFromPoint, &pt, UIFIND_VISIBLE | UIFIND_HITTEST | UIFIND_TOP_FIRST);
	}

	// 递归求 pt 处最内层（最深）可见容器。忽略 IsMouseEnabled，纯按矩形+可见性判定；
	// 只返回容器（叶子控件不算），无更深命中时返回覆盖 pt 的自身容器。
	static CControlUI* __FindDeepestContainerAtRec(CControlUI* pCtrl, POINT pt)
	{
		if( pCtrl == NULL ) return NULL;
		if( !pCtrl->IsVisible() ) return NULL;
		if( !::PtInRect(&pCtrl->GetPos(), pt) ) return NULL;

		IContainerUI* pC = static_cast<IContainerUI*>(pCtrl->GetInterface(_T("IContainer")));
		if( pC != NULL ) {
			// 优先深层的容器命中；TOP_FIRST 语义：最后加入的在上层，最内层命中优先
			for( int it = pC->GetCount() - 1; it >= 0; it-- ) {
				CControlUI* pDeep = __FindDeepestContainerAtRec(pC->GetItemAt(it), pt);
				if( pDeep != NULL ) return pDeep;
			}
			// 子项无更深容器命中 → 返回自身（此容器覆盖 pt）
			return pCtrl;
		}
		// 叶子控件：交由父容器的回退处理，自身不作为“容器”返回
		return NULL;
	}

	CControlUI* CPaintManagerUI::FindDeepestContainerAt(POINT pt) const
	{
		if( m_pRoot == NULL ) return NULL;
		return __FindDeepestContainerAtRec(m_pRoot, pt);
	}

	CControlUI* CPaintManagerUI::FindControl(LPCTSTR pstrName) const
	{
		// AttachDialog 前 / 根已拆除时允许按名查找：返回 NULL，勿 ASSERT
		//（OnCreate 里 SetDPI 会 PostMessage UIMSG_SET_DPI；阴影 CreateWindow 等可能重入派发）
		if( m_pRoot == NULL ) return NULL;
		return static_cast<CControlUI*>(m_mNameHash.Find(pstrName));
	}

	CControlUI* CPaintManagerUI::FindSubControlByPoint(CControlUI* pParent, POINT pt) const
	{
		if( pParent == NULL ) pParent = GetRoot();
		ASSERT(pParent);
		return pParent->FindControl(__FindControlFromPoint, &pt, UIFIND_VISIBLE | UIFIND_HITTEST | UIFIND_TOP_FIRST);
	}

	CControlUI* CPaintManagerUI::FindSubControlByName(CControlUI* pParent, LPCTSTR pstrName) const
	{
		if( pParent == NULL ) pParent = GetRoot();
		ASSERT(pParent);
		return pParent->FindControl(__FindControlFromName, (LPVOID)pstrName, UIFIND_ALL);
	}

	CControlUI* CPaintManagerUI::FindSubControlByClass(CControlUI* pParent, LPCTSTR pstrClass, int iIndex)
	{
		if( pParent == NULL ) pParent = GetRoot();
		ASSERT(pParent);
		m_aFoundControls.Resize(iIndex + 1);
		return pParent->FindControl(__FindControlFromClass, (LPVOID)pstrClass, UIFIND_ALL);
	}

	CStdPtrArray* CPaintManagerUI::FindSubControlsByClass(CControlUI* pParent, LPCTSTR pstrClass)
	{
		if( pParent == NULL ) pParent = GetRoot();
		ASSERT(pParent);
		m_aFoundControls.Empty();
		pParent->FindControl(__FindControlsFromClass, (LPVOID)pstrClass, UIFIND_ALL);
		return &m_aFoundControls;
	}

	CStdPtrArray* CPaintManagerUI::GetFoundControls()
	{
		return &m_aFoundControls;
	}

	CControlUI* CALLBACK CPaintManagerUI::__FindControlFromNameHash(CControlUI* pThis, LPVOID pData)
	{
		CPaintManagerUI* pManager = static_cast<CPaintManagerUI*>(pData);
		const CDuiString& sName = pThis->GetName();
		if( sName.IsEmpty() ) return NULL;
		// Add this control to the hash list
		pManager->m_mNameHash.Set(sName, pThis);
		return NULL; // Attempt to add all controls
	}

	CControlUI* CALLBACK CPaintManagerUI::__FindControlFromCount(CControlUI* /*pThis*/, LPVOID pData)
	{
		int* pnCount = static_cast<int*>(pData);
		(*pnCount)++;
		return NULL;  // Count all controls
	}

	CControlUI* CALLBACK CPaintManagerUI::__FindControlFromPoint(CControlUI* pThis, LPVOID pData)
	{
		LPPOINT pPoint = static_cast<LPPOINT>(pData);
		return ::PtInRect(&pThis->GetPos(), *pPoint) ? pThis : NULL;
	}

	CControlUI* CALLBACK CPaintManagerUI::__FindControlFromTab(CControlUI* pThis, LPVOID pData)
	{
		FINDTABINFO* pInfo = static_cast<FINDTABINFO*>(pData);
		if( pInfo->pFocus == pThis ) {
			if( pInfo->bForward ) pInfo->bNextIsIt = true;
			return pInfo->bForward ? NULL : pInfo->pLast;
		}
		if( (pThis->GetControlFlags() & UIFLAG_TABSTOP) == 0 ) return NULL;
		pInfo->pLast = pThis;
		if( pInfo->bNextIsIt ) return pThis;
		if( pInfo->pFocus == NULL ) return pThis;
		return NULL;  // Examine all controls
	}

	CControlUI* CALLBACK CPaintManagerUI::__FindControlFromShortcut(CControlUI* pThis, LPVOID pData)
	{
		if( !pThis->IsVisible() ) return NULL; 
		FINDSHORTCUT* pFS = static_cast<FINDSHORTCUT*>(pData);
		if( pFS->ch == toupper(pThis->GetShortcut()) ) pFS->bPickNext = true;
		if( _tcsstr(pThis->GetClass(), _T("LabelUI")) != NULL ) return NULL;   // Labels never get focus!
		return pFS->bPickNext ? pThis : NULL;
	}

	CControlUI* CALLBACK CPaintManagerUI::__FindControlFromName(CControlUI* pThis, LPVOID pData)
	{
		LPCTSTR pstrName = static_cast<LPCTSTR>(pData);
		const CDuiString& sName = pThis->GetName();
		if( sName.IsEmpty() ) return NULL;
		return (_tcsicmp(sName.GetData(), pstrName) == 0) ? pThis : NULL;
	}

	CControlUI* CALLBACK CPaintManagerUI::__FindControlFromClass(CControlUI* pThis, LPVOID pData)
	{
		LPCTSTR pstrType = static_cast<LPCTSTR>(pData);
		LPCTSTR pType = pThis->GetClass();
		CStdPtrArray* pFoundControls = pThis->GetManager()->GetFoundControls();
		if( _tcscmp(pstrType, _T("*")) == 0 || _tcscmp(pstrType, pType) == 0 ) {
			int iIndex = -1;
			while( pFoundControls->GetAt(++iIndex) != NULL ) ;
			if( iIndex < pFoundControls->GetSize() ) pFoundControls->SetAt(iIndex, pThis);
		}
		if( pFoundControls->GetAt(pFoundControls->GetSize() - 1) != NULL ) return pThis; 
		return NULL;
	}

	CControlUI* CALLBACK CPaintManagerUI::__FindControlsFromClass(CControlUI* pThis, LPVOID pData)
	{
		LPCTSTR pstrType = static_cast<LPCTSTR>(pData);
		LPCTSTR pType = pThis->GetClass();
		if( _tcscmp(pstrType, _T("*")) == 0 || _tcscmp(pstrType, pType) == 0 ) 
			pThis->GetManager()->GetFoundControls()->Add((LPVOID)pThis);
		return NULL;
	}

	CControlUI* CALLBACK CPaintManagerUI::__FindControlsFromUpdate(CControlUI* pThis, LPVOID pData)
	{
		if( pThis->IsUpdateNeeded() ) {
			pThis->GetManager()->GetFoundControls()->Add((LPVOID)pThis);
			return pThis;
		}
		return NULL;
	}

	bool CPaintManagerUI::TranslateAccelerator(LPMSG pMsg)
	{
		for (int i = 0; i < m_aTranslateAccelerator.GetSize(); i++)
		{
			LRESULT lResult = static_cast<ITranslateAccelerator *>(m_aTranslateAccelerator[i])->TranslateAccelerator(pMsg);
			if( lResult == S_OK ) return true;
		}
		return false;
	}

	bool CPaintManagerUI::TranslateMessage(const LPMSG pMsg)
	{
		// Pretranslate Message takes care of system-wide messages, such as
		// tabbing and shortcut key-combos. We'll look for all messages for
		// each window and any child control attached.
		UINT uStyle = GetWindowStyle(pMsg->hwnd);
		UINT uChildRes = uStyle & WS_CHILD;	
		LRESULT lRes = 0;
		if (uChildRes != 0)
		{
			HWND hWndParent = ::GetParent(pMsg->hwnd);

			for( int i = 0; i < m_aPreMessages.GetSize(); i++ ) 
			{
				CPaintManagerUI* pT = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);        
				HWND hTempParent = hWndParent;
				while(hTempParent)
				{
					if(pMsg->hwnd == pT->GetPaintWindow() || hTempParent == pT->GetPaintWindow())
					{
						if (pT->TranslateAccelerator(pMsg))
							return true;

						// 与顶层窗口一致：PreMessageFilter 处理成功则吞掉消息（否则子 Edit 收键时 PageUp 等无法翻页）
						if( pT->PreMessageHandler(pMsg->message, pMsg->wParam, pMsg->lParam, lRes) )
							return true;
					}
					hTempParent = GetParent(hTempParent);
				}
			}
		}
		else
		{
			for( int i = 0; i < m_aPreMessages.GetSize(); i++ ) 
			{
				CPaintManagerUI* pT = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);
				if(pMsg->hwnd == pT->GetPaintWindow())
				{
					if (pT->TranslateAccelerator(pMsg))
						return true;

					if( pT->PreMessageHandler(pMsg->message, pMsg->wParam, pMsg->lParam, lRes) ) 
						return true;

					return false;
				}
			}
		}
		return false;
	}

	bool CPaintManagerUI::AddTranslateAccelerator(ITranslateAccelerator *pTranslateAccelerator)
	{
		ASSERT(m_aTranslateAccelerator.Find(pTranslateAccelerator) < 0);
		return m_aTranslateAccelerator.Add(pTranslateAccelerator);
	}

	bool CPaintManagerUI::RemoveTranslateAccelerator(ITranslateAccelerator *pTranslateAccelerator)
	{
		for (int i = 0; i < m_aTranslateAccelerator.GetSize(); i++)
		{
			if (static_cast<ITranslateAccelerator *>(m_aTranslateAccelerator[i]) == pTranslateAccelerator)
			{
				return m_aTranslateAccelerator.Remove(i);
			}
		}
		return false;
	}

	void CPaintManagerUI::UsedVirtualWnd(bool bUsed)
	{
		m_bUsedVirtualWnd = bUsed;
	}

	// ???????
	void CPaintManagerUI::AddStyle(LPCTSTR pName, LPCTSTR pDeclarationList, bool bShared)
	{
		CDuiString* pStyle = new CDuiString(pDeclarationList);

		if(bShared || m_bForceUseSharedRes){
			if( !m_SharedResInfo.m_StyleHash.Insert(pName, pStyle) ) {
				delete pStyle;
			}
		}
		else
		{
			if( !m_ResInfo.m_StyleHash.Insert(pName, pStyle) ) {
				delete pStyle;
			}
		}
	}

	LPCTSTR CPaintManagerUI::GetStyle(LPCTSTR pName) const
	{
		CDuiString* pStyle = static_cast<CDuiString*>(m_ResInfo.m_StyleHash.Find(pName));
		if( !pStyle ) pStyle = static_cast<CDuiString*>(m_SharedResInfo.m_StyleHash.Find(pName));
		if( pStyle ) return pStyle->GetData();
		else return NULL;
	}

	BOOL CPaintManagerUI::RemoveStyle(LPCTSTR pName, bool bShared)
	{
		CDuiString* pStyle = NULL;
		if (bShared) 
		{
			pStyle = static_cast<CDuiString*>(m_SharedResInfo.m_StyleHash.Find(pName));
			if (pStyle)
			{
				delete pStyle;
				m_SharedResInfo.m_StyleHash.Remove(pName);
			}
		}
		else
		{
			pStyle = static_cast<CDuiString*>(m_ResInfo.m_StyleHash.Find(pName));
			if (pStyle)
			{
				delete pStyle;
				m_ResInfo.m_StyleHash.Remove(pName);
			}
		}
		return true;
	}

	const CStdStringPtrMap& CPaintManagerUI::GetStyles(bool bShared) const
	{
		if(bShared) return m_SharedResInfo.m_StyleHash;
		else return m_ResInfo.m_StyleHash;
	}

	void CPaintManagerUI::RemoveAllStyle(bool bShared)
	{
		if (bShared)
		{
			CDuiString* pStyle;
			for( int i = 0; i< m_SharedResInfo.m_StyleHash.GetSize(); i++ ) {
				if(LPCTSTR key = m_SharedResInfo.m_StyleHash.GetAt(i)) {
					pStyle = static_cast<CDuiString*>(m_SharedResInfo.m_StyleHash.Find(key));
					delete pStyle;
				}
			}
			m_SharedResInfo.m_StyleHash.RemoveAll();
		}
		else
		{
			CDuiString* pStyle;
			for( int i = 0; i< m_ResInfo.m_StyleHash.GetSize(); i++ ) {
				if(LPCTSTR key = m_ResInfo.m_StyleHash.GetAt(i)) {
					pStyle = static_cast<CDuiString*>(m_ResInfo.m_StyleHash.Find(key));
					delete pStyle;
				}
			}
			m_ResInfo.m_StyleHash.RemoveAll();
		}
	}

	void CPaintManagerUI::AddCssRule(LPCTSTR pstrSelector, LPCTSTR pstrAttrList)
	{
		CDuiString sSelector = pstrSelector;
		sSelector.Trim();
		if (sSelector.IsEmpty() || pstrAttrList == NULL || *pstrAttrList == _T('\0')) return;

		CStdStringPtrMap* pMap = NULL;
		CDuiString sKey;
		if (sSelector[0] == _T('#')) {
			sKey = sSelector.Mid(1);
			sKey.MakeLower();
			pMap = &m_ResInfo.m_CssIdRules;
		}
		else {
			sKey = sSelector;
			sKey.MakeLower();
			pMap = &m_ResInfo.m_CssTypeRules;
		}

		CDuiString* pExisting = static_cast<CDuiString*>(pMap->Find(sKey));
		if (pExisting != NULL) {
			// 同选择器合并；后写属性排在后面，ApplyAttributeList 时覆盖同名
			if (!pExisting->IsEmpty()) *pExisting += _T(' ');
			*pExisting += pstrAttrList;
			return;
		}

		CDuiString* pAttr = new CDuiString(pstrAttrList);
		if (!pMap->Insert(sKey, pAttr)) {
			delete pAttr;
		}
	}

	LPCTSTR CPaintManagerUI::GetCssTypeRule(LPCTSTR pstrType) const
	{
		CDuiString sType = pstrType;
		sType.MakeLower();
		CDuiString* pAttr = static_cast<CDuiString*>(m_ResInfo.m_CssTypeRules.Find(sType));
		if (pAttr) return pAttr->GetData();
		return NULL;
	}

	LPCTSTR CPaintManagerUI::GetCssIdRule(LPCTSTR pstrId) const
	{
		CDuiString sId = pstrId;
		sId.MakeLower();
		CDuiString* pAttr = static_cast<CDuiString*>(m_ResInfo.m_CssIdRules.Find(sId));
		if (pAttr) return pAttr->GetData();
		return NULL;
	}

	void CPaintManagerUI::RemoveAllCssRules()
	{
		CDuiString* pAttr;
		for (int i = 0; i < m_ResInfo.m_CssTypeRules.GetSize(); i++) {
			if (LPCTSTR key = m_ResInfo.m_CssTypeRules.GetAt(i)) {
				pAttr = static_cast<CDuiString*>(m_ResInfo.m_CssTypeRules.Find(key));
				delete pAttr;
			}
		}
		m_ResInfo.m_CssTypeRules.RemoveAll();

		for (int i = 0; i < m_ResInfo.m_CssIdRules.GetSize(); i++) {
			if (LPCTSTR key = m_ResInfo.m_CssIdRules.GetAt(i)) {
				pAttr = static_cast<CDuiString*>(m_ResInfo.m_CssIdRules.Find(key));
				delete pAttr;
			}
		}
		m_ResInfo.m_CssIdRules.RemoveAll();
	}

	const TImageInfo* CPaintManagerUI::GetImageString(LPCTSTR pStrImage, LPCTSTR pStrModify)
	{
		CDuiString sImageName = pStrImage;
		CDuiString sImageResType = _T("");
		DWORD dwMask = 0;
		CDuiString sItem;
		CDuiString sValue;
		LPTSTR pstr = NULL;

		for( int i = 0; i < 2; ++i) {
			if( i == 1)
				pStrImage = pStrModify;

			if( !pStrImage ) continue;

			while( *pStrImage != _T('\0') ) {
				sItem.Empty();
				sValue.Empty();
				while( *pStrImage > _T('\0') && *pStrImage <= _T(' ') ) pStrImage = ::CharNext(pStrImage);
				while( *pStrImage != _T('\0') && *pStrImage != _T('=') && *pStrImage > _T(' ') ) {
					LPTSTR pstrTemp = ::CharNext(pStrImage);
					while( pStrImage < pstrTemp) {
						sItem += *pStrImage++;
					}
				}
				while( *pStrImage > _T('\0') && *pStrImage <= _T(' ') ) pStrImage = ::CharNext(pStrImage);
				if( *pStrImage++ != _T('=') ) break;
				while( *pStrImage > _T('\0') && *pStrImage <= _T(' ') ) pStrImage = ::CharNext(pStrImage);
				if( *pStrImage++ != _T('\'') ) break;
				while( *pStrImage != _T('\0') && *pStrImage != _T('\'') ) {
					LPTSTR pstrTemp = ::CharNext(pStrImage);
					while( pStrImage < pstrTemp) {
						sValue += *pStrImage++;
					}
				}
				if( *pStrImage++ != _T('\'') ) break;
				if( !sValue.IsEmpty() ) {
					if( sItem == _T("file") || sItem == _T("res") ) {
						sImageName = sValue;
					}
					else if( sItem == _T("restype") ) {
						sImageResType = sValue;
					}
					else if( sItem == _T("mask") ) 
					{
						if( sValue[0] == _T('#')) dwMask = _tcstoul(sValue.GetData() + 1, &pstr, 16);
						else dwMask = _tcstoul(sValue.GetData(), &pstr, 16);
					}

				}
				if( *pStrImage++ != _T(' ') ) break;
			}
		}
		return GetImageEx(sImageName.GetData(), sImageResType.GetData(), dwMask);
	}

	bool CPaintManagerUI::EnableDragDrop(bool bEnable)
	{
		if(m_bDragDrop == bEnable) return false;
		m_bDragDrop = bEnable;

		if(bEnable) {
			AddRef();

			if(FAILED(RegisterDragDrop(m_hWndPaint, this))) {
				return false;
			}

			SetTargetWnd(m_hWndPaint);

			FORMATETC ftetc={0};
			ftetc.cfFormat = CF_BITMAP;
			ftetc.dwAspect = DVASPECT_CONTENT;
			ftetc.lindex = -1;
			ftetc.tymed = TYMED_GDI;
			AddSuportedFormat(ftetc);
			ftetc.cfFormat = CF_DIB;
			ftetc.tymed = TYMED_HGLOBAL;
			AddSuportedFormat(ftetc);
			ftetc.cfFormat = CF_HDROP;
			ftetc.tymed = TYMED_HGLOBAL;
			AddSuportedFormat(ftetc);
			ftetc.cfFormat = CF_ENHMETAFILE;
			ftetc.tymed = TYMED_ENHMF;
			AddSuportedFormat(ftetc);
		}
		else{
			Release();
			if(FAILED(RevokeDragDrop(m_hWndPaint))) {
				return false;
			}
		}
		return true;
	}

	void CPaintManagerUI::SetDragDrop(IDragDropUI* pDragDrop)
	{
		m_pDragDrop = pDragDrop;
	}

	static WORD DIBNumColors(void* pv) 
	{     
		int bits;     
		LPBITMAPINFOHEADER  lpbi;     
		LPBITMAPCOREHEADER  lpbc;      
		lpbi = ((LPBITMAPINFOHEADER)pv);     
		lpbc = ((LPBITMAPCOREHEADER)pv);      
		/*  With the BITMAPINFO format headers, the size of the palette 
		*  is in biClrUsed, whereas in the BITMAPCORE - style headers, it      
		*  is dependent on the bits per pixel ( = 2 raised to the power of      
		*  bits/pixel).
		*/     
		if (lpbi->biSize != sizeof(BITMAPCOREHEADER))
		{         
			if (lpbi->biClrUsed != 0)
				return (WORD)lpbi->biClrUsed;         
			bits = lpbi->biBitCount;     
		}     
		else         
			bits = lpbc->bcBitCount;
		switch (bits)
		{         
		case 1:                 
			return 2;         
		case 4:                 
			return 16;         
		case 8:       
			return 256;
		default:
			/* A 24 bitcount DIB has no color table */                 
			return 0;
		} 
	} 
	//code taken from SEEDIB MSDN sample
	static WORD ColorTableSize(LPVOID lpv)
	{
		LPBITMAPINFOHEADER lpbih = (LPBITMAPINFOHEADER)lpv;

		if (lpbih->biSize != sizeof(BITMAPCOREHEADER))
		{
			if (((LPBITMAPINFOHEADER)(lpbih))->biCompression == BI_BITFIELDS)
				/* Remember that 16/32bpp dibs can still have a color table */
				return (sizeof(DWORD) * 3) + (DIBNumColors (lpbih) * sizeof (RGBQUAD));
			else
				return (WORD)(DIBNumColors (lpbih) * sizeof (RGBQUAD));
		}
		else
			return (WORD)(DIBNumColors (lpbih) * sizeof (RGBTRIPLE));
	}

	bool CPaintManagerUI::OnDrop(FORMATETC* pFmtEtc, STGMEDIUM& medium,DWORD *pdwEffect)
	{
		POINT ptMouse = {0};
		GetCursorPos(&ptMouse);
		::SendMessage(m_hTargetWnd, WM_LBUTTONUP, NULL, MAKELPARAM(ptMouse.x, ptMouse.y));

		if(pFmtEtc->cfFormat == CF_DIB && medium.tymed == TYMED_HGLOBAL)
		{
			if(medium.hGlobal != NULL)
			{
				LPBITMAPINFOHEADER  lpbi = (BITMAPINFOHEADER*)GlobalLock(medium.hGlobal);
				if(lpbi != NULL)
				{
					HBITMAP hbm = NULL;
					HDC hdc = GetDC(NULL);
					if(hdc != NULL)
					{
						hbm = CreateDIBitmap(hdc,(LPBITMAPINFOHEADER)lpbi,
							(LONG)CBM_INIT,
							(LPSTR)lpbi + lpbi->biSize + ColorTableSize(lpbi),
							(LPBITMAPINFO)lpbi,DIB_RGB_COLORS);

						::ReleaseDC(NULL,hdc);
					}
					GlobalUnlock(medium.hGlobal);
					if(hbm != NULL)
						hbm = (HBITMAP)SendMessage(m_hTargetWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbm);
					if(hbm != NULL)
						DeleteObject(hbm);
					return true; //release the medium
				}
			}
		}
		if(pFmtEtc->cfFormat == CF_BITMAP && medium.tymed == TYMED_GDI)
		{
			if(medium.hBitmap != NULL)
			{
				HBITMAP hBmp = (HBITMAP)SendMessage(m_hTargetWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)medium.hBitmap);
				if(hBmp != NULL)
					DeleteObject(hBmp);
				return false; //don't free the bitmap
			}
		}
		if(pFmtEtc->cfFormat == CF_ENHMETAFILE && medium.tymed == TYMED_ENHMF)
		{
			ENHMETAHEADER emh;
			GetEnhMetaFileHeader(medium.hEnhMetaFile, sizeof(ENHMETAHEADER),&emh);
			RECT rc;//={0,0,EnhMetaHdr.rclBounds.right-EnhMetaHdr.rclBounds.left, EnhMetaHdr.rclBounds.bottom-EnhMetaHdr.rclBounds.top};
			HDC hDC= GetDC(m_hTargetWnd);
			//start code: taken from ENHMETA.EXE MSDN Sample
			//*ALSO NEED to GET the pallete (select and RealizePalette it, but i was too lazy*
			// Get the characteristics of the output device
			float PixelsX = (float)GetDeviceCaps( hDC, HORZRES );
			float PixelsY = (float)GetDeviceCaps( hDC, VERTRES );
			float MMX = (float)GetDeviceCaps( hDC, HORZSIZE );
			float MMY = (float)GetDeviceCaps( hDC, VERTSIZE );
			// Calculate the rect in which to draw the metafile based on the
			// intended size and the current output device resolution
			// Remember that the intended size is given in 0.01mm units, so
			// convert those to device units on the target device
			rc.top = (int)((float)(emh.rclFrame.top) * PixelsY / (MMY*100.0f));
			rc.left = (int)((float)(emh.rclFrame.left) * PixelsX / (MMX*100.0f));
			rc.right = (int)((float)(emh.rclFrame.right) * PixelsX / (MMX*100.0f));
			rc.bottom = (int)((float)(emh.rclFrame.bottom) * PixelsY / (MMY*100.0f));
			//end code: taken from ENHMETA.EXE MSDN Sample

			HDC hdcMem = CreateCompatibleDC(hDC);
			HGDIOBJ hBmpMem = CreateCompatibleBitmap(hDC, emh.rclBounds.right, emh.rclBounds.bottom);
			HGDIOBJ hOldBmp = ::SelectObject(hdcMem, hBmpMem);
			PlayEnhMetaFile(hdcMem,medium.hEnhMetaFile,&rc);
			HBITMAP hBmp = (HBITMAP)::SelectObject(hdcMem, hOldBmp);
			DeleteDC(hdcMem);
			ReleaseDC(m_hTargetWnd,hDC);
			hBmp = (HBITMAP)SendMessage(m_hTargetWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
			if(hBmp != NULL)
				DeleteObject(hBmp);
			return true;
		}
		if(pFmtEtc->cfFormat == CF_HDROP && medium.tymed == TYMED_HGLOBAL)
		{
			HDROP hDrop = (HDROP)GlobalLock(medium.hGlobal);
			if(hDrop != NULL)
			{
				TCHAR szFileName[MAX_PATH];
				UINT cFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0); 
				if(cFiles > 0)
				{
					DragQueryFile(hDrop, 0, szFileName, sizeof(szFileName)); 
					HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, szFileName,IMAGE_BITMAP,0,0,LR_DEFAULTSIZE|LR_LOADFROMFILE);
					if(hBitmap)
					{
						HBITMAP hBmp = (HBITMAP)SendMessage(m_hTargetWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
						if(hBmp != NULL)
							DeleteObject(hBmp);
					}
				}
			}
			GlobalUnlock(medium.hGlobal);
		}
		return true; //let base free the medium
	}
} // namespace DuiLib

#pragma warning(pop)
