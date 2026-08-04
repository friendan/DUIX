#include "StdAfx.h"
#include "UITimeline.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CTimelineItemUI)

	CTimelineItemUI::CTimelineItemUI()
		: m_eStatus(StatusFinish)
		, m_dwDotColor(0)
	{
		SetMouseEnabled(false);
	}

	LPCTSTR CTimelineItemUI::GetClass() const { return _T("TimelineItemUI"); }

	LPVOID CTimelineItemUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_TIMELINEITEM) == 0 ) return static_cast<CTimelineItemUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	void CTimelineItemUI::SetTimeText(LPCTSTR pstr) { m_sTime = pstr ? pstr : _T(""); }
	LPCTSTR CTimelineItemUI::GetTimeText() const { return m_sTime; }
	void CTimelineItemUI::SetDescription(LPCTSTR pstr) { m_sDescription = pstr ? pstr : _T(""); }
	LPCTSTR CTimelineItemUI::GetDescription() const { return m_sDescription; }
	void CTimelineItemUI::SetStatus(Status e) { m_eStatus = e; }
	CTimelineItemUI::Status CTimelineItemUI::GetStatus() const { return m_eStatus; }
	void CTimelineItemUI::SetDotColor(DWORD clr) { m_dwDotColor = clr; }
	DWORD CTimelineItemUI::GetDotColor() const { return m_dwDotColor; }

	void CTimelineItemUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("title")) == 0 || _tcsicmp(pstrName, _T("text")) == 0 ) {
			SetText(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("time")) == 0 || _tcsicmp(pstrName, _T("timestamp")) == 0 ) {
			SetTimeText(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("description")) == 0 || _tcsicmp(pstrName, _T("desc")) == 0
			|| _tcsicmp(pstrName, _T("content")) == 0 ) {
			SetDescription(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("status")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("process")) == 0 || _tcsicmp(pstrValue, _T("active")) == 0 )
				SetStatus(StatusProcess);
			else if( _tcsicmp(pstrValue, _T("wait")) == 0 || _tcsicmp(pstrValue, _T("pending")) == 0 )
				SetStatus(StatusWait);
			else
				SetStatus(StatusFinish);
		}
		else if( _tcsicmp(pstrName, _T("color")) == 0 || _tcsicmp(pstrName, _T("dot-color")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) SetDotColor(clr);
		}
		else {
			CControlUI::SetAttribute(pstrName, pstrValue);
		}
	}

	bool CTimelineItemUI::DoPaint(IRenderContext& /*ctx*/, const RECT& /*rcPaint*/, CControlUI* /*pStopControl*/)
	{
		return true;
	}

	SIZE CTimelineItemUI::EstimateSize(SIZE /*szAvailable*/)
	{
		SIZE sz = { 0, 0 };
		return sz;
	}

	//////////////////////////////////////////////////////////////////////////
	IMPLEMENT_DUICONTROL(CTimelineUI)

	CTimelineUI::CTimelineUI()
		: m_bItemsApplied(false)
		, m_bPending(false)
		, m_nDotSize(10)
		, m_nItemGap(28)
		, m_dwFinishColor(0x1677FFFF)
		, m_dwProcessColor(0x1677FFFF)
		, m_dwWaitColor(0x00000040)
		, m_dwLineColor(0x00000026)
		, m_dwTitleColor(0x333333FF)
		, m_dwTimeColor(0x8C8C8CFF)
		, m_dwDescColor(0x8C8C8CFF)
	{
		SetKind(CONTROLKIND_NONE);
	}

	LPCTSTR CTimelineUI::GetClass() const { return _T("TimelineUI"); }

	LPVOID CTimelineUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_TIMELINE) == 0 ) return static_cast<CTimelineUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	void CTimelineUI::SetItems(LPCTSTR pstrItems)
	{
		m_sItemsAttr = pstrItems ? pstrItems : _T("");
		m_bItemsApplied = false;
		EnsureFromItemsAttr();
		NeedUpdate();
		Invalidate();
	}

	void CTimelineUI::SetPending(bool bPending)
	{
		m_bPending = bPending;
		Invalidate();
	}

	bool CTimelineUI::IsPending() const { return m_bPending; }

	int CTimelineUI::ScaleValue(int v) const
	{
		return m_pManager ? m_pManager->GetDPIObj()->Scale(v) : v;
	}

	void CTimelineUI::EnsureFromItemsAttr()
	{
		if( m_bItemsApplied ) return;
		m_bItemsApplied = true;
		if( m_sItemsAttr.IsEmpty() ) return;
		if( GetCount() > 0 ) return;

		CDuiString s = m_sItemsAttr;
		int start = 0;
		int index = 0;
		while( start <= s.GetLength() ) {
			int sep = s.Find(_T('|'), start);
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

			CTimelineItemUI* p = new CTimelineItemUI;
			// "HH:mm 标题" 或 "标题"
			int sp = part.Find(_T(' '));
			if( sp > 0 && sp <= 8 ) {
				p->SetTimeText(part.Left(sp));
				CDuiString title = part.Mid(sp + 1);
				title.TrimLeft();
				p->SetText(title);
			}
			else {
				p->SetText(part);
			}
			p->SetStatus(CTimelineItemUI::StatusFinish);
			Add(p);
			++index;
		}
		if( GetCount() > 0 ) {
			CTimelineItemUI* pLast = static_cast<CTimelineItemUI*>(GetItemAt(GetCount() - 1)->GetInterface(DUI_CTR_TIMELINEITEM));
			if( pLast ) pLast->SetStatus(m_bPending ? CTimelineItemUI::StatusProcess : CTimelineItemUI::StatusFinish);
		}
	}

	void CTimelineUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		EnsureFromItemsAttr();
		RECT rcNull = { 0, 0, 0, 0 };
		for( int i = 0; i < GetCount(); ++i ) {
			CControlUI* p = GetItemAt(i);
			if( p ) p->SetPos(rcNull, false);
		}
	}

	SIZE CTimelineUI::EstimateSize(SIZE /*szAvailable*/)
	{
		EnsureFromItemsAttr();
		SIZE sz = GetFixedSize();
		int n = GetCount();
		if( n < 1 ) n = 1;
		if( sz.cy <= 0 ) sz.cy = ScaleValue(m_nItemGap + 24) * n;
		if( sz.cx <= 0 ) sz.cx = ScaleValue(240);
		return sz;
	}

	bool CTimelineUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		EnsureFromItemsAttr();
		CControlUI::DoPaint(ctx, rcPaint, pStopControl);
		PaintTimeline(ctx);
		return true;
	}

	void CTimelineUI::PaintTimeline(IRenderContext& ctx)
	{
		int n = GetCount();
		if( n <= 0 ) return;

		RECT rc = m_rcItem;
		CDuiBox rcPad = GetPadding();
		rc.left += rcPad.left;
		rc.top += rcPad.top;
		rc.right -= rcPad.right;
		rc.bottom -= rcPad.bottom;

		int dot = ScaleValue(m_nDotSize);
		int gap = ScaleValue(m_nItemGap);
		int lineX = rc.left + ScaleValue(6) + dot / 2;
		int contentLeft = rc.left + ScaleValue(6) + dot + ScaleValue(12);

		// 先画贯通竖线
		if( n >= 1 ) {
			int y0 = rc.top + ScaleValue(8) + dot / 2;
			int y1 = rc.top + ScaleValue(8) + (n - 1) * gap + dot / 2;
			if( m_bPending ) y1 += gap / 2;
			RECT rcLine = { lineX - ScaleValue(1), y0, lineX + ScaleValue(1), y1 };
			if( rcLine.bottom > rcLine.top )
				ctx.DrawColor(rcLine, GetAdjustColor(m_dwLineColor));
		}

		for( int i = 0; i < n; ++i ) {
			CControlUI* pCtrl = GetItemAt(i);
			CTimelineItemUI* pItem = pCtrl
				? static_cast<CTimelineItemUI*>(pCtrl->GetInterface(DUI_CTR_TIMELINEITEM))
				: NULL;
			if( pItem == NULL ) continue;

			int y = rc.top + ScaleValue(8) + i * gap;
			RECT rcDot = { lineX - dot / 2, y, lineX + dot / 2, y + dot };

			DWORD clrDot = pItem->GetDotColor();
			if( clrDot == 0 ) {
				CTimelineItemUI::Status st = pItem->GetStatus();
				if( st == CTimelineItemUI::StatusProcess ) clrDot = m_dwProcessColor;
				else if( st == CTimelineItemUI::StatusWait ) clrDot = m_dwWaitColor;
				else clrDot = m_dwFinishColor;
			}
			SIZE szR = { dot / 2, dot / 2 };
			ctx.FillRoundRect(rcDot, szR.cx, szR.cy, GetAdjustColor(clrDot));
			// 白心
			if( pItem->GetStatus() == CTimelineItemUI::StatusProcess ) {
				int inset = ScaleValue(3);
				RECT rcInner = { rcDot.left + inset, rcDot.top + inset, rcDot.right - inset, rcDot.bottom - inset };
				SIZE szIn = { (rcInner.right - rcInner.left) / 2, (rcInner.bottom - rcInner.top) / 2 };
				if( szIn.cx > 0 )
					ctx.FillRoundRect(rcInner, szIn.cx, szIn.cy, GetAdjustColor(0xFFFFFFFF));
			}

			int textTop = y - ScaleValue(2);
			LPCTSTR pTime = pItem->GetTimeText();
			if( pTime && *pTime ) {
				RECT rcTime = { contentLeft, textTop, rc.right, textTop + ScaleValue(18) };
				ctx.DrawText(rcTime, pTime, GetAdjustColor(m_dwTimeColor), -1,
					DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);
				textTop = rcTime.bottom;
			}
			LPCTSTR pTitle = pItem->GetText();
			if( pTitle && *pTitle ) {
				RECT rcTitle = { contentLeft, textTop, rc.right, textTop + ScaleValue(20) };
				ctx.DrawText(rcTitle, pTitle, GetAdjustColor(m_dwTitleColor), -1,
					DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);
				textTop = rcTitle.bottom;
			}
			LPCTSTR pDesc = pItem->GetDescription();
			if( pDesc && *pDesc ) {
				RECT rcDesc = { contentLeft, textTop, rc.right, textTop + ScaleValue(18) };
				ctx.DrawText(rcDesc, pDesc, GetAdjustColor(m_dwDescColor), -1,
					DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);
			}
		}
	}

	void CTimelineUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("items")) == 0 ) {
			SetItems(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("pending")) == 0 ) {
			SetPending(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("dot-size")) == 0 ) {
			m_nDotSize = _ttoi(pstrValue);
			if( m_nDotSize < 6 ) m_nDotSize = 6;
			Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("item-gap")) == 0 ) {
			m_nItemGap = _ttoi(pstrValue);
			NeedUpdate();
			Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("finish-color")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) m_dwFinishColor = clr;
		}
		else if( _tcsicmp(pstrName, _T("process-color")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) m_dwProcessColor = clr;
		}
		else if( _tcsicmp(pstrName, _T("wait-color")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) m_dwWaitColor = clr;
		}
		else if( _tcsicmp(pstrName, _T("line-color")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) m_dwLineColor = clr;
		}
		else if( _tcsicmp(pstrName, _T("title-color")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) m_dwTitleColor = clr;
		}
		else if( _tcsicmp(pstrName, _T("time-color")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) m_dwTimeColor = clr;
		}
		else if( _tcsicmp(pstrName, _T("description-color")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) m_dwDescColor = clr;
		}
		else {
			CContainerUI::SetAttribute(pstrName, pstrValue);
		}
	}
}
