#include "StdAfx.h"
#include "UISegmented.h"

namespace DuiLib
{
	//////////////////////////////////////////////////////////////////////////
	IMPLEMENT_DUICONTROL(CSegmentItemUI)

	CSegmentItemUI::CSegmentItemUI()
	{
		SetMouseEnabled(false);
	}

	LPCTSTR CSegmentItemUI::GetClass() const { return _T("SegmentItemUI"); }

	LPVOID CSegmentItemUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_SEGMENTITEM) == 0 ) return static_cast<CSegmentItemUI*>(this);
		if( _tcsicmp(pstrName, _T("Segment")) == 0 ) return static_cast<CSegmentItemUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	void CSegmentItemUI::SetValue(LPCTSTR pstr)
	{
		m_sValue = pstr ? pstr : _T("");
	}

	LPCTSTR CSegmentItemUI::GetValue() const
	{
		if( !m_sValue.IsEmpty() ) return m_sValue.GetData();
		return m_sText.GetData();
	}

	void CSegmentItemUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("text")) == 0 || _tcsicmp(pstrName, _T("title")) == 0 || _tcsicmp(pstrName, _T("label")) == 0 ) {
			SetText(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("value")) == 0 ) {
			SetValue(pstrValue);
		}
		else {
			CControlUI::SetAttribute(pstrName, pstrValue);
		}
	}

	bool CSegmentItemUI::DoPaint(IRenderContext& /*ctx*/, const RECT& /*rcPaint*/, CControlUI* /*pStopControl*/)
	{
		return true;
	}

	SIZE CSegmentItemUI::EstimateSize(SIZE /*szAvailable*/)
	{
		SIZE sz = { 0, 0 };
		return sz;
	}

	//////////////////////////////////////////////////////////////////////////
	IMPLEMENT_DUICONTROL(CSegmentedUI)

	CSegmentedUI::CSegmentedUI()
		: m_nSelected(0)
		, m_nHover(-1)
		, m_bBlock(true)
		, m_nItemPad(12)
		, m_nInset(2)
		, m_bOptionsApplied(false)
		, m_nHitCount(0)
		, m_dwTrackColor(0x0000000A)
		, m_dwSelectedBk(0xFFFFFFFF)
		, m_dwSelectedColor(0x000000E0)
		, m_dwNormalColor(0x000000A6)
		, m_dwHoverColor(0x000000E0)
		, m_dwDisabledColor(0x00000040)
	{
		::ZeroMemory(m_rcHits, sizeof(m_rcHits));
		SetKind(CONTROLKIND_NONE);
		SetCursor(DUI_HAND);
		SIZE szRound = { 6, 6 };
		SetBorderRadius(szRound);
		SetPadding(CDuiBox(0, 0, 0, 0));
	}

	LPCTSTR CSegmentedUI::GetClass() const { return _T("SegmentedUI"); }

	LPVOID CSegmentedUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_SEGMENTED) == 0 ) return static_cast<CSegmentedUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	UINT CSegmentedUI::GetControlFlags() const
	{
		return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
	}

	bool CSegmentedUI::PreferClientHit() const
	{
		return true;
	}

	int CSegmentedUI::ScaleValue(int v) const
	{
		return m_pManager ? m_pManager->GetDPIObj()->Scale(v) : v;
	}

	CSegmentItemUI* CSegmentedUI::ItemAt(int i) const
	{
		CControlUI* p = GetItemAt(i);
		if( p == NULL ) return NULL;
		return static_cast<CSegmentItemUI*>(p->GetInterface(DUI_CTR_SEGMENTITEM));
	}

	CDuiString CSegmentedUI::ItemText(int i) const
	{
		CSegmentItemUI* p = ItemAt(i);
		if( p == NULL ) return _T("");
		return p->GetText();
	}

	CDuiString CSegmentedUI::ItemValue(int i) const
	{
		CSegmentItemUI* p = ItemAt(i);
		if( p == NULL ) return _T("");
		return p->GetValue();
	}

	int CSegmentedUI::GetItemCount() const
	{
		const_cast<CSegmentedUI*>(this)->EnsureFromOptionsAttr();
		return GetCount();
	}

	CSegmentItemUI* CSegmentedUI::GetSegment(int nIndex) const
	{
		return ItemAt(nIndex);
	}

	void CSegmentedUI::SetSelected(int nIndex, bool bNotify)
	{
		EnsureFromOptionsAttr();
		int n = GetCount();
		if( n <= 0 ) {
			m_nSelected = 0;
			return;
		}
		if( nIndex < 0 ) nIndex = 0;
		if( nIndex >= n ) nIndex = n - 1;
		if( m_nSelected == nIndex ) {
			Invalidate();
			return;
		}
		m_nSelected = nIndex;
		Invalidate();
		if( bNotify && m_pManager )
			m_pManager->SendNotify(this, DUI_MSGTYPE_SELECTCHANGED, (WPARAM)m_nSelected);
	}

	int CSegmentedUI::GetSelected() const
	{
		return m_nSelected;
	}

	void CSegmentedUI::SetSelectedValue(LPCTSTR pstrValue, bool bNotify)
	{
		EnsureFromOptionsAttr();
		if( pstrValue == NULL ) return;
		for( int i = 0; i < GetCount(); ++i ) {
			if( ItemValue(i).Compare(pstrValue) == 0 ) {
				SetSelected(i, bNotify);
				return;
			}
		}
	}

	LPCTSTR CSegmentedUI::GetSelectedValue() const
	{
		CSegmentItemUI* p = ItemAt(m_nSelected);
		if( p == NULL ) return _T("");
		return p->GetValue();
	}

	void CSegmentedUI::SetOptions(LPCTSTR pstrOptions)
	{
		m_sOptionsAttr = pstrOptions ? pstrOptions : _T("");
		m_bOptionsApplied = false;
		EnsureFromOptionsAttr();
		NeedUpdate();
		Invalidate();
	}

	void CSegmentedUI::SetBlock(bool bBlock)
	{
		m_bBlock = bBlock;
		NeedUpdate();
		Invalidate();
	}

	bool CSegmentedUI::IsBlock() const { return m_bBlock; }

	void CSegmentedUI::SetItemPadding(int nPad)
	{
		if( nPad < 4 ) nPad = 4;
		m_nItemPad = nPad;
		NeedUpdate();
		Invalidate();
	}

	int CSegmentedUI::GetItemPadding() const { return m_nItemPad; }

	void CSegmentedUI::EnsureFromOptionsAttr()
	{
		if( m_bOptionsApplied ) return;
		m_bOptionsApplied = true;
		if( m_sOptionsAttr.IsEmpty() ) return;
		if( GetCount() > 0 ) return;

		CDuiString s = m_sOptionsAttr;
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

			CDuiString sText = part;
			CDuiString sValue;
			int colon = part.Find(_T(':'));
			if( colon >= 0 ) {
				sText = part.Left(colon);
				sValue = part.Mid(colon + 1);
				sText.TrimLeft(); sText.TrimRight();
				sValue.TrimLeft(); sValue.TrimRight();
			}

			CSegmentItemUI* p = new CSegmentItemUI;
			p->SetText(sText.GetData());
			if( !sValue.IsEmpty() ) p->SetValue(sValue.GetData());
			Add(p);
		}
	}

	int CSegmentedUI::MeasureItemWidth(int i) const
	{
		CDuiString s = ItemText(i);
		RECT rcText = { 0, 0, 9999, 40 };
		RenderMeasureText(m_pManager, rcText, s.GetData(), 0, -1,
			DT_CALCRECT | DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		int w = (rcText.right - rcText.left) + ScaleValue(m_nItemPad) * 2;
		if( w < ScaleValue(40) ) w = ScaleValue(40);
		return w;
	}

	void CSegmentedUI::LayoutHitRects()
	{
		EnsureFromOptionsAttr();
		m_nHitCount = 0;
		int n = GetCount();
		if( n <= 0 ) return;
		if( n > 32 ) n = 32;
		m_nHitCount = n;

		RECT rc = m_rcItem;
		CDuiBox pad = GetPadding();
		int inset = ScaleValue(m_nInset);
		rc.left += pad.left + inset;
		rc.top += pad.top + inset;
		rc.right -= pad.right + inset;
		rc.bottom -= pad.bottom + inset;
		if( rc.right <= rc.left || rc.bottom <= rc.top ) return;

		if( m_bBlock ) {
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
			int x = rc.left;
			for( int i = 0; i < n; ++i ) {
				int w = MeasureItemWidth(i);
				m_rcHits[i].left = x;
				m_rcHits[i].right = x + w;
				m_rcHits[i].top = rc.top;
				m_rcHits[i].bottom = rc.bottom;
				x += w;
				if( m_rcHits[i].right > rc.right )
					m_rcHits[i].right = rc.right;
			}
		}
	}

	int CSegmentedUI::HitTest(POINT pt) const
	{
		for( int i = 0; i < m_nHitCount; ++i ) {
			if( ::PtInRect(&m_rcHits[i], pt) ) return i;
		}
		return -1;
	}

	void CSegmentedUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		EnsureFromOptionsAttr();
		RECT rcNull = { 0, 0, 0, 0 };
		for( int i = 0; i < GetCount(); ++i ) {
			CControlUI* p = GetItemAt(i);
			if( p ) p->SetPos(rcNull, false);
		}
		LayoutHitRects();
	}

	SIZE CSegmentedUI::EstimateSize(SIZE /*szAvailable*/)
	{
		EnsureFromOptionsAttr();
		SIZE sz = GetFixedSize();
		if( sz.cy <= 0 ) sz.cy = ScaleValue(32);
		if( sz.cx <= 0 ) {
			int n = GetCount();
			if( n < 1 ) n = 3;
			if( m_bBlock )
				sz.cx = ScaleValue(72) * n + ScaleValue(m_nInset) * 2;
			else {
				sz.cx = ScaleValue(m_nInset) * 2;
				for( int i = 0; i < n; ++i )
					sz.cx += MeasureItemWidth(i);
			}
		}
		return sz;
	}

	void CSegmentedUI::PaintTrack(IRenderContext& ctx)
	{
		RECT rc = m_rcItem;
		SIZE br = GetBorderRadius();
		int rx = br.cx > 0 ? ScaleValue(br.cx) : ScaleValue(6);
		int ry = br.cy > 0 ? ScaleValue(br.cy) : rx;
		DWORD clr = IsEnabled() ? m_dwTrackColor : DuiColorSetA(m_dwTrackColor, (BYTE)(DuiColorA(m_dwTrackColor) / 2));
		if( GetBackgroundColor() != 0 )
			clr = GetPaintBackgroundColor();
		ctx.FillRoundRect(rc, rx, ry, GetAdjustColor(clr));
	}

	void CSegmentedUI::PaintSegment(IRenderContext& ctx, int i)
	{
		if( i < 0 || i >= m_nHitCount ) return;
		RECT rc = m_rcHits[i];
		bool bSel = (i == m_nSelected);
		bool bHover = (i == m_nHover) && IsEnabled();

		SIZE br = GetBorderRadius();
		int rx = br.cx > 0 ? ScaleValue(br.cx) : ScaleValue(6);
		int ry = br.cy > 0 ? ScaleValue(br.cy) : rx;
		int insetR = ScaleValue(1);
		if( rx > insetR ) rx -= insetR;
		if( ry > insetR ) ry -= insetR;

		if( bSel && m_dwSelectedBk != 0 ) {
			RECT rcPill = rc;
			ctx.FillRoundRect(rcPill, rx, ry, GetAdjustColor(m_dwSelectedBk));
			// 顶边细线：按选中底亮度选黑/白半透明，深色主题也能看见
			const int r = (int)DuiColorR(m_dwSelectedBk);
			const int g = (int)DuiColorG(m_dwSelectedBk);
			const int b = (int)DuiColorB(m_dwSelectedBk);
			const int lum = (r * 299 + g * 587 + b * 114) / 1000;
			const DWORD dwEdge = (lum >= 160) ? 0x00000014u : 0xFFFFFF22u;
			RECT rcTop = rcPill;
			rcTop.bottom = rcTop.top + ScaleValue(1);
			ctx.DrawColor(rcTop, GetAdjustColor(dwEdge));
		}
		else if( bHover ) {
			DWORD dwHoverBk = m_dwTrackColor;
			if( dwHoverBk == 0 ) dwHoverBk = 0xF0F0F0FF;
			const int r = (int)DuiColorR(dwHoverBk);
			const int g = (int)DuiColorG(dwHoverBk);
			const int b = (int)DuiColorB(dwHoverBk);
			const int lum = (r * 299 + g * 587 + b * 114) / 1000;
			const DWORD dwFill = (lum >= 160) ? 0x0000000Cu : 0xFFFFFF18u;
			ctx.FillRoundRect(rc, rx, ry, GetAdjustColor(dwFill));
		}

		CDuiString s = ItemText(i);
		DWORD clr = m_dwNormalColor;
		if( !IsEnabled() ) clr = m_dwDisabledColor;
		else if( bSel ) clr = m_dwSelectedColor;
		else if( bHover ) clr = m_dwHoverColor;
		ctx.DrawText(rc, s.GetData(), GetAdjustColor(clr), -1,
			DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS);
	}

	bool CSegmentedUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* /*pStopControl*/)
	{
		EnsureFromOptionsAttr();
		LayoutHitRects();
		PaintTrack(ctx);
		for( int i = 0; i < m_nHitCount; ++i )
			PaintSegment(ctx, i);
		PaintBorder(ctx);
		return true;
	}

	void CSegmentedUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( !BubbleEvent(event) ) CContainerUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETCURSOR && IsEnabled() ) {
			::SetCursor(::LoadCursor(NULL, IDC_HAND));
			return;
		}

		if( event.Type == UIEVENT_MOUSEMOVE && IsEnabled() ) {
			int h = HitTest(event.ptMouse);
			if( h != m_nHover ) {
				m_nHover = h;
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSELEAVE ) {
			if( m_nHover != -1 ) { m_nHover = -1; Invalidate(); }
			return;
		}

		if( (event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK) && IsEnabled() ) {
			int h = HitTest(event.ptMouse);
			if( h >= 0 ) SetSelected(h, true);
			return;
		}

		if( event.Type == UIEVENT_KEYDOWN && IsEnabled() ) {
			int n = GetCount();
			if( n <= 0 ) return;
			if( event.chKey == VK_LEFT || event.chKey == VK_UP ) {
				SetSelected((m_nSelected + n - 1) % n, true);
				return;
			}
			if( event.chKey == VK_RIGHT || event.chKey == VK_DOWN ) {
				SetSelected((m_nSelected + 1) % n, true);
				return;
			}
			if( event.chKey == VK_HOME ) { SetSelected(0, true); return; }
			if( event.chKey == VK_END ) { SetSelected(n - 1, true); return; }
		}

		CContainerUI::DoEvent(event);
	}

	void CSegmentedUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("selected")) == 0 || _tcsicmp(pstrName, _T("current")) == 0 || _tcsicmp(pstrName, _T("active")) == 0 ) {
			// 纯数字 → 下标；否则按 value 匹配
			bool bDigit = true;
			for( LPCTSTR p = pstrValue; p && *p; ++p ) {
				if( *p < _T('0') || *p > _T('9') ) { bDigit = false; break; }
			}
			if( bDigit && pstrValue && *pstrValue )
				SetSelected(_ttoi(pstrValue), false);
			else
				SetSelectedValue(pstrValue, false);
		}
		else if( _tcsicmp(pstrName, _T("options")) == 0 || _tcsicmp(pstrName, _T("items")) == 0 ) {
			SetOptions(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("block")) == 0 ) {
			SetBlock(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("item-padding")) == 0 || _tcsicmp(pstrName, _T("itempadding")) == 0 ) {
			SetItemPadding(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("inset")) == 0 ) {
			m_nInset = _ttoi(pstrValue);
			if( m_nInset < 0 ) m_nInset = 0;
			Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("track-color")) == 0 || _tcsicmp(pstrName, _T("trackcolor")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) { m_dwTrackColor = clr; Invalidate(); }
		}
		else if( _tcsicmp(pstrName, _T("selected-background-color")) == 0 || _tcsicmp(pstrName, _T("selected-bkcolor")) == 0
			|| _tcsicmp(pstrName, _T("thumb-color")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) { m_dwSelectedBk = clr; Invalidate(); }
		}
		else if( _tcsicmp(pstrName, _T("selected-color")) == 0 || _tcsicmp(pstrName, _T("selected-textcolor")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) { m_dwSelectedColor = clr; Invalidate(); }
		}
		else if( _tcsicmp(pstrName, _T("color")) == 0 || _tcsicmp(pstrName, _T("textcolor")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) { m_dwNormalColor = clr; Invalidate(); }
		}
		else if( _tcsicmp(pstrName, _T("hover-color")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) { m_dwHoverColor = clr; Invalidate(); }
		}
		else {
			CContainerUI::SetAttribute(pstrName, pstrValue);
		}
	}
}
