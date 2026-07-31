#include "StdAfx.h"
#include "UILinearLayout.h"

namespace DuiLib
{
	CLinearLayoutUI::CLinearLayoutUI(LayoutDirection eDirection)
		: m_eDirection(eDirection)
		, m_iMainAxis(eDirection == LAYOUT_VERTICAL ? 1 : 0)
		, m_iSepSize(0)
		, m_uButtonState(0)
		, m_bImmMode(false)
	{
		ptLastMouse.x = ptLastMouse.y = 0;
		::ZeroMemory(&m_rcNewPos, sizeof(m_rcNewPos));
	}

	// --- Axis dispatch helpers ---

	CScrollBarUI* CLinearLayoutUI::MainScrollBar() const
	{
		return m_eDirection == LAYOUT_VERTICAL ? m_pVerticalScrollBar : m_pHorizontalScrollBar;
	}
	CScrollBarUI* CLinearLayoutUI::CrossScrollBar() const
	{
		return m_eDirection == LAYOUT_VERTICAL ? m_pHorizontalScrollBar : m_pVerticalScrollBar;
	}
	UINT CLinearLayoutUI::MainChildAlign() const
	{
		return m_eDirection == LAYOUT_VERTICAL ? GetChildVAlign() : GetChildAlign();
	}
	UINT CLinearLayoutUI::CrossChildAlign() const
	{
		return m_eDirection == LAYOUT_VERTICAL ? GetChildAlign() : GetChildVAlign();
	}
	UINT CLinearLayoutUI::MainAlignCenter() const { return m_eDirection == LAYOUT_VERTICAL ? DT_VCENTER : DT_CENTER; }
	UINT CLinearLayoutUI::MainAlignEnd() const    { return m_eDirection == LAYOUT_VERTICAL ? DT_BOTTOM : DT_RIGHT; }
	UINT CLinearLayoutUI::CrossAlignCenter() const { return m_eDirection == LAYOUT_VERTICAL ? DT_CENTER : DT_VCENTER; }
	UINT CLinearLayoutUI::CrossAlignEnd() const    { return m_eDirection == LAYOUT_VERTICAL ? DT_RIGHT : DT_BOTTOM; }
	int CLinearLayoutUI::CtrlMainMin(CControlUI* p) const  { return m_eDirection == LAYOUT_VERTICAL ? p->GetMinHeight() : p->GetMinWidth(); }
	int CLinearLayoutUI::CtrlMainMax(CControlUI* p) const  { return m_eDirection == LAYOUT_VERTICAL ? p->GetMaxHeight() : p->GetMaxWidth(); }
	int CLinearLayoutUI::CtrlCrossMin(CControlUI* p) const { return m_eDirection == LAYOUT_VERTICAL ? p->GetMinWidth() : p->GetMinHeight(); }
	int CLinearLayoutUI::CtrlCrossMax(CControlUI* p) const { return m_eDirection == LAYOUT_VERTICAL ? p->GetMaxWidth() : p->GetMaxHeight(); }
	int CLinearLayoutUI::SelfMainMin() const { return m_eDirection == LAYOUT_VERTICAL ? GetMinHeight() : GetMinWidth(); }
	int CLinearLayoutUI::SelfMainMax() const { return m_eDirection == LAYOUT_VERTICAL ? GetMaxHeight() : GetMaxWidth(); }

	UINT CLinearLayoutUI::ResolveCrossAlign(CControlUI* pControl) const
	{
		LPCTSTR pAttr = pControl->GetCustomAttribute(_T("crossalign"));
		if( pAttr != NULL ) {
			if( _tcsicmp(pAttr, _T("center")) == 0 ) return CrossAlignCenter();
			if( _tcsicmp(pAttr, _T("end")) == 0 ) return CrossAlignEnd();
			return CrossChildAlign();
		}
		return CrossChildAlign();
	}

	LayoutDirection CLinearLayoutUI::GetDirection() const { return m_eDirection; }

