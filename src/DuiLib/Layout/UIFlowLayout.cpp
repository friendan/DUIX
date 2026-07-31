#include "StdAfx.h"
#include "UIFlowLayout.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CFlowLayoutUI)

	CFlowLayoutUI::CFlowLayoutUI() : m_bAutoWrap(true), m_iLineSpacing(0) {}

	LPCTSTR CFlowLayoutUI::GetClass() const
	{
		return _T("FlowLayoutUI");
	}

	LPVOID CFlowLayoutUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, _T("FlowLayout")) == 0 ) return static_cast<CFlowLayoutUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	bool CFlowLayoutUI::IsAutoWrap() const { return m_bAutoWrap; }
	void CFlowLayoutUI::SetAutoWrap(bool bWrap) { m_bAutoWrap = bWrap; NeedUpdate(); }
	int CFlowLayoutUI::GetLineSpacing() const { return m_iLineSpacing; }
	void CFlowLayoutUI::SetLineSpacing(int iSpacing) { m_iLineSpacing = iSpacing; NeedUpdate(); }

	void CFlowLayoutUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("autowrap")) == 0 || _tcsicmp(pstrName, _T("wrap")) == 0 ) {
			SetAutoWrap(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("linespacing")) == 0 ) {
			SetLineSpacing(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("padding")) == 0 ) {
			// CSS padding → 容器内边距（inset）
			RECT rcInset = { 0 };
			LPTSTR pstr = NULL;
			rcInset.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			rcInset.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			rcInset.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
			rcInset.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
			SetInset(rcInset);
		}
		else if( _tcsicmp(pstrName, _T("margin")) == 0 ) {
			// CSS margin → 控件外边距（DuiLib padding）
			RECT rcMargin = { 0 };
			LPTSTR pstr = NULL;
			rcMargin.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			rcMargin.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			rcMargin.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
			rcMargin.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
			SetPadding(rcMargin);
		}
		else if( _tcsicmp(pstrName, _T("align")) == 0 ) {
			// 与 childalign 同义：每行内容水平对齐
			if( _tcsicmp(pstrValue, _T("left")) == 0 ) SetChildAlign(DT_LEFT);
			else if( _tcsicmp(pstrValue, _T("center")) == 0 ) SetChildAlign(DT_CENTER);
			else if( _tcsicmp(pstrValue, _T("right")) == 0 ) SetChildAlign(DT_RIGHT);
		}
		else CContainerUI::SetAttribute(pstrName, pstrValue);
	}

	SIZE CFlowLayoutUI::EstimateSize(SIZE szAvailable)
	{
		SIZE szFixed = GetFixedSize();
		if( szFixed.cx > 0 && szFixed.cy > 0 ) return szFixed;

		int cxAvailable = szAvailable.cx;
		if( szFixed.cx > 0 ) cxAvailable = szFixed.cx;

		RECT rcInset = GetInset();
		int cxContent = cxAvailable - rcInset.left - rcInset.right;
		if( cxContent < 0 ) cxContent = 0;
		int iChildPadding = GetChildPadding();

		int curX = 0;
		int curLineHeight = 0;
		int totalHeight = 0;
		bool bFirstInLine = true;

		for( int i = 0; i < m_items.GetSize(); i++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[i]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) continue;

			RECT rcPadding = pControl->GetPadding();
			SIZE szItem = pControl->EstimateSize({cxContent, 0});
			if( szItem.cx == 0 ) szItem.cx = cxContent;
			int itemWidth = szItem.cx + rcPadding.left + rcPadding.right;
			int itemHeight = szItem.cy + rcPadding.top + rcPadding.bottom;

			if( m_bAutoWrap && !bFirstInLine && curX + itemWidth > cxContent ) {
				totalHeight += curLineHeight + m_iLineSpacing;
				curX = 0;
				curLineHeight = 0;
				bFirstInLine = true;
			}

			curX += itemWidth + (bFirstInLine ? 0 : iChildPadding);
			if( itemHeight > curLineHeight ) curLineHeight = itemHeight;
			bFirstInLine = false;
		}
		totalHeight += curLineHeight;

		SIZE szResult = {0, 0};
		szResult.cx = szFixed.cx > 0 ? szFixed.cx : cxAvailable;
		szResult.cy = szFixed.cy > 0 ? szFixed.cy : totalHeight + rcInset.top + rcInset.bottom;
		return szResult;
	}

	void CFlowLayoutUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		rc = m_rcItem;

		// 容器内边距（inset / padding）
		RECT rcInset = GetInset();
		rc.left += rcInset.left;
		rc.top += rcInset.top;
		rc.right -= rcInset.right;
		rc.bottom -= rcInset.bottom;

		if( m_items.GetSize() == 0 ) {
			ProcessScrollBar(rc, 0, 0);
			return;
		}

		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() )
			rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() )
			rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

		int cxContent = rc.right - rc.left;
		if( cxContent < 0 ) cxContent = 0;
		int iChildPadding = GetChildPadding();
		UINT uAlign = GetChildAlign();
		UINT uVAlign = GetChildVAlign();

		int startY = rc.top;
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() )
			startY -= m_pVerticalScrollBar->GetScrollPos();

		struct LineInfo { int startIdx; int endIdx; int lineHeight; int lineWidth; };
		CStdValArray lines(sizeof(LineInfo), 16);

		int curX = 0;
		int curLineHeight = 0;
		int lineStart = 0;
		bool bFirstInLine = true;

		for( int i = 0; i < m_items.GetSize(); i++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[i]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) {
				SetFloatPos(i);
				continue;
			}

			RECT rcPadding = pControl->GetPadding();
			SIZE szAvail = { cxContent - rcPadding.left - rcPadding.right, rc.bottom - rc.top };
			if( szAvail.cx < 0 ) szAvail.cx = 0;
			SIZE szItem = pControl->EstimateSize(szAvail);
			if( szItem.cx == 0 ) szItem.cx = szAvail.cx;
			if( szItem.cx < pControl->GetMinWidth() ) szItem.cx = pControl->GetMinWidth();
			if( szItem.cx > pControl->GetMaxWidth() ) szItem.cx = pControl->GetMaxWidth();
			if( szItem.cy < pControl->GetMinHeight() ) szItem.cy = pControl->GetMinHeight();
			if( szItem.cy > pControl->GetMaxHeight() ) szItem.cy = pControl->GetMaxHeight();

			int itemWidth = szItem.cx + rcPadding.left + rcPadding.right;
			int itemHeight = szItem.cy + rcPadding.top + rcPadding.bottom;
			int gap = bFirstInLine ? 0 : iChildPadding;

			if( m_bAutoWrap && !bFirstInLine && curX + gap + itemWidth > cxContent ) {
				LineInfo li = { lineStart, i, curLineHeight, curX };
				lines.Add(&li);
				curX = 0;
				curLineHeight = 0;
				bFirstInLine = true;
				lineStart = i;
				gap = 0;
			}

			curX += gap + itemWidth;
			if( itemHeight > curLineHeight ) curLineHeight = itemHeight;
			bFirstInLine = false;
		}
		if( !bFirstInLine ) {
			LineInfo li = { lineStart, m_items.GetSize(), curLineHeight, curX };
			lines.Add(&li);
		}

		// 自上而下逐行定位；每行按 childalign 水平对齐
		int posY = startY;
		for( int ln = 0; ln < lines.GetSize(); ln++ ) {
			LineInfo* pLine = (LineInfo*)lines.GetAt(ln);
			int posX = rc.left;
			if( uAlign == DT_CENTER )
				posX = rc.left + (cxContent - pLine->lineWidth) / 2;
			else if( uAlign == DT_RIGHT )
				posX = rc.right - pLine->lineWidth;
			if( posX < rc.left ) posX = rc.left;

			bool bFirst = true;
			for( int i = pLine->startIdx; i < pLine->endIdx; i++ ) {
				CControlUI* pControl = static_cast<CControlUI*>(m_items[i]);
				if( !pControl->IsVisible() ) continue;
				if( pControl->IsFloat() ) continue;

				RECT rcPadding = pControl->GetPadding();
				SIZE szAvail = { cxContent - rcPadding.left - rcPadding.right, pLine->lineHeight - rcPadding.top - rcPadding.bottom };
				if( szAvail.cx < 0 ) szAvail.cx = 0;
				if( szAvail.cy < 0 ) szAvail.cy = 0;
				SIZE szItem = pControl->EstimateSize(szAvail);
				if( szItem.cx == 0 ) szItem.cx = szAvail.cx;
				if( szItem.cx < pControl->GetMinWidth() ) szItem.cx = pControl->GetMinWidth();
				if( szItem.cx > pControl->GetMaxWidth() ) szItem.cx = pControl->GetMaxWidth();
				if( szItem.cy == 0 ) szItem.cy = szAvail.cy;
				if( szItem.cy < pControl->GetMinHeight() ) szItem.cy = pControl->GetMinHeight();
				if( szItem.cy > pControl->GetMaxHeight() ) szItem.cy = pControl->GetMaxHeight();

				if( !bFirst ) posX += iChildPadding;
				bFirst = false;

				int itemTop = posY + rcPadding.top;
				int contentH = pLine->lineHeight - rcPadding.top - rcPadding.bottom;
				if( contentH < 0 ) contentH = 0;
				if( uVAlign == DT_VCENTER && szItem.cy < contentH )
					itemTop = posY + rcPadding.top + (contentH - szItem.cy) / 2;
				else if( uVAlign == DT_BOTTOM && szItem.cy < contentH )
					itemTop = posY + rcPadding.top + (contentH - szItem.cy);

				RECT rcPos = {
					posX + rcPadding.left,
					itemTop,
					posX + rcPadding.left + szItem.cx,
					itemTop + szItem.cy
				};
				pControl->SetPos(rcPos, bNeedInvalidate);
				posX = rcPos.right + rcPadding.right;
			}
			posY += pLine->lineHeight + m_iLineSpacing;
		}

		int cyNeeded = posY - startY - (lines.GetSize() > 0 ? m_iLineSpacing : 0);
		ProcessScrollBar(rc, 0, cyNeeded);
	}
}
