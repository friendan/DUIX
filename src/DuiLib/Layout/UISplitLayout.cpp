#include "StdAfx.h"
#include "UISplitLayout.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CSplitLayoutUI)
	IMPLEMENT_DUICONTROL(CHSplitLayoutUI)
	IMPLEMENT_DUICONTROL(CVSplitLayoutUI)

	CSplitLayoutUI::CSplitLayoutUI()
		: CSplitLayoutUI(LAYOUT_HORIZONTAL)
	{
	}

	CSplitLayoutUI::CSplitLayoutUI(LayoutDirection eDirection)
		: m_eDirection(eDirection)
		, m_iMainAxis(eDirection == LAYOUT_VERTICAL ? 1 : 0)
		, m_iSepSize(6)
		, m_uButtonState(0)
		, m_iActiveSep(-1)
		, m_iHotSep(-1)
		, m_nDragSizeA(0)
		, m_nDragSizeB(0)
		, m_ptDown()
		, m_bImmMode(true)
		, m_dwSepColor(0)
		, m_dwSepHoverColor(0)
		, m_dwSepActiveColor(0)
	{
	}

	LPCTSTR CSplitLayoutUI::GetClass() const
	{
		return _T("SplitLayoutUI");
	}

	LPVOID CSplitLayoutUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_SPLITLAYOUT) == 0 )
			return static_cast<CSplitLayoutUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	UINT CSplitLayoutUI::GetControlFlags() const
	{
		return IsEnabled() ? UIFLAG_SETCURSOR : 0;
	}

	bool CSplitLayoutUI::PreferClientHit() const
	{
		return true;
	}

	LayoutDirection CSplitLayoutUI::GetOrientation() const
	{
		return m_eDirection;
	}

	void CSplitLayoutUI::SetOrientation(LayoutDirection eDirection)
	{
		if( m_eDirection == eDirection ) return;
		m_eDirection = eDirection;
		m_iMainAxis = (eDirection == LAYOUT_VERTICAL) ? 1 : 0;
		NeedUpdate();
	}

	void CSplitLayoutUI::SetSepSize(int iSize)
	{
		if( iSize < 0 ) iSize = 0;
		if( m_iSepSize == iSize ) return;
		m_iSepSize = iSize;
		NeedUpdate();
	}

	int CSplitLayoutUI::GetSepSize() const
	{
		return m_iSepSize;
	}

	void CSplitLayoutUI::SetSepImmMode(bool bImmediately)
	{
		m_bImmMode = bImmediately;
	}

	bool CSplitLayoutUI::IsSepImmMode() const
	{
		return m_bImmMode;
	}

	void CSplitLayoutUI::SetSepColor(DWORD dwColor)
	{
		if( m_dwSepColor == dwColor ) return;
		m_dwSepColor = dwColor;
		Invalidate();
	}

	DWORD CSplitLayoutUI::GetSepColor() const
	{
		return m_dwSepColor;
	}

	void CSplitLayoutUI::SetSepHoverColor(DWORD dwColor)
	{
		if( m_dwSepHoverColor == dwColor ) return;
		m_dwSepHoverColor = dwColor;
		Invalidate();
	}

	DWORD CSplitLayoutUI::GetSepHoverColor() const
	{
		return m_dwSepHoverColor;
	}

	void CSplitLayoutUI::SetSepActiveColor(DWORD dwColor)
	{
		if( m_dwSepActiveColor == dwColor ) return;
		m_dwSepActiveColor = dwColor;
		Invalidate();
	}

	DWORD CSplitLayoutUI::GetSepActiveColor() const
	{
		return m_dwSepActiveColor;
	}

	SIZE CSplitLayoutUI::EstimateSize(SIZE szAvailable)
	{
		return CControlUI::EstimateSize(szAvailable);
	}

	void CSplitLayoutUI::CollectPanes(CStdPtrArray& aPanes) const
	{
		aPanes.Empty();
		for( int i = 0; i < m_items.GetSize(); ++i ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[i]);
			if( pControl == NULL ) continue;
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsAbsolute() ) continue;
			aPanes.Add(pControl);
		}
	}

	int CSplitLayoutUI::GetSepSizePx() const
	{
		int n = m_iSepSize;
		if( n < 0 ) n = 0;
		if( m_pManager != NULL ) n = m_pManager->GetDPIObj()->Scale(n);
		return n;
	}

	int CSplitLayoutUI::CtrlMainMin(CControlUI* p) const
	{
		if( p == NULL ) return 0;
		return m_eDirection == LAYOUT_VERTICAL ? p->GetMinHeight() : p->GetMinWidth();
	}

	int CSplitLayoutUI::CtrlMainMax(CControlUI* p) const
	{
		if( p == NULL ) return 9999;
		return m_eDirection == LAYOUT_VERTICAL ? p->GetMaxHeight() : p->GetMaxWidth();
	}

	int CSplitLayoutUI::MainSize(const RECT& rc) const
	{
		return RcMainEnd(rc) - RcMainStart(rc);
	}

	void CSplitLayoutUI::FreezeMain(CControlUI* p, int nPixel)
	{
		if( p == NULL || nPixel < 0 ) return;
		int nLogic = nPixel;
		if( m_pManager != NULL ) nLogic = m_pManager->GetDPIObj()->ScaleBack(nPixel);
		if( nLogic < 0 ) nLogic = 0;
		if( m_eDirection == LAYOUT_VERTICAL )
			p->SetFixedHeight(nLogic);
		else
			p->SetFixedWidth(nLogic);
	}

	void CSplitLayoutUI::ApplySepSizes(int iSep, int nPixelA, int nPixelB)
	{
		CStdPtrArray aPanes;
		CollectPanes(aPanes);
		if( iSep < 0 || iSep + 1 >= aPanes.GetSize() ) return;
		CControlUI* pA = static_cast<CControlUI*>(aPanes[iSep]);
		CControlUI* pB = static_cast<CControlUI*>(aPanes[iSep + 1]);
		FreezeMain(pA, nPixelA);
		FreezeMain(pB, nPixelB);
	}

	void CSplitLayoutUI::UpdateHotSep(POINT pt)
	{
		int iHot = IsEnabled() ? HitSep(pt) : -1;
		if( iHot == m_iHotSep ) return;
		m_iHotSep = iHot;
		Invalidate();
	}

	DWORD CSplitLayoutUI::ResolveThemeColor(LPCTSTR pstrToken, DWORD dwFallback) const
	{
		CThemeManager* tm = CThemeManager::GetInstance();
		if( tm == NULL ) return dwFallback;
		return tm->GetColor(pstrToken, dwFallback);
	}

	DWORD CSplitLayoutUI::GetSepPaintColor(int iSep) const
	{
		const bool bActive = ((m_uButtonState & UISTATE_CAPTURED) != 0) && (iSep == m_iActiveSep);
		const bool bHot = !bActive && (iSep == m_iHotSep);
		if( bActive ) {
			if( m_dwSepActiveColor != 0 ) return m_dwSepActiveColor;
			if( m_dwSepHoverColor != 0 ) return m_dwSepHoverColor;
			return ResolveThemeColor(_T("color-primary-active"),
				ResolveThemeColor(_T("color-primary"), 0x0A58CAFF));
		}
		if( bHot ) {
			if( m_dwSepHoverColor != 0 ) return m_dwSepHoverColor;
			return ResolveThemeColor(_T("color-primary"), 0x0D6EFDFF);
		}
		if( m_dwSepColor != 0 ) return m_dwSepColor;
		return ResolveThemeColor(_T("color-border"), 0xDEE2E6FF);
	}

	int CSplitLayoutUI::GetSepLinePx(bool bEmphasis) const
	{
		int n = bEmphasis ? 3 : 1;
		if( m_pManager != NULL ) n = m_pManager->GetDPIObj()->Scale(n);
		int nSep = GetSepSizePx();
		if( nSep > 0 && n > nSep ) n = nSep;
		if( n < 1 ) n = 1;
		return n;
	}

	bool CSplitLayoutUI::ParseSepColorAttr(LPCTSTR pstrName, LPCTSTR pstrValue, DWORD& dwColor)
	{
		dwColor = 0;
		if( pstrValue == NULL || pstrValue[0] == _T('\0') ) return true;
		if( pstrName != NULL && _tcsnicmp(pstrValue, _T("var("), 4) == 0 ) {
			CDuiString key;
			key.Format(_T("_tvar:%s"), pstrName);
			AddCustomAttribute(key.GetData(), pstrValue);
		}
		return ParseColorString(pstrValue, dwColor);
	}

	RECT CSplitLayoutUI::GetSepRect(int iSep) const
	{
		RECT rcEmpty = { 0, 0, 0, 0 };
		CStdPtrArray aPanes;
		CollectPanes(aPanes);
		if( iSep < 0 || iSep + 1 >= aPanes.GetSize() ) return rcEmpty;

		CControlUI* pA = static_cast<CControlUI*>(aPanes[iSep]);
		CControlUI* pB = static_cast<CControlUI*>(aPanes[iSep + 1]);
		if( pA == NULL || pB == NULL ) return rcEmpty;

		RECT rcA = pA->GetPos();
		RECT rcB = pB->GetPos();
		RECT rcPad = GetPadding();
		RECT rc = m_rcItem;
		rc.left += rcPad.left;
		rc.top += rcPad.top;
		rc.right -= rcPad.right;
		rc.bottom -= rcPad.bottom;

		if( m_eDirection == LAYOUT_VERTICAL ) {
			rc.top = rcA.bottom;
			rc.bottom = rcB.top;
		}
		else {
			rc.left = rcA.right;
			rc.right = rcB.left;
		}

		int nSep = GetSepSizePx();
		if( nSep < 1 ) nSep = 1;
		if( RcMainEnd(rc) <= RcMainStart(rc) ) {
			LONG mid = (RcMainEnd(rcA) + RcMainStart(rcB)) / 2;
			RcMainStart(rc) = mid - nSep / 2;
			RcMainEnd(rc) = RcMainStart(rc) + nSep;
		}
		return rc;
	}

	int CSplitLayoutUI::HitSep(POINT pt) const
	{
		CStdPtrArray aPanes;
		CollectPanes(aPanes);
		const int nSepCount = aPanes.GetSize() - 1;
		for( int i = 0; i < nSepCount; ++i ) {
			RECT rcSep = GetSepRect(i);
			if( ::PtInRect(&rcSep, pt) ) return i;
		}
		return -1;
	}

	void CSplitLayoutUI::SetPos(RECT rc, bool bNeedInvalidate)
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

		for( int itAbs = 0; itAbs < m_items.GetSize(); ++itAbs ) {
			CControlUI* pAbs = static_cast<CControlUI*>(m_items[itAbs]);
			if( pAbs != NULL && pAbs->IsAbsolute() )
				SetAbsolutePos(itAbs);
		}

		CStdPtrArray aPanes;
		CollectPanes(aPanes);
		const int nPane = aPanes.GetSize();
		if( nPane <= 0 ) return;

		const int nSepPx = GetSepSizePx();
		SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
		const int nPaneBudget = SzMain(szAvailable) - nSepPx * (nPane - 1);

		int nAdjustables = 0;
		int nFixed = 0;
		SIZE* psz = nPane > 0 ? new SIZE[nPane] : NULL;
		if( psz == NULL ) return;
		for( int i = 0; i < nPane; ++i ) {
			psz[i].cx = 0;
			psz[i].cy = 0;
			CControlUI* pControl = static_cast<CControlUI*>(aPanes[i]);
			RECT rcMargin = pControl->GetMargin();
			SIZE szAvailChild = szAvailable;
			SzMain(szAvailChild) = MAX(0, nPaneBudget - RcMainStart(rcMargin) - RcMainEnd(rcMargin));
			SzCross(szAvailChild) -= RcCrossStart(rcMargin) + RcCrossEnd(rcMargin);
			if( SzCross(szAvailChild) < 0 ) SzCross(szAvailChild) = 0;
			SIZE sz = pControl->EstimateSize(szAvailChild);
			SzMain(sz) = MAX(0, SzMain(sz));
			if( SzMain(sz) == 0 ) {
				nAdjustables++;
			}
			else {
				if( SzMain(sz) < CtrlMainMin(pControl) ) SzMain(sz) = CtrlMainMin(pControl);
				if( SzMain(sz) > CtrlMainMax(pControl) ) SzMain(sz) = CtrlMainMax(pControl);
				nFixed += SzMain(sz);
			}
			nFixed += RcMainStart(rcMargin) + RcMainEnd(rcMargin);
			psz[i] = sz;
		}

		int nExpand = 0;
		int nRemain = nPaneBudget - nFixed;
		if( nAdjustables > 0 ) nExpand = MAX(0, nRemain / nAdjustables);

		int iMainPos = RcMainStart(rc);
		int iAdjustable = 0;
		int nRemainExpand = nRemain;
		for( int i = 0; i < nPane; ++i ) {
			CControlUI* pControl = static_cast<CControlUI*>(aPanes[i]);
			RECT rcMargin = pControl->GetMargin();
			SIZE sz = psz[i];

			if( SzMain(sz) == 0 ) {
				iAdjustable++;
				if( iAdjustable == nAdjustables )
					SzMain(sz) = MAX(0, nRemainExpand);
				else
					SzMain(sz) = nExpand;
				nRemainExpand -= SzMain(sz);
				if( SzMain(sz) < CtrlMainMin(pControl) ) SzMain(sz) = CtrlMainMin(pControl);
				if( SzMain(sz) > CtrlMainMax(pControl) ) SzMain(sz) = CtrlMainMax(pControl);
			}

			if( nAdjustables <= 0 && i == nPane - 1 ) {
				int nUsed = iMainPos - RcMainStart(rc);
				int nLeft = SzMain(szAvailable) - nUsed - RcMainStart(rcMargin) - RcMainEnd(rcMargin);
				if( nLeft > SzMain(sz) ) SzMain(sz) = nLeft;
				if( SzMain(sz) > CtrlMainMax(pControl) ) SzMain(sz) = CtrlMainMax(pControl);
			}

			SzCross(sz) = SzCross(szAvailable) - RcCrossStart(rcMargin) - RcCrossEnd(rcMargin);
			if( SzCross(sz) < 0 ) SzCross(sz) = 0;

			RECT rcCtrl = { 0 };
			RcMainStart(rcCtrl) = iMainPos + RcMainStart(rcMargin);
			RcMainEnd(rcCtrl) = RcMainStart(rcCtrl) + SzMain(sz);
			RcCrossStart(rcCtrl) = RcCrossStart(rc) + RcCrossStart(rcMargin);
			RcCrossEnd(rcCtrl) = RcCrossStart(rcCtrl) + SzCross(sz);
			pControl->SetPos(rcCtrl, false);

			iMainPos += SzMain(sz) + RcMainStart(rcMargin) + RcMainEnd(rcMargin);
			if( i < nPane - 1 ) iMainPos += nSepPx;
		}
		delete[] psz;
	}

	void CSplitLayoutUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( !BubbleEvent(event) ) CContainerUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETCURSOR && IsEnabled() ) {
			int iSep = HitSep(event.ptMouse);
			if( iSep >= 0 || (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
				::SetCursor(::LoadCursor(NULL,
					m_eDirection == LAYOUT_VERTICAL ? IDC_SIZENS : IDC_SIZEWE));
				return;
			}
		}

		if( event.Type == UIEVENT_BUTTONDOWN && IsEnabled() ) {
			int iSep = HitSep(event.ptMouse);
			if( iSep >= 0 ) {
				CStdPtrArray aPanes;
				CollectPanes(aPanes);
				if( iSep + 1 < aPanes.GetSize() ) {
					CControlUI* pA = static_cast<CControlUI*>(aPanes[iSep]);
					CControlUI* pB = static_cast<CControlUI*>(aPanes[iSep + 1]);
					m_iActiveSep = iSep;
					m_nDragSizeA = MainSize(pA->GetPos());
					m_nDragSizeB = MainSize(pB->GetPos());
					m_ptDown = event.ptMouse;
					m_uButtonState |= UISTATE_CAPTURED;
					::SetCursor(::LoadCursor(NULL,
						m_eDirection == LAYOUT_VERTICAL ? IDC_SIZENS : IDC_SIZEWE));
					if( m_bImmMode )
						ApplySepSizes(iSep, m_nDragSizeA, m_nDragSizeB);
					m_iHotSep = iSep;
					Invalidate();
				}
				return;
			}
		}

		if( event.Type == UIEVENT_BUTTONUP ) {
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
				int delta = PtMain(event.ptMouse) - PtMain(m_ptDown);
				int nA = m_nDragSizeA;
				int nB = m_nDragSizeB;
				CStdPtrArray aPanes;
				CollectPanes(aPanes);
				if( m_iActiveSep >= 0 && m_iActiveSep + 1 < aPanes.GetSize() ) {
					CControlUI* pA = static_cast<CControlUI*>(aPanes[m_iActiveSep]);
					CControlUI* pB = static_cast<CControlUI*>(aPanes[m_iActiveSep + 1]);
					int lo = MAX(CtrlMainMin(pA) - m_nDragSizeA, m_nDragSizeB - CtrlMainMax(pB));
					int hi = MIN(CtrlMainMax(pA) - m_nDragSizeA, m_nDragSizeB - CtrlMainMin(pB));
					if( delta < lo ) delta = lo;
					if( delta > hi ) delta = hi;
					nA = m_nDragSizeA + delta;
					nB = m_nDragSizeB - delta;
					ApplySepSizes(m_iActiveSep, nA, nB);
					if( m_pManager != NULL )
						m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED, (WPARAM)m_iActiveSep);
				}
				m_uButtonState &= ~UISTATE_CAPTURED;
				m_iActiveSep = -1;
				UpdateHotSep(event.ptMouse);
				return;
			}
		}

		if( event.Type == UIEVENT_MOUSEMOVE ) {
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 && m_iActiveSep >= 0 ) {
				int delta = PtMain(event.ptMouse) - PtMain(m_ptDown);
				CStdPtrArray aPanes;
				CollectPanes(aPanes);
				if( m_iActiveSep + 1 < aPanes.GetSize() ) {
					CControlUI* pA = static_cast<CControlUI*>(aPanes[m_iActiveSep]);
					CControlUI* pB = static_cast<CControlUI*>(aPanes[m_iActiveSep + 1]);
					int lo = MAX(CtrlMainMin(pA) - m_nDragSizeA, m_nDragSizeB - CtrlMainMax(pB));
					int hi = MIN(CtrlMainMax(pA) - m_nDragSizeA, m_nDragSizeB - CtrlMainMin(pB));
					if( delta < lo ) delta = lo;
					if( delta > hi ) delta = hi;
					int nA = m_nDragSizeA + delta;
					int nB = m_nDragSizeB - delta;
					if( m_bImmMode )
						ApplySepSizes(m_iActiveSep, nA, nB);
					else
						Invalidate();
					::SetCursor(::LoadCursor(NULL,
						m_eDirection == LAYOUT_VERTICAL ? IDC_SIZENS : IDC_SIZEWE));
				}
				return;
			}
			UpdateHotSep(event.ptMouse);
		}

		if( event.Type == UIEVENT_MOUSEENTER ) {
			UpdateHotSep(event.ptMouse);
		}
		if( event.Type == UIEVENT_MOUSELEAVE ) {
			if( m_iHotSep >= 0 && (m_uButtonState & UISTATE_CAPTURED) == 0 ) {
				m_iHotSep = -1;
				Invalidate();
			}
		}

		CContainerUI::DoEvent(event);
	}

	bool CSplitLayoutUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		if( !CContainerUI::DoPaint(ctx, rcPaint, pStopControl) ) return false;
		PaintSeparators(ctx);
		return true;
	}

	void CSplitLayoutUI::PaintSeparators(IRenderContext& ctx)
	{
		CStdPtrArray aPanes;
		CollectPanes(aPanes);
		const int nSepCount = aPanes.GetSize() - 1;
		if( nSepCount <= 0 ) return;

		for( int i = 0; i < nSepCount; ++i ) {
			const bool bEmphasis = ((m_uButtonState & UISTATE_CAPTURED) != 0 && i == m_iActiveSep)
				|| (i == m_iHotSep);
			DWORD dwColor = GetAdjustColor(GetSepPaintColor(i));
			RECT rcSep = GetSepRect(i);
			RECT rcLine = rcSep;
			const int nLine = GetSepLinePx(bEmphasis);
			if( m_eDirection == LAYOUT_VERTICAL ) {
				int mid = (rcSep.top + rcSep.bottom - nLine) / 2;
				rcLine.top = mid;
				rcLine.bottom = mid + nLine;
			}
			else {
				int mid = (rcSep.left + rcSep.right - nLine) / 2;
				rcLine.left = mid;
				rcLine.right = mid + nLine;
			}
			if( rcLine.right <= rcLine.left ) rcLine.right = rcLine.left + 1;
			if( rcLine.bottom <= rcLine.top ) rcLine.bottom = rcLine.top + 1;
			ctx.DrawColor(rcLine, dwColor);
		}
	}

	CControlUI* CSplitLayoutUI::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
	{
		if( (uFlags & UIFIND_HITTEST) != 0 && IsEnabled() && pData != NULL ) {
			POINT pt = *static_cast<LPPOINT>(pData);
			if( HitSep(pt) >= 0 ) {
				if( (uFlags & UIFIND_VISIBLE) != 0 && !IsVisible() ) return NULL;
				if( (uFlags & UIFIND_ENABLED) != 0 && !IsEnabled() ) return NULL;
				if( !::PtInRect(&m_rcItem, pt) ) return NULL;
				return Proc(this, pData);
			}
		}
		return CContainerUI::FindControl(Proc, pData, uFlags);
	}

	void CSplitLayoutUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("orientation")) == 0
			|| _tcsicmp(pstrName, _T("direction")) == 0 ) {
			if( pstrValue == NULL ) return;
			if( _tcsicmp(pstrValue, _T("vertical")) == 0
				|| _tcsicmp(pstrValue, _T("v")) == 0
				|| _tcsicmp(pstrValue, _T("vert")) == 0
				|| _tcsicmp(pstrValue, _T("column")) == 0
				|| _tcscmp(pstrValue, _T("纵")) == 0
				|| _tcscmp(pstrValue, _T("竖直")) == 0
				|| _tcscmp(pstrValue, _T("垂直")) == 0 ) {
				SetOrientation(LAYOUT_VERTICAL);
			}
			else {
				SetOrientation(LAYOUT_HORIZONTAL);
			}
		}
		else if( _tcsicmp(pstrName, _T("sep-size")) == 0
			|| _tcsicmp(pstrName, _T("sep-width")) == 0
			|| _tcsicmp(pstrName, _T("sep-height")) == 0 ) {
			SetSepSize(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("sep-imm")) == 0 ) {
			SetSepImmMode(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("sep-color")) == 0 ) {
			DWORD clr = 0;
			if( ParseSepColorAttr(pstrName, pstrValue, clr) )
				SetSepColor(clr);
		}
		else if( _tcsicmp(pstrName, _T("sep-color-hover")) == 0 ) {
			DWORD clr = 0;
			if( ParseSepColorAttr(pstrName, pstrValue, clr) )
				SetSepHoverColor(clr);
		}
		else if( _tcsicmp(pstrName, _T("sep-color-active")) == 0 ) {
			DWORD clr = 0;
			if( ParseSepColorAttr(pstrName, pstrValue, clr) )
				SetSepActiveColor(clr);
		}
		else CContainerUI::SetAttribute(pstrName, pstrValue);
	}

	CHSplitLayoutUI::CHSplitLayoutUI()
		: CSplitLayoutUI(LAYOUT_HORIZONTAL)
	{
	}

	LPCTSTR CHSplitLayoutUI::GetClass() const
	{
		return _T("HSplitLayoutUI");
	}

	LPVOID CHSplitLayoutUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_HSPLIT) == 0 )
			return static_cast<CHSplitLayoutUI*>(this);
		return CSplitLayoutUI::GetInterface(pstrName);
	}

	CVSplitLayoutUI::CVSplitLayoutUI()
		: CSplitLayoutUI(LAYOUT_VERTICAL)
	{
	}

	LPCTSTR CVSplitLayoutUI::GetClass() const
	{
		return _T("VSplitLayoutUI");
	}

	LPVOID CVSplitLayoutUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_VSPLIT) == 0 )
			return static_cast<CVSplitLayoutUI*>(this);
		return CSplitLayoutUI::GetInterface(pstrName);
	}

} // namespace DuiLib
