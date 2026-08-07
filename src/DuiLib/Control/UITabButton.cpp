#include "StdAfx.h"
#include "UITabButton.h"
#include "UITabBar.h"
#include "UISvgBox.h"
#include "UILoading.h"

namespace DuiLib
{
	namespace
	{
		DWORD ParseTabColor(LPCTSTR pstrValue)
		{
			DWORD c = 0;
			if( pstrValue != NULL && ParseColorString(pstrValue, c) ) return c;
			return 0;
		}
	}

	IMPLEMENT_DUICONTROL(CTabButtonUI)

	CTabButtonUI::CTabButtonUI()
		: m_pLeftPad(NULL)
		, m_pIcon(NULL)
		, m_pRasterIcon(NULL)
		, m_pLoading(NULL)
		, m_pIconGap(NULL)
		, m_pTitle(NULL)
		, m_pClose(NULL)
		, m_bActive(false)
		, m_bLocked(false)
		, m_bCloseHovered(false)
		, m_bHover(false)
		, m_bMemIcon(false)
		, m_bIconTint(false)
		, m_bIconTintAuto(false)
		, m_bRasterUsingTint(false)
		, m_dwIconTint(0)
		, m_nIconSize(14)
		, m_hRasterTint(NULL)
		, m_dwRasterTintColor(0)
		, m_nRasterTintW(0)
		, m_nRasterTintH(0)
		, m_pPendingIconData(NULL)
		, m_dwPendingIconSize(0)
		, m_hPendingIcon(NULL)
		, m_nPendingIconW(0)
		, m_nPendingIconH(0)
		, m_bPendingIconAlpha(true)
	{
		SetFixedWidth(150);
		SetMouseEnabled(false);
		SetAlignItems(DT_VCENTER);
		EnsureChildren();
		UpdateStyle();
	}

	CTabButtonUI::~CTabButtonUI()
	{
		ClearRasterTintCache();
		ClearPendingMemIcon();
		ReleaseMemIcon();
	}

	LPCTSTR CTabButtonUI::GetClass() const
	{
		return _T("TabButtonUI");
	}

