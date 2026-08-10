#include "StdAfx.h"
#include "UISteps.h"

namespace DuiLib
{
	//////////////////////////////////////////////////////////////////////////
	IMPLEMENT_DUICONTROL(CStepItemUI)

	CStepItemUI::CStepItemUI()
		: m_eStatus(StatusAuto)
	{
		SetMouseEnabled(false);
	}

	LPCTSTR CStepItemUI::GetClass() const { return _T("StepItemUI"); }

	LPVOID CStepItemUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_STEPITEM) == 0 ) return static_cast<CStepItemUI*>(this);
		if( _tcsicmp(pstrName, _T("Step")) == 0 ) return static_cast<CStepItemUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	void CStepItemUI::SetDescription(LPCTSTR pstr)
	{
		m_sDescription = pstr ? pstr : _T("");
	}

	LPCTSTR CStepItemUI::GetDescription() const { return m_sDescription.GetData(); }

	void CStepItemUI::SetStatus(Status e) { m_eStatus = e; }
	CStepItemUI::Status CStepItemUI::GetStatus() const { return m_eStatus; }

	void CStepItemUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("title")) == 0 || _tcsicmp(pstrName, _T("text")) == 0 ) {
			SetText(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("description")) == 0 || _tcsicmp(pstrName, _T("desc")) == 0 ) {
			SetDescription(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("status")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("wait")) == 0 ) SetStatus(StatusWait);
			else if( _tcsicmp(pstrValue, _T("process")) == 0 || _tcsicmp(pstrValue, _T("active")) == 0 ) SetStatus(StatusProcess);
			else if( _tcsicmp(pstrValue, _T("finish")) == 0 || _tcsicmp(pstrValue, _T("done")) == 0 ) SetStatus(StatusFinish);
			else if( _tcsicmp(pstrValue, _T("error")) == 0 ) SetStatus(StatusError);
			else SetStatus(StatusAuto);
		}
		else {
			CControlUI::SetAttribute(pstrName, pstrValue);
		}
	}

	bool CStepItemUI::DoPaint(IRenderContext& /*ctx*/, const RECT& /*rcPaint*/, CControlUI* /*pStopControl*/)
	{
		return true;
	}

	SIZE CStepItemUI::EstimateSize(SIZE /*szAvailable*/)
	{
		SIZE sz = { 0, 0 };
		return sz;
	}

	//////////////////////////////////////////////////////////////////////////
	IMPLEMENT_DUICONTROL(CStepsUI)

	CStepsUI::CStepsUI()
		: m_nCurrent(0)
		, m_eDirection(Horizontal)
		, m_bClickable(false)
		, m_bItemsApplied(false)
		, m_nDotSize(24)
		, m_nHoverIndex(-1)
		, m_nHitCount(0)
		, m_dwFinishColor(0x1677FFFF)
		, m_dwProcessColor(0x1677FFFF)
		, m_dwWaitColor(0x00000040)
		, m_dwErrorColor(0xFF4D4FFF)
		, m_dwTitleColor(0x333333FF)
		, m_dwDescColor(0x8C8C8CFF)
		, m_dwWaitTitleColor(0x00000073)
	{
		::ZeroMemory(m_rcHits, sizeof(m_rcHits));
		SetKind(CONTROLKIND_NONE);
	}

	LPCTSTR CStepsUI::GetClass() const { return _T("StepsUI"); }

	LPVOID CStepsUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_STEPS) == 0 ) return static_cast<CStepsUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	UINT CStepsUI::GetControlFlags() const
	{
		return m_bClickable ? UIFLAG_SETCURSOR : 0;
	}

	bool CStepsUI::PreferClientHit() const
	{
		return m_bClickable;
	}

	void CStepsUI::SetCurrent(int nIndex)
	{
		if( nIndex < 0 ) nIndex = 0;
		if( m_nCurrent == nIndex ) return;
		m_nCurrent = nIndex;
		Invalidate();
	}

	int CStepsUI::GetCurrent() const { return m_nCurrent; }

	void CStepsUI::SetDirection(Direction e)
	{
		if( m_eDirection == e ) return;
		m_eDirection = e;
		NeedUpdate();
		Invalidate();
	}

	CStepsUI::Direction CStepsUI::GetDirection() const { return m_eDirection; }

	void CStepsUI::SetClickable(bool b)
	{
		m_bClickable = b;
		if( b ) SetCursor(DUI_HAND);
	}

	bool CStepsUI::IsClickable() const { return m_bClickable; }

	void CStepsUI::SetItems(LPCTSTR pstrItems)
	{
		m_sItemsAttr = pstrItems ? pstrItems : _T("");
		m_bItemsApplied = false;
		EnsureFromItemsAttr();
		NeedUpdate();
		Invalidate();
	}

	int CStepsUI::ScaleValue(int v) const
	{
		return m_pManager ? m_pManager->GetDPIObj()->Scale(v) : v;
	}

	void CStepsUI::EnsureFromItemsAttr()
	{
		if( m_bItemsApplied ) return;
		m_bItemsApplied = true;
		if( m_sItemsAttr.IsEmpty() ) return;
		if( GetCount() > 0 ) return;

		CDuiString s = m_sItemsAttr;
		int start = 0;
		while( start <= s.GetLength() ) {
			int bar = s.Find(_T('|'), start);
			int comma = s.Find(_T(','), start);
			int sep = -1;
			if( bar >= 0 && comma >= 0 ) sep = (bar < comma) ? bar : comma;
			else if( bar >= 0 ) sep = bar;
			else sep = comma;

			CDuiString part;
			if( sep < 0 ) {
				part = s.Mid(start);
				start = s.GetLength() + 1;
			}
			else {
				part = s.Mid(start, sep - start);
				start = sep + 1;
			}
			part.TrimLeft();
			part.TrimRight();
			if( part.IsEmpty() ) continue;
			CStepItemUI* p = new CStepItemUI;
			p->SetText(part.GetData());
			Add(p);
		}
	}

	CStepItemUI::Status CStepsUI::ResolveStatus(int index, CStepItemUI* pItem) const
	{
		if( pItem && pItem->GetStatus() != CStepItemUI::StatusAuto )
			return pItem->GetStatus();
		if( index < m_nCurrent ) return CStepItemUI::StatusFinish;
		if( index == m_nCurrent ) return CStepItemUI::StatusProcess;
		return CStepItemUI::StatusWait;
	}

	void CStepsUI::LayoutHitRects()
	{
		EnsureFromItemsAttr();
		m_nHitCount = 0;
		int n = GetCount();
		if( n <= 0 ) return;
		if( n > 64 ) n = 64;
		m_nHitCount = n;

			RECT rc = m_rcItem;
			CDuiBox rcPad = GetPadding();
			rc.left += rcPad.left;
			rc.top += rcPad.top;
			rc.right -= rcPad.right;
			rc.bottom -= rcPad.bottom;

		if( m_eDirection == Horizontal ) {
			int w = (rc.right - rc.left) / n;
			if( w < 1 ) w = 1;
			for( int i = 0; i < n; ++i ) {
				m_rcHits[i].left = rc.left + i * w;
				m_rcHits[i].right = (i == n - 1) ? rc.right : (rc.left + (i + 1) * w);
				m_rcHits[i].top = rc.top;
				m_rcHits[i].bottom = rc.bottom;
			}
		}
		else {
			int h = (rc.bottom - rc.top) / n;
			if( h < ScaleValue(48) ) h = ScaleValue(48);
			for( int i = 0; i < n; ++i ) {
				m_rcHits[i].left = rc.left;
				m_rcHits[i].right = rc.right;
				m_rcHits[i].top = rc.top + i * h;
				m_rcHits[i].bottom = m_rcHits[i].top + h;
				if( m_rcHits[i].bottom > rc.bottom && i == n - 1 )
					m_rcHits[i].bottom = rc.bottom;
			}
		}
	}

	int CStepsUI::HitTest(POINT pt) const
	{
		for( int i = 0; i < m_nHitCount; ++i ) {
			if( ::PtInRect(&m_rcHits[i], pt) ) return i;
		}
		return -1;
	}

	void CStepsUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		EnsureFromItemsAttr();
		RECT rcNull = { 0, 0, 0, 0 };
		for( int i = 0; i < GetCount(); ++i ) {
			CControlUI* p = GetItemAt(i);
			if( p ) p->SetPos(rcNull, false);
		}
		LayoutHitRects();
	}

	SIZE CStepsUI::EstimateSize(SIZE /*szAvailable*/)
	{
		EnsureFromItemsAttr();
		SIZE sz = GetFixedSize();
		if( m_eDirection == Horizontal ) {
			if( sz.cy <= 0 ) sz.cy = ScaleValue(64);
			if( sz.cx <= 0 ) sz.cx = ScaleValue(280);
		}
		else {
			int n = GetCount();
			if( n < 1 ) n = 1;
			if( sz.cy <= 0 ) sz.cy = ScaleValue(56) * n;
			if( sz.cx <= 0 ) sz.cx = ScaleValue(200);
		}
		return sz;
	}

	bool CStepsUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		EnsureFromItemsAttr();
		LayoutHitRects();
		CControlUI::DoPaint(ctx, rcPaint, pStopControl);
		if( m_eDirection == Horizontal ) PaintHorizontal(ctx);
		else PaintVertical(ctx);
		return true;
	}

	void CStepsUI::PaintHorizontal(IRenderContext& ctx)
	{
		int n = m_nHitCount;
		if( n <= 0 ) return;
		int dot = ScaleValue(m_nDotSize);
		int lineY = 0;

		for( int i = 0; i < n; ++i ) {
			CControlUI* pCtrl = GetItemAt(i);
			CStepItemUI* pItem = pCtrl
				? static_cast<CStepItemUI*>(pCtrl->GetInterface(DUI_CTR_STEPITEM))
				: NULL;
			CStepItemUI::Status st = ResolveStatus(i, pItem);
			DWORD clrDot = m_dwWaitColor;
			DWORD clrTitle = m_dwWaitTitleColor;
			if( st == CStepItemUI::StatusFinish ) { clrDot = m_dwFinishColor; clrTitle = m_dwTitleColor; }
			else if( st == CStepItemUI::StatusProcess ) { clrDot = m_dwProcessColor; clrTitle = m_dwProcessColor; }
			else if( st == CStepItemUI::StatusError ) { clrDot = m_dwErrorColor; clrTitle = m_dwErrorColor; }

			RECT rcHit = m_rcHits[i];
			int cx = (rcHit.left + rcHit.right) / 2;
			RECT rcDot = { cx - dot / 2, rcHit.top + ScaleValue(4), cx + dot / 2, rcHit.top + ScaleValue(4) + dot };
			lineY = (rcDot.top + rcDot.bottom) / 2;

			if( i + 1 < n ) {
				int nx = (m_rcHits[i + 1].left + m_rcHits[i + 1].right) / 2;
				DWORD clrLine = (i < m_nCurrent) ? m_dwFinishColor : m_dwWaitColor;
				RECT rcLine = { rcDot.right + ScaleValue(4), lineY - ScaleValue(1), nx - dot / 2 - ScaleValue(4), lineY + ScaleValue(1) };
				if( rcLine.right > rcLine.left )
					ctx.DrawColor(rcLine, GetAdjustColor(clrLine));
			}

			SIZE szR = { dot / 2, dot / 2 };
			ctx.FillRoundRect(rcDot, szR.cx, szR.cy, GetAdjustColor(clrDot));

			CDuiString sMark;
			if( st == CStepItemUI::StatusFinish ) sMark = _T("✓");
			else if( st == CStepItemUI::StatusError ) sMark = _T("!");
			else sMark.SmallFormat(_T("%d"), i + 1);
			ctx.DrawText(rcDot, sMark.GetData(), GetAdjustColor(0xFFFFFFFF), -1, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

			CDuiString sTitle = pItem ? pItem->GetText() : CDuiString(_T(""));
			LPCTSTR pTitle = sTitle.GetData();
			RECT rcTitle = { rcHit.left + ScaleValue(2), rcDot.bottom + ScaleValue(4), rcHit.right - ScaleValue(2), rcDot.bottom + ScaleValue(22) };
			ctx.DrawText(rcTitle, pTitle, GetAdjustColor(clrTitle), -1, DT_SINGLELINE | DT_CENTER | DT_END_ELLIPSIS);

			LPCTSTR pDesc = pItem ? pItem->GetDescription() : _T("");
			if( pDesc && *pDesc ) {
				RECT rcDesc = { rcTitle.left, rcTitle.bottom, rcTitle.right, rcHit.bottom };
				ctx.DrawText(rcDesc, pDesc, GetAdjustColor(m_dwDescColor), -1, DT_SINGLELINE | DT_CENTER | DT_END_ELLIPSIS);
			}
		}
	}

	void CStepsUI::PaintVertical(IRenderContext& ctx)
	{
		int n = m_nHitCount;
		if( n <= 0 ) return;
		int dot = ScaleValue(m_nDotSize);
		int leftPad = ScaleValue(4);

		for( int i = 0; i < n; ++i ) {
			CControlUI* pCtrl = GetItemAt(i);
			CStepItemUI* pItem = pCtrl
				? static_cast<CStepItemUI*>(pCtrl->GetInterface(DUI_CTR_STEPITEM))
				: NULL;
			CStepItemUI::Status st = ResolveStatus(i, pItem);
			DWORD clrDot = m_dwWaitColor;
			DWORD clrTitle = m_dwWaitTitleColor;
			if( st == CStepItemUI::StatusFinish ) { clrDot = m_dwFinishColor; clrTitle = m_dwTitleColor; }
			else if( st == CStepItemUI::StatusProcess ) { clrDot = m_dwProcessColor; clrTitle = m_dwProcessColor; }
			else if( st == CStepItemUI::StatusError ) { clrDot = m_dwErrorColor; clrTitle = m_dwErrorColor; }

			RECT rcHit = m_rcHits[i];
			RECT rcDot = { rcHit.left + leftPad, rcHit.top + ScaleValue(4), rcHit.left + leftPad + dot, rcHit.top + ScaleValue(4) + dot };

			if( i + 1 < n ) {
				DWORD clrLine = (i < m_nCurrent) ? m_dwFinishColor : m_dwWaitColor;
				int lx = (rcDot.left + rcDot.right) / 2;
				RECT rcLine = { lx - ScaleValue(1), rcDot.bottom + ScaleValue(2), lx + ScaleValue(1), m_rcHits[i + 1].top + ScaleValue(4) };
				if( rcLine.bottom > rcLine.top )
					ctx.DrawColor(rcLine, GetAdjustColor(clrLine));
			}

			SIZE szR = { dot / 2, dot / 2 };
			ctx.FillRoundRect(rcDot, szR.cx, szR.cy, GetAdjustColor(clrDot));
			CDuiString sMark;
			if( st == CStepItemUI::StatusFinish ) sMark = _T("✓");
			else if( st == CStepItemUI::StatusError ) sMark = _T("!");
			else sMark.SmallFormat(_T("%d"), i + 1);
			ctx.DrawText(rcDot, sMark.GetData(), GetAdjustColor(0xFFFFFFFF), -1, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

			RECT rcText = { rcDot.right + ScaleValue(10), rcHit.top, rcHit.right, rcHit.bottom };
			CDuiString sTitle = pItem ? pItem->GetText() : CDuiString(_T(""));
			LPCTSTR pTitle = sTitle.GetData();
			RECT rcTitle = rcText;
			rcTitle.bottom = rcTitle.top + ScaleValue(22);
			ctx.DrawText(rcTitle, pTitle, GetAdjustColor(clrTitle), -1, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);
			LPCTSTR pDesc = pItem ? pItem->GetDescription() : _T("");
			if( pDesc && *pDesc ) {
				RECT rcDesc = { rcText.left, rcTitle.bottom, rcText.right, rcText.bottom };
				ctx.DrawText(rcDesc, pDesc, GetAdjustColor(m_dwDescColor), -1, DT_SINGLELINE | DT_LEFT | DT_TOP | DT_END_ELLIPSIS);
			}
		}
	}

	void CStepsUI::DoEvent(TEventUI& event)
	{
		if( !m_bClickable ) {
			CContainerUI::DoEvent(event);
			return;
		}
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			CContainerUI::DoEvent(event);
			return;
		}
		if( event.Type == UIEVENT_MOUSEMOVE ) {
			int idx = HitTest(event.ptMouse);
			if( idx != m_nHoverIndex ) { m_nHoverIndex = idx; Invalidate(); }
			return;
		}
		if( event.Type == UIEVENT_MOUSELEAVE ) {
			if( m_nHoverIndex >= 0 ) { m_nHoverIndex = -1; Invalidate(); }
			return;
		}
		if( event.Type == UIEVENT_BUTTONUP ) {
			int idx = HitTest(event.ptMouse);
			if( idx >= 0 && idx != m_nCurrent ) {
				SetCurrent(idx);
				if( m_pManager ) m_pManager->SendNotify(this, DUI_MSGTYPE_SELECTCHANGED, (WPARAM)idx, 0);
			}
			return;
		}
		CContainerUI::DoEvent(event);
	}

	void CStepsUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("current")) == 0 ) {
			SetCurrent(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("direction")) == 0 ) {
			SetDirection(_tcsicmp(pstrValue, _T("vertical")) == 0 ? Vertical : Horizontal);
		}
		else if( _tcsicmp(pstrName, _T("clickable")) == 0 ) {
			SetClickable(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("items")) == 0 ) {
			SetItems(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("dot-size")) == 0 ) {
			m_nDotSize = _ttoi(pstrValue);
			if( m_nDotSize < 12 ) m_nDotSize = 12;
			Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("finish-color")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) m_dwFinishColor = clr;
		}
		else if( _tcsicmp(pstrName, _T("process-color")) == 0 || _tcsicmp(pstrName, _T("active-color")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) m_dwProcessColor = clr;
		}
		else if( _tcsicmp(pstrName, _T("wait-color")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) m_dwWaitColor = clr;
		}
		else if( _tcsicmp(pstrName, _T("error-color")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) m_dwErrorColor = clr;
		}
		else if( _tcsicmp(pstrName, _T("title-color")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) m_dwTitleColor = clr;
		}
		else if( _tcsicmp(pstrName, _T("description-color")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) m_dwDescColor = clr;
		}
		else {
			CContainerUI::SetAttribute(pstrName, pstrValue);
		}
	}
}
