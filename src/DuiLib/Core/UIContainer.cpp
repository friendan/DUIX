#include "StdAfx.h"

namespace DuiLib
{

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	IMPLEMENT_DUICONTROL(CContainerUI)

		CContainerUI::CContainerUI()
		: m_iGap(0),
		m_iChildAlign(DT_LEFT),
		m_iChildVAlign(DT_TOP),
		m_bAutoDestroy(true),
		m_bDelayedDestroy(true),
		m_bMouseChildEnabled(true),
		m_nScrollStepSize(0),
		m_bFixedScrollbar(false),
		m_bShowScrollbar(true),
		m_nUpdateLock(0),
		m_pVerticalScrollBar(NULL),
		m_pHorizontalScrollBar(NULL)
	{
	}

	CContainerUI::~CContainerUI()
	{
		m_bDelayedDestroy = false;
		RemoveAll();
		if( m_pVerticalScrollBar ) {
			delete m_pVerticalScrollBar;
			m_pVerticalScrollBar = NULL;
		}
		if( m_pHorizontalScrollBar ) {
			delete m_pHorizontalScrollBar;
			m_pHorizontalScrollBar = NULL;
		}
	}

	LPCTSTR CContainerUI::GetClass() const
	{
		return _T("ContainerUI");
	}

	LPVOID CContainerUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, _T("IContainer")) == 0 ) return static_cast<IContainerUI*>(this);
		else if( _tcsicmp(pstrName, DUI_CTR_CONTAINER) == 0 ) return static_cast<CContainerUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	CControlUI* CContainerUI::GetItemAt(int iIndex) const
	{
		if( iIndex < 0 || iIndex >= m_items.GetSize() ) return NULL;
		return static_cast<CControlUI*>(m_items[iIndex]);
	}

	int CContainerUI::GetItemIndex(CControlUI* pControl) const
	{
		for( int it = 0; it < m_items.GetSize(); it++ ) {
			if( static_cast<CControlUI*>(m_items[it]) == pControl ) {
				return it;
			}
		}

		return -1;
	}

	bool CContainerUI::SetItemIndex(CControlUI* pControl, int iIndex)
	{
		for( int it = 0; it < m_items.GetSize(); it++ ) {
			if( static_cast<CControlUI*>(m_items[it]) == pControl ) {
				if( m_nUpdateLock == 0 ) NeedUpdate();
				m_items.Remove(it);
				return m_items.InsertAt(iIndex, pControl);
			}
		}

		return false;
	}

	int CContainerUI::GetCount() const
	{
		return m_items.GetSize();
	}

	bool CContainerUI::Add(CControlUI* pControl)
	{
		if( pControl == NULL) return false;

		if( m_pManager != NULL ) {
			m_pManager->InitControls(pControl, this);
			// XML 首次挂载走 AttachDialog→ApplyToExistingManager；运行时 Add 需补 chrome / var
			CThemeManager::GetInstance()->ApplyChromeToControl(pControl);
		}
		if( IsVisible() && m_nUpdateLock == 0 ) NeedUpdate();
		else pControl->SetInternVisible(false);
		return m_items.Add(pControl);   
	}

	bool CContainerUI::AddAt(CControlUI* pControl, int iIndex)
	{
		if( pControl == NULL) return false;

		if( m_pManager != NULL ) {
			m_pManager->InitControls(pControl, this);
			CThemeManager::GetInstance()->ApplyChromeToControl(pControl);
		}
		if( IsVisible() && m_nUpdateLock == 0 ) NeedUpdate();
		else pControl->SetInternVisible(false);
		return m_items.InsertAt(iIndex, pControl);
	}

	bool CContainerUI::Remove(CControlUI* pControl)
	{
		if( pControl == NULL) return false;

		for( int it = 0; it < m_items.GetSize(); it++ ) {
			if( static_cast<CControlUI*>(m_items[it]) == pControl ) {
				if( m_nUpdateLock == 0 ) NeedUpdate();
				DestroyChild(pControl);
				return m_items.Remove(it);
			}
		}
		return false;
	}

	bool CContainerUI::RemoveAt(int iIndex)
	{
		CControlUI* pControl = GetItemAt(iIndex);
		if (pControl != NULL) {
			return CContainerUI::Remove(pControl);
		}

		return false;
	}

	void CContainerUI::RemoveAll()
	{
 		for( int it = 0; it < m_items.GetSize(); it++ ) {
			DestroyChild(static_cast<CControlUI*>(m_items[it]));
		}
		m_items.Empty();
		if( m_nUpdateLock == 0 ) NeedUpdate();
	}

	bool CContainerUI::IsAutoDestroy() const
	{
		return m_bAutoDestroy;
	}

	void CContainerUI::SetAutoDestroy(bool bAuto)
	{
		m_bAutoDestroy = bAuto;
	}

	bool CContainerUI::IsDelayedDestroy() const
	{
		return m_bDelayedDestroy;
	}

	void CContainerUI::SetDelayedDestroy(bool bDelayed)
	{
		m_bDelayedDestroy = bDelayed;
	}

	void CContainerUI::SetPadding(CDuiBox rcPadding)
	{
		m_rcPadding = rcPadding;
		NeedUpdate();
	}

	int CContainerUI::GetGap() const
	{
		if (m_pManager) return m_pManager->GetDPIObj()->Scale(m_iGap);
		return m_iGap;
	}


	void CContainerUI::SetGap(int iPadding)
	{
		m_iGap = iPadding;
		NeedUpdate();
	}

	bool CContainerUI::IsMainAxisVertical() const
	{
		CContainerUI* pThis = const_cast<CContainerUI*>(this);
		if( pThis->GetInterface(DUI_CTR_VERTICALLAYOUT) != NULL ) return true;
		if( pThis->GetInterface(DUI_CTR_VBOX) != NULL ) return true;
		return false;
	}

	UINT CContainerUI::GetJustifyContent() const
	{
		return IsMainAxisVertical() ? m_iChildVAlign : m_iChildAlign;
	}

	void CContainerUI::SetJustifyContent(UINT uAlign)
	{
		if( IsMainAxisVertical() ) m_iChildVAlign = uAlign;
		else m_iChildAlign = uAlign;
		NeedUpdate();
	}

	UINT CContainerUI::GetAlignItems() const
	{
		return IsMainAxisVertical() ? m_iChildAlign : m_iChildVAlign;
	}

	void CContainerUI::SetAlignItems(UINT uAlign)
	{
		if( IsMainAxisVertical() ) m_iChildAlign = uAlign;
		else m_iChildVAlign = uAlign;
		NeedUpdate();
	}

	bool CContainerUI::IsMouseChildEnabled() const
	{
		return m_bMouseChildEnabled;
	}

	void CContainerUI::SetMouseChildEnabled(bool bEnable)
	{
		m_bMouseChildEnabled = bEnable;
	}
	
	bool CContainerUI::IsFixedScrollbar()
	{
		return m_bFixedScrollbar;
	}

	void CContainerUI::SetFixedScrollbar(bool bFixed)
	{
		m_bFixedScrollbar = bFixed;
		Invalidate();
	}

	bool CContainerUI::IsShowScrollbar()
	{
		return m_bShowScrollbar;
	}

	void CContainerUI::SetShowScrollbar(bool bShow)
	{
		m_bShowScrollbar = bShow;
		
		if( m_pVerticalScrollBar != NULL ) m_pVerticalScrollBar->SetShow(bShow);
		if( m_pHorizontalScrollBar != NULL ) m_pHorizontalScrollBar->SetShow(bShow);
	}

	void CContainerUI::SetVisible(bool bVisible)
	{
		if( m_bVisible == bVisible ) return;
		CControlUI::SetVisible(bVisible);
		for( int it = 0; it < m_items.GetSize(); it++ ) {
			static_cast<CControlUI*>(m_items[it])->SetInternVisible(IsVisible());
		}
	}

	// 逻辑上，对于Container控件不公开此方法
	// 调用此方法的结果是，内部子控件隐藏，控件本身依然显示，背景等效果存在
	void CContainerUI::SetInternVisible(bool bVisible)
	{
		CControlUI::SetInternVisible(bVisible);
		if( m_items.IsEmpty() ) return;
		for( int it = 0; it < m_items.GetSize(); it++ ) {
			// 控制子控件显示状态
			// InternVisible状态应由子控件自己控制
			static_cast<CControlUI*>(m_items[it])->SetInternVisible(IsVisible());
		}
	}

	void CContainerUI::SetEnabled(bool bEnabled)
	{
		if( m_bEnabled == bEnabled ) return;

		m_bEnabled = bEnabled;

		for( int it = 0; it < m_items.GetSize(); it++ ) {
			static_cast<CControlUI*>(m_items[it])->SetEnabled(m_bEnabled);
		}

		Invalidate();
	}

	void CContainerUI::SetMouseEnabled(bool bEnabled)
	{
		if( m_pVerticalScrollBar != NULL ) m_pVerticalScrollBar->SetMouseEnabled(bEnabled);
		if( m_pHorizontalScrollBar != NULL ) m_pHorizontalScrollBar->SetMouseEnabled(bEnabled);
		CControlUI::SetMouseEnabled(bEnabled);
	}

	void CContainerUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			BubbleEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETFOCUS ) 
		{
			m_bFocused = true;
			return;
		}
		if( event.Type == UIEVENT_KILLFOCUS ) 
		{
			m_bFocused = false;
			return;
		}
		if( m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible() && m_pVerticalScrollBar->IsEnabled() )
		{
			if( event.Type == UIEVENT_KEYDOWN ) 
			{
				switch( event.chKey ) {
				case VK_DOWN:
					LineDown();
					return;
				case VK_UP:
					LineUp();
					return;
				case VK_NEXT:
					PageDown();
					return;
				case VK_PRIOR:
					PageUp();
					return;
				case VK_HOME:
					HomeUp();
					return;
				case VK_END:
					EndDown();
					return;
				}
			}
			else if( event.Type == UIEVENT_SCROLLWHEEL )
			{
				switch( LOWORD(event.wParam) ) {
				case SB_LINEUP:
					LineUp();
					return;
				case SB_LINEDOWN:
					LineDown();
					return;
				}
			}
		}
		if( m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible() && m_pHorizontalScrollBar->IsEnabled() ) {
			if( event.Type == UIEVENT_KEYDOWN ) 
			{
				switch( event.chKey ) {
				case VK_RIGHT:
					LineRight();
					return;
				case VK_LEFT:
					LineLeft();
					return;
				case VK_NEXT:
					PageRight();
					return;
				case VK_PRIOR:
					PageLeft();
					return;
				case VK_HOME:
					HomeLeft();
					return;
				case VK_END:
					EndRight();
					return;
				}
			}
			else if( event.Type == UIEVENT_SCROLLWHEEL )
			{
				switch( LOWORD(event.wParam) ) {
				case SB_LINEUP:
					LineLeft();
					return;
				case SB_LINEDOWN:
					LineRight();
					return;
				}
			}
		}
		CControlUI::DoEvent(event);
	}

	SIZE CContainerUI::GetScrollPos() const
	{
		SIZE sz = {0, 0};
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) sz.cy = m_pVerticalScrollBar->GetScrollPos();
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) sz.cx = m_pHorizontalScrollBar->GetScrollPos();
		return sz;
	}

	SIZE CContainerUI::GetScrollRange() const
	{
		SIZE sz = {0, 0};
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) sz.cy = m_pVerticalScrollBar->GetScrollRange();
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) sz.cx = m_pHorizontalScrollBar->GetScrollRange();
		return sz;
	}

	void CContainerUI::SetScrollPos(SIZE szPos, bool bMsg)
	{
		int cx = 0;
		int cy = 0;
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
			int iLastScrollPos = m_pVerticalScrollBar->GetScrollPos();
			m_pVerticalScrollBar->SetScrollPos(szPos.cy);
			cy = m_pVerticalScrollBar->GetScrollPos() - iLastScrollPos;
		}

		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
			int iLastScrollPos = m_pHorizontalScrollBar->GetScrollPos();
			m_pHorizontalScrollBar->SetScrollPos(szPos.cx);
			cx = m_pHorizontalScrollBar->GetScrollPos() - iLastScrollPos;
		}

		if( cx == 0 && cy == 0 ) return;

		RECT rcPos;
		for( int it2 = 0; it2 < m_items.GetSize(); it2++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsAbsolute() ) continue;

			rcPos = pControl->GetPos();
			rcPos.left -= cx;
			rcPos.right -= cx;
			rcPos.top -= cy;
			rcPos.bottom -= cy;
			pControl->SetPos(rcPos);
		}

		Invalidate();

		if(m_pVerticalScrollBar)
		{
			// 发送滚动消息
			if( m_pManager != NULL && bMsg ) {
				int nPage = (m_pVerticalScrollBar->GetScrollPos() + m_pVerticalScrollBar->GetLineSize()) / m_pVerticalScrollBar->GetLineSize();
				m_pManager->SendNotify(this, DUI_MSGTYPE_SCROLL, (WPARAM)nPage);
			}
		}
	}

	void CContainerUI::SetScrollStepSize(int nSize)
	{
		if (nSize >0)
			m_nScrollStepSize = nSize;
	}

	int CContainerUI::GetScrollStepSize() const
	{
		if(m_pManager )return m_pManager->GetDPIObj()->Scale(m_nScrollStepSize);

		return m_nScrollStepSize;
	}

	void CContainerUI::LineUp()
	{
		int cyLine = GetScrollStepSize();
		if (cyLine == 0) {
			cyLine = 8;
			if( m_pManager ) cyLine = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8;
		}

		SIZE sz = GetScrollPos();
		sz.cy -= cyLine;
		SetScrollPos(sz);
	}

	void CContainerUI::LineDown()
	{
		int cyLine = GetScrollStepSize();
		if (cyLine == 0) {
			cyLine = 8;
			if( m_pManager ) cyLine = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8;
		}

		SIZE sz = GetScrollPos();
		sz.cy += cyLine;
		SetScrollPos(sz);
	}

	void CContainerUI::PageUp()
	{
		SIZE sz = GetScrollPos();
		RECT rcPadding = GetPadding();
		int iOffset = m_rcItem.bottom - m_rcItem.top - rcPadding.top - rcPadding.bottom;
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) iOffset -= m_pHorizontalScrollBar->GetFixedHeight();
		sz.cy -= iOffset;
		SetScrollPos(sz);
	}

	void CContainerUI::PageDown()
	{
		SIZE sz = GetScrollPos();
		RECT rcPadding = GetPadding();
		int iOffset = m_rcItem.bottom - m_rcItem.top - rcPadding.top - rcPadding.bottom;
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) iOffset -= m_pHorizontalScrollBar->GetFixedHeight();
		sz.cy += iOffset;
		SetScrollPos(sz);
	}

	void CContainerUI::HomeUp()
	{
		SIZE sz = GetScrollPos();
		sz.cy = 0;
		SetScrollPos(sz);
	}

	void CContainerUI::EndDown()
	{
		if(m_pManager) {
			::UpdateWindow(m_pManager->GetPaintWindow());
		}
		SIZE sz = GetScrollPos();
		sz.cy = GetScrollRange().cy;
		SetScrollPos(sz);
	}

	void CContainerUI::LineLeft()
	{
		int nScrollStepSize = GetScrollStepSize();
		int cxLine = nScrollStepSize == 0 ? 8 : nScrollStepSize;

		SIZE sz = GetScrollPos();
		sz.cx -= cxLine;
		SetScrollPos(sz);
	}

	void CContainerUI::LineRight()
	{
		int nScrollStepSize = GetScrollStepSize();
		int cxLine = nScrollStepSize == 0 ? 8 : nScrollStepSize;

		SIZE sz = GetScrollPos();
		sz.cx += cxLine;
		SetScrollPos(sz);
	}

	void CContainerUI::PageLeft()
	{
		SIZE sz = GetScrollPos();

		RECT rcPadding = GetPadding();
		int iOffset = m_rcItem.right - m_rcItem.left - rcPadding.left - rcPadding.right;
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) iOffset -= m_pVerticalScrollBar->GetFixedWidth();
		sz.cx -= iOffset;
		SetScrollPos(sz);
	}

	void CContainerUI::PageRight()
	{
		SIZE sz = GetScrollPos();

		RECT rcPadding = GetPadding();
		int iOffset = m_rcItem.right - m_rcItem.left - rcPadding.left - rcPadding.right;
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) iOffset -= m_pVerticalScrollBar->GetFixedWidth();
		sz.cx += iOffset;
		SetScrollPos(sz);
	}

	void CContainerUI::HomeLeft()
	{
		SIZE sz = GetScrollPos();
		sz.cx = 0;
		SetScrollPos(sz);
	}

	void CContainerUI::EndRight()
	{
		if(m_pManager) {
			::UpdateWindow(m_pManager->GetPaintWindow());
		}
		SIZE sz = GetScrollPos();
		sz.cx = GetScrollRange().cx;
		SetScrollPos(sz);
	}

	void CContainerUI::EnableScrollBar(bool bEnableVertical, bool bEnableHorizontal)
	{
		if( bEnableVertical && !m_pVerticalScrollBar ) {
			m_pVerticalScrollBar = new CScrollBarUI;
			m_pVerticalScrollBar->SetOwner(this);
			m_pVerticalScrollBar->SetManager(m_pManager, NULL, false);
			if ( m_pManager ) {
				LPCTSTR pDefaultAttributes = m_pManager->GetDefaultAttributeList(_T("VScrollBar"));
				if( pDefaultAttributes ) {
					m_pVerticalScrollBar->ApplyAttributeList(pDefaultAttributes);
				}
				LPCTSTR pCssAttrs = m_pManager->GetCssTypeRule(_T("VScrollBar"));
				if( pCssAttrs ) {
					m_pVerticalScrollBar->ApplyAttributeList(pCssAttrs);
				}

				m_pVerticalScrollBar->SetShow(m_bShowScrollbar);
			}
		}
		else if( !bEnableVertical && m_pVerticalScrollBar ) {
			delete m_pVerticalScrollBar;
			m_pVerticalScrollBar = NULL;
		}

		if( bEnableHorizontal && !m_pHorizontalScrollBar ) {
			m_pHorizontalScrollBar = new CScrollBarUI;
			m_pHorizontalScrollBar->SetHorizontal(true);
			m_pHorizontalScrollBar->SetOwner(this);
			m_pHorizontalScrollBar->SetManager(m_pManager, NULL, false);

			if ( m_pManager ) {
				LPCTSTR pDefaultAttributes = m_pManager->GetDefaultAttributeList(_T("HScrollBar"));
				if( pDefaultAttributes ) {
					m_pHorizontalScrollBar->ApplyAttributeList(pDefaultAttributes);
				}
				LPCTSTR pCssAttrs = m_pManager->GetCssTypeRule(_T("HScrollBar"));
				if( pCssAttrs ) {
					m_pHorizontalScrollBar->ApplyAttributeList(pCssAttrs);
				}

				m_pHorizontalScrollBar->SetShow(m_bShowScrollbar);
			}
		}
		else if( !bEnableHorizontal && m_pHorizontalScrollBar ) {
			delete m_pHorizontalScrollBar;
			m_pHorizontalScrollBar = NULL;
		}

		NeedUpdate();
	}

	CScrollBarUI* CContainerUI::GetVerticalScrollBar() const
	{
		return m_pVerticalScrollBar;
	}

	CScrollBarUI* CContainerUI::GetHorizontalScrollBar() const
	{
		return m_pHorizontalScrollBar;
	}

	int CContainerUI::FindSelectable(int iIndex, bool bForward /*= true*/) const
	{
		// NOTE: This is actually a helper-function for the list/combo/ect controls
		//       that allow them to find the next enabled/available selectable item
		if( GetCount() == 0 ) return -1;
		iIndex = CLAMP(iIndex, 0, GetCount() - 1);
		if( bForward ) {
			for( int i = iIndex; i < GetCount(); i++ ) {
				if( GetItemAt(i)->GetInterface(_T("ListItem")) != NULL 
					&& GetItemAt(i)->IsVisible()
					&& GetItemAt(i)->IsEnabled() ) return i;
			}
			return -1;
		}
		else {
			for( int i = iIndex; i >= 0; --i ) {
				if( GetItemAt(i)->GetInterface(_T("ListItem")) != NULL 
					&& GetItemAt(i)->IsVisible()
					&& GetItemAt(i)->IsEnabled() ) return i;
			}
			return FindSelectable(0, true);
		}
	}

	SIZE CContainerUI::EstimateSize(SIZE szAvailable)
	{
		SIZE szFixed = CControlUI::EstimateSize(szAvailable);
		if( szFixed.cx != 0 && szFixed.cy != 0 ) return szFixed;

		SIZE szContent = MeasureContent(szAvailable);
		RECT rcPadding = GetPadding();
		if( szFixed.cx == 0 && szContent.cx > 0 ) {
			szFixed.cx = szContent.cx + rcPadding.left + rcPadding.right;
			if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() )
				szFixed.cx += m_pVerticalScrollBar->GetFixedWidth();
		}
		if( szFixed.cy == 0 && szContent.cy > 0 ) {
			szFixed.cy = szContent.cy + rcPadding.top + rcPadding.bottom;
			if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() )
				szFixed.cy += m_pHorizontalScrollBar->GetFixedHeight();
		}
		return szFixed;
	}

	SIZE CContainerUI::MeasureContent(SIZE szAvailable)
	{
		int cxMax = 0;
		int cyMax = 0;
		for( int i = 0; i < m_items.GetSize(); i++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[i]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsAbsolute() ) continue;
			SIZE sz = pControl->EstimateSize(szAvailable);
			RECT rcMargin = pControl->GetMargin();
			if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
			if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
			if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
			if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
			int cxChild = sz.cx + rcMargin.left + rcMargin.right;
			int cyChild = sz.cy + rcMargin.top + rcMargin.bottom;
			if( cxChild > cxMax ) cxMax = cxChild;
			if( cyChild > cyMax ) cyMax = cyChild;
		}
		SIZE szContent = { cxMax, cyMax };
		return szContent;
	}

	RECT CContainerUI::GetClientPos() const
	{
		RECT rc = CControlUI::GetClientPos();

		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
			rc.top -= m_pVerticalScrollBar->GetScrollPos();
			rc.bottom -= m_pVerticalScrollBar->GetScrollPos();
			rc.bottom += m_pVerticalScrollBar->GetScrollRange();
			rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		}
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
			rc.left -= m_pHorizontalScrollBar->GetScrollPos();
			rc.right -= m_pHorizontalScrollBar->GetScrollPos();
			rc.right += m_pHorizontalScrollBar->GetScrollRange();
			rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
		}
		return rc;
	}

	void CContainerUI::Move(SIZE szOffset, bool bNeedInvalidate)
	{
		CControlUI::Move(szOffset, bNeedInvalidate);
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) m_pVerticalScrollBar->Move(szOffset, false);
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) m_pHorizontalScrollBar->Move(szOffset, false);
		for( int it = 0; it < m_items.GetSize(); it++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
			if( pControl != NULL && pControl->IsVisible() ) pControl->Move(szOffset, false);
		}
	}

	void CContainerUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		if( m_items.IsEmpty() ) return;

		rc = m_rcItem;
		RECT rcPadding = GetPadding();
		rc.left += rcPadding.left;
		rc.top += rcPadding.top;
		rc.right -= rcPadding.right;
		rc.bottom -= rcPadding.bottom;

		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
			rc.top -= m_pVerticalScrollBar->GetScrollPos();
			rc.bottom -= m_pVerticalScrollBar->GetScrollPos();
			rc.bottom += m_pVerticalScrollBar->GetScrollRange();
			rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		}
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
			rc.left -= m_pHorizontalScrollBar->GetScrollPos();
			rc.right -= m_pHorizontalScrollBar->GetScrollPos();
			rc.right += m_pHorizontalScrollBar->GetScrollRange();
			rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
		}

		for( int it = 0; it < m_items.GetSize(); it++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsAbsolute() ) {
				SetAbsolutePos(it);
			}
			else { 
				SIZE sz = { rc.right - rc.left, rc.bottom - rc.top };
				if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
				if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
				if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
				if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();

				int iPosX = rc.left;
				int iPosY = rc.top;
				if( m_iChildAlign == DT_CENTER ) iPosX = rc.left + (rc.right - rc.left - sz.cx) / 2;
				else if( m_iChildAlign == DT_RIGHT ) iPosX = rc.right - sz.cx;
				if( m_iChildVAlign == DT_VCENTER ) iPosY = rc.top + (rc.bottom - rc.top - sz.cy) / 2;
				else if( m_iChildVAlign == DT_BOTTOM ) iPosY = rc.bottom - sz.cy;

				RECT rcCtrl = { iPosX, iPosY, iPosX + sz.cx, iPosY + sz.cy };
				pControl->SetPos(rcCtrl, false);
			}
		}
	}

	void CContainerUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("mouse-child")) == 0 ) SetMouseChildEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("pointer-events")) == 0 ) {
			// 容器：none → 自身与子均不接收；auto → 恢复默认可点
			bool bEnabled = true;
			if( ParseCssPointerEventsEnabled(pstrValue, bEnabled) ) {
				SetMouseEnabled(bEnabled);
				SetMouseChildEnabled(bEnabled);
			}
		}
		else if( _tcsicmp(pstrName, _T("overflow")) == 0 ) {
			// CSS：1 值双轴；2 值为 overflow-x overflow-y → h/v 滚动条
			bool bX = false, bY = false;
			if( ParseCssOverflowShorthand(pstrValue, bX, bY) )
				EnableScrollBar(bY, bX);
		}
		else if( _tcsicmp(pstrName, _T("overflow-x")) == 0 ) {
			bool bEnable = false;
			if( ParseCssOverflowEnablesScroll(pstrValue, bEnable) )
				EnableScrollBar(GetVerticalScrollBar() != NULL, bEnable);
		}
		else if( _tcsicmp(pstrName, _T("overflow-y")) == 0 ) {
			bool bEnable = false;
			if( ParseCssOverflowEnablesScroll(pstrValue, bEnable) )
				EnableScrollBar(bEnable, GetHorizontalScrollBar() != NULL);
		}
		else if( _tcsicmp(pstrName, _T("v-scrollbar")) == 0
			|| _tcsicmp(pstrName, _T("vscrollbar")) == 0 ) {
			EnableScrollBar(_tcsicmp(pstrValue, _T("true")) == 0, GetHorizontalScrollBar() != NULL);
		}
		else if( _tcsicmp(pstrName, _T("v-scrollbar-style")) == 0 ) {
			m_sVerticalScrollBarStyle = pstrValue;
			EnableScrollBar(TRUE, GetHorizontalScrollBar() != NULL);
			if( GetVerticalScrollBar() ) {
				LPCTSTR pStyle = m_pManager->GetStyle(m_sVerticalScrollBarStyle.GetData());
				if( pStyle ) {
					GetVerticalScrollBar()->ApplyAttributeList(pStyle);
				}
				else {
					GetVerticalScrollBar()->ApplyAttributeList(pstrValue);
				}
			}
		}
		else if( _tcsicmp(pstrName, _T("h-scrollbar")) == 0
			|| _tcsicmp(pstrName, _T("hscrollbar")) == 0 ) {
			EnableScrollBar(GetVerticalScrollBar() != NULL, _tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("h-scrollbar-style")) == 0 ) {
			m_sHorizontalScrollBarStyle = pstrValue;
			EnableScrollBar(TRUE, GetHorizontalScrollBar() != NULL);
			if( GetHorizontalScrollBar() ) {
				LPCTSTR pStyle = m_pManager->GetStyle(m_sHorizontalScrollBarStyle.GetData());
				if( pStyle ) {
					GetHorizontalScrollBar()->ApplyAttributeList(pStyle);
				}
				else {
					GetHorizontalScrollBar()->ApplyAttributeList(pstrValue);
				}
			}
		}
		else if( _tcsicmp(pstrName, _T("gap")) == 0 ) SetGap(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("justify-content")) == 0 ) {
			// 主轴：HBox 水平；VBox 竖直
			bool bVert = IsMainAxisVertical();
			if( _tcsicmp(pstrValue, _T("left")) == 0 || _tcsicmp(pstrValue, _T("flex-start")) == 0
				|| _tcsicmp(pstrValue, _T("start")) == 0 || _tcsicmp(pstrValue, _T("top")) == 0 )
				SetJustifyContent(bVert ? DT_TOP : DT_LEFT);
			else if( _tcsicmp(pstrValue, _T("center")) == 0 || _tcsicmp(pstrValue, _T("middle")) == 0
				|| _tcsicmp(pstrValue, _T("vcenter")) == 0 )
				SetJustifyContent(bVert ? DT_VCENTER : DT_CENTER);
			else if( _tcsicmp(pstrValue, _T("right")) == 0 || _tcsicmp(pstrValue, _T("flex-end")) == 0
				|| _tcsicmp(pstrValue, _T("end")) == 0 || _tcsicmp(pstrValue, _T("bottom")) == 0 )
				SetJustifyContent(bVert ? DT_BOTTOM : DT_RIGHT);
		}
		else if( _tcsicmp(pstrName, _T("align-items")) == 0 ) {
			// 交叉轴：HBox 竖直；VBox 水平
			bool bVert = IsMainAxisVertical();
			if( bVert ) {
				if( _tcsicmp(pstrValue, _T("left")) == 0 || _tcsicmp(pstrValue, _T("flex-start")) == 0
					|| _tcsicmp(pstrValue, _T("start")) == 0 )
					SetAlignItems(DT_LEFT);
				else if( _tcsicmp(pstrValue, _T("center")) == 0 || _tcsicmp(pstrValue, _T("middle")) == 0 )
					SetAlignItems(DT_CENTER);
				else if( _tcsicmp(pstrValue, _T("right")) == 0 || _tcsicmp(pstrValue, _T("flex-end")) == 0
					|| _tcsicmp(pstrValue, _T("end")) == 0 )
					SetAlignItems(DT_RIGHT);
			}
			else {
				if( _tcsicmp(pstrValue, _T("top")) == 0 || _tcsicmp(pstrValue, _T("flex-start")) == 0
					|| _tcsicmp(pstrValue, _T("start")) == 0 )
					SetAlignItems(DT_TOP);
				else if( _tcsicmp(pstrValue, _T("vcenter")) == 0 || _tcsicmp(pstrValue, _T("middle")) == 0
					|| _tcsicmp(pstrValue, _T("center")) == 0 )
					SetAlignItems(DT_VCENTER);
				else if( _tcsicmp(pstrValue, _T("bottom")) == 0 || _tcsicmp(pstrValue, _T("flex-end")) == 0
					|| _tcsicmp(pstrValue, _T("end")) == 0 )
					SetAlignItems(DT_BOTTOM);
			}
		}
		else if( _tcsicmp(pstrName, _T("scroll-step-size")) == 0 ) SetScrollStepSize(_ttoi(pstrValue));
		else if (_tcsicmp(pstrName, _T("fixed-scrollbar")) == 0) SetFixedScrollbar(_tcsicmp(pstrValue, _T("true")) == 0);
		else if (_tcsicmp(pstrName, _T("show-scrollbar")) == 0) SetShowScrollbar(_tcsicmp(pstrValue, _T("true")) == 0);
		else CControlUI::SetAttribute(pstrName, pstrValue);
	}

	void CContainerUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
	{
		for( int it = 0; it < m_items.GetSize(); it++ ) {
			static_cast<CControlUI*>(m_items[it])->SetManager(pManager, this, bInit);
		}

		if( m_pVerticalScrollBar != NULL ) m_pVerticalScrollBar->SetManager(pManager, this, bInit);
		if( m_pHorizontalScrollBar != NULL ) m_pHorizontalScrollBar->SetManager(pManager, this, bInit);
		CControlUI::SetManager(pManager, pParent, bInit);
	}

	CControlUI* CContainerUI::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
	{
		if( (uFlags & UIFIND_VISIBLE) != 0 && !IsVisible() ) return NULL;
		if( (uFlags & UIFIND_ENABLED) != 0 && !IsEnabled() ) return NULL;
		if( (uFlags & UIFIND_HITTEST) != 0 && !::PtInRect(&m_rcItem, *(static_cast<LPPOINT>(pData))) ) return NULL;
		if( (uFlags & UIFIND_UPDATETEST) != 0 && Proc(this, pData) != NULL ) return NULL;

		CControlUI* pResult = NULL;
		if( (uFlags & UIFIND_ME_FIRST) != 0 ) {
			if( (uFlags & UIFIND_HITTEST) == 0 || IsMouseEnabled() ) pResult = Proc(this, pData);
		}
		if( pResult == NULL && m_pVerticalScrollBar != NULL ) {
			if( (uFlags & UIFIND_HITTEST) == 0 || IsMouseEnabled() ) pResult = m_pVerticalScrollBar->FindControl(Proc, pData, uFlags);
		}
		if( pResult == NULL && m_pHorizontalScrollBar != NULL ) {
			if( (uFlags & UIFIND_HITTEST) == 0 || IsMouseEnabled() ) pResult = m_pHorizontalScrollBar->FindControl(Proc, pData, uFlags);
		}
		if( pResult != NULL ) return pResult;

		if( (uFlags & UIFIND_HITTEST) == 0 || IsMouseChildEnabled() ) {
			RECT rc = m_rcItem;
			
			RECT rcPadding = GetPadding();
			rc.left += rcPadding.left;
			rc.top += rcPadding.top;
			rc.right -= rcPadding.right;
			rc.bottom -= rcPadding.bottom;

			if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
			if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
			if( (uFlags & UIFIND_TOP_FIRST) != 0 ) {
				for( int it = m_items.GetSize() - 1; it >= 0; it-- ) {
					pResult = static_cast<CControlUI*>(m_items[it])->FindControl(Proc, pData, uFlags);
					if( pResult != NULL ) {
						if( (uFlags & UIFIND_HITTEST) != 0 && !pResult->IsAbsolute() && !::PtInRect(&rc, *(static_cast<LPPOINT>(pData))) )
							continue;
						else 
							return pResult;
					}          
				}
			}
			else {
				for( int it = 0; it < m_items.GetSize(); it++ ) {
					pResult = static_cast<CControlUI*>(m_items[it])->FindControl(Proc, pData, uFlags);
					if( pResult != NULL ) {
						if( (uFlags & UIFIND_HITTEST) != 0 && !pResult->IsAbsolute() && !::PtInRect(&rc, *(static_cast<LPPOINT>(pData))) )
							continue;
						else 
							return pResult;
					} 
				}
			}
		}

		pResult = NULL;
		if( pResult == NULL && (uFlags & UIFIND_ME_FIRST) == 0 ) {
			if( (uFlags & UIFIND_HITTEST) == 0 || IsMouseEnabled() ) pResult = Proc(this, pData);
		}
		return pResult;
	}

	bool CContainerUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		RECT rcTemp = { 0 };
		if( !::IntersectRect(&rcTemp, &rcPaint, &m_rcItem) ) return true;

		// 有 BorderRadius 时用圆角裁剪包住「自身 + 子控件」，否则子控件直角会盖住圆角透明区
		SIZE cxyRound = GetBorderRadius();
		if( cxyRound.cx > 0 || cxyRound.cy > 0 ) {
			CRenderClipScope roundClip(ctx, rcTemp, m_rcItem, cxyRound.cx, cxyRound.cy);
			return DoPaintContent(ctx, rcPaint, pStopControl);
		}

		CRenderClipScope clip(ctx, rcTemp);
		return DoPaintContent(ctx, rcPaint, pStopControl);
	}

	bool CContainerUI::DoPaintContent(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		RECT rcTemp = { 0 };
		CControlUI::DoPaint(ctx, rcPaint, pStopControl);

		if( m_items.GetSize() > 0 ) {
			RECT rcPadding = GetPadding();
			RECT rc = m_rcItem;
			rc.left += rcPadding.left;
			rc.top += rcPadding.top;
			rc.right -= rcPadding.right;
			rc.bottom -= rcPadding.bottom;
			if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
			if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

			if( !::IntersectRect(&rcTemp, &rcPaint, &rc) ) {
				for( int it = 0; it < m_items.GetSize(); it++ ) {
					CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
					if( pControl == pStopControl ) return false;
					if( !pControl->IsVisible() ) continue;
					if( !::IntersectRect(&rcTemp, &rcPaint, &pControl->GetPos()) ) continue;
					if( pControl ->IsAbsolute() ) {
						if( !::IntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos()) ) continue;
						if( !pControl->Paint(ctx, rcPaint, pStopControl) ) return false;
					}
				}
			}
			else {
				CRenderClipScope childClip(ctx, rcTemp);
				for( int it = 0; it < m_items.GetSize(); it++ ) {
					CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
					if( pControl == pStopControl ) return false;
					if( !pControl->IsVisible() ) continue;
					if( !::IntersectRect(&rcTemp, &rcPaint, &pControl->GetPos()) ) continue;
					if( pControl->IsAbsolute() ) {
						if( !::IntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos()) ) continue;
						ctx.SuspendClip();
                        if( !pControl->Paint(ctx, rcPaint, pStopControl) ) return false;
						ctx.ResumeClip();
					}
					else {
						if( !::IntersectRect(&rcTemp, &rc, &pControl->GetPos()) ) continue;
                        if( !pControl->Paint(ctx, rcPaint, pStopControl) ) return false;
					}
				}
			}
		}

		if( m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible() ) {
			if( m_pVerticalScrollBar == pStopControl ) return false;
			if( ::IntersectRect(&rcTemp, &rcPaint, &m_pVerticalScrollBar->GetPos()) ) {
				if( !m_pVerticalScrollBar->Paint(ctx, rcPaint, pStopControl) ) return false;
			}
		}

		if( m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible() ) {
			if( m_pHorizontalScrollBar == pStopControl ) return false;
			if( ::IntersectRect(&rcTemp, &rcPaint, &m_pHorizontalScrollBar->GetPos()) ) {
				if( !m_pHorizontalScrollBar->Paint(ctx, rcPaint, pStopControl) ) return false;
			}
		}
		return true;
	}

	void CContainerUI::SetAbsolutePos(int iIndex)
	{
		// 因为CControlUI::SetPos对float的操作影响，这里不能对float组件添加滚动条的影响
		if( iIndex < 0 || iIndex >= m_items.GetSize() ) return;

		CControlUI* pControl = static_cast<CControlUI*>(m_items[iIndex]);

		if( !pControl->IsVisible() ) return;
		if( !pControl->IsAbsolute() ) return;

		SIZE szXY = pControl->GetFixedXY();
		RECT rcMargin = pControl->GetMargin();
		// CSS-like：绝对定位偏移优先用 margin；FixedXY 仍可供 C++ / 旧调用叠加
		szXY.cx += rcMargin.left;
		szXY.cy += rcMargin.top;
		SIZE sz = {pControl->GetFixedWidth(), pControl->GetFixedHeight()};

		int nParentWidth = m_rcItem.right - m_rcItem.left;
		int nParentHeight = m_rcItem.bottom - m_rcItem.top;

		UINT uAlign = pControl->GetAbsoluteAlign();
		if(uAlign != 0) {
			RECT rcCtrl = {0, 0, sz.cx, sz.cy};
			if((uAlign & DT_CENTER) != 0) {
				::OffsetRect(&rcCtrl, (nParentWidth - sz.cx) / 2, 0);
			}
			else if((uAlign & DT_RIGHT) != 0) {
				::OffsetRect(&rcCtrl, nParentWidth - sz.cx, 0);
			}
			else {
				::OffsetRect(&rcCtrl, szXY.cx, 0);
			}

			if((uAlign & DT_VCENTER) != 0) {
				::OffsetRect(&rcCtrl, 0, (nParentHeight - sz.cy) / 2);
			}
			else if((uAlign & DT_BOTTOM) != 0) {
				::OffsetRect(&rcCtrl, 0, nParentHeight - sz.cy);
			}
			else {
				::OffsetRect(&rcCtrl, 0, szXY.cy);
			}

			::OffsetRect(&rcCtrl, m_rcItem.left, m_rcItem.top);
			pControl->SetPos(rcCtrl, false);
		}
		else {
			TPercentInfo rcPercent = pControl->GetAbsolutePercent();
			LONG width = m_rcItem.right - m_rcItem.left;
			LONG height = m_rcItem.bottom - m_rcItem.top;
			LONG left = szXY.cx < 0 ? m_rcItem.right : m_rcItem.left;
			LONG top = szXY.cy < 0 ? m_rcItem.bottom : m_rcItem.top;
			RECT rcCtrl = { 0 };
			rcCtrl.left = (LONG)(width*rcPercent.left) + szXY.cx + left;
			rcCtrl.top = (LONG)(height*rcPercent.top) + szXY.cy + top;
			rcCtrl.right = (LONG)(width*rcPercent.right) + szXY.cx + sz.cx + left;
			rcCtrl.bottom = (LONG)(height*rcPercent.bottom) + szXY.cy + sz.cy + top;
			pControl->SetPos(rcCtrl, false);
		}
	}

	void CContainerUI::ProcessScrollBar(RECT rc, int cxRequired, int cyRequired)
	{
		if (m_pHorizontalScrollBar)
		{
			if (cxRequired > rc.right - rc.left && !m_pHorizontalScrollBar->IsVisible())
			{
				m_pHorizontalScrollBar->SetVisible(true);
				m_pHorizontalScrollBar->SetScrollRange(cxRequired - (rc.right - rc.left));
				m_pHorizontalScrollBar->SetScrollPos(0);
				SetPos(m_rcItem);
			}
			else if (m_pHorizontalScrollBar->IsVisible())
			{
				int cxScroll = cxRequired - (rc.right - rc.left);
				if (cxScroll <= 0)
				{
					m_pHorizontalScrollBar->SetVisible(false);
					m_pHorizontalScrollBar->SetScrollPos(0);
					m_pHorizontalScrollBar->SetScrollRange(0);
					SetPos(m_rcItem);
				}
				else
				{
					RECT rcScrollBarPos = { rc.left, rc.bottom, rc.right, rc.bottom + m_pHorizontalScrollBar->GetFixedHeight() };
					m_pHorizontalScrollBar->SetPos(rcScrollBarPos);

					if (m_pHorizontalScrollBar->GetScrollRange() != cxScroll) 
					{
						int iScrollPos = m_pHorizontalScrollBar->GetScrollPos();
						m_pHorizontalScrollBar->SetScrollRange(::abs(cxScroll));
						if(iScrollPos > m_pHorizontalScrollBar->GetScrollPos()) 
						{
							SetPos(m_rcItem);
						}
					}
				}
			}
		}

		if (m_pVerticalScrollBar)
		{
			if (cyRequired > rc.bottom - rc.top && !m_pVerticalScrollBar->IsVisible()) 
			{
				m_pVerticalScrollBar->SetVisible(true);
				m_pVerticalScrollBar->SetScrollRange(cyRequired - (rc.bottom - rc.top));
				m_pVerticalScrollBar->SetScrollPos(0);
				SetPos(m_rcItem);
			}
			else if (m_pVerticalScrollBar->IsVisible())
			{
				int cyScroll = cyRequired - (rc.bottom - rc.top);
				if (cyScroll <= 0) 
				{
					m_pVerticalScrollBar->SetVisible(false);
					m_pVerticalScrollBar->SetScrollPos(0);
					m_pVerticalScrollBar->SetScrollRange(0);
					SetPos(m_rcItem);
				}
				else
				{
					RECT rcScrollBarPos = { rc.right, rc.top, rc.right + m_pVerticalScrollBar->GetFixedWidth(), rc.bottom };
					m_pVerticalScrollBar->SetPos(rcScrollBarPos);

					if (m_pVerticalScrollBar->GetScrollRange() != cyScroll)
					{
						int iScrollPos = m_pVerticalScrollBar->GetScrollPos();
						m_pVerticalScrollBar->SetScrollRange(::abs(cyScroll));
						if(iScrollPos > m_pVerticalScrollBar->GetScrollPos()) 
						{
							SetPos(m_rcItem);
						}
					}
				}
			}
		}
	}

	bool CContainerUI::SetSubControlText( LPCTSTR pstrSubControlName,LPCTSTR pstrText )
	{
		CControlUI* pSubControl=NULL;
		pSubControl=this->FindSubControl(pstrSubControlName);
		if (pSubControl!=NULL) {
			pSubControl->SetText(pstrText);
			return TRUE;
		}
		else return FALSE;
	}

	bool CContainerUI::SetSubControlFixedHeight( LPCTSTR pstrSubControlName,int cy )
	{
		CControlUI* pSubControl=NULL;
		pSubControl=this->FindSubControl(pstrSubControlName);
		if (pSubControl!=NULL) {
			pSubControl->SetFixedHeight(cy);
			return TRUE;
		}
		else return FALSE;
	}

	bool CContainerUI::SetSubControlFixedWdith( LPCTSTR pstrSubControlName,int cx )
	{
		CControlUI* pSubControl=NULL;
		pSubControl=this->FindSubControl(pstrSubControlName);
		if (pSubControl!=NULL) {
			pSubControl->SetFixedWidth(cx);
			return TRUE;
		}
		else return FALSE;
	}

	bool CContainerUI::SetSubControlUserData( LPCTSTR pstrSubControlName,LPCTSTR pstrText )
	{
		CControlUI* pSubControl=NULL;
		pSubControl=this->FindSubControl(pstrSubControlName);
		if (pSubControl!=NULL) {
			pSubControl->SetUserData(pstrText);
			return TRUE;
		}
		else return FALSE;
	}

	DuiLib::CDuiString CContainerUI::GetSubControlText( LPCTSTR pstrSubControlName )
	{
		CControlUI* pSubControl=NULL;
		pSubControl=this->FindSubControl(pstrSubControlName);
		if (pSubControl==NULL)
			return _T("");
		else
			return pSubControl->GetText();
	}

	int CContainerUI::GetSubControlFixedHeight( LPCTSTR pstrSubControlName )
	{
		CControlUI* pSubControl=NULL;
		pSubControl=this->FindSubControl(pstrSubControlName);
		if (pSubControl==NULL) return -1;
		else return pSubControl->GetFixedHeight();
	}

	int CContainerUI::GetSubControlFixedWdith( LPCTSTR pstrSubControlName )
	{
		CControlUI* pSubControl=NULL;
		pSubControl=this->FindSubControl(pstrSubControlName);
		if (pSubControl==NULL) return -1;
		else return pSubControl->GetFixedWidth();
	}

	const CDuiString CContainerUI::GetSubControlUserData( LPCTSTR pstrSubControlName )
	{
		CControlUI* pSubControl=NULL;
		pSubControl=this->FindSubControl(pstrSubControlName);
		if (pSubControl==NULL) return _T("");
		else return pSubControl->GetUserData();
	}

	CControlUI* CContainerUI::FindSubControl( LPCTSTR pstrSubControlName )
	{
		CControlUI* pSubControl=NULL;
		pSubControl = static_cast<CControlUI*>(GetManager()->FindSubControlByName(this, pstrSubControlName));
		return pSubControl;
	}

	void CContainerUI::DestroyChild(CControlUI* pControl)
	{
		if( !m_bAutoDestroy ) return;
		// 立即删除路径也要先 Reap；延迟路径里 AddDelayedCleanup 会再 Reap 一次（幂等）。
		if( m_pManager != NULL )
			m_pManager->ReapObjects(pControl);
		if( m_bDelayedDestroy && m_pManager )
			m_pManager->AddDelayedCleanup(pControl);
		else
			delete pControl;
	}

	void CContainerUI::SuspendLayout()
	{
		++m_nUpdateLock;
	}

	void CContainerUI::ResumeLayout()
	{
		if( m_nUpdateLock <= 0 ) return;
		if( --m_nUpdateLock == 0 ) NeedUpdate();
	}

} // namespace DuiLib