	LPVOID CTabButtonUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_TABBUTTON) == 0 ) return static_cast<CTabButtonUI*>(this);
		return CHorizontalLayoutUI::GetInterface(pstrName);
	}

	CTabBarUI* CTabButtonUI::GetOwnerBar() const
	{
		if( m_pParent == NULL ) return NULL;
		return static_cast<CTabBarUI*>(m_pParent->GetInterface(DUI_CTR_TABBAR));
	}

	void CTabButtonUI::EnsureChildren()
	{
		if( m_pTitle != NULL ) return;

		m_pLeftPad = new CControlUI;
		m_pLeftPad->SetFixedWidth(8);
		m_pLeftPad->SetMouseEnabled(false);
		CHorizontalLayoutUI::Add(m_pLeftPad);

		m_pIcon = new CSvgBoxUI;
		m_pIcon->SetMouseEnabled(false);
		m_pIcon->SetVisible(false);
		m_pIcon->SetFixedWidth(0);
		m_pIcon->SetFixedHeight(0);
		CHorizontalLayoutUI::Add(m_pIcon);

		m_pRasterIcon = new CControlUI;
		m_pRasterIcon->SetMouseEnabled(false);
		m_pRasterIcon->SetVisible(false);
		m_pRasterIcon->SetFixedWidth(0);
		m_pRasterIcon->SetFixedHeight(0);
		CHorizontalLayoutUI::Add(m_pRasterIcon);

		m_pLoading = new CLoadingUI;
		m_pLoading->SetMouseEnabled(false);
		m_pLoading->SetVisible(false);
		m_pLoading->SetFixedWidth(0);
		m_pLoading->SetFixedHeight(0);
		m_pLoading->SetAttribute(_T("type"), _T("spoke"));
		CHorizontalLayoutUI::Add(m_pLoading);
		m_pLoading->Stop();

		m_pIconGap = new CControlUI;
		m_pIconGap->SetMouseEnabled(false);
		m_pIconGap->SetFixedWidth(0);
		CHorizontalLayoutUI::Add(m_pIconGap);

		m_pTitle = new CLabelUI;
		m_pTitle->SetMouseEnabled(false);
		m_pTitle->SetAttribute(_T("text-overflow"), _T("ellipsis"));
		CHorizontalLayoutUI::Add(m_pTitle);
		ApplyTitleTextAlign();

		m_pClose = new CLabelUI;
		m_pClose->SetText(_T("\x2715"));
		m_pClose->SetFixedWidth(20);
		m_pClose->SetMouseEnabled(false);
		m_pClose->SetAttribute(_T("text-align"), _T("center"));
		CHorizontalLayoutUI::Add(m_pClose);

		CControlUI* pRight = new CControlUI;
		pRight->SetFixedWidth(4);
		pRight->SetMouseEnabled(false);
		CHorizontalLayoutUI::Add(pRight);
	}

	void CTabButtonUI::DoInit()
	{
		EnsureChildren();
		FlushPendingMemIcon();
		UpdateStyle();
		if( m_pLoading != NULL && m_pLoading->IsVisible() && m_pLoading->IsStopped() )
			m_pLoading->Start();
		CHorizontalLayoutUI::DoInit();
	}

	void CTabButtonUI::SetTabTitle(LPCTSTR pstrTitle)
	{
		EnsureChildren();
		if( m_pTitle != NULL ) m_pTitle->SetText(pstrTitle ? pstrTitle : _T(""));
	}

	void CTabButtonUI::SetText(LPCTSTR pstrText)
	{
		// 可见标题在子 Label 上；勿只写基类 m_sText
		SetTabTitle(pstrText);
		CHorizontalLayoutUI::SetText(pstrText);
	}

	CDuiString CTabButtonUI::GetTabTitle() const
	{
		if( m_pTitle != NULL ) return m_pTitle->GetText();
		return CDuiString();
	}

	void CTabButtonUI::SetActive(bool bActive)
	{
		if( m_bActive == bActive ) {
			UpdateStyle();
			return;
		}
		m_bActive = bActive;
		m_bHover = false;
		UpdateStyle();
		Invalidate();
	}

	void CTabButtonUI::SetLocked(bool bLocked)
	{
		if( m_bLocked == bLocked ) {
			EnsureChildren();
			if( m_pClose != NULL )
				m_pClose->SetVisible(!m_bLocked);
			return;
		}
		m_bLocked = bLocked;
		EnsureChildren();
		if( m_pClose != NULL )
			m_pClose->SetVisible(!m_bLocked);
		NeedUpdate();
		CTabBarUI* pBar = GetOwnerBar();
		if( pBar != NULL )
			pBar->OnTabLockChanged(this);
	}

	void CTabButtonUI::SetCloseHover(bool bHover)
	{
		EnsureChildren();
		if( m_pClose == NULL || m_bLocked ) return;
		m_bCloseHovered = bHover;

		CTabBarUI* pBar = GetOwnerBar();
		DWORD dwCloseText = pBar ? pBar->GetCloseColor() : 0x8C8C8CFF;
		DWORD dwCloseHotBk = pBar ? pBar->GetCloseHoverBackgroundColor() : 0xDC3C3CFF;
		DWORD dwCloseHotText = pBar ? pBar->GetCloseHoverColor() : 0xFFFFFFFF;

		if( bHover ) {
			m_pClose->SetBackgroundColor(dwCloseHotBk);
			m_pClose->SetColor(dwCloseHotText);
		}
		else {
			m_pClose->SetBackgroundColor(0);
			m_pClose->SetColor(dwCloseText);
		}
		m_pClose->Invalidate();
	}

	RECT CTabButtonUI::GetCloseRect() const
	{
		RECT rc = { 0, 0, 0, 0 };
		if( m_pClose != NULL && m_pClose->IsVisible() && IsCloseFullyVisible() )
			rc = m_pClose->GetPos();
		return rc;
	}

	bool CTabButtonUI::IsCloseFullyVisible() const
	{
		if( m_pClose == NULL || !m_pClose->IsVisible() || m_bLocked )
			return false;
		CTabBarUI* pBar = GetOwnerBar();
		if( pBar == NULL ) return true;
		// 标签本身未完整入视口时关闭钮一并视为不可用
		return pBar->IsTabFullyInViewport(this);
	}

	void CTabButtonUI::SetUrl(LPCTSTR pstrUrl)
	{
		m_sUrl = pstrUrl ? pstrUrl : _T("");
	}

	void CTabButtonUI::SetDir(LPCTSTR pstrDir)
	{
		m_sDir = pstrDir ? pstrDir : _T("");
	}

	void CTabButtonUI::SetButtonWidth(int nWidth)
	{
		if( nWidth < 1 ) nWidth = 1;
		SetFixedWidth(nWidth);
	}

	int CTabButtonUI::GetButtonWidth() const
	{
		return GetFixedWidth();
	}

	void CTabButtonUI::SetIconSize(int nSize)
	{
		if( nSize < 8 ) nSize = 8;
		if( nSize > 32 ) nSize = 32;
		m_nIconSize = nSize;
		ApplyIconSize();
		NeedUpdate();
	}

	void CTabButtonUI::SetIconTint(DWORD dwColor)
	{
		m_bIconTint = (dwColor != 0);
		m_dwIconTint = dwColor;
		if( m_bIconTint ) m_bIconTintAuto = false;
		ClearRasterTintCache();
		UpdateStyle();
		Invalidate();
	}

	void CTabButtonUI::SetIconTintAuto(bool bAuto)
	{
		const bool bClearExplicit = bAuto && m_bIconTint;
		if( m_bIconTintAuto == bAuto && !bClearExplicit ) return;
		m_bIconTintAuto = bAuto;
		if( bAuto ) {
			m_bIconTint = false;
			m_dwIconTint = 0;
		}
		ClearRasterTintCache();
		UpdateStyle();
		Invalidate();
	}

	void CTabButtonUI::ApplyIconSize()
	{
		EnsureChildren();
		const bool bSvg = (m_pIcon != NULL && m_pIcon->IsVisible());
		const bool bRaster = (m_pRasterIcon != NULL && m_pRasterIcon->IsVisible());
		const bool bLoading = (m_pLoading != NULL && m_pLoading->IsVisible());
		if( m_pIcon != NULL ) {
			if( !bSvg ) {
				m_pIcon->SetFixedWidth(0);
				m_pIcon->SetFixedHeight(0);
			}
			else {
				m_pIcon->SetFixedWidth(m_nIconSize);
				m_pIcon->SetFixedHeight(m_nIconSize);
			}
		}
		if( m_pRasterIcon != NULL ) {
			if( !bRaster ) {
				m_pRasterIcon->SetFixedWidth(0);
				m_pRasterIcon->SetFixedHeight(0);
			}
			else {
				m_pRasterIcon->SetFixedWidth(m_nIconSize);
				m_pRasterIcon->SetFixedHeight(m_nIconSize);
				RefreshRasterIconImage();
			}
		}
		if( m_pLoading != NULL ) {
			if( !bLoading ) {
				m_pLoading->SetFixedWidth(0);
				m_pLoading->SetFixedHeight(0);
			}
			else {
				m_pLoading->SetFixedWidth(m_nIconSize);
				m_pLoading->SetFixedHeight(m_nIconSize);
			}
		}
		if( m_pIconGap != NULL )
			m_pIconGap->SetFixedWidth((bSvg || bRaster || bLoading) ? 4 : 0);
	}

	bool CTabButtonUI::IsRasterImagePath(LPCTSTR pstrPath)
	{
		if( pstrPath == NULL || *pstrPath == _T('\0') ) return false;
		CDuiString s(pstrPath);
		s.MakeLower();
		// 支持 file='xxx.png' dest='...' 形式：取扩展名
		LPCTSTR pExt = NULL;
		for( LPCTSTR p = s.GetData(); *p != _T('\0'); ++p ) {
			if( *p == _T('.') ) pExt = p;
			else if( *p == _T('\'') || *p == _T('"') || *p == _T(' ') || *p == _T('\t') ) {
				if( pExt != NULL ) break;
			}
		}
		if( pExt == NULL ) return false;
		return _tcsncmp(pExt, _T(".bmp"), 4) == 0
			|| _tcsncmp(pExt, _T(".png"), 4) == 0
			|| _tcsncmp(pExt, _T(".jpg"), 4) == 0
			|| _tcsncmp(pExt, _T(".jpeg"), 5) == 0;
	}

	void CTabButtonUI::RefreshRasterIconImage()
	{
		SyncRasterIconAppearance();
	}

	void CTabButtonUI::ClearRasterTintCache()
	{
		if( m_hRasterTint != NULL ) {
			IRenderDevice* pDev = GetRenderDevice();
			if( pDev != NULL ) pDev->InvalidateBitmapGpu(m_hRasterTint);
			::DeleteObject(m_hRasterTint);
			m_hRasterTint = NULL;
		}
		m_dwRasterTintColor = 0;
		m_nRasterTintW = 0;
		m_nRasterTintH = 0;
	}

	bool CTabButtonUI::ShouldTintRasterIcon() const
	{
		if( m_pRasterIcon == NULL || !m_pRasterIcon->IsVisible() ) return false;
		if( m_sIconPath.IsEmpty() ) return false;
		return m_bIconTintAuto || m_bIconTint;
	}

	DWORD CTabButtonUI::ResolvePaintIconColor() const
	{
		DWORD c = ResolveIconColor();
		if( c != 0 ) return c;
		// auto：跟标签文字色（与 SVG UpdateStyle 一致）
		CTabBarUI* pBar = GetOwnerBar();
		DWORD dwText = pBar ? pBar->GetTabColor() : 0x8C8C8CFF;
		DWORD dwHotText = pBar ? pBar->GetTabHoverColor() : 0x1677FFFF;
		DWORD dwSelText = pBar ? pBar->GetTabSelectedColor() : 0x1677FFFF;
		if( m_bActive ) return dwSelText != 0 ? dwSelText : dwText;
		if( m_bHover ) return dwHotText != 0 ? dwHotText : dwText;
		return dwText;
	}

	bool CTabButtonUI::EnsureRasterTintCache(DWORD dwColor)
	{
		if( m_pManager == NULL || m_sIconPath.IsEmpty() || dwColor == 0 )
			return false;

		const int nSize = m_nIconSize;
		if( nSize <= 0 ) return false;

		if( m_hRasterTint != NULL && m_dwRasterTintColor == dwColor
			&& m_nRasterTintW == nSize && m_nRasterTintH == nSize )
			return true;

		ClearRasterTintCache();

		CDuiString sName = m_sIconPath;
		const int nFile = sName.Find(_T("file='"));
		if( nFile >= 0 ) {
			sName = sName.Mid(nFile + 6);
			const int nEnd = sName.Find(_T('\''));
			if( nEnd >= 0 ) sName = sName.Left(nEnd);
		}
		else {
			const int nUrl = sName.Find(_T("url("));
			if( nUrl >= 0 ) {
				CDuiString sPath;
				if( ParseCssUrlImage(m_sIconPath.GetData(), sPath) )
					sName = sPath;
			}
		}

		const TImageInfo* pSrc = m_pManager->GetImageEx(sName.GetData());
		if( pSrc == NULL || pSrc->hBitmap == NULL || pSrc->nX <= 0 || pSrc->nY <= 0 )
			return false;

		BITMAP bm = { 0 };
		if( !::GetObject(pSrc->hBitmap, sizeof(bm), &bm) || bm.bmWidth <= 0 || bm.bmHeight <= 0 )
			return false;

		LPBYTE pSrcBits = NULL;
		BYTE* pTempBits = NULL;
		if( bm.bmBits != NULL ) {
			pSrcBits = (LPBYTE)bm.bmBits;
		}
		else if( pSrc->pBits != NULL ) {
			pSrcBits = pSrc->pBits;
		}
		else {
			pTempBits = new BYTE[pSrc->nX * pSrc->nY * 4];
			BITMAPINFO bmi = { 0 };
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = pSrc->nX;
			bmi.bmiHeader.biHeight = -pSrc->nY;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;
			HDC hScreen = ::GetDC(NULL);
			int nCopied = ::GetDIBits(hScreen, pSrc->hBitmap, 0, pSrc->nY, pTempBits, &bmi, DIB_RGB_COLORS);
			::ReleaseDC(NULL, hScreen);
			if( nCopied == 0 ) {
				delete[] pTempBits;
				return false;
			}
			pSrcBits = pTempBits;
		}

		BITMAPINFO bmiOut = { 0 };
		bmiOut.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmiOut.bmiHeader.biWidth = nSize;
		bmiOut.bmiHeader.biHeight = -nSize;
		bmiOut.bmiHeader.biPlanes = 1;
		bmiOut.bmiHeader.biBitCount = 32;
		bmiOut.bmiHeader.biCompression = BI_RGB;
		LPBYTE pDest = NULL;
		HBITMAP hTint = ::CreateDIBSection(NULL, &bmiOut, DIB_RGB_COLORS, (void**)&pDest, NULL, 0);
		if( hTint == NULL || pDest == NULL ) {
			delete[] pTempBits;
			return false;
		}

		const BYTE tR = DuiColorR(dwColor);
		const BYTE tG = DuiColorG(dwColor);
		const BYTE tB = DuiColorB(dwColor);
		const int srcW = pSrc->nX;
		const int srcH = pSrc->nY;

		for( int y = 0; y < nSize; ++y ) {
			const int sy = y * srcH / nSize;
			for( int x = 0; x < nSize; ++x ) {
				const int sx = x * srcW / nSize;
				const BYTE* pS = pSrcBits + (sy * srcW + sx) * 4;
				BYTE* pD = pDest + (y * nSize + x) * 4;
				BYTE a = pS[3];
				if( !pSrc->bAlpha ) {
					const int lum = (pS[2] * 30 + pS[1] * 59 + pS[0] * 11) / 100;
					a = (BYTE)(255 - lum);
				}
				pD[0] = (BYTE)((DWORD)tB * a / 255);
				pD[1] = (BYTE)((DWORD)tG * a / 255);
				pD[2] = (BYTE)((DWORD)tR * a / 255);
				pD[3] = a;
			}
		}

		delete[] pTempBits;
		m_hRasterTint = hTint;
		m_dwRasterTintColor = dwColor;
		m_nRasterTintW = nSize;
		m_nRasterTintH = nSize;
		return true;
	}

	void CTabButtonUI::SyncRasterIconAppearance()
	{
		if( m_pRasterIcon == NULL || m_sIconPath.IsEmpty() || !m_pRasterIcon->IsVisible() ) {
			ClearRasterTintCache();
			m_bRasterUsingTint = false;
			return;
		}

		if( ShouldTintRasterIcon() ) {
			const DWORD paint = ResolvePaintIconColor();
			if( paint != 0 && EnsureRasterTintCache(paint) ) {
				m_pRasterIcon->SetBackgroundImage(_T(""));
				m_bRasterUsingTint = true;
				return;
			}
		}

		ClearRasterTintCache();
		m_bRasterUsingTint = false;

		CDuiString sImg;
		if( m_bMemIcon ) {
			sImg.Format(_T("file='%s' dest='0,0,%d,%d'"),
				m_sIconPath.GetData(), m_nIconSize, m_nIconSize);
		}
		else {
			sImg = m_sIconPath;
			if( sImg.Find(_T("file=")) < 0 && sImg.Find(_T("res=")) < 0
				&& sImg.Find(_T("url(")) < 0 ) {
				CDuiString sFmt;
				sFmt.Format(_T("file='%s' dest='0,0,%d,%d'"), m_sIconPath.GetData(), m_nIconSize, m_nIconSize);
				sImg = sFmt;
			}
			else if( sImg.Find(_T("dest=")) < 0 ) {
				CDuiString sFmt;
				sFmt.Format(_T("%s dest='0,0,%d,%d'"), m_sIconPath.GetData(), m_nIconSize, m_nIconSize);
				sImg = sFmt;
			}
		}
		m_pRasterIcon->SetBackgroundImage(sImg.GetData());
	}

	void CTabButtonUI::ReleaseMemIcon()
	{
		if( !m_sMemIconKey.IsEmpty() && m_pManager != NULL )
			m_pManager->RemoveImage(m_sMemIconKey.GetData());
		m_sMemIconKey.Empty();
		m_bMemIcon = false;
	}

	void CTabButtonUI::ClearPendingMemIcon()
	{
		if( m_pPendingIconData != NULL ) {
			delete[] m_pPendingIconData;
			m_pPendingIconData = NULL;
		}
		m_dwPendingIconSize = 0;
		if( m_hPendingIcon != NULL ) {
			::DeleteObject(m_hPendingIcon);
			m_hPendingIcon = NULL;
		}
		m_nPendingIconW = m_nPendingIconH = 0;
		m_bPendingIconAlpha = true;
	}

	bool CTabButtonUI::InstallMemIcon(HBITMAP hBitmap, int nWidth, int nHeight, bool bAlpha)
	{
		if( hBitmap == NULL || nWidth <= 0 || nHeight <= 0 ) return false;
		if( m_pManager == NULL ) return false;

		ReleaseMemIcon();
		static volatile LONG s_nMemIconSeq = 0;
		LONG nSeq = ::InterlockedIncrement(&s_nMemIconSeq);
		m_sMemIconKey.Format(_T("tabicon_mem_%p_%ld"), this, nSeq);

		const TImageInfo* pAdded = m_pManager->AddImage(
			m_sMemIconKey.GetData(), hBitmap, nWidth, nHeight, bAlpha, false);
		if( pAdded == NULL ) {
			::DeleteObject(hBitmap);
			m_sMemIconKey.Empty();
			return false;
		}
		// AddImage 接管 hBitmap
		m_bMemIcon = true;
		ShowRasterIcon(m_sMemIconKey.GetData(), true);
		ApplyIconSize();
		UpdateStyle();
		NeedUpdate();
		return true;
	}

	bool CTabButtonUI::FlushPendingMemIcon()
	{
		if( m_pManager == NULL ) return false;
		if( m_hPendingIcon != NULL ) {
			HBITMAP h = m_hPendingIcon;
			int w = m_nPendingIconW;
			int hgt = m_nPendingIconH;
			bool bA = m_bPendingIconAlpha;
			m_hPendingIcon = NULL;
			m_nPendingIconW = m_nPendingIconH = 0;
			return InstallMemIcon(h, w, hgt, bA);
		}
		if( m_pPendingIconData != NULL && m_dwPendingIconSize > 0 ) {
			TImageInfo* pInfo = CRenderEngine::LoadImageFromMemory(
				m_pPendingIconData, m_dwPendingIconSize, 0);
			delete[] m_pPendingIconData;
			m_pPendingIconData = NULL;
			m_dwPendingIconSize = 0;
			if( pInfo == NULL || pInfo->hBitmap == NULL ) {
				if( pInfo != NULL ) CRenderEngine::FreeImage(pInfo);
				return false;
			}
			HBITMAP hBmp = pInfo->hBitmap;
			int w = pInfo->nX;
			int h = pInfo->nY;
			bool bA = pInfo->bAlpha;
			pInfo->hBitmap = NULL;
			CRenderEngine::FreeImage(pInfo);
			return InstallMemIcon(hBmp, w, h, bA);
		}
		return false;
	}

	void CTabButtonUI::HideLoadingIcon()
	{
		if( m_pLoading == NULL ) return;
		m_pLoading->Stop();
		m_pLoading->SetVisible(false);
		m_pLoading->SetFixedWidth(0);
		m_pLoading->SetFixedHeight(0);
	}

	void CTabButtonUI::ApplyLoadingAppearance()
	{
		if( m_pLoading == NULL ) return;
		CTabBarUI* pBar = GetOwnerBar();
		LPCTSTR pType = NULL;
		if( !m_sLoadingType.IsEmpty() )
			pType = m_sLoadingType.GetData();
		else if( pBar != NULL )
			pType = pBar->GetTabLoadingType();
		if( pType == NULL || *pType == _T('\0') )
			pType = _T("spoke");
		m_pLoading->SetAttribute(_T("type"), pType);

		DWORD dwColor = 0;
		if( pBar != NULL )
			dwColor = pBar->GetTabLoadingColor();
		if( dwColor == 0 )
			dwColor = ResolveIconColor();
		if( dwColor == 0 )
			dwColor = 0x4FC3F7FF;
		CDuiString sClr;
		sClr.Format(_T("#%08X"), dwColor);
		m_pLoading->SetAttribute(_T("color"), sClr.GetData());
	}

	DWORD CTabButtonUI::ResolveIconColor() const
	{
		if( m_bIconTint && m_dwIconTint != 0 )
			return m_dwIconTint;
		CTabBarUI* pBar = GetOwnerBar();
		if( pBar == NULL ) return 0;
		DWORD dwBarIcon = 0;
		if( m_bActive )
			dwBarIcon = pBar->GetTabIconSelectedColor();
		else if( m_bHover )
			dwBarIcon = pBar->GetTabIconHoverColor();
		else
			dwBarIcon = pBar->GetTabIconColor();
		if( dwBarIcon == 0 )
			dwBarIcon = pBar->GetTabIconColor();
		return dwBarIcon;
	}

	void CTabButtonUI::SetTabLoading(bool bLoading)
	{
		EnsureChildren();
		if( m_pLoading == NULL ) return;
		if( bLoading ) {
			if( m_pIcon != NULL ) {
				m_pIcon->SetVisible(false);
				m_pIcon->SetFixedWidth(0);
				m_pIcon->SetFixedHeight(0);
			}
			if( m_pRasterIcon != NULL ) {
				m_pRasterIcon->SetVisible(false);
				m_pRasterIcon->SetFixedWidth(0);
				m_pRasterIcon->SetFixedHeight(0);
			}
			ApplyLoadingAppearance();
			m_pLoading->SetVisible(true);
			ApplyIconSize();
			m_pLoading->Start();
			UpdateStyle();
			NeedUpdate();
		}
		else {
			HideLoadingIcon();
			ApplyIconSize();
			NeedUpdate();
		}
	}

	bool CTabButtonUI::IsTabLoading() const
	{
		return m_pLoading != NULL && m_pLoading->IsVisible();
	}

	void CTabButtonUI::SetLoadingType(LPCTSTR pstrType)
	{
		m_sLoadingType = pstrType ? pstrType : _T("");
		if( IsTabLoading() )
			ApplyLoadingAppearance();
	}

	void CTabButtonUI::ShowSvgIcon()
	{
		EnsureChildren();
		HideLoadingIcon();
		ReleaseMemIcon();
		ClearPendingMemIcon();
		ClearRasterTintCache();
		m_bRasterUsingTint = false;
		if( m_pRasterIcon != NULL ) {
			m_pRasterIcon->SetVisible(false);
			m_pRasterIcon->SetBackgroundImage(_T(""));
			m_pRasterIcon->SetFixedWidth(0);
			m_pRasterIcon->SetFixedHeight(0);
		}
		if( m_pIcon != NULL )
			m_pIcon->SetVisible(true);
	}

	void CTabButtonUI::ShowRasterIcon(LPCTSTR pstrPath, bool bMemoryKey)
	{
		EnsureChildren();
		HideLoadingIcon();
		ClearRasterTintCache();
		m_bRasterUsingTint = false;
		if( m_pIcon != NULL ) {
			m_pIcon->SetVisible(false);
			m_pIcon->SetFixedWidth(0);
			m_pIcon->SetFixedHeight(0);
		}
		m_sIconPath = pstrPath ? pstrPath : _T("");
		m_bMemIcon = bMemoryKey;
		if( m_pRasterIcon != NULL )
			m_pRasterIcon->SetVisible(true);
		RefreshRasterIconImage();
	}

	void CTabButtonUI::SetTabIcon(LPCTSTR pstrPath)
	{
		EnsureChildren();
		if( pstrPath == NULL || *pstrPath == _T('\0') ) {
			ClearTabIcon();
			return;
		}
		ClearPendingMemIcon();
		ReleaseMemIcon();
		if( IsRasterImagePath(pstrPath) ) {
			ShowRasterIcon(pstrPath, false);
		}
		else {
			// .svg 或其它：走 SvgBox 文件加载
			m_sIconPath = pstrPath;
			m_bMemIcon = false;
			ShowSvgIcon();
			if( m_pIcon != NULL )
				m_pIcon->SetAttribute(_T("src"), pstrPath);
		}
		ApplyIconSize();
		UpdateStyle();
		NeedUpdate();
	}

	bool CTabButtonUI::SetTabIcon(const BYTE* pData, DWORD dwSize)
	{
		EnsureChildren();
		if( pData == NULL || dwSize == 0 ) {
			ClearTabIcon();
			return false;
		}
		ClearPendingMemIcon();
		ReleaseMemIcon();

		if( m_pManager == NULL ) {
			m_pPendingIconData = new BYTE[dwSize];
			if( m_pPendingIconData == NULL ) return false;
			::CopyMemory(m_pPendingIconData, pData, dwSize);
			m_dwPendingIconSize = dwSize;
			return true;
		}

		TImageInfo* pInfo = CRenderEngine::LoadImageFromMemory(pData, dwSize, 0);
		if( pInfo == NULL || pInfo->hBitmap == NULL ) {
			if( pInfo != NULL ) CRenderEngine::FreeImage(pInfo);
			return false;
		}
		HBITMAP hBmp = pInfo->hBitmap;
		int w = pInfo->nX;
		int h = pInfo->nY;
		bool bA = pInfo->bAlpha;
		pInfo->hBitmap = NULL;
		CRenderEngine::FreeImage(pInfo);
		return InstallMemIcon(hBmp, w, h, bA);
	}

	bool CTabButtonUI::SetTabIcon(HBITMAP hBitmap, int nWidth, int nHeight, bool bAlpha)
	{
		EnsureChildren();
		if( hBitmap == NULL ) {
			ClearTabIcon();
			return false;
		}
		if( nWidth <= 0 || nHeight <= 0 ) {
			BITMAP bm = { 0 };
			if( ::GetObject(hBitmap, sizeof(bm), &bm) == 0 ) return false;
			nWidth = bm.bmWidth;
			nHeight = bm.bmHeight;
		}
		if( nWidth <= 0 || nHeight <= 0 ) return false;

		ClearPendingMemIcon();
		ReleaseMemIcon();

		HBITMAP hCopy = (HBITMAP)::CopyImage(hBitmap, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
		if( hCopy == NULL ) return false;

		if( m_pManager == NULL ) {
			m_hPendingIcon = hCopy;
			m_nPendingIconW = nWidth;
			m_nPendingIconH = nHeight;
			m_bPendingIconAlpha = bAlpha;
			return true;
		}
		return InstallMemIcon(hCopy, nWidth, nHeight, bAlpha);
	}

	void CTabButtonUI::SetTabIconLib(LPCTSTR pstrLib, LPCTSTR pstrName)
	{
		EnsureChildren();
		if( pstrLib != NULL && (_tcsicmp(pstrLib, _T("icon-src")) == 0 || _tcsicmp(pstrLib, _T("icon")) == 0) ) {
			SetTabIcon(pstrName);
			return;
		}
		if( pstrLib == NULL || *pstrLib == _T('\0')
			|| pstrName == NULL || *pstrName == _T('\0')
			|| !IsIconAttr(pstrLib) ) {
			ClearTabIcon();
			return;
		}
		ClearPendingMemIcon();
		ReleaseMemIcon();
		m_sIconPath.Empty();
		m_bMemIcon = false;
		ShowSvgIcon();
		if( m_pIcon != NULL )
			m_pIcon->SetAttribute(pstrLib, pstrName);
		ApplyIconSize();
		UpdateStyle();
		NeedUpdate();
	}

	void CTabButtonUI::ClearTabIcon()
	{
		EnsureChildren();
		HideLoadingIcon();
		ClearPendingMemIcon();
		ReleaseMemIcon();
		ClearRasterTintCache();
		m_bRasterUsingTint = false;
		m_sIconPath.Empty();
		m_bMemIcon = false;
		if( m_pIcon != NULL ) {
			m_pIcon->SetVisible(false);
			m_pIcon->SetFixedWidth(0);
			m_pIcon->SetFixedHeight(0);
		}
		if( m_pRasterIcon != NULL ) {
			m_pRasterIcon->SetVisible(false);
			m_pRasterIcon->SetBackgroundImage(_T(""));
			m_pRasterIcon->SetFixedWidth(0);
			m_pRasterIcon->SetFixedHeight(0);
		}
		if( m_pIconGap != NULL )
			m_pIconGap->SetFixedWidth(0);
		NeedUpdate();
	}

	bool CTabButtonUI::HasTabIcon() const
	{
		return (m_pIcon != NULL && m_pIcon->IsVisible())
			|| (m_pRasterIcon != NULL && m_pRasterIcon->IsVisible())
			|| (m_pLoading != NULL && m_pLoading->IsVisible());
	}

	void CTabButtonUI::SetTitleTextAlign(LPCTSTR pstrAlign)
	{
		m_sTextAlign = pstrAlign ? pstrAlign : _T("");
		ApplyTitleTextAlign();
		Invalidate();
	}

	void CTabButtonUI::SetTitleVerticalAlign(LPCTSTR pstrAlign)
	{
		m_sVerticalAlign = pstrAlign ? pstrAlign : _T("");
		ApplyTitleTextAlign();
		Invalidate();
	}

	void CTabButtonUI::ApplyTitleTextAlign()
	{
		if( m_pTitle == NULL ) return;
		CDuiString sH = m_sTextAlign;
		CDuiString sV = m_sVerticalAlign;
		CTabBarUI* pBar = GetOwnerBar();
		if( sH.IsEmpty() && pBar != NULL ) sH = pBar->GetTabTextAlign();
		if( sV.IsEmpty() && pBar != NULL ) sV = pBar->GetTabVerticalAlign();
		if( sH.IsEmpty() ) sH = _T("left");
		if( sV.IsEmpty() ) sV = _T("vcenter");
		m_pTitle->SetAttribute(_T("text-align"), sH.GetData());
		m_pTitle->SetAttribute(_T("vertical-align"), sV.GetData());
	}

	void CTabButtonUI::ApplyHoverStyle(bool bHover)
	{
		if( m_bHover == bHover ) return;
		m_bHover = bHover;
		if( m_bActive ) return;
		UpdateStyle();
		Invalidate();
	}

	void CTabButtonUI::UpdateStyle()
	{
		EnsureChildren();

		CTabBarUI* pBar = GetOwnerBar();
		DWORD dwBk = pBar ? pBar->GetTabBackgroundColor() : 0;
		DWORD dwHotBk = pBar ? pBar->GetTabHoverBackgroundColor() : 0xD6EBFFFF;
		DWORD dwSelBk = pBar ? pBar->GetTabSelectedBackgroundColor() : 0xBAE0FFFF;
		DWORD dwText = pBar ? pBar->GetTabColor() : 0x8C8C8CFF;
		DWORD dwHotText = pBar ? pBar->GetTabHoverColor() : 0x1677FFFF;
		DWORD dwSelText = pBar ? pBar->GetTabSelectedColor() : 0x1677FFFF;
		DWORD dwBorder = pBar ? pBar->GetTabBorderColor() : 0;
		DWORD dwSelBorder = pBar ? pBar->GetTabSelectedBorderColor() : 0x1677FFFF;
		int nBorder = pBar ? pBar->GetTabBorderWidth() : 0;
		int nSelBorder = pBar ? pBar->GetTabSelectedBorderWidth() : 2;
		DWORD dwCloseText = pBar ? pBar->GetCloseColor() : 0x8C8C8CFF;

		// 清掉单边边框，避免选中底边指示残留到未选中态
		RECT rcBorderEmpty = { 0, 0, 0, 0 };
		SetBorderWidth(rcBorderEmpty);
		SetBorderWidth(0);
		SetBorderColor(0);

		// 分隔：用右边框；末项不加；选中项及其左侧邻居不加（贴着选中底色更干净）
		bool bSepRight = false;
		DWORD dwSep = 0;
		if( pBar != NULL && pBar->IsShowTabSeparator() ) {
			dwSep = pBar->GetTabSeparatorColor();
			int idx = pBar->GetTabIndex(this);
			int nCount = pBar->GetTabCount();
			int iActive = pBar->GetActiveTab();
			bSepRight = (dwSep != 0 && idx >= 0 && idx < nCount - 1
				&& idx != iActive && (idx + 1) != iActive);
		}

		auto applySepOrBorder = [&]() {
			if( nBorder > 0 && dwBorder != 0 ) {
				SetBorderWidth(nBorder);
				SetBorderColor(dwBorder);
			}
			else if( bSepRight ) {
				RECT rcSep = { 0, 0, 1, 0 };
				SetBorderWidth(rcSep);
				SetBorderColor(dwSep);
			}
		};

		DWORD clrText = dwText;
		if( m_bActive ) {
			SetBackgroundColor(dwSelBk);
			clrText = dwSelText;
			DWORD dwInd = (dwSelBorder != 0) ? dwSelBorder : dwSelText;
			SetBorderColor(dwInd);
			if( nSelBorder > 0 ) {
				// 现代标签：底部色条指示选中（非整框描边）
				RECT rcInd = { 0, 0, 0, nSelBorder };
				SetBorderWidth(rcInd);
			}
		}
		else if( m_bHover ) {
			SetBackgroundColor(dwHotBk);
			clrText = (dwHotText != 0) ? dwHotText : dwText;
			applySepOrBorder();
		}
		else {
			SetBackgroundColor(dwBk);
			clrText = dwText;
			applySepOrBorder();
		}

		if( m_pTitle != NULL ) m_pTitle->SetColor(clrText);
		ApplyTitleTextAlign();
		if( m_pIcon != NULL && m_pIcon->IsVisible() ) {
			DWORD clrIcon = clrText;
			DWORD dwBarIcon = ResolveIconColor();
			if( dwBarIcon != 0 )
				clrIcon = dwBarIcon;
			else if( m_bIconTint )
				clrIcon = m_dwIconTint;
			m_pIcon->SetColor(clrIcon);
		}
		if( m_pRasterIcon != NULL && m_pRasterIcon->IsVisible() )
			SyncRasterIconAppearance();
		if( m_pLoading != NULL && m_pLoading->IsVisible() )
			ApplyLoadingAppearance();

		if( m_pClose != NULL && !m_bCloseHovered ) {
			m_pClose->SetBackgroundColor(0);
			m_pClose->SetColor(dwCloseText);
		}
	}

	bool CTabButtonUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		bool bHideClose = false;
		if( m_pClose != NULL && m_pClose->IsVisible() && !IsCloseFullyVisible() ) {
			m_pClose->SetInternVisible(false);
			bHideClose = true;
		}
		bool bRet = CHorizontalLayoutUI::DoPaint(ctx, rcPaint, pStopControl);
		if( bHideClose )
			m_pClose->SetInternVisible(true);

		// 光栅 tint：子控件清空 bkimage，在此按布局矩形绘制着色位图
		if( m_bRasterUsingTint && m_hRasterTint != NULL
			&& m_pRasterIcon != NULL && m_pRasterIcon->IsVisible() ) {
			RECT rcIcon = m_pRasterIcon->GetPos();
			RECT rcBmp = { 0, 0, m_nRasterTintW, m_nRasterTintH };
			RECT rcCorners = { 0, 0, 0, 0 };
			ctx.DrawImage(m_hRasterTint, rcIcon, rcPaint, rcBmp, rcCorners, true);
		}
		return bRet;
	}

	bool CTabButtonUI::IsIconAttr(LPCTSTR pstrName) const
	{
		return _tcsicmp(pstrName, _T("bsicon")) == 0
			|| _tcsicmp(pstrName, _T("iconpark")) == 0
			|| _tcsicmp(pstrName, _T("lucide")) == 0
			|| _tcsicmp(pstrName, _T("tabler-outline")) == 0
			|| _tcsicmp(pstrName, _T("tabler-filled")) == 0
			|| _tcsicmp(pstrName, _T("remixicon")) == 0
			|| _tcsicmp(pstrName, _T("twicon")) == 0
			|| _tcsicmp(pstrName, _T("icon-src")) == 0
			|| _tcsicmp(pstrName, _T("icon")) == 0;
	}

	void CTabButtonUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("text")) == 0 || _tcsicmp(pstrName, _T("title")) == 0 ) {
			SetTabTitle(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("active")) == 0 ) {
			SetActive(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("locked")) == 0 ) {
			SetLocked(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("url")) == 0 ) {
			SetUrl(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("dir")) == 0 ) {
			SetDir(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("icon-size")) == 0 ) {
			SetIconSize(_ttoi(pstrValue));
		}
		else if( IsIconAttr(pstrName) ) {
			if( pstrValue == NULL || *pstrValue == _T('\0') ) {
				ClearTabIcon();
				return;
			}
			if( _tcsicmp(pstrName, _T("icon-src")) == 0 || _tcsicmp(pstrName, _T("icon")) == 0 )
				SetTabIcon(pstrValue);
			else
				SetTabIconLib(pstrName, pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("icon-tint")) == 0
			|| _tcsicmp(pstrName, _T("icon-color")) == 0 ) {
			EnsureChildren();
			if( pstrValue == NULL || *pstrValue == _T('\0')
				|| _tcsicmp(pstrValue, _T("none")) == 0
				|| _tcsicmp(pstrValue, _T("false")) == 0
				|| _tcsicmp(pstrValue, _T("original")) == 0 ) {
				SetIconTintAuto(false);
				SetIconTint(0);
			}
			else if( _tcsicmp(pstrValue, _T("auto")) == 0
				|| _tcsicmp(pstrValue, _T("true")) == 0 ) {
				SetIconTintAuto(true);
			}
			else {
				SetIconTint(ParseTabColor(pstrValue));
			}
		}
		else if( _tcsicmp(pstrName, _T("loading")) == 0 ) {
			SetTabLoading(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("loading-type")) == 0 ) {
			SetLoadingType(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("text-align")) == 0 || _tcsicmp(pstrName, _T("align")) == 0 ) {
			SetTitleTextAlign(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("vertical-align")) == 0 || _tcsicmp(pstrName, _T("valign")) == 0 ) {
			SetTitleVerticalAlign(pstrValue);
		}
		else {
			CHorizontalLayoutUI::SetAttribute(pstrName, pstrValue);
		}
	}
}
