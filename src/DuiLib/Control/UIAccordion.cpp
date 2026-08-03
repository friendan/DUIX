#include "StdAfx.h"
#include "UIAccordion.h"

namespace DuiLib
{
	/////////////////////////////////////////////////////////////////////////////////////
	// Header row: click + hover

	class CAccordionHeaderUI : public CHorizontalLayoutUI
	{
	public:
		CAccordionHeaderUI() : m_pOwner(NULL) {}
		void SetOwner(CAccordionItemUI* pOwner) { m_pOwner = pOwner; }
		UINT GetControlFlags() const override
		{
			return IsEnabled() ? UIFLAG_SETCURSOR : 0;
		}
		void DoEvent(TEventUI& event) override
		{
			if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
				if( m_pParent != NULL ) m_pParent->DoEvent(event);
				else CHorizontalLayoutUI::DoEvent(event);
				return;
			}
			if( event.Type == UIEVENT_SETCURSOR ) {
				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
				return;
			}
			if( event.Type == UIEVENT_MOUSEENTER ) {
				if( m_pOwner != NULL ) m_pOwner->OnHeaderHoverChanged(true);
				return;
			}
			if( event.Type == UIEVENT_MOUSELEAVE ) {
				if( m_pOwner != NULL ) m_pOwner->OnHeaderHoverChanged(false);
				return;
			}
			if( (event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK) && IsEnabled() ) {
				if( m_pOwner != NULL ) m_pOwner->OnHeaderClick();
				return;
			}
			CHorizontalLayoutUI::DoEvent(event);
		}
	private:
		CAccordionItemUI* m_pOwner;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	// CAccordionUI

	IMPLEMENT_DUICONTROL(CAccordionUI)

	CAccordionUI::CAccordionUI()
		: m_bMultiple(false)
		, m_nDefaultHeaderHeight(40)
	{
		// 不设默认底色：剩余区域由父容器 background-color 铺满；避免未撑满时露出离屏缓冲黑底
	}

	CAccordionUI::~CAccordionUI() {}

	LPCTSTR CAccordionUI::GetClass() const
	{
		return _T("AccordionUI");
	}

