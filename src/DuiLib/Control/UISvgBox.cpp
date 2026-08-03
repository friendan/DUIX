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
		: m_dwColor(0)
		, m_dwHoverColor(0)
		, m_dwActiveColor(0)
		, m_dwDisabledColor(0)
		, m_uButtonState(0)
		, m_hCacheBitmap(NULL)
		, m_nCacheW(0)
		, m_nCacheH(0)
		, m_dwCacheColor(0)
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

	UINT CSvgBoxUI::GetControlFlags() const
	{
		// 与 PreferClientHit 对齐：有热态时带 SETCURSOR，供 WM_SETCURSOR；勿回调 PreferClientHit
		if( !IsEnabled() ) return 0;
		if( m_dwHoverColor != 0 || m_dwActiveColor != 0 ) return UIFLAG_SETCURSOR;
		return 0;
	}

	bool CSvgBoxUI::PreferClientHit() const
	{
		if( !IsEnabled() ) return false;
		if( m_dwHoverColor != 0 || m_dwActiveColor != 0 ) return true;
		return CControlUI::PreferClientHit();
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
		DWORD clr = 0;
		if( pstrValue != NULL && ParseColorString(pstrValue, clr) ) return clr;
		return 0;
	}

	CSvgBoxUI::TintMode CSvgBoxUI::DetectTintMode(const std::string& svgUtf8)
	{
		if( svgUtf8.empty() ) return TintBoth;

		auto has = [&](const char* s) -> bool {
			return svgUtf8.find(s) != std::string::npos;
		};

		// Twemoji 等多色图标：保留原色，不单色着色
		if( (has("fill=\"#") || has("fill='#")) && !has("currentColor") )
			return TintSkip;

		const bool bFillNone = has("fill=\"none\"") || has("fill='none'");
		const bool bStrokeCurrent = has("stroke=\"currentColor\"") || has("stroke='currentColor'");
		const bool bFillCurrent = has("fill=\"currentColor\"") || has("fill='currentColor'");
		const bool bHasStroke = has("stroke=") || has("stroke:");

		// Lucide / Tabler Outline / IconPark：描边为主
		if( bStrokeCurrent || (bFillNone && bHasStroke) )
			return TintStroke;
		// Bootstrap / Remix / Tabler Filled：填充为主
		if( bFillCurrent || !bHasStroke )
			return TintFill;
		return TintBoth;
	}

	void CSvgBoxUI::ClearCache()
	{
		if( m_hCacheBitmap != NULL ) {
			::DeleteObject(m_hCacheBitmap);
			m_hCacheBitmap = NULL;
		}
		m_nCacheW = 0;
		m_nCacheH = 0;
		m_dwCacheColor = 0;
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

	void CSvgBoxUI::SetColor(DWORD dwColor)
	{
		if( m_dwColor == dwColor ) return;
		m_dwColor = dwColor;
		Invalidate();
	}

	DWORD CSvgBoxUI::GetColor() const
	{
		return m_dwColor;
	}

	void CSvgBoxUI::SetHoverColor(DWORD dwColor)
	{
		if( m_dwHoverColor == dwColor ) return;
		m_dwHoverColor = dwColor;
		Invalidate();
	}

	DWORD CSvgBoxUI::GetHoverColor() const
	{
		return m_dwHoverColor;
	}

	void CSvgBoxUI::SetActiveColor(DWORD dwColor)
	{
		if( m_dwActiveColor == dwColor ) return;
		m_dwActiveColor = dwColor;
		Invalidate();
	}

	DWORD CSvgBoxUI::GetActiveColor() const
	{
		return m_dwActiveColor;
	}

	void CSvgBoxUI::SetDisabledColor(DWORD dwColor)
	{
		if( m_dwDisabledColor == dwColor ) return;
		m_dwDisabledColor = dwColor;
		Invalidate();
	}

	DWORD CSvgBoxUI::GetDisabledColor() const
	{
		return m_dwDisabledColor;
	}

	void CSvgBoxUI::SetEnabled(bool bEnable)
	{
		CControlUI::SetEnabled(bEnable);
		if( bEnable ) m_uButtonState &= ~UISTATE_DISABLED;
		else m_uButtonState |= UISTATE_DISABLED;
		Invalidate();
	}

	DWORD CSvgBoxUI::GetPaintColor() const
	{
		if( !IsEnabled() || (m_uButtonState & UISTATE_DISABLED) != 0 ) {
			if( m_dwDisabledColor != 0 ) return m_dwDisabledColor;
			return m_dwColor;
		}
		if( (m_uButtonState & UISTATE_PUSHED) != 0 && m_dwActiveColor != 0 )
			return m_dwActiveColor;
		if( (m_uButtonState & UISTATE_HOT) != 0 && m_dwHoverColor != 0 )
			return m_dwHoverColor;
		return m_dwColor;
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
		else if( _tcsicmp(pstrName, _T("color")) == 0 ) {
			SetColor(ParseColorValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("color-hover")) == 0 ) {
			SetHoverColor(ParseColorValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("color-active")) == 0 ) {
			SetActiveColor(ParseColorValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("color-disabled")) == 0 ) {
			SetDisabledColor(ParseColorValue(pstrValue));
		}
		else {
			CControlUI::SetAttribute(pstrName, pstrValue);
		}
	}

	bool CSvgBoxUI::EnsureCache(int w, int h, DWORD dwColor)
	{
		if( w <= 0 || h <= 0 ) return false;
		if( m_hCacheBitmap != NULL && m_nCacheW == w && m_nCacheH == h && m_dwCacheColor == dwColor )
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

		if( dwColor != 0 ) {
			std::string sProbe;
			if( !m_sSvgUtf8.empty() ) {
				sProbe = m_sSvgUtf8;
			}
			else if( !m_sSvgData.IsEmpty() ) {
				sProbe = DuiStringToUtf8(m_sSvgData.GetData());
			}
			else if( !m_sSvgPath.IsEmpty() ) {
				CDuiString sPath = ResolveFilePath(m_sSvgPath.GetData());
				FILE* fp = NULL;
#ifdef _UNICODE
				_wfopen_s(&fp, sPath.GetData(), L"rb");
#else
				fopen_s(&fp, sPath.GetData(), "rb");
#endif
				if( fp != NULL ) {
					fseek(fp, 0, SEEK_END);
					long nLen = ftell(fp);
					fseek(fp, 0, SEEK_SET);
					if( nLen > 0 && nLen < 2 * 1024 * 1024 ) {
						sProbe.resize((size_t)nLen);
						fread(&sProbe[0], 1, (size_t)nLen, fp);
					}
					fclose(fp);
				}
			}

			const TintMode mode = sProbe.empty() ? TintBoth : DetectTintMode(sProbe);
			if( mode != TintSkip ) {
				const BYTE r = (BYTE)((dwColor >> 16) & 0xFF);
				const BYTE g = (BYTE)((dwColor >> 8) & 0xFF);
				const BYTE b = (BYTE)(dwColor & 0xFF);
				char style[256];
				if( mode == TintStroke ) {
					// 描边图标：保持 fill:none，只改 stroke，避免 fill 把线标糊成色块
					sprintf_s(style, sizeof(style),
						"* { fill: none; stroke: #%02x%02x%02x; }", r, g, b);
				}
				else if( mode == TintFill ) {
					// 填充图标：清掉 stroke，避免描边导致「加粗」
					sprintf_s(style, sizeof(style),
						"* { fill: #%02x%02x%02x; stroke: none; }", r, g, b);
				}
				else {
					sprintf_s(style, sizeof(style),
						"* { fill: #%02x%02x%02x; stroke: #%02x%02x%02x; }",
						r, g, b, r, g, b);
				}
				document->applyStyleSheet(style);
			}
		}

		lunasvg::Bitmap bitmap = document->renderToBitmap(w, h);
		if( bitmap.isNull() ) return false;

		m_hCacheBitmap = CreatePremultHBitmap(bitmap);
		if( m_hCacheBitmap == NULL ) return false;
		m_nCacheW = w;
		m_nCacheH = h;
		m_dwCacheColor = dwColor;
		return true;
	}

	void CSvgBoxUI::PaintStatusImage(IRenderContext& ctx)
	{
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~UISTATE_DISABLED;

		const int w = (int)(m_rcItem.right - m_rcItem.left);
		const int h = (int)(m_rcItem.bottom - m_rcItem.top);
		const DWORD dwColor = GetPaintColor();
		if( !EnsureCache(w, h, dwColor) || m_hCacheBitmap == NULL ) return;

		RECT rcBmpPart = { 0, 0, m_nCacheW, m_nCacheH };
		RECT rcCorners = { 0, 0, 0, 0 };
		ctx.DrawImage(m_hCacheBitmap, m_rcItem, m_rcPaint, rcBmpPart, rcCorners, true);
	}
}
