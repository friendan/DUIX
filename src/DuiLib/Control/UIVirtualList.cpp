#include "StdAfx.h"
#include "UIVirtualList.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CVirtualListUI)

	CVirtualListUI::CVirtualListUI()
		: m_nItemCount(0)
		, m_nItemHeight(28)
		, m_iCurSel(-1)
		, m_iHoverIndex(-1)
		, m_pCallback(NULL)
		, m_iItemFont(-1)
		, m_uItemTextStyle(DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS)
		, m_dwItemColor(0x333333FF)
		, m_dwItemBackgroundColor(0)
		, m_dwItemHoverColor(0)
		, m_dwItemHoverBackgroundColor(0xE6F4FFFF)
		, m_dwItemSelectedColor(0)
		, m_dwItemSelectedBackgroundColor(0xBAE0FFFF)
		, m_dwItemDisabledColor(0xBFBFBFFF)
		, m_dwItemDisabledBackgroundColor(0)
		, m_dwItemLineColor(0xF0F0F0FF)
		, m_bAlternateBk(false)
		, m_bShowRowLine(false)
		, 		m_bShowHtml(false)
	{
		m_rcItemPadding.left = 12;
		m_rcItemPadding.top = 0;
		m_rcItemPadding.right = 12;
		m_rcItemPadding.bottom = 0;
		SetScrollStepSize(m_nItemHeight);
	}

	CVirtualListUI::~CVirtualListUI()
	{
		m_pCallback = NULL;
	}

	void CVirtualListUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
	{
		CContainerUI::SetManager(pManager, pParent, bInit);
		if( bInit ) {
			EnableScrollBar(true, false);
			SyncScrollLineSize();
		}
	}

	LPCTSTR CVirtualListUI::GetClass() const
	{
		return _T("VirtualListUI");
	}

	LPVOID CVirtualListUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_VIRTUALLIST) == 0 ) return static_cast<CVirtualListUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	UINT CVirtualListUI::GetControlFlags() const
	{
		if( !IsEnabled() ) return CControlUI::GetControlFlags();
		return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
	}

	void CVirtualListUI::SetItemCount(int nCount)
	{
		if( nCount < 0 ) nCount = 0;
		if( m_nItemCount == nCount ) return;
		m_nItemCount = nCount;
		if( m_iCurSel >= m_nItemCount ) m_iCurSel = m_nItemCount > 0 ? m_nItemCount - 1 : -1;
		if( m_iHoverIndex >= m_nItemCount ) m_iHoverIndex = -1;
		NeedUpdate();
	}

	int CVirtualListUI::GetItemCount() const
	{
		return m_nItemCount;
	}

	void CVirtualListUI::SetItemHeight(int nHeight)
	{
		if( nHeight < 1 ) nHeight = 1;
		if( m_nItemHeight == nHeight ) return;
		m_nItemHeight = nHeight;
		SetScrollStepSize(m_nItemHeight);
		SyncScrollLineSize();
		NeedUpdate();
	}

	int CVirtualListUI::GetItemHeight() const
	{
		return m_nItemHeight;
	}

	void CVirtualListUI::SetCallback(IVirtualListCallback* pCallback)
	{
		m_pCallback = pCallback;
		Invalidate();
	}

	IVirtualListCallback* CVirtualListUI::GetCallback() const
	{
		return m_pCallback;
	}

	int CVirtualListUI::GetScaledItemHeight() const
	{
		int h = m_nItemHeight;
		if( m_pManager != NULL ) h = m_pManager->GetDPIObj()->Scale(h);
		return h > 1 ? h : 1;
	}

	void CVirtualListUI::SyncScrollLineSize()
	{
		if( m_pVerticalScrollBar != NULL )
			m_pVerticalScrollBar->SetLineSize(GetScaledItemHeight());
	}

	RECT CVirtualListUI::GetListClientRect() const
	{
		RECT rc = m_rcItem;
		RECT rcPadding = GetPadding();
		rc.left += rcPadding.left;
		rc.top += rcPadding.top;
		rc.right -= rcPadding.right;
		rc.bottom -= rcPadding.bottom;
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() )
			rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() )
			rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
		return rc;
	}

	void CVirtualListUI::GetVisibleRange(int& iFirst, int& iLast) const
	{
		iFirst = 0;
		iLast = -1;
		if( m_nItemCount <= 0 ) return;

		RECT rc = GetListClientRect();
		int nItemH = GetScaledItemHeight();
		int nScroll = GetScrollPos().cy;
		int nClientH = rc.bottom - rc.top;
		if( nClientH < 0 ) nClientH = 0;

		iFirst = nScroll / nItemH;
		if( iFirst < 0 ) iFirst = 0;
		if( iFirst >= m_nItemCount ) iFirst = m_nItemCount - 1;

		iLast = (nScroll + nClientH) / nItemH;
		if( iLast >= m_nItemCount ) iLast = m_nItemCount - 1;
		if( iLast < iFirst ) iLast = iFirst;
	}

	RECT CVirtualListUI::GetItemRect(int iIndex) const
	{
		RECT rc = { 0, 0, 0, 0 };
		if( iIndex < 0 || iIndex >= m_nItemCount ) return rc;
		RECT rcClient = GetListClientRect();
		int nItemH = GetScaledItemHeight();
		int nScroll = GetScrollPos().cy;
		rc.left = rcClient.left;
		rc.right = rcClient.right;
		rc.top = rcClient.top - nScroll + iIndex * nItemH;
		rc.bottom = rc.top + nItemH;
		return rc;
	}

	int CVirtualListUI::HitTestItem(POINT pt) const
	{
		RECT rcClient = GetListClientRect();
		if( !::PtInRect(&rcClient, pt) ) return -1;
		if( m_nItemCount <= 0 ) return -1;
		int nItemH = GetScaledItemHeight();
		int nScroll = GetScrollPos().cy;
		int iIndex = (pt.y - rcClient.top + nScroll) / nItemH;
		if( iIndex < 0 || iIndex >= m_nItemCount ) return -1;
		return iIndex;
	}

	int CVirtualListUI::GetCurSel() const
	{
		return m_iCurSel;
	}

	bool CVirtualListUI::SelectItem(int iIndex, bool bTakeFocus)
	{
		if( iIndex < -1 ) iIndex = -1;
		if( iIndex >= m_nItemCount ) return false;
		if( m_iCurSel == iIndex ) {
			if( bTakeFocus && m_pManager ) m_pManager->SetFocus(this);
			return true;
		}
		m_iCurSel = iIndex;
		Invalidate();
		if( bTakeFocus && m_pManager ) m_pManager->SetFocus(this);
		if( m_pManager != NULL && iIndex >= 0 )
			m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELECT, (WPARAM)iIndex);
		return true;
	}

	void CVirtualListUI::EnsureVisible(int iIndex)
	{
		if( iIndex < 0 || iIndex >= m_nItemCount ) return;
		RECT rcClient = GetListClientRect();
		int nClientH = rcClient.bottom - rcClient.top;
		if( nClientH <= 0 ) return;
		int nItemH = GetScaledItemHeight();
		int nScroll = GetScrollPos().cy;
		int nItemTop = iIndex * nItemH;
		int nItemBottom = nItemTop + nItemH;
		SIZE sz = GetScrollPos();
		if( nItemTop < nScroll )
			sz.cy = nItemTop;
		else if( nItemBottom > nScroll + nClientH )
			sz.cy = nItemBottom - nClientH;
		else
			return;
		SetScrollPos(sz);
	}

	bool CVirtualListUI::Add(CControlUI* /*pControl*/)
	{
		// 虚拟列表不托管行控件
		return false;
	}

	bool CVirtualListUI::AddAt(CControlUI* /*pControl*/, int /*iIndex*/)
	{
		return false;
	}

	void CVirtualListUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);

		rc = m_rcItem;
		RECT rcPadding = GetPadding();
		rc.left += rcPadding.left;
		rc.top += rcPadding.top;
		rc.right -= rcPadding.right;
		rc.bottom -= rcPadding.bottom;

		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() )
			rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() )
			rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

		int nItemH = GetScaledItemHeight();
		int cyRequired = (m_nItemCount > 0) ? (m_nItemCount * nItemH) : 0;
		ProcessScrollBar(rc, 0, cyRequired);
		SyncScrollLineSize();
	}

	void CVirtualListUI::PaintItemDefault(IRenderContext& ctx, int iIndex, const RECT& rcItem, UINT uState)
	{
		DWORD dwBk = m_dwItemBackgroundColor;
		DWORD dwText = m_dwItemColor;
		bool bSelected = (uState & UISTATE_SELECTED) != 0;
		bool bHot = (uState & UISTATE_HOT) != 0;
		bool bDisabled = (uState & UISTATE_DISABLED) != 0;

		if( bDisabled ) {
			if( m_dwItemDisabledBackgroundColor != 0 ) dwBk = m_dwItemDisabledBackgroundColor;
			dwText = m_dwItemDisabledColor;
		}
		else if( bSelected ) {
			if( m_dwItemSelectedBackgroundColor != 0 ) dwBk = m_dwItemSelectedBackgroundColor;
			if( m_dwItemSelectedColor != 0 ) dwText = m_dwItemSelectedColor;
		}
		else if( bHot ) {
			if( m_dwItemHoverBackgroundColor != 0 ) dwBk = m_dwItemHoverBackgroundColor;
			if( m_dwItemHoverColor != 0 ) dwText = m_dwItemHoverColor;
		}
		else if( m_bAlternateBk && (iIndex % 2) == 1 ) {
			dwBk = 0xFAFAFAFF;
		}

		if( dwBk != 0 )
			ctx.DrawColor(rcItem, GetAdjustColor(dwBk));

		if( m_bShowRowLine && m_dwItemLineColor != 0 ) {
			RECT rcLine = { rcItem.left, rcItem.bottom - 1, rcItem.right, rcItem.bottom };
			ctx.DrawColor(rcLine, GetAdjustColor(m_dwItemLineColor));
		}

		LPCTSTR pText = NULL;
		if( m_pCallback != NULL )
			pText = m_pCallback->GetItemText(this, iIndex);
		if( pText == NULL || pText[0] == _T('\0') ) return;

		RECT rcPad = m_rcItemPadding;
		if( m_pManager != NULL ) rcPad = m_pManager->GetDPIObj()->Scale(rcPad);
		RECT rcText = rcItem;
		rcText.left += rcPad.left;
		rcText.top += rcPad.top;
		rcText.right -= rcPad.right;
		rcText.bottom -= rcPad.bottom;

		int iFont = m_iItemFont;
		if( m_bShowHtml ) {
			int nLinks = 0;
			ctx.DrawHtmlText(rcText, pText, GetAdjustColor(dwText), NULL, NULL, nLinks, iFont, m_uItemTextStyle);
		}
		else {
			ctx.DrawText(rcText, pText, GetAdjustColor(dwText), iFont, m_uItemTextStyle);
		}
	}

	bool CVirtualListUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		RECT rcTemp = { 0 };
		if( !::IntersectRect(&rcTemp, &rcPaint, &m_rcItem) ) return true;

		SIZE cxyRound = GetBorderRadius();
		if( cxyRound.cx > 0 || cxyRound.cy > 0 ) {
			CRenderClipScope roundClip(ctx, rcTemp, m_rcItem, cxyRound.cx, cxyRound.cy);
			return PaintItemsAndScrollBars(ctx, rcPaint, pStopControl);
		}

		CRenderClipScope clip(ctx, rcTemp);
		return PaintItemsAndScrollBars(ctx, rcPaint, pStopControl);
	}

	bool CVirtualListUI::PaintItemsAndScrollBars(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		CControlUI::DoPaint(ctx, rcPaint, pStopControl);

		RECT rcClient = GetListClientRect();
		RECT rcTemp = { 0 };
		if( ::IntersectRect(&rcTemp, &rcPaint, &rcClient) ) {
			CRenderClipScope childClip(ctx, rcTemp);
			int iFirst = 0, iLast = -1;
			GetVisibleRange(iFirst, iLast);
			for( int i = iFirst; i <= iLast; ++i ) {
				RECT rcItem = GetItemRect(i);
				if( !::IntersectRect(&rcTemp, &rcPaint, &rcItem) ) continue;

				UINT uState = 0;
				if( !IsEnabled() ) uState |= UISTATE_DISABLED;
				if( i == m_iCurSel ) uState |= UISTATE_SELECTED;
				if( i == m_iHoverIndex ) uState |= UISTATE_HOT;

				bool bCustom = false;
				if( m_pCallback != NULL )
					bCustom = m_pCallback->PaintItem(this, ctx, i, rcItem, uState);
				if( !bCustom )
					PaintItemDefault(ctx, i, rcItem, uState);
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

	void CVirtualListUI::UpdateHover(POINT pt)
	{
		int iHover = HitTestItem(pt);
		if( iHover == m_iHoverIndex ) return;
		m_iHoverIndex = iHover;
		Invalidate();
	}

	void CVirtualListUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( !BubbleEvent(event) ) CContainerUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETCURSOR && IsEnabled() ) {
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			return;
		}

		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK ) {
			if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) {
				int iHit = HitTestItem(event.ptMouse);
				if( iHit >= 0 ) {
					SelectItem(iHit, true);
					if( m_pManager != NULL ) {
						if( event.Type == UIEVENT_DBLCLICK )
							m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMACTIVATE, (WPARAM)iHit);
						else
							m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMCLICK, (WPARAM)iHit);
					}
				}
			}
			return;
		}

		if( event.Type == UIEVENT_RBUTTONUP ) {
			if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) {
				int iHit = HitTestItem(event.ptMouse);
				if( iHit >= 0 ) {
					SelectItem(iHit, true);
					if( m_pManager != NULL )
						m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMRCLICK, (WPARAM)iHit);
				}
			}
			return;
		}

		if( event.Type == UIEVENT_MOUSEMOVE ) {
			UpdateHover(event.ptMouse);
			return;
		}

		if( event.Type == UIEVENT_MOUSELEAVE ) {
			if( m_iHoverIndex != -1 ) {
				m_iHoverIndex = -1;
				Invalidate();
			}
			return;
		}

		if( event.Type == UIEVENT_SCROLLWHEEL ) {
			CContainerUI::DoEvent(event);
			POINT pt = event.ptMouse;
			UpdateHover(pt);
			return;
		}

		if( event.Type == UIEVENT_KEYDOWN && IsEnabled() ) {
			switch( event.chKey ) {
			case VK_UP:
				if( m_iCurSel > 0 ) {
					SelectItem(m_iCurSel - 1);
					EnsureVisible(m_iCurSel);
				}
				else if( m_iCurSel < 0 && m_nItemCount > 0 ) {
					SelectItem(0);
					EnsureVisible(0);
				}
				return;
			case VK_DOWN:
				if( m_iCurSel + 1 < m_nItemCount ) {
					SelectItem(m_iCurSel < 0 ? 0 : m_iCurSel + 1);
					EnsureVisible(m_iCurSel);
				}
				return;
			case VK_PRIOR:
				PageUp();
				if( m_nItemCount > 0 ) {
					int iFirst = 0, iLast = 0;
					GetVisibleRange(iFirst, iLast);
					SelectItem(iFirst);
				}
				return;
			case VK_NEXT:
				PageDown();
				if( m_nItemCount > 0 ) {
					int iFirst = 0, iLast = 0;
					GetVisibleRange(iFirst, iLast);
					SelectItem(iLast);
				}
				return;
			case VK_HOME:
				if( m_nItemCount > 0 ) {
					SelectItem(0);
					EnsureVisible(0);
				}
				return;
			case VK_END:
				if( m_nItemCount > 0 ) {
					SelectItem(m_nItemCount - 1);
					EnsureVisible(m_nItemCount - 1);
				}
				return;
			case VK_RETURN:
				if( m_iCurSel >= 0 && m_pManager != NULL )
					m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMACTIVATE, (WPARAM)m_iCurSel);
				return;
			}
		}

		CContainerUI::DoEvent(event);
	}

	void CVirtualListUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("item-count")) == 0 ) {
			SetItemCount(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("item-height")) == 0 ) {
			SetItemHeight(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("item-font")) == 0 ) {
			m_iItemFont = _ttoi(pstrValue);
			Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("item-font-family")) == 0 || _tcsicmp(pstrName, _T("item-font-size")) == 0 ) {
			CDuiString sFamily;
			int nSize = 0;
			if( _tcsicmp(pstrName, _T("item-font-family")) == 0 ) sFamily = pstrValue ? pstrValue : _T("");
			else {
				LPTSTR pEnd = NULL;
				long v = _tcstol(pstrValue, &pEnd, 10);
				if( pEnd != pstrValue && v > 0 ) nSize = (int)v;
			}
			if( m_pManager != NULL ) {
				TFontInfo* pInfo = m_pManager->GetFontInfo(m_iItemFont);
				if( pInfo == NULL ) pInfo = m_pManager->GetDefaultFontInfo();
				if( pInfo != NULL ) {
					if( sFamily.IsEmpty() ) sFamily = pInfo->sFontName;
					if( nSize <= 0 ) nSize = pInfo->iSize;
				}
				if( sFamily.IsEmpty() ) sFamily = _T("Microsoft YaHei UI");
				if( nSize <= 0 ) nSize = 12;
				int id = m_pManager->EnsureFont(sFamily, nSize, false, false, false, false);
				if( id >= 0 ) m_iItemFont = id;
				Invalidate();
			}
		}
		else if( _tcsicmp(pstrName, _T("item-text-align")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("center")) == 0 )
				m_uItemTextStyle = (m_uItemTextStyle & ~(DT_LEFT | DT_RIGHT)) | DT_CENTER;
			else if( _tcsicmp(pstrValue, _T("right")) == 0 )
				m_uItemTextStyle = (m_uItemTextStyle & ~(DT_LEFT | DT_CENTER)) | DT_RIGHT;
			else
				m_uItemTextStyle = (m_uItemTextStyle & ~(DT_CENTER | DT_RIGHT)) | DT_LEFT;
			Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("item-padding")) == 0 ) {
			RECT rc = { 0 };
			if( ParseCssBoxToRect(pstrValue, rc) ) {
				m_rcItemPadding = rc;
				Invalidate();
			}
		}
		else if( _tcsicmp(pstrName, _T("item-color")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) { m_dwItemColor = clr; Invalidate(); }
		}
		else if( _tcsicmp(pstrName, _T("item-background-color")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) { m_dwItemBackgroundColor = clr; Invalidate(); }
		}
		else if( _tcsicmp(pstrName, _T("item-color-hover")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) { m_dwItemHoverColor = clr; Invalidate(); }
		}
		else if( _tcsicmp(pstrName, _T("item-background-color-hover")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) { m_dwItemHoverBackgroundColor = clr; Invalidate(); }
		}
		else if( _tcsicmp(pstrName, _T("item-color-selected")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) { m_dwItemSelectedColor = clr; Invalidate(); }
		}
		else if( _tcsicmp(pstrName, _T("item-background-color-selected")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) { m_dwItemSelectedBackgroundColor = clr; Invalidate(); }
		}
		else if( _tcsicmp(pstrName, _T("item-color-disabled")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) { m_dwItemDisabledColor = clr; Invalidate(); }
		}
		else if( _tcsicmp(pstrName, _T("item-background-color-disabled")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) { m_dwItemDisabledBackgroundColor = clr; Invalidate(); }
		}
		else if( _tcsicmp(pstrName, _T("item-line-color")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) { m_dwItemLineColor = clr; Invalidate(); }
		}
		else if( _tcsicmp(pstrName, _T("item-show-row-line")) == 0 ) {
			m_bShowRowLine = (_tcsicmp(pstrValue, _T("true")) == 0);
			Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("item-alternate-background")) == 0 ) {
			m_bAlternateBk = (_tcsicmp(pstrValue, _T("true")) == 0);
			Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("item-show-html")) == 0 ) {
			m_bShowHtml = (_tcsicmp(pstrValue, _T("true")) == 0);
			Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("selected")) == 0 ) {
			SelectItem(_ttoi(pstrValue));
		}
		else {
			CContainerUI::SetAttribute(pstrName, pstrValue);
		}
	}
}