	LPVOID CAccordionUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_ACCORDION) == 0 ) return static_cast<CAccordionUI*>(this);
		return CVerticalLayoutUI::GetInterface(pstrName);
	}

	void CAccordionUI::SetMode(bool bMultiple)
	{
		m_bMultiple = bMultiple;
	}

	void CAccordionUI::SetDefaultHeaderHeight(int nHeight)
	{
		if( nHeight < 1 ) nHeight = 1;
		m_nDefaultHeaderHeight = nHeight;
	}

	bool CAccordionUI::Add(CControlUI* pControl)
	{
		if( !CVerticalLayoutUI::Add(pControl) ) return false;
		CAccordionItemUI* pItem = static_cast<CAccordionItemUI*>(pControl->GetInterface(DUI_CTR_ACCORDIONITEM));
		if( pItem != NULL ) pItem->ApplyDefaultHeaderHeight(m_nDefaultHeaderHeight);
		return true;
	}

	bool CAccordionUI::AddAt(CControlUI* pControl, int iIndex)
	{
		if( !CVerticalLayoutUI::AddAt(pControl, iIndex) ) return false;
		CAccordionItemUI* pItem = static_cast<CAccordionItemUI*>(pControl->GetInterface(DUI_CTR_ACCORDIONITEM));
		if( pItem != NULL ) pItem->ApplyDefaultHeaderHeight(m_nDefaultHeaderHeight);
		return true;
	}

	CAccordionItemUI* CAccordionUI::GetActiveItem()
	{
		for( int i = 0; i < GetCount(); ++i ) {
			CControlUI* pControl = GetItemAt(i);
			if( pControl == NULL ) continue;
			CAccordionItemUI* pItem = static_cast<CAccordionItemUI*>(pControl->GetInterface(DUI_CTR_ACCORDIONITEM));
			if( pItem != NULL && pItem->IsActive() ) return pItem;
		}
		return NULL;
	}

	void CAccordionUI::ToggleItem(CAccordionItemUI* pItem)
	{
		if( pItem == NULL || pItem->IsDisabled() ) return;

		if( !m_bMultiple ) {
			for( int i = 0; i < GetCount(); ++i ) {
				CControlUI* pControl = GetItemAt(i);
				if( pControl == NULL ) continue;
				CAccordionItemUI* pOther = static_cast<CAccordionItemUI*>(pControl->GetInterface(DUI_CTR_ACCORDIONITEM));
				if( pOther != NULL && pOther != pItem && pOther->IsActive() && !pOther->IsDisabled() )
					pOther->SetActive(false, true);
			}
		}

		pItem->SetActive(!pItem->IsActive(), true);
		NeedUpdate();
	}

	SIZE CAccordionUI::EstimateSize(SIZE szAvailable)
	{
		SIZE szFixed = GetFixedSize();
		if( szFixed.cx > 0 && szFixed.cy > 0 ) return szFixed;

		SIZE szContent = MeasureContent(szAvailable);
		RECT rcPadding = GetPadding();
		SIZE sz = { 0, 0 };
		sz.cx = szFixed.cx > 0 ? szFixed.cx : (szAvailable.cx > 0 ? szAvailable.cx : szContent.cx + rcPadding.left + rcPadding.right);
		sz.cy = szFixed.cy > 0 ? szFixed.cy : (szContent.cy + rcPadding.top + rcPadding.bottom);
		return sz;
	}

	void CAccordionUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("mode")) == 0 ) {
			SetMode(_tcsicmp(pstrValue, _T("multiple")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("header-height")) == 0 ) {
			SetDefaultHeaderHeight(_ttoi(pstrValue));
		}
		else CVerticalLayoutUI::SetAttribute(pstrName, pstrValue);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// CAccordionItemUI

	IMPLEMENT_DUICONTROL(CAccordionItemUI)

	CAccordionItemUI::CAccordionItemUI()
		: m_pHeader(NULL)
		, m_pTitle(NULL)
		, m_pChevron(NULL)
		, m_bActive(false)
		, m_bDisabled(false)
		, m_bHeaderHover(false)
		, m_bHeaderHeightExplicit(false)
		, m_nHeaderHeight(40)
		, m_dwHeaderBk(0xF8F9FAFF)
		, m_dwHeaderHoverBk(0xE9ECEFFF)
		, m_dwHeaderActiveBk(0xCFE2FFFF)
		, m_dwHeaderActiveHoverBk(0xB6D4FEFF)
	{
		m_rcContentPadding = CDuiBox(8);
		SetBackgroundColor(0xFFFFFFFF);
		SetMargin(CDuiBox(0, 0, 1, 0));
		EnsureHeader();
		UpdateFixedHeight();
	}

	CAccordionItemUI::~CAccordionItemUI() {}

	LPCTSTR CAccordionItemUI::GetClass() const
	{
		return _T("AccordionItemUI");
	}

	LPVOID CAccordionItemUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_ACCORDIONITEM) == 0 ) return static_cast<CAccordionItemUI*>(this);
		return CVerticalLayoutUI::GetInterface(pstrName);
	}

	void CAccordionItemUI::DoInit()
	{
		CVerticalLayoutUI::DoInit();
		EnsureHeader();
		SyncHeaderChrome();
		SyncContentVisibility();
		UpdateFixedHeight();
	}

	void CAccordionItemUI::EnsureHeader()
	{
		if( m_pHeader != NULL ) return;

		CAccordionHeaderUI* pHeader = new CAccordionHeaderUI;
		pHeader->SetOwner(this);
		pHeader->SetFixedHeight(m_nHeaderHeight);
		pHeader->SetBackgroundColor(m_dwHeaderBk);
		pHeader->SetMouseEnabled(true);
		pHeader->SetGap(8);
		pHeader->SetAlignItems(DT_VCENTER);
		pHeader->SetPadding(CDuiBox(0, 16, 0, 16));
		CVerticalLayoutUI::Add(pHeader);
		m_pHeader = pHeader;

		m_pTitle = new CLabelUI;
		m_pTitle->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
		m_pTitle->SetColor(0x212529FF);
		m_pTitle->SetFont(0);
		m_pTitle->SetMouseEnabled(false);
		m_pTitle->SetFixedWidth(0);
		m_pHeader->Add(m_pTitle);

		m_pChevron = new CLabelUI;
		m_pChevron->SetText(_T("\u25BC"));
		m_pChevron->SetTextStyle(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_pChevron->SetColor(0x212529FF);
		m_pChevron->SetFixedWidth(20);
		m_pChevron->SetMouseEnabled(false);
		m_pHeader->Add(m_pChevron);
	}

	CAccordionUI* CAccordionItemUI::GetOwnerAccordion() const
	{
		CControlUI* pParent = GetParent();
		while( pParent != NULL ) {
			CAccordionUI* pAcc = static_cast<CAccordionUI*>(pParent->GetInterface(DUI_CTR_ACCORDION));
			if( pAcc != NULL ) return pAcc;
			pParent = pParent->GetParent();
		}
		return NULL;
	}

	void CAccordionItemUI::OnHeaderClick()
	{
		if( m_bDisabled ) return;
		CAccordionUI* pAcc = GetOwnerAccordion();
		if( pAcc != NULL ) pAcc->ToggleItem(this);
		else SetActive(!m_bActive, true);
	}

	void CAccordionItemUI::OnHeaderHoverChanged(bool bHot)
	{
		if( m_bDisabled ) bHot = false;
		if( m_bHeaderHover == bHot ) return;
		m_bHeaderHover = bHot;
		SyncHeaderChrome();
	}

	void CAccordionItemUI::SetTitle(LPCTSTR pstrText)
	{
		EnsureHeader();
		if( m_pTitle != NULL ) m_pTitle->SetText(pstrText);
	}

	CDuiString CAccordionItemUI::GetTitle() const
	{
		if( m_pTitle != NULL ) return m_pTitle->GetText();
		return CDuiString();
	}

	void CAccordionItemUI::ApplyDefaultHeaderHeight(int nHeight)
	{
		if( m_bHeaderHeightExplicit ) return;
		if( nHeight < 1 ) nHeight = 1;
		m_nHeaderHeight = nHeight;
		EnsureHeader();
		if( m_pHeader != NULL ) m_pHeader->SetFixedHeight(m_nHeaderHeight);
		UpdateFixedHeight();
		RequestAncestorLayout();
	}

	void CAccordionItemUI::ApplyContentPadding(CControlUI* pControl)
	{
		if( pControl == NULL || pControl == m_pHeader ) return;
		pControl->SetMargin(m_rcContentPadding);
	}

	void CAccordionItemUI::SyncContentVisibility()
	{
		for( int i = 0; i < GetCount(); ++i ) {
			CControlUI* pControl = GetItemAt(i);
			if( pControl == NULL || pControl == m_pHeader ) continue;
			pControl->SetVisible(m_bActive);
			// 解析阶段 Add 可能把子控件 InternVisible 置 false，展开时必须恢复
			if( m_bActive ) pControl->SetInternVisible(true);
		}
	}

	void CAccordionItemUI::SyncHeaderChrome()
	{
		EnsureHeader();
		if( m_pHeader == NULL ) return;
		DWORD dwBk = m_dwHeaderBk;
		if( m_bActive ) {
			dwBk = (m_bHeaderHover && !m_bDisabled) ? m_dwHeaderActiveHoverBk : m_dwHeaderActiveBk;
		}
		else if( m_bHeaderHover && !m_bDisabled ) {
			dwBk = m_dwHeaderHoverBk;
		}
		m_pHeader->SetBackgroundColor(dwBk);
		if( m_pChevron != NULL ) {
			m_pChevron->SetVisible(!m_bDisabled);
			m_pChevron->SetText(m_bActive ? _T("\u25B2") : _T("\u25BC"));
		}
	}

	void CAccordionItemUI::UpdateFixedHeight()
	{
		if( !m_bActive ) {
			SetFixedHeight(m_nHeaderHeight);
			return;
		}
		// 展开：用内容估高写成固定高度，避免父布局把 cy=0 当分摊伸缩后高度不足
		SIZE szAvail = { GetFixedWidth() > 0 ? GetFixedWidth() : 400, 9999 };
		if( m_pManager != NULL ) {
			RECT rc = GetPos();
			int cx = rc.right - rc.left;
			if( cx > 0 ) szAvail.cx = cx;
		}
		SIZE sz = EstimateSize(szAvail);
		if( sz.cy < m_nHeaderHeight ) sz.cy = m_nHeaderHeight;
		SetFixedHeight(sz.cy);
	}

	void CAccordionItemUI::RequestAncestorLayout()
	{
		// 手风琴高度变化必须让 root 重新 SetPos。
		// 若只标记 Accordion 自身 update，PaintManager 会走「用旧矩形再 SetPos」分支，折叠起步时无法长高。
		for( CControlUI* p = this; p != NULL; p = p->GetParent() ) {
			p->NeedUpdate();
		}
		if( m_pManager != NULL ) m_pManager->NeedUpdate();
	}

	void CAccordionItemUI::SetActive(bool bActive, bool bNotify)
	{
		if( m_bDisabled ) {
			bActive = true;
		}
		if( m_bActive == bActive ) {
			SyncContentVisibility();
			SyncHeaderChrome();
			UpdateFixedHeight();
			return;
		}
		m_bActive = bActive;
		SyncContentVisibility();
		SyncHeaderChrome();
		UpdateFixedHeight();
		RequestAncestorLayout();
		if( bNotify && m_pManager != NULL ) {
			m_pManager->SendNotify(this, m_bActive ? DUI_MSGTYPE_ITEMEXPAND : DUI_MSGTYPE_ITEMCOLLAPSE);
		}
	}

	void CAccordionItemUI::SetDisabled(bool bDisabled)
	{
		m_bDisabled = bDisabled;
		if( m_bDisabled ) {
			m_bActive = true;
			SyncContentVisibility();
			SyncHeaderChrome();
			UpdateFixedHeight();
			RequestAncestorLayout();
			if( m_pManager != NULL )
				m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMEXPAND);
		}
		else {
			SyncHeaderChrome();
		}
	}

	bool CAccordionItemUI::Add(CControlUI* pControl)
	{
		EnsureHeader();
		if( pControl == m_pHeader ) return CVerticalLayoutUI::Add(pControl);
		if( !CVerticalLayoutUI::Add(pControl) ) return false;
		ApplyContentPadding(pControl);
		pControl->SetVisible(m_bActive);
		if( m_bActive ) pControl->SetInternVisible(true);
		if( m_bActive ) {
			UpdateFixedHeight();
			RequestAncestorLayout();
		}
		return true;
	}

	bool CAccordionItemUI::AddAt(CControlUI* pControl, int iIndex)
	{
		EnsureHeader();
		if( pControl == m_pHeader ) return CVerticalLayoutUI::AddAt(pControl, iIndex);
		if( iIndex <= 0 ) iIndex = 1;
		if( !CVerticalLayoutUI::AddAt(pControl, iIndex) ) return false;
		ApplyContentPadding(pControl);
		pControl->SetVisible(m_bActive);
		if( m_bActive ) pControl->SetInternVisible(true);
		if( m_bActive ) {
			UpdateFixedHeight();
			RequestAncestorLayout();
		}
		return true;
	}

	static SIZE EstimateAccordionChild(CControlUI* pControl, SIZE szAvail)
	{
		SIZE sz = pControl->EstimateSize(szAvail);
		if( sz.cy <= 0 ) {
			CLinearLayoutUI* pLinear = static_cast<CLinearLayoutUI*>(pControl->GetInterface(_T("LinearLayout")));
			if( pLinear != NULL ) {
				SIZE szContent = pLinear->MeasureContent(szAvail);
				if( szContent.cy > 0 ) sz.cy = szContent.cy;
				if( sz.cx <= 0 && szContent.cx > 0 ) sz.cx = szContent.cx;
			}
		}
		if( sz.cy <= 0 ) {
			int nMin = pControl->GetMinHeight();
			sz.cy = nMin > 0 ? nMin : 22;
		}
		return sz;
	}

	SIZE CAccordionItemUI::EstimateSize(SIZE szAvailable)
	{
		SIZE szFixed = GetFixedSize();
		if( !m_bActive ) {
			SIZE sz = { szFixed.cx > 0 ? szFixed.cx : szAvailable.cx, m_nHeaderHeight };
			return sz;
		}

		RECT rcPadding = GetPadding();
		int cx = szAvailable.cx - rcPadding.left - rcPadding.right;
		if( cx < 0 ) cx = 0;
		int cy = m_nHeaderHeight;
		int nContent = 0;
		for( int i = 0; i < GetCount(); ++i ) {
			CControlUI* pControl = GetItemAt(i);
			if( pControl == NULL || pControl == m_pHeader ) continue;
			if( !pControl->IsVisible() ) continue;
			RECT rcPad = pControl->GetMargin();
			SIZE szAvail = { cx - rcPad.left - rcPad.right, szAvailable.cy };
			if( szAvail.cx < 0 ) szAvail.cx = 0;
			SIZE sz = EstimateAccordionChild(pControl, szAvail);
			cy += sz.cy + rcPad.top + rcPad.bottom;
			if( nContent > 0 && GetGap() > 0 ) cy += GetGap();
			nContent++;
		}
		cy += rcPadding.top + rcPadding.bottom;

		SIZE szResult = { szFixed.cx > 0 ? szFixed.cx : szAvailable.cx, cy };
		return szResult;
	}

	DWORD CAccordionItemUI::ParseColorValue(LPCTSTR pstrValue)
	{
		DWORD clr = 0;
		if( pstrValue != NULL && ParseColorString(pstrValue, clr) ) return clr;
		return 0;
	}

	void CAccordionItemUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("title")) == 0 ) {
			SetTitle(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("active")) == 0 ) {
			SetActive(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0, false);
		}
		else if( _tcsicmp(pstrName, _T("disabled")) == 0 ) {
			SetDisabled(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("content-padding")) == 0 ) {
			// 与全局 padding 相同：CSS top,right,bottom,left
			CDuiBox rc;
			if( ParseCssBox(pstrValue, rc) ) {
				m_rcContentPadding = rc;
				for( int i = 0; i < GetCount(); ++i ) {
					CControlUI* pControl = GetItemAt(i);
					if( pControl != NULL && pControl != m_pHeader ) ApplyContentPadding(pControl);
				}
			}
		}
		else if( _tcsicmp(pstrName, _T("header-align")) == 0 ) {
			EnsureHeader();
			if( m_pTitle == NULL ) return;
			UINT uStyle = DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;
			if( _tcsicmp(pstrValue, _T("center")) == 0 ) uStyle |= DT_CENTER;
			else if( _tcsicmp(pstrValue, _T("right")) == 0 ) uStyle |= DT_RIGHT;
			else uStyle |= DT_LEFT;
			m_pTitle->SetTextStyle(uStyle);
		}
		else if( _tcsicmp(pstrName, _T("header-height")) == 0 ) {
			m_bHeaderHeightExplicit = true;
			m_nHeaderHeight = _ttoi(pstrValue);
			if( m_nHeaderHeight < 1 ) m_nHeaderHeight = 1;
			EnsureHeader();
			if( m_pHeader != NULL ) m_pHeader->SetFixedHeight(m_nHeaderHeight);
			UpdateFixedHeight();
			RequestAncestorLayout();
		}
		else if( _tcsicmp(pstrName, _T("header-color")) == 0 ) {
			EnsureHeader();
			if( m_pTitle != NULL ) m_pTitle->SetColor(ParseColorValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("header-background-color")) == 0 ) {
			m_dwHeaderBk = ParseColorValue(pstrValue);
			SyncHeaderChrome();
		}
		else if( _tcsicmp(pstrName, _T("header-background-color-hover")) == 0 ) {
			m_dwHeaderHoverBk = ParseColorValue(pstrValue);
			SyncHeaderChrome();
		}
		else if( _tcsicmp(pstrName, _T("header-background-color-active")) == 0 ) {
			m_dwHeaderActiveBk = ParseColorValue(pstrValue);
			SyncHeaderChrome();
		}
		else if( _tcsicmp(pstrName, _T("header-background-color-active-hover")) == 0 ) {
			m_dwHeaderActiveHoverBk = ParseColorValue(pstrValue);
			SyncHeaderChrome();
		}
		else CVerticalLayoutUI::SetAttribute(pstrName, pstrValue);
	}
}
