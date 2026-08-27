#include "StdAfx.h"
#include "UIAppIcon.h"
#include <shellapi.h>
#include <commoncontrols.h>
#include <ShlObj.h>

#pragma comment(lib, "shell32.lib")

#ifndef SHIL_EXTRALARGE
#define SHIL_EXTRALARGE 2
#endif
#ifndef SHIL_JUMBO
#define SHIL_JUMBO 4
#endif
#ifndef SHIL_SMALL
#define SHIL_SMALL 0
#endif
#ifndef SHIL_LARGE
#define SHIL_LARGE 1
#endif

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CAppIconUI)

	CAppIconUI::CAppIconUI()
		: m_bWidthFromSkin(false)
		, m_bHeightFromSkin(false)
		, m_bTipFromSkin(false)
		, m_bTipAuto(false)
		, m_bTextIconBkCustom(false)
		, m_bTextIconFgCustom(false)
		, m_dwTextIconBk(0)
		, m_dwTextIconFg(0)
		, m_nBadgeCount(0)
		, m_nBadgeOverflow(99)
		, m_bBadgeShowZero(false)
		, m_bBadgeDot(false)
		, m_bBadgeHang(true)
		, m_dwBadgeColor(0xFF4D4FFF)
		, m_dwBadgeTextColor(0xFFFFFFFF)
		, m_nBadgeDotSize(8)
		, m_nBadgeHeight(18)
	{
		m_szBadgeOffset.cx = 0;
		m_szBadgeOffset.cy = 0;

		// 桌面图标默认：上图下文、格稍紧（少留白）、悬停浅底
		SetIconPosition(_T("top"));
		SetIconSize(56);
		SetIconGap(4);
		SIZE szRadius = { 12, 12 };
		SetBorderRadius(szRadius);
		SetKind(CONTROLKIND_NONE); // 内部会 ApplyDefaultHoverChrome
		SetTextStyle(DT_SINGLELINE | DT_CENTER | DT_END_ELLIPSIS);
		SetCursor(DUI_HAND);
		SyncAutoSize(); // 初始无标题 → 正方形；皮肤写 text 后再拉高
	}

	LPCTSTR CAppIconUI::GetClass() const
	{
		return _T("AppIconUI");
	}

	LPVOID CAppIconUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_APPICON) == 0 )
			return static_cast<CAppIconUI*>(this);
		return CButtonUI::GetInterface(pstrName);
	}

	void CAppIconUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
	{
		CButtonUI::SetManager(pManager, pParent, bInit);
		if( m_pManager != NULL && !m_sFileIcon.IsEmpty() )
			ApplyFileIcon();
	}

	bool CAppIconUI::PreferClientHit() const
	{
		if( !IsEnabled() ) return false;
		if( IsTextIcon() ) return true;
		return CButtonUI::PreferClientHit();
	}

	bool CAppIconUI::IsIconOnly() const
	{
		// 纯图标，或文字图标（无下方标题行）→ 正方形格
		if( HasSubText() ) return false;
		if( GetText().IsEmpty() ) return true;
		return IsTextIcon();
	}

	bool CAppIconUI::IsTextIcon() const
	{
		return !HasIcon() && !IsLoading() && !GetText().IsEmpty();
	}

	void CAppIconUI::RefreshLayout()
	{
		SyncTextIconToolTip();
		SyncAutoSize();
		Invalidate();
	}

	HBITMAP CAppIconUI::HIconToHBitmap(HICON hIcon, int cx, int cy)
	{
		return CButtonUI::CreateBitmapFromHIcon(hIcon, cx, cy);
	}

	HICON CAppIconUI::LoadFileHIcon(LPCTSTR pstrPath, int cx, int cy)
	{
		if( pstrPath == NULL || *pstrPath == _T('\0') ) return NULL;

		const DWORD dwAttr = ::GetFileAttributes(pstrPath);
		const bool bMissing = (dwAttr == INVALID_FILE_ATTRIBUTES);

		CDuiString s(pstrPath);
		s.MakeLower();
		LPCTSTR pExt = NULL;
		for( LPCTSTR p = s.GetData(); *p != _T('\0'); ++p ) {
			if( *p == _T('.') ) pExt = p;
		}

		// 文件仍在：优先读 .ico / PE 内嵌高清图标 / 系统索引
		if( !bMissing ) {
			if( pExt != NULL && _tcscmp(pExt, _T(".ico")) == 0 ) {
				HICON h = (HICON)::LoadImage(NULL, pstrPath, IMAGE_ICON, cx, cy, LR_LOADFROMFILE);
				if( h != NULL ) return h;
			}

			// EXE/DLL：按 256 取出资源内最佳图标（比外壳 48px 缓存清晰）
			if( pExt != NULL && (_tcscmp(pExt, _T(".exe")) == 0 || _tcscmp(pExt, _T(".dll")) == 0
				|| _tcscmp(pExt, _T(".cpl")) == 0 || _tcscmp(pExt, _T(".scr")) == 0) ) {
				const int extract = (cx > 256 || cy > 256) ? ((cx > cy) ? cx : cy) : 256;
				HICON hExtract = NULL;
				UINT uId = 0;
				UINT n = ::PrivateExtractIcons(pstrPath, 0, extract, extract, &hExtract, &uId, 1, LR_DEFAULTCOLOR);
				if( n > 0 && hExtract != NULL )
					return hExtract;
			}

			SHFILEINFO sfi = {};
			DWORD_PTR dwRet = ::SHGetFileInfo(pstrPath, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX);
			if( dwRet != 0 ) {
				// 目标 >48 时取 JUMBO(256) 再缩小，避免 EXTRALARGE(48)→56 放大发糊
				int shil = SHIL_LARGE;
				const int side = (cx > cy) ? cx : cy;
				if( side > 48 ) shil = SHIL_JUMBO;
				else if( side > 32 ) shil = SHIL_EXTRALARGE;
				else if( side <= 16 ) shil = SHIL_SMALL;
				IImageList* piml = NULL;
				HICON hIcon = NULL;
				if( SUCCEEDED(::SHGetImageList(shil, IID_IImageList, (void**)&piml)) && piml != NULL ) {
					piml->GetIcon(sfi.iIcon, ILD_TRANSPARENT, &hIcon);
					piml->Release();
				}
				if( hIcon != NULL ) return hIcon;
			}

			HICON hLarge = NULL;
			HICON hSmall = NULL;
			UINT n = ::ExtractIconEx(pstrPath, 0, &hLarge, &hSmall, 1);
			if( n > 0 ) {
				if( hLarge != NULL ) {
					if( hSmall != NULL ) ::DestroyIcon(hSmall);
					return hLarge;
				}
				if( hSmall != NULL ) return hSmall;
			}

			::ZeroMemory(&sfi, sizeof(sfi));
			if( ::SHGetFileInfo(pstrPath, 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_LARGEICON)
				&& sfi.hIcon != NULL )
				return sfi.hIcon;
		}

		// 文件已删 / 不可读：按扩展名取外壳类型图标（静默，不弹错）
		{
			DWORD dwFileAttr = FILE_ATTRIBUTE_NORMAL;
			if( !bMissing && (dwAttr & FILE_ATTRIBUTE_DIRECTORY) )
				dwFileAttr = FILE_ATTRIBUTE_DIRECTORY;

			SHFILEINFO sfi = {};
			DWORD_PTR dwRet = ::SHGetFileInfo(pstrPath, dwFileAttr, &sfi, sizeof(sfi),
				SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);
			if( dwRet != 0 ) {
				int shil = SHIL_LARGE;
				const int side = (cx > cy) ? cx : cy;
				if( side > 48 ) shil = SHIL_JUMBO;
				else if( side > 32 ) shil = SHIL_EXTRALARGE;
				else if( side <= 16 ) shil = SHIL_SMALL;
				IImageList* piml = NULL;
				HICON hIcon = NULL;
				if( SUCCEEDED(::SHGetImageList(shil, IID_IImageList, (void**)&piml)) && piml != NULL ) {
					piml->GetIcon(sfi.iIcon, ILD_TRANSPARENT, &hIcon);
					piml->Release();
				}
				if( hIcon != NULL ) return hIcon;
			}

			::ZeroMemory(&sfi, sizeof(sfi));
			if( ::SHGetFileInfo(pstrPath, dwFileAttr, &sfi, sizeof(sfi),
				SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES) && sfi.hIcon != NULL )
				return sfi.hIcon;
		}
		return NULL;
	}

	bool CAppIconUI::ApplyFileIcon()
	{
		if( m_sFileIcon.IsEmpty() ) return false;
		if( m_pManager == NULL ) return false;

		LPCTSTR path = m_sFileIcon.GetData();

		// 光栅图：仅当资源/磁盘可读时用图片内容；缺失则回退外壳类型图标（不报错）
		if( IsRasterImagePath(path) ) {
			const TImageInfo* pInfo = m_pManager->GetImageEx(path, NULL);
			if( pInfo != NULL ) {
				// 勿走 AppIcon::SetIconSrc（会清空 m_sFileIcon）
				CButtonUI::SetIconSrc(path);
				return HasIcon();
			}
		}
		else {
			// SVG：走 SvgBox（同 icon-src）；可读才设，缺失静默回退外壳
			CDuiString s(path);
			s.MakeLower();
			LPCTSTR pExt = NULL;
			for( LPCTSTR p = s.GetData(); *p != _T('\0'); ++p ) {
				if( *p == _T('.') ) pExt = p;
			}
			if( pExt != NULL && _tcsncmp(pExt, _T(".svg"), 4) == 0 ) {
				BYTE* pData = NULL;
				DWORD dwSize = 0;
				if( CPaintManagerUI::LoadResourceData(path, &pData, &dwSize)
					&& pData != NULL && dwSize > 0 ) {
					delete[] pData;
					CButtonUI::SetIconSrc(path);
					return HasIcon();
				}
				delete[] pData;
			}
		}

		int cx = ScaleValue(GetIconSize());
		if( cx < 16 ) cx = 16;
		HICON hIcon = LoadFileHIcon(path, cx, cx);
		if( hIcon == NULL ) {
			CButtonUI::ClearIcon();
			return false;
		}
		HBITMAP hbm = HIconToHBitmap(hIcon, cx, cx);
		::DestroyIcon(hIcon);
		if( hbm == NULL ) {
			CButtonUI::ClearIcon();
			return false;
		}
		if( !SetIconBitmap(hbm, cx, cx, true) ) {
			// SetIconBitmap 失败时已 DeleteObject
			CButtonUI::ClearIcon();
			return false;
		}
		return true;
	}

	void CAppIconUI::SetFileIcon(LPCTSTR pstrPath)
	{
		if( pstrPath == NULL || *pstrPath == _T('\0') ) {
			ClearIcon();
			return;
		}
		m_sFileIcon = pstrPath;
		// 盘符路径统一用 /，避免皮肤/日志里 \n \t 等被误读；Windows API 接受 /
		m_sFileIcon.Replace(_T("\\"), _T("/"));
		// 缺失/损坏一律静默：Apply 失败则无图标（有 text 时走文字图标）
		ApplyFileIcon();
		RefreshLayout();
	}

	void CAppIconUI::ClearIcon()
	{
		m_sFileIcon.Empty();
		CButtonUI::ClearIcon();
		RefreshLayout();
	}

	void CAppIconUI::SetIconLib(LPCTSTR pstrLib, LPCTSTR pstrName)
	{
		m_sFileIcon.Empty();
		CButtonUI::SetIconLib(pstrLib, pstrName);
		RefreshLayout();
	}

	void CAppIconUI::SetIconSrc(LPCTSTR pstrPath)
	{
		m_sFileIcon.Empty();
		CButtonUI::SetIconSrc(pstrPath);
		RefreshLayout();
	}

	bool CAppIconUI::SetIconFromMemory(const BYTE* pData, DWORD dwSize)
	{
		m_sFileIcon.Empty();
		const bool ok = CButtonUI::SetIconFromMemory(pData, dwSize);
		RefreshLayout();
		return ok;
	}

	void CAppIconUI::OnResetDpiAssets()
	{
		if( !m_sFileIcon.IsEmpty() ) {
			ApplyFileIcon();
			RefreshLayout();
		}
	}

	void CAppIconUI::SetTextIconBackground(DWORD dw)
	{
		m_dwTextIconBk = dw;
		m_bTextIconBkCustom = (dw != 0);
		Invalidate();
	}

	void CAppIconUI::SetTextIconColor(DWORD dw)
	{
		m_dwTextIconFg = dw;
		m_bTextIconFgCustom = (dw != 0);
		Invalidate();
	}

	void CAppIconUI::SyncTextIconToolTip()
	{
		if( m_bTipFromSkin ) return;
		if( IsTextIcon() ) {
			SetToolTip(GetText().GetData());
			m_bTipAuto = true;
		}
		else if( m_bTipAuto ) {
			SetToolTip(_T(""));
			m_bTipAuto = false;
		}
	}

	DWORD CAppIconUI::ResolveTextIconBk() const
	{
		const bool bEnabled = IsEnabled();
		const bool bActive = bEnabled && (m_uButtonState & UISTATE_PUSHED) != 0;
		const bool bHot = bEnabled && (m_uButtonState & UISTATE_HOT) != 0;

		// icon-bk / text-icon-background：仅图标区
		if( m_bTextIconBkCustom && m_dwTextIconBk != 0 )
			return m_dwTextIconBk;

		// background-color：整格底；文字图标时图标区跟随（含悬停/按下）
		if( GetBackgroundColor() != 0 ) {
			DWORD paint = GetPaintBackgroundColor();
			return (paint != 0) ? paint : GetBackgroundColor();
		}

		if( GetKind() != CONTROLKIND_NONE ) {
			int idx = (int)GetKind();
			if( idx < 0 || idx >= 11 ) idx = 1;
			if( bActive ) return g_kindColors[idx].Active.dwBackgroundColor;
			if( bHot ) return g_kindColors[idx].Hover.dwBackgroundColor;
			return g_kindColors[idx].Normal.dwBackgroundColor;
		}
		CThemeManager* tm = CThemeManager::GetInstance();
		if( tm != NULL ) {
			if( bActive )
				return tm->GetColor(_T("color-primary-active"),
					tm->GetColor(_T("color-primary-hover"), 0x0B5ED7FF));
			if( bHot )
				return tm->GetColor(_T("color-primary-hover"), 0x0B5ED7FF);
			return tm->GetColor(_T("color-primary"), 0x0D6EFDFF);
		}
		if( bHot || bActive ) return 0x0B5ED7FF;
		return 0x0D6EFDFF;
	}

	DWORD CAppIconUI::ResolveTextIconFg() const
	{
		if( !IsEnabled() ) {
			DWORD dis = GetDisabledColor();
			if( dis != 0 ) return dis;
			CThemeManager* tm = CThemeManager::GetInstance();
			if( tm != NULL )
				return tm->GetColor(_T("color-text-disabled"), 0xFFFFFF73);
			return 0xFFFFFF73;
		}
		if( m_bTextIconFgCustom && m_dwTextIconFg != 0 )
			return m_dwTextIconFg;
		if( GetKind() != CONTROLKIND_NONE ) {
			int idx = (int)GetKind();
			if( idx < 0 || idx >= 11 ) idx = 1;
			return g_kindColors[idx].Normal.dwColor;
		}
		CThemeManager* tm = CThemeManager::GetInstance();
		if( tm != NULL )
			return tm->GetColor(_T("color-primary-text"), 0xFFFFFFFF);
		return 0xFFFFFFFF;
	}

	bool CAppIconUI::LayoutTextIconPlate(const RECT& rcContent, RECT& rcIcon) const
	{
		::ZeroMemory(&rcIcon, sizeof(rcIcon));
		int nSize = m_nIconSize;
		if( m_pManager != NULL )
			nSize = m_pManager->GetDPIObj()->Scale(m_nIconSize);
		const int cw = rcContent.right - rcContent.left;
		const int ch = rcContent.bottom - rcContent.top;
		if( cw <= 0 || ch <= 0 ) return false;
		if( nSize > cw ) nSize = cw;
		if( nSize > ch ) nSize = ch;
		rcIcon.left = rcContent.left + (cw - nSize) / 2;
		rcIcon.top = rcContent.top + (ch - nSize) / 2;
		rcIcon.right = rcIcon.left + nSize;
		rcIcon.bottom = rcIcon.top + nSize;
		return true;
	}

	void CAppIconUI::PaintText(IRenderContext& ctx)
	{
		if( !IsTextIcon() ) {
			CButtonUI::PaintText(ctx);
			return;
		}

		if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~UISTATE_FOCUSED;
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~UISTATE_DISABLED;
		SyncControlStateFromButton();

		RECT rcPadding = GetPadding();
		RECT rcTextPadding = GetTextPadding();
		if( m_pManager != NULL )
			m_pManager->GetDPIObj()->Scale(&rcTextPadding);
		RECT rcContent = m_rcItem;
		rcContent.left += rcPadding.left + rcTextPadding.left;
		rcContent.right -= rcPadding.right + rcTextPadding.right;
		rcContent.top += rcPadding.top + rcTextPadding.top;
		rcContent.bottom -= rcPadding.bottom + rcTextPadding.bottom;

		RECT rcIcon = { 0 };
		if( !LayoutTextIconPlate(rcContent, rcIcon) )
			return;

		DWORD dwBk = ResolveTextIconBk();
		if( dwBk != 0 ) {
			SIZE szR = GetBorderRadius();
			int r = szR.cx;
			int iconW = rcIcon.right - rcIcon.left;
			if( r <= 0 ) r = iconW / 5;
			if( r < 1 ) r = 1;
			if( r * 2 > iconW ) r = iconW / 2;
			ctx.FillRoundRect(rcIcon, r, r, GetAdjustColor(dwBk));
		}

		CDuiString sText = GetText();
		if( sText.IsEmpty() ) return;

		const int inset = ScaleValue(6);
		RECT rcText = rcIcon;
		rcText.left += inset;
		rcText.right -= inset;
		rcText.top += inset;
		rcText.bottom -= inset;
		if( rcText.right <= rcText.left || rcText.bottom <= rcText.top )
			rcText = rcIcon;

		// 可换行；超出图标区直接裁切（不省略号）。短文垂直居中。
		const UINT uStyle = DT_CENTER | DT_WORDBREAK | DT_NOPREFIX;
		if( m_pManager != NULL ) {
			RECT rcMeasure = rcText;
			RenderMeasureText(m_pManager, rcMeasure, sText.GetData(), 0, GetFont(),
				uStyle | DT_CALCRECT);
			const int textH = rcMeasure.bottom - rcMeasure.top;
			const int boxH = rcText.bottom - rcText.top;
			if( textH > 0 && textH < boxH ) {
				rcText.top += (boxH - textH) / 2;
				rcText.bottom = rcText.top + textH;
			}
		}
		ctx.DrawText(rcText, sText.GetData(), GetAdjustColor(ResolveTextIconFg()),
			GetFont(), uStyle);
	}

	void CAppIconUI::SyncAutoSize()
	{
		const int nPad = 16; // 左右各约 8
		int side = m_nIconSize + nPad;
		if( side < 40 ) side = 40;

		if( !m_bWidthFromSkin )
			CControlUI::SetFixedWidth(side);

		if( !m_bHeightFromSkin ) {
			if( IsIconOnly() ) {
				int h = GetFixedWidth();
				if( h <= 0 ) h = side;
				CControlUI::SetFixedHeight(h);
			}
			else {
				// 56+4+20+8 ≈ 88：与原先默认竖格接近
				CControlUI::SetFixedHeight(m_nIconSize + GetIconGap() + 28);
			}
		}
	}

	void CAppIconUI::SetText(LPCTSTR pstrText)
	{
		CButtonUI::SetText(pstrText);
		SyncTextIconToolTip();
		SyncAutoSize();
	}

	void CAppIconUI::SetSubText(LPCTSTR pstrText)
	{
		CButtonUI::SetSubText(pstrText);
		SyncAutoSize();
	}

	void CAppIconUI::SetIconSize(int nSize)
	{
		CButtonUI::SetIconSize(nSize);
		if( !m_sFileIcon.IsEmpty() )
			ApplyFileIcon();
		SyncAutoSize();
	}

	void CAppIconUI::ApplyDefaultHoverChrome()
	{
		// 无主题时退回半透明黑；有主题则用图标钮专用 token（深/浅色自适应）
		DWORD hover = 0x00000014;
		DWORD active = 0x00000022;
		CThemeManager* tm = CThemeManager::GetInstance();
		if( tm != NULL ) {
			hover = tm->GetColor(_T("color-bg-hover-medium"), hover);
			active = tm->GetColor(_T("color-bg-hover-primary"),
				tm->GetColor(_T("color-bg-hover"), active));
		}
		SetHoverBackgroundColor(hover);
		SetActiveBackgroundColor(active);
	}

	void CAppIconUI::SetKind(ControlKind kind)
	{
		CButtonUI::SetKind(kind);
		if( kind == CONTROLKIND_NONE )
			ApplyDefaultHoverChrome();
	}

	int CAppIconUI::ScaleValue(int v) const
	{
		return m_pManager ? m_pManager->GetDPIObj()->Scale(v) : v;
	}

	void CAppIconUI::SetBadgeCount(int n)
	{
		if( m_nBadgeCount == n ) return;
		m_nBadgeCount = n;
		Invalidate();
	}

	void CAppIconUI::SetBadgeOverflow(int n)
	{
		if( n < 1 ) n = 1;
		if( m_nBadgeOverflow == n ) return;
		m_nBadgeOverflow = n;
		Invalidate();
	}

	void CAppIconUI::SetBadgeShowZero(bool b)
	{
		if( m_bBadgeShowZero == b ) return;
		m_bBadgeShowZero = b;
		Invalidate();
	}

	void CAppIconUI::SetBadgeDot(bool b)
	{
		if( m_bBadgeDot == b ) return;
		m_bBadgeDot = b;
		Invalidate();
	}

	void CAppIconUI::SetBadgeHang(bool b)
	{
		if( m_bBadgeHang == b ) return;
		m_bBadgeHang = b;
		Invalidate();
	}

	void CAppIconUI::SetBadgeOffset(SIZE sz)
	{
		m_szBadgeOffset = sz;
		Invalidate();
	}

	void CAppIconUI::SetBadgeColor(DWORD dw)
	{
		if( m_dwBadgeColor == dw ) return;
		m_dwBadgeColor = dw;
		Invalidate();
	}

	void CAppIconUI::SetBadgeTextColor(DWORD dw)
	{
		if( m_dwBadgeTextColor == dw ) return;
		m_dwBadgeTextColor = dw;
		Invalidate();
	}

	bool CAppIconUI::ShouldShowBadge() const
	{
		if( m_bBadgeDot ) return true;
		if( m_nBadgeCount < 0 ) return false;
		if( m_nBadgeCount == 0 ) return m_bBadgeShowZero;
		return true;
	}

	CDuiString CAppIconUI::FormatBadgeCount() const
	{
		CDuiString s;
		if( m_nBadgeCount > m_nBadgeOverflow )
			s.Format(_T("%d+"), m_nBadgeOverflow);
		else
			s.Format(_T("%d"), m_nBadgeCount);
		return s;
	}

	SIZE CAppIconUI::MeasureBadgeSize() const
	{
		SIZE sz = { 0, 0 };
		if( !ShouldShowBadge() ) return sz;
		if( m_bBadgeDot ) {
			int d = ScaleValue(m_nBadgeDotSize);
			sz.cx = sz.cy = d;
			return sz;
		}
		CDuiString s = FormatBadgeCount();
		int h = ScaleValue(m_nBadgeHeight);
		RECT rcText = { 0, 0, 9999, h };
		RenderMeasureText(m_pManager, rcText, s.GetData(), 0, -1,
			DT_CALCRECT | DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		int tw = rcText.right - rcText.left;
		int pad = ScaleValue(6);
		int w = tw + pad * 2;
		if( w < h ) w = h;
		sz.cx = w;
		sz.cy = h;
		return sz;
	}

	RECT CAppIconUI::CalcIconHostRect() const
	{
		RECT rcPadding = GetPadding();
		RECT rcTextPadding = GetTextPadding();
		if( m_pManager != NULL )
			m_pManager->GetDPIObj()->Scale(&rcTextPadding);

		RECT rcContent = m_rcItem;
		rcContent.left += rcPadding.left + rcTextPadding.left;
		rcContent.right -= rcPadding.right + rcTextPadding.right;
		rcContent.top += rcPadding.top + rcTextPadding.top;
		rcContent.bottom -= rcPadding.bottom + rcTextPadding.bottom;

		RECT rcIcon = { 0 };
		RECT rcText = { 0 };
		if( HasIcon() && LayoutIconAndText(rcContent, rcIcon, rcText) )
			return rcIcon;
		if( IsTextIcon() && LayoutTextIconPlate(rcContent, rcIcon) )
			return rcIcon;
		return rcContent;
	}

	RECT CAppIconUI::CalcBadgeRect(const RECT& rcHost) const
	{
		RECT rc = { 0, 0, 0, 0 };
		SIZE sz = MeasureBadgeSize();
		int w = sz.cx;
		int h = sz.cy;
		if( w <= 0 || h <= 0 ) return rc;

		int ox = ScaleValue(m_szBadgeOffset.cx);
		int oy = ScaleValue(m_szBadgeOffset.cy);

		if( m_szBadgeOffset.cx != 0 || m_szBadgeOffset.cy != 0 ) {
			rc.right = rcHost.right + ox;
			rc.left = rc.right - w;
			rc.top = rcHost.top + oy;
			rc.bottom = rc.top + h;
		}
		else if( m_bBadgeHang ) {
			rc.left = rcHost.right - w / 2;
			rc.right = rc.left + w;
			rc.top = rcHost.top - h / 2;
			rc.bottom = rc.top + h;
		}
		else {
			rc.right = rcHost.right;
			rc.left = rc.right - w;
			rc.top = rcHost.top;
			rc.bottom = rc.top + h;
		}

		// 夹进本控件，避免被父裁剪成半截
		if( rc.right > m_rcItem.right ) {
			int d = rc.right - m_rcItem.right;
			rc.left -= d;
			rc.right -= d;
		}
		if( rc.left < m_rcItem.left ) {
			int d = m_rcItem.left - rc.left;
			rc.left += d;
			rc.right += d;
		}
		if( rc.top < m_rcItem.top ) {
			int d = m_rcItem.top - rc.top;
			rc.top += d;
			rc.bottom += d;
		}
		if( rc.bottom > m_rcItem.bottom ) {
			int d = rc.bottom - m_rcItem.bottom;
			rc.top -= d;
			rc.bottom -= d;
		}
		return rc;
	}

	void CAppIconUI::PaintBadge(IRenderContext& ctx)
	{
		if( !ShouldShowBadge() ) return;
		RECT rcHost = CalcIconHostRect();
		RECT rc = CalcBadgeRect(rcHost);
		int h = rc.bottom - rc.top;
		if( h <= 0 ) return;
		int r = h / 2;
		if( r < 1 ) r = 1;
		ctx.FillRoundRect(rc, r, r, GetAdjustColor(m_dwBadgeColor));
		if( !m_bBadgeDot ) {
			CDuiString s = FormatBadgeCount();
			ctx.DrawText(rc, s.GetData(), GetAdjustColor(m_dwBadgeTextColor), -1,
				DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		}
	}

	bool CAppIconUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		if( !CControlUI::DoPaint(ctx, rcPaint, pStopControl) )
			return false;
		PaintBadge(ctx);
		return true;
	}

	void CAppIconUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("width")) == 0
			|| _tcsicmp(pstrName, _T("min-width")) == 0
			|| _tcsicmp(pstrName, _T("max-width")) == 0
			|| _tcsicmp(pstrName, _T("fixedwidth")) == 0 ) {
			m_bWidthFromSkin = true;
			CButtonUI::SetAttribute(pstrName, pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("height")) == 0
			|| _tcsicmp(pstrName, _T("min-height")) == 0
			|| _tcsicmp(pstrName, _T("max-height")) == 0
			|| _tcsicmp(pstrName, _T("fixedheight")) == 0 ) {
			m_bHeightFromSkin = true;
			CButtonUI::SetAttribute(pstrName, pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("tooltip")) == 0 || _tcsicmp(pstrName, _T("tip")) == 0 ) {
			m_bTipFromSkin = true;
			m_bTipAuto = false;
			CButtonUI::SetAttribute(pstrName, pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("iconimage")) == 0 || _tcsicmp(pstrName, _T("icon-image")) == 0 ) {
			m_sFileIcon.Empty();
			SetIconSrc(pstrValue);
			SyncTextIconToolTip();
			SyncAutoSize();
		}
		else if( _tcsicmp(pstrName, _T("file")) == 0
			|| _tcsicmp(pstrName, _T("file-icon")) == 0
			|| _tcsicmp(pstrName, _T("icon-file")) == 0
			|| _tcsicmp(pstrName, _T("exe")) == 0
			|| _tcsicmp(pstrName, _T("exe-icon")) == 0 ) {
			SetFileIcon(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("background-color")) == 0
			|| _tcsicmp(pstrName, _T("bkcolor")) == 0 ) {
			// 整格底色（有 lucide 时铺满圆角格；文字图标时图标区也会跟随，见 ResolveTextIconBk）
			CButtonUI::SetAttribute(pstrName, pstrValue);
			Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("icon-background")) == 0
			|| _tcsicmp(pstrName, _T("icon-bk")) == 0
			|| _tcsicmp(pstrName, _T("text-icon-background")) == 0 ) {
			DWORD dw = 0;
			if( ParseColorString(pstrValue, dw) )
				SetTextIconBackground(dw);
		}
		else if( _tcsicmp(pstrName, _T("icon-color")) == 0
			|| _tcsicmp(pstrName, _T("text-icon-color")) == 0 ) {
			DWORD dw = 0;
			if( ParseColorString(pstrValue, dw) ) {
				m_dwTextIconFg = dw;
				m_bTextIconFgCustom = true;
			}
		}
		else if( _tcsicmp(pstrName, _T("badge")) == 0
			|| _tcsicmp(pstrName, _T("badge-count")) == 0
			|| _tcsicmp(pstrName, _T("badgecount")) == 0 ) {
			SetBadgeCount(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("badge-overflow")) == 0
			|| _tcsicmp(pstrName, _T("overflow-count")) == 0 ) {
			SetBadgeOverflow(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("badge-show-zero")) == 0
			|| _tcsicmp(pstrName, _T("show-zero")) == 0 ) {
			SetBadgeShowZero(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("badge-dot")) == 0 || _tcsicmp(pstrName, _T("dot")) == 0 ) {
			SetBadgeDot(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("badge-hang")) == 0 ) {
			SetBadgeHang(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("badge-offset")) == 0 ) {
			SIZE sz = { 0, 0 };
			LPTSTR p = NULL;
			sz.cx = _tcstol(pstrValue, &p, 10);
			if( p && *p == _T(',') ) sz.cy = _tcstol(p + 1, &p, 10);
			SetBadgeOffset(sz);
		}
		else if( _tcsicmp(pstrName, _T("badge-color")) == 0 ) {
			DWORD dw = 0;
			if( ParseColorString(pstrValue, dw) )
				SetBadgeColor(dw);
		}
		else if( _tcsicmp(pstrName, _T("badge-text-color")) == 0
			|| _tcsicmp(pstrName, _T("badge-color-text")) == 0 ) {
			DWORD dw = 0;
			if( ParseColorString(pstrValue, dw) )
				SetBadgeTextColor(dw);
		}
		else {
			// 换图标源时清 file 路径；icon-size 不在此列（走虚 SetIconSize → 可重取外壳图）
			const bool bMayToggleTextIcon = IsIconAttr(pstrName)
				|| _tcsicmp(pstrName, _T("icon")) == 0
				|| _tcsicmp(pstrName, _T("icon-src")) == 0
				|| _tcsicmp(pstrName, _T("iconsrc")) == 0;
			CButtonUI::SetAttribute(pstrName, pstrValue);
			if( bMayToggleTextIcon ) {
				m_sFileIcon.Empty();
				SyncTextIconToolTip();
				SyncAutoSize();
			}
		}
	}
}
