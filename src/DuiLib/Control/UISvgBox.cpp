#include "StdAfx.h"
#include "UISvgBox.h"
#include "UIBootstrapIcons.h"
#include "UILucideIcons.h"
#include "UIIconParkIcons.h"
#include "UITablerIcons.h"
#include "UIRemixIconIcons.h"
#include "UITwemojiIcons.h"
#include <lunasvg.h>
#include <string>

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CSvgBoxUI)

	static std::string DuiStringToUtf8(LPCTSTR pstr)
	{
		if( pstr == NULL || *pstr == _T('\0') ) return std::string();
#ifdef _UNICODE
		int n = ::WideCharToMultiByte(CP_UTF8, 0, pstr, -1, NULL, 0, NULL, NULL);
		if( n <= 1 ) return std::string();
		std::string s((size_t)(n - 1), '\0');
		::WideCharToMultiByte(CP_UTF8, 0, pstr, -1, &s[0], n, NULL, NULL);
		return s;
#else
		return std::string(pstr);
#endif
	}

	static HBITMAP CreatePremultHBitmap(const lunasvg::Bitmap& bitmap)
	{
		const int w = bitmap.width();
		const int h = bitmap.height();
		if( w <= 0 || h <= 0 || bitmap.isNull() ) return NULL;

		BITMAPINFO bmi;
		::ZeroMemory(&bmi, sizeof(bmi));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = w;
		bmi.bmiHeader.biHeight = -h;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = (DWORD)(w * h * 4);

		LPBYTE pDest = NULL;
		HBITMAP hBitmap = ::CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&pDest, NULL, 0);
		if( hBitmap == NULL || pDest == NULL ) return NULL;

		const uint8_t* pSrc = bitmap.data();
		const int stride = bitmap.stride();
		const int rowBytes = w * 4;
		for( int y = 0; y < h; ++y ) {
			::memcpy(pDest + y * rowBytes, pSrc + y * stride, (size_t)rowBytes);
		}
		return hBitmap;
	}

	CSvgBoxUI::CSvgBoxUI()
		: m_dwTintColor(0)
		, m_dwHotTintColor(0)
		, m_dwPushedTintColor(0)
		, m_dwDisabledTintColor(0)
		, m_uButtonState(0)
		, m_hCacheBitmap(NULL)
		, m_nCacheW(0)
		, m_nCacheH(0)
		, m_dwCacheTint(0)
	{
	}

	CSvgBoxUI::~CSvgBoxUI()
	{
		ClearCache();
	}

	LPCTSTR CSvgBoxUI::GetClass() const
	{
		return _T("SvgBoxUI");
	}

	LPVOID CSvgBoxUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_SVGBOX) == 0 ) return static_cast<CSvgBoxUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	CDuiString CSvgBoxUI::ResolveFilePath(LPCTSTR pstrPath)
	{
		if( pstrPath == NULL || *pstrPath == _T('\0') ) return CDuiString();
		if( (pstrPath[0] == _T('\\') || pstrPath[0] == _T('/')) ||
			(_tcslen(pstrPath) >= 2 && pstrPath[1] == _T(':')) ) {
			return CDuiString(pstrPath);
		}
		CDuiString sFile = CPaintManagerUI::GetResourcePath();
		sFile += pstrPath;
		return sFile;
	}

	DWORD CSvgBoxUI::ParseColorValue(LPCTSTR pstrValue)
	{
		if( pstrValue == NULL || *pstrValue == _T('\0') ) return 0;
		DWORD clr = 0;
		if( ParseColorString(pstrValue, clr) ) return clr;
		if( *pstrValue == _T('#') ) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		return _tcstoul(pstrValue, &pstr, 16);
	}

	void CSvgBoxUI::ClearCache()
	{
		if( m_hCacheBitmap != NULL ) {
			::DeleteObject(m_hCacheBitmap);
			m_hCacheBitmap = NULL;
		}
		m_nCacheW = 0;
		m_nCacheH = 0;
		m_dwCacheTint = 0;
	}

	void CSvgBoxUI::LoadFromFile(LPCTSTR pstrPath)
	{
		m_sSvgPath = pstrPath ? pstrPath : _T("");
		m_sSvgData.Empty();
		m_sSvgUtf8.clear();
		ClearCache();
		Invalidate();
	}

	void CSvgBoxUI::LoadFromData(LPCTSTR pstrSvgContent)
	{
		m_sSvgData = pstrSvgContent ? pstrSvgContent : _T("");
		m_sSvgPath.Empty();
		m_sSvgUtf8.clear();
		ClearCache();
		Invalidate();
	}

	void CSvgBoxUI::LoadFromUtf8Data(const char* utf8Svg)
	{
		m_sSvgUtf8 = utf8Svg ? utf8Svg : "";
		m_sSvgPath.Empty();
		m_sSvgData.Empty();
		ClearCache();
		Invalidate();
	}

	void CSvgBoxUI::SetTintColor(DWORD dwColor)
	{
		if( m_dwTintColor == dwColor ) return;
		m_dwTintColor = dwColor;
		Invalidate();
	}

	DWORD CSvgBoxUI::GetTintColor() const
	{
		return m_dwTintColor;
	}

	void CSvgBoxUI::SetHotTintColor(DWORD dwColor)
	{
		if( m_dwHotTintColor == dwColor ) return;
		m_dwHotTintColor = dwColor;
		Invalidate();
	}

	DWORD CSvgBoxUI::GetHotTintColor() const
	{
		return m_dwHotTintColor;
	}

	void CSvgBoxUI::SetPushedTintColor(DWORD dwColor)
	{
		if( m_dwPushedTintColor == dwColor ) return;
		m_dwPushedTintColor = dwColor;
		Invalidate();
	}

	DWORD CSvgBoxUI::GetPushedTintColor() const
	{
		return m_dwPushedTintColor;
	}

	void CSvgBoxUI::SetDisabledTintColor(DWORD dwColor)
	{
		if( m_dwDisabledTintColor == dwColor ) return;
		m_dwDisabledTintColor = dwColor;
		Invalidate();
	}

	DWORD CSvgBoxUI::GetDisabledTintColor() const
	{
		return m_dwDisabledTintColor;
	}

	void CSvgBoxUI::SetEnabled(bool bEnable)
	{
		CControlUI::SetEnabled(bEnable);
		if( bEnable ) m_uButtonState &= ~UISTATE_DISABLED;
		else m_uButtonState |= UISTATE_DISABLED;
		Invalidate();
	}

	DWORD CSvgBoxUI::GetPaintTintColor() const
	{
		if( !IsEnabled() || (m_uButtonState & UISTATE_DISABLED) != 0 ) {
			if( m_dwDisabledTintColor != 0 ) return m_dwDisabledTintColor;
			return m_dwTintColor;
		}
		if( (m_uButtonState & UISTATE_PUSHED) != 0 && m_dwPushedTintColor != 0 )
			return m_dwPushedTintColor;
		if( (m_uButtonState & UISTATE_HOT) != 0 && m_dwHotTintColor != 0 )
			return m_dwHotTintColor;
		return m_dwTintColor;
	}

	void CSvgBoxUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( !BubbleEvent(event) ) CControlUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
		{
			if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) {
				m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEMOVE )
		{
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
				if( ::PtInRect(&m_rcItem, event.ptMouse) )
					m_uButtonState |= UISTATE_PUSHED;
				else
					m_uButtonState &= ~UISTATE_PUSHED;
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_BUTTONUP )
		{
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
				m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
				Invalidate();
				if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() && m_pManager != NULL )
					m_pManager->SendNotify(this, DUI_MSGTYPE_CLICK);
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEENTER )
		{
			if( IsEnabled() ) {
				m_uButtonState |= UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSELEAVE )
		{
			if( IsEnabled() ) {
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		CControlUI::DoEvent(event);
	}

	void CSvgBoxUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("src")) == 0 ) {
			LoadFromFile(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("data")) == 0 ) {
			LoadFromData(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("bsicon")) == 0 ) {
			LoadFromUtf8Data(BootstrapIcons::GetIcon(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("iconpark")) == 0 ) {
			LoadFromUtf8Data(IconParkIcons::GetIcon(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("lucide")) == 0 ) {
			LoadFromUtf8Data(LucideIcons::GetIcon(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("tabler-filled")) == 0 ) {
			LoadFromUtf8Data(TablerFilledIcons::GetIcon(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("tabler-outline")) == 0 ) {
			LoadFromUtf8Data(TablerOutlineIcons::GetIcon(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("remixicon")) == 0 ) {
			LoadFromUtf8Data(RemixIconIcons::GetIcon(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("twicon")) == 0 ) {
			LoadFromUtf8Data(TwemojiIcons::GetIcon(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("color")) == 0 || _tcsicmp(pstrName, _T("fill")) == 0
			|| _tcsicmp(pstrName, _T("tint")) == 0 || _tcsicmp(pstrName, _T("tintcolor")) == 0 ) {
			SetTintColor(ParseColorValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("color-hover")) == 0 || _tcsicmp(pstrName, _T("fill-hover")) == 0
			|| _tcsicmp(pstrName, _T("hottint")) == 0 || _tcsicmp(pstrName, _T("hottintcolor")) == 0 ) {
			SetHotTintColor(ParseColorValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("color-active")) == 0 || _tcsicmp(pstrName, _T("fill-active")) == 0
			|| _tcsicmp(pstrName, _T("pushedtint")) == 0 || _tcsicmp(pstrName, _T("pushedtintcolor")) == 0 ) {
			SetPushedTintColor(ParseColorValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("color-disabled")) == 0 || _tcsicmp(pstrName, _T("fill-disabled")) == 0
			|| _tcsicmp(pstrName, _T("disabledtint")) == 0 || _tcsicmp(pstrName, _T("disabledtintcolor")) == 0 ) {
			SetDisabledTintColor(ParseColorValue(pstrValue));
		}
		else {
			CControlUI::SetAttribute(pstrName, pstrValue);
		}
	}

	bool CSvgBoxUI::EnsureCache(int w, int h, DWORD dwTint)
	{
		if( w <= 0 || h <= 0 ) return false;
		if( m_hCacheBitmap != NULL && m_nCacheW == w && m_nCacheH == h && m_dwCacheTint == dwTint )
			return true;

		ClearCache();
		if( m_sSvgPath.IsEmpty() && m_sSvgData.IsEmpty() && m_sSvgUtf8.empty() ) return false;

		std::unique_ptr<lunasvg::Document> document;
		if( !m_sSvgPath.IsEmpty() ) {
			CDuiString sPath = ResolveFilePath(m_sSvgPath.GetData());
			document = lunasvg::Document::loadFromFile(DuiStringToUtf8(sPath.GetData()));
		}
		else if( !m_sSvgUtf8.empty() ) {
			document = lunasvg::Document::loadFromData(m_sSvgUtf8);
		}
		else {
			document = lunasvg::Document::loadFromData(DuiStringToUtf8(m_sSvgData.GetData()));
		}
		if( !document ) return false;

		if( dwTint != 0 ) {
			const BYTE r = (BYTE)((dwTint >> 16) & 0xFF);
			const BYTE g = (BYTE)((dwTint >> 8) & 0xFF);
			const BYTE b = (BYTE)(dwTint & 0xFF);
			char style[256];
			sprintf_s(style, sizeof(style),
				"* { fill: #%02x%02x%02x; stroke: #%02x%02x%02x; }",
				r, g, b, r, g, b);
			document->applyStyleSheet(style);
		}

		lunasvg::Bitmap bitmap = document->renderToBitmap(w, h);
		if( bitmap.isNull() ) return false;

		m_hCacheBitmap = CreatePremultHBitmap(bitmap);
		if( m_hCacheBitmap == NULL ) return false;
		m_nCacheW = w;
		m_nCacheH = h;
		m_dwCacheTint = dwTint;
		return true;
	}

	void CSvgBoxUI::PaintStatusImage(IRenderContext& ctx)
	{
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~UISTATE_DISABLED;

		const int w = (int)(m_rcItem.right - m_rcItem.left);
		const int h = (int)(m_rcItem.bottom - m_rcItem.top);
		const DWORD dwTint = GetPaintTintColor();
		if( !EnsureCache(w, h, dwTint) || m_hCacheBitmap == NULL ) return;

		RECT rcBmpPart = { 0, 0, m_nCacheW, m_nCacheH };
		RECT rcCorners = { 0, 0, 0, 0 };
		ctx.DrawImage(m_hCacheBitmap, m_rcItem, m_rcPaint, rcBmpPart, rcCorners, true);
	}
}