	LPVOID CLinearLayoutUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, _T("LinearLayout")) == 0 ) return static_cast<CLinearLayoutUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	// --- Public API ---

	UINT CLinearLayoutUI::GetControlFlags() const
	{
		if( IsEnabled() && m_iSepSize != 0 ) return UIFLAG_SETCURSOR;
		return 0;
	}

	void CLinearLayoutUI::SetSepSize(int iSize) { m_iSepSize = iSize; }
	int CLinearLayoutUI::GetSepSize() const { return m_iSepSize; }

	void CLinearLayoutUI::SetSepImmMode(bool bImmediately)
	{
		if( m_bImmMode == bImmediately ) return;
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 && !m_bImmMode && m_pManager != NULL ) {
			m_pManager->RemovePostPaint(this);
		}
		m_bImmMode = bImmediately;
	}

	bool CLinearLayoutUI::IsSepImmMode() const { return m_bImmMode; }

	void CLinearLayoutUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("sepsize")) == 0 ) SetSepSize(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("sepimm")) == 0 ) SetSepImmMode(_tcsicmp(pstrValue, _T("true")) == 0);
		else CContainerUI::SetAttribute(pstrName, pstrValue);
	}

	SIZE CLinearLayoutUI::EstimateSize(SIZE szAvailable)
	{
		// Linear layouts do not auto-size from content.
		// Return fixed size only; 0 means "stretch to fill parent".
		return CControlUI::EstimateSize(szAvailable);
	}

	SIZE CLinearLayoutUI::MeasureContent(SIZE szAvailable)
	{
		int iChildPadding = GetChildPadding();
		int mainTotal = 0;
		int crossMax = 0;
		int nCount = 0;
		for( int i = 0; i < m_items.GetSize(); i++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[i]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) continue;
			SIZE sz = pControl->EstimateSize(szAvailable);
			RECT rcPadding = pControl->GetPadding();
			if( SzMain(sz) == 0 ) continue;
			if( SzMain(sz) < CtrlMainMin(pControl) ) SzMain(sz) = CtrlMainMin(pControl);
			if( SzMain(sz) > CtrlMainMax(pControl) ) SzMain(sz) = CtrlMainMax(pControl);
			mainTotal += SzMain(sz) + RcMainStart(rcPadding) + RcMainEnd(rcPadding);
			int crossChild = SzCross(sz) + RcCrossStart(rcPadding) + RcCrossEnd(rcPadding);
			if( crossChild > crossMax ) crossMax = crossChild;
			nCount++;
		}
		if( nCount > 1 ) mainTotal += (nCount - 1) * iChildPadding;
		SIZE szContent = {0, 0};
		SzMain(szContent) = mainTotal;
		SzCross(szContent) = crossMax;
		return szContent;
	}

	void CLinearLayoutUI::PositionChildCrossAxis(CControlUI* pControl, UINT iCrossAlign,
		const RECT& rc, const RECT& rcPadding, int iMainPos, int szMainChild, int szCrossChild,
		CScrollBarUI* pCrossScroll)
	{
		RECT rcCtrl;
		RcMainStart(rcCtrl) = iMainPos + RcMainStart(rcPadding);
		RcMainEnd(rcCtrl)   = iMainPos + szMainChild + RcMainStart(rcPadding);

		if (iCrossAlign == CrossAlignCenter()) {
			int iCrossPos = (RcCrossEnd(rc) + RcCrossStart(rc)) / 2;
			if( pCrossScroll && pCrossScroll->IsVisible() ) {
				iCrossPos += pCrossScroll->GetScrollRange() / 2;
				iCrossPos -= pCrossScroll->GetScrollPos();
			}
			RcCrossStart(rcCtrl) = iCrossPos - szCrossChild / 2;
			RcCrossEnd(rcCtrl)   = iCrossPos + szCrossChild - szCrossChild / 2;
		}
		else if (iCrossAlign == CrossAlignEnd()) {
			int iCrossPos = RcCrossEnd(rc);
			if( pCrossScroll && pCrossScroll->IsVisible() ) {
				iCrossPos += pCrossScroll->GetScrollRange();
				iCrossPos -= pCrossScroll->GetScrollPos();
			}
			RcCrossStart(rcCtrl) = iCrossPos - RcCrossEnd(rcPadding) - szCrossChild;
			RcCrossEnd(rcCtrl)   = iCrossPos - RcCrossEnd(rcPadding);
		}
		else {
			int iCrossPos = RcCrossStart(rc);
			if( pCrossScroll && pCrossScroll->IsVisible() ) {
				iCrossPos -= pCrossScroll->GetScrollPos();
			}
			RcCrossStart(rcCtrl) = iCrossPos + RcCrossStart(rcPadding);
			RcCrossEnd(rcCtrl)   = iCrossPos + RcCrossStart(rcPadding) + szCrossChild;
		}
		pControl->SetPos(rcCtrl, false);
	}

	// --- SetPos: unified layout algorithm ---

	void CLinearLayoutUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		rc = m_rcItem;

		RECT rcInset = GetInset();
		rc.left += rcInset.left;
		rc.top += rcInset.top;
		rc.right -= rcInset.right;
		rc.bottom -= rcInset.bottom;
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

		if( m_items.GetSize() == 0) {
			ProcessScrollBar(rc, 0, 0);
			return;
		}

		int iChildPadding = GetChildPadding();
		SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };

		CScrollBarUI* pCrossScroll = CrossScrollBar();
		if( pCrossScroll && pCrossScroll->IsVisible() )
			SzCross(szAvailable) += pCrossScroll->GetScrollRange();
		{
			CScrollBarUI* pMainScrollMeasure = MainScrollBar();
			if( pMainScrollMeasure && pMainScrollMeasure->IsVisible() )
				SzMain(szAvailable) += pMainScrollMeasure->GetScrollRange();
		}

		int crossNeeded = 0;
		int nAdjustables = 0;
		int mainFixed = 0;
		int nEstimateNum = 0;
		SIZE szControlAvailable;
		int iControlMaxWidth = 0;
		int iControlMaxHeight = 0;

		// --- Pass 1: measure ---
		for( int it1 = 0; it1 < m_items.GetSize(); it1++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) continue;
			szControlAvailable = szAvailable;
			RECT rcPadding = pControl->GetPadding();
			SzCross(szControlAvailable) -= RcCrossStart(rcPadding) + RcCrossEnd(rcPadding);
			iControlMaxWidth = pControl->GetFixedWidth();
			iControlMaxHeight = pControl->GetFixedHeight();
			if (iControlMaxWidth <= 0) iControlMaxWidth = pControl->GetMaxWidth();
			if (iControlMaxHeight <= 0) iControlMaxHeight = pControl->GetMaxHeight();
			if (szControlAvailable.cx > iControlMaxWidth) szControlAvailable.cx = iControlMaxWidth;
			if (szControlAvailable.cy > iControlMaxHeight) szControlAvailable.cy = iControlMaxHeight;
			SIZE sz = pControl->EstimateSize(szControlAvailable);
			if( SzMain(sz) == 0 ) {
				nAdjustables++;
			}
			else {
				if( SzMain(sz) < CtrlMainMin(pControl) ) SzMain(sz) = CtrlMainMin(pControl);
				if( SzMain(sz) > CtrlMainMax(pControl) ) SzMain(sz) = CtrlMainMax(pControl);
			}
			mainFixed += SzMain(sz) + RcMainStart(rcPadding) + RcMainEnd(rcPadding);

			SzCross(sz) = MAX(SzCross(sz), 0);
			if( SzCross(sz) < CtrlCrossMin(pControl) ) SzCross(sz) = CtrlCrossMin(pControl);
			if( SzCross(sz) > CtrlCrossMax(pControl) ) SzCross(sz) = CtrlCrossMax(pControl);
			crossNeeded = MAX(crossNeeded, SzCross(sz) + RcCrossStart(rcPadding) + RcCrossEnd(rcPadding));
			nEstimateNum++;
		}
		if( nEstimateNum > 0 ) mainFixed += (nEstimateNum - 1) * iChildPadding;

		// --- Pass 2: position ---
		int mainNeeded = 0;
		int mainExpand = 0;
		if( nAdjustables > 0 ) mainExpand = MAX(0, (SzMain(szAvailable) - mainFixed) / nAdjustables);

		SIZE szRemaining = szAvailable;
		int iMainPos = RcMainStart(rc);

		CScrollBarUI* pMainScroll = MainScrollBar();
		if( pMainScroll && pMainScroll->IsVisible() ) {
			iMainPos -= pMainScroll->GetScrollPos();
		}
		else {
			if( nAdjustables <= 0 ) {
				UINT iAlign = MainChildAlign();
				if (iAlign == MainAlignCenter()) {
					iMainPos += (SzMain(szAvailable) - mainFixed) / 2;
				}
				else if (iAlign == MainAlignEnd()) {
					iMainPos += (SzMain(szAvailable) - mainFixed);
				}
			}
		}

		int iEstimate = 0;
		int iAdjustable = 0;
		int mainFixedRemaining = mainFixed;

		for( int it2 = 0; it2 < m_items.GetSize(); it2++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) {
				SetFloatPos(it2);
				continue;
			}

			iEstimate += 1;
			RECT rcPadding = pControl->GetPadding();
			SzMain(szRemaining) -= RcMainStart(rcPadding);

			szControlAvailable = szRemaining;
			SzCross(szControlAvailable) -= RcCrossStart(rcPadding) + RcCrossEnd(rcPadding);
			iControlMaxWidth = pControl->GetFixedWidth();
			iControlMaxHeight = pControl->GetFixedHeight();
			if (iControlMaxWidth <= 0) iControlMaxWidth = pControl->GetMaxWidth();
			if (iControlMaxHeight <= 0) iControlMaxHeight = pControl->GetMaxHeight();
			if (szControlAvailable.cx > iControlMaxWidth) szControlAvailable.cx = iControlMaxWidth;
			if (szControlAvailable.cy > iControlMaxHeight) szControlAvailable.cy = iControlMaxHeight;
			mainFixedRemaining = mainFixedRemaining - (RcMainStart(rcPadding) + RcMainEnd(rcPadding));
			if (iEstimate > 1) mainFixedRemaining = mainFixedRemaining - iChildPadding;
			SIZE sz = pControl->EstimateSize(szControlAvailable);

			// Main-axis sizing
			if( SzMain(sz) == 0 ) {
				iAdjustable++;
				SzMain(sz) = mainExpand;
				if( iAdjustable == nAdjustables ) {
					SzMain(sz) = MAX(0, SzMain(szRemaining) - RcMainEnd(rcPadding) - mainFixedRemaining);
				}
				if( SzMain(sz) < CtrlMainMin(pControl) ) SzMain(sz) = CtrlMainMin(pControl);
				if( SzMain(sz) > CtrlMainMax(pControl) ) SzMain(sz) = CtrlMainMax(pControl);
			}
			else {
				if( SzMain(sz) < CtrlMainMin(pControl) ) SzMain(sz) = CtrlMainMin(pControl);
				if( SzMain(sz) > CtrlMainMax(pControl) ) SzMain(sz) = CtrlMainMax(pControl);
				mainFixedRemaining -= SzMain(sz);
			}

			SzCross(sz) = MAX(SzCross(sz), 0);
			if( SzCross(sz) == 0 ) SzCross(sz) = SzCross(szAvailable) - RcCrossStart(rcPadding) - RcCrossEnd(rcPadding);
			if( SzCross(sz) < 0 ) SzCross(sz) = 0;
			if( SzCross(sz) > SzCross(szControlAvailable) ) SzCross(sz) = SzCross(szControlAvailable);
			if( SzCross(sz) < CtrlCrossMin(pControl) ) SzCross(sz) = CtrlCrossMin(pControl);

			PositionChildCrossAxis(pControl, ResolveCrossAlign(pControl),
				rc, rcPadding, iMainPos, SzMain(sz), SzCross(sz), pCrossScroll);

			iMainPos += SzMain(sz) + iChildPadding + RcMainStart(rcPadding) + RcMainEnd(rcPadding);
			mainNeeded += SzMain(sz) + RcMainStart(rcPadding) + RcMainEnd(rcPadding);
			SzMain(szRemaining) -= SzMain(sz) + iChildPadding + RcMainEnd(rcPadding);
		}
		if( nEstimateNum > 0 ) mainNeeded += (nEstimateNum - 1) * iChildPadding;

		SIZE szNeeded = {0, 0};
		SzMain(szNeeded) = mainNeeded;
		SzCross(szNeeded) = crossNeeded;
		ProcessScrollBar(rc, szNeeded.cx, szNeeded.cy);
	}

	// --- DoPostPaint ---

	void CLinearLayoutUI::DoPostPaint(IRenderContext& ctx, const RECT& rcPaint)
	{
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 && !m_bImmMode ) {
			RECT rcSeparator = GetThumbRect(true);
			ctx.DrawColor(rcSeparator, 0xAA000000);
		}
	}

	// --- DoEvent: separator drag ---

	void CLinearLayoutUI::DoEvent(TEventUI& event)
	{
		if( m_iSepSize != 0 ) {
			if( event.Type == UIEVENT_BUTTONDOWN && IsEnabled() )
			{
				RECT rcSeparator = GetThumbRect(false);
				if( ::PtInRect(&rcSeparator, event.ptMouse) ) {
					m_uButtonState |= UISTATE_CAPTURED;
					ptLastMouse = event.ptMouse;
					m_rcNewPos = m_rcItem;
					if( !m_bImmMode && m_pManager ) m_pManager->AddPostPaint(this);
					return;
				}
			}
			if( event.Type == UIEVENT_BUTTONUP )
			{
				if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
					m_uButtonState &= ~UISTATE_CAPTURED;
					m_rcItem = m_rcNewPos;
					if( !m_bImmMode && m_pManager ) m_pManager->RemovePostPaint(this);
					NeedParentUpdate();
					return;
				}
			}
			if( event.Type == UIEVENT_MOUSEMOVE )
			{
				if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
					LONG delta = PtMain(event.ptMouse) - PtMain(ptLastMouse);
					ptLastMouse = event.ptMouse;
					RECT rc = m_rcNewPos;
					int minMain = SelfMainMin();
					int maxMain = SelfMainMax();

					if( m_iSepSize >= 0 ) {
						if( delta > 0 && PtMain(event.ptMouse) < RcMainEnd(m_rcNewPos) + m_iSepSize ) return;
						if( delta < 0 && PtMain(event.ptMouse) > RcMainEnd(m_rcNewPos) ) return;
						RcMainEnd(rc) += delta;
						int mainSize = RcMainEnd(rc) - RcMainStart(rc);
						int oldMainSize = RcMainEnd(m_rcNewPos) - RcMainStart(m_rcNewPos);
						if( mainSize <= minMain ) {
							if( oldMainSize <= minMain ) return;
							RcMainEnd(rc) = RcMainStart(rc) + minMain;
						}
						if( mainSize >= maxMain ) {
							if( oldMainSize >= maxMain ) return;
							RcMainEnd(rc) = RcMainStart(rc) + maxMain;
						}
					}
					else {
						if( delta > 0 && PtMain(event.ptMouse) < RcMainStart(m_rcNewPos) ) return;
						if( delta < 0 && PtMain(event.ptMouse) > RcMainStart(m_rcNewPos) + m_iSepSize ) return;
						RcMainStart(rc) += delta;
						int mainSize = RcMainEnd(rc) - RcMainStart(rc);
						int oldMainSize = RcMainEnd(m_rcNewPos) - RcMainStart(m_rcNewPos);
						if( mainSize <= minMain ) {
							if( oldMainSize <= minMain ) return;
							RcMainStart(rc) = RcMainEnd(rc) - minMain;
						}
						if( mainSize >= maxMain ) {
							if( oldMainSize >= maxMain ) return;
							RcMainStart(rc) = RcMainEnd(rc) - maxMain;
						}
					}

					CDuiRect rcInvalidate = GetThumbRect(true);
					m_rcNewPos = rc;

					LONG newMainSize = RcMainEnd(m_rcNewPos) - RcMainStart(m_rcNewPos);
					SzMain(m_cxyFixed) = GetManager()->GetDPIObj()->ScaleBack(newMainSize);

					if( m_bImmMode ) {
						m_rcItem = m_rcNewPos;
						NeedParentUpdate();
					}
					else {
						rcInvalidate.Join(GetThumbRect(true));
						rcInvalidate.Join(GetThumbRect(false));
						if( m_pManager ) m_pManager->Invalidate(rcInvalidate);
					}
					return;
				}
			}
			if( event.Type == UIEVENT_SETCURSOR )
			{
				RECT rcSeparator = GetThumbRect(false);
				if( IsEnabled() && ::PtInRect(&rcSeparator, event.ptMouse) ) {
					::SetCursor(::LoadCursor(NULL, m_eDirection == LAYOUT_VERTICAL ? IDC_SIZENS : IDC_SIZEWE));
					return;
				}
			}
		}
		CContainerUI::DoEvent(event);
	}

	// --- GetThumbRect ---

	RECT CLinearLayoutUI::GetThumbRect(bool bUseNew) const
	{
		const RECT& rcRef = ((m_uButtonState & UISTATE_CAPTURED) != 0 && bUseNew) ? m_rcNewPos : m_rcItem;
		RECT rcThumb = rcRef;
		if( m_iSepSize >= 0 ) {
			LONG sepStart = RcMainEnd(rcRef) - m_iSepSize;
			sepStart = MAX(sepStart, RcMainStart(rcRef));
			RcMainStart(rcThumb) = sepStart;
		}
		else {
			LONG sepEnd = RcMainStart(rcRef) - m_iSepSize;
			sepEnd = MIN(sepEnd, RcMainEnd(rcRef));
			RcMainEnd(rcThumb) = sepEnd;
		}
		return rcThumb;
	}
}