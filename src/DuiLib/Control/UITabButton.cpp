#include "StdAfx.h"
#include "UITabButton.h"
#include "UITabBar.h"
#include "UISvgBox.h"

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
		, m_pIconGap(NULL)
		, m_pTitle(NULL)
		, m_pClose(NULL)
		, m_bActive(false)
		, m_bLocked(false)
		, m_bCloseHovered(false)
		, m_bHover(false)
		, m_nIconSize(14)
	{
		SetFixedWidth(150);
		SetMouseEnabled(false);
		SetAlignItems(DT_VCENTER);
		EnsureChildren();
		UpdateStyle();
	}

	CTabButtonUI::~CTabButtonUI()
	{
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

		m_pIconGap = new CControlUI;
		m_pIconGap->SetMouseEnabled(false);
		m_pIconGap->SetFixedWidth(0);
		CHorizontalLayoutUI::Add(m_pIconGap);

		m_pTitle = new CLabelUI;
		m_pTitle->SetMouseEnabled(false);
		m_pTitle->SetAttribute(_T("text-align"), _T("left"));
		m_pTitle->SetAttribute(_T("text-overflow"), _T("ellipsis"));
		CHorizontalLayoutUI::Add(m_pTitle);

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
		UpdateStyle();
		CHorizontalLayoutUI::DoInit();
	}

	void CTabButtonUI::SetTabTitle(LPCTSTR pstrTitle)
	{
		EnsureChildren();
		if( m_pTitle != NULL ) m_pTitle->SetText(pstrTitle ? pstrTitle : _T(""));
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

	void CTabButtonUI::ApplyIconSize()
	{
		EnsureChildren();
		if( m_pIcon == NULL ) return;
		if( !m_pIcon->IsVisible() ) {
			m_pIcon->SetFixedWidth(0);
			m_pIcon->SetFixedHeight(0);
			if( m_pIconGap != NULL ) m_pIconGap->SetFixedWidth(0);
			return;
		}
		m_pIcon->SetFixedWidth(m_nIconSize);
		m_pIcon->SetFixedHeight(m_nIconSize);
		if( m_pIconGap != NULL ) m_pIconGap->SetFixedWidth(4);
	}

	void CTabButtonUI::ClearTabIcon()
	{
		EnsureChildren();
		if( m_pIcon == NULL ) return;
		m_pIcon->SetVisible(false);
		ApplyIconSize();
		NeedUpdate();
	}

	bool CTabButtonUI::HasTabIcon() const
	{
		return m_pIcon != NULL && m_pIcon->IsVisible();
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
		if( m_pIcon != NULL && m_pIcon->IsVisible() )
			m_pIcon->SetColor(clrText);

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
			EnsureChildren();
			if( m_pIcon == NULL ) return;
			if( pstrValue == NULL || *pstrValue == _T('\0') ) {
				ClearTabIcon();
				return;
			}
			if( _tcsicmp(pstrName, _T("icon-src")) == 0 || _tcsicmp(pstrName, _T("icon")) == 0 )
				m_pIcon->SetAttribute(_T("src"), pstrValue);
			else
				m_pIcon->SetAttribute(pstrName, pstrValue);
			m_pIcon->SetVisible(true);
			ApplyIconSize();
			UpdateStyle();
			NeedUpdate();
		}
		else if( _tcsicmp(pstrName, _T("icon-tint")) == 0 ) {
			EnsureChildren();
			if( m_pIcon != NULL ) m_pIcon->SetColor(ParseTabColor(pstrValue));
		}
		else {
			CHorizontalLayoutUI::SetAttribute(pstrName, pstrValue);
		}
	}
}
