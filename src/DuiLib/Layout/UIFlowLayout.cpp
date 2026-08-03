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
		if( _tcsicmp(pstrName, _T("wrap")) == 0 || _tcsicmp(pstrName, _T("flex-wrap")) == 0 ) {
			bool bWrap = true;
			if( _tcsicmp(pstrValue, _T("nowrap")) == 0 || _tcsicmp(pstrValue, _T("false")) == 0
				|| _tcscmp(pstrValue, _T("0")) == 0 )
				bWrap = false;
			else if( _tcsicmp(pstrValue, _T("wrap")) == 0 || _tcsicmp(pstrValue, _T("true")) == 0
				|| _tcscmp(pstrValue, _T("1")) == 0 )
				bWrap = true;
			SetAutoWrap(bWrap);
		}
		else if( _tcsicmp(pstrName, _T("line-spacing")) == 0 || _tcsicmp(pstrName, _T("row-gap")) == 0 ) {
			SetLineSpacing(_ttoi(pstrValue));
		}
		// margin / padding：走 CControlUI / CContainerUI（margin=外边距，padding=内边距）
		else if( _tcsicmp(pstrName, _T("justify-content")) == 0 || _tcsicmp(pstrName, _T("align")) == 0 ) {
			// 主轴（行内）水平对齐；align 为旧别名
			if( _tcsicmp(pstrValue, _T("left")) == 0 || _tcsicmp(pstrValue, _T("flex-start")) == 0 )
				SetJustifyContent(DT_LEFT);
			else if( _tcsicmp(pstrValue, _T("center")) == 0 )
				SetJustifyContent(DT_CENTER);
			else if( _tcsicmp(pstrValue, _T("right")) == 0 || _tcsicmp(pstrValue, _T("flex-end")) == 0 )
				SetJustifyContent(DT_RIGHT);
		}
		else CContainerUI::SetAttribute(pstrName, pstrValue);
	}

	SIZE CFlowLayoutUI::EstimateSize(SIZE szAvailable)
	{
		SIZE szFixed = GetFixedSize();
		if( szFixed.cx > 0 && szFixed.cy > 0 ) return szFixed;

		// 与 LinearLayout 一致：自身无固定高且存在「撑满」子项时，向父级声明 cy=0
		if( szFixed.cy <= 0 ) {
			for( int i = 0; i < m_items.GetSize(); i++ ) {
				CControlUI* pControl = static_cast<CControlUI*>(m_items[i]);
				if( !pControl->IsVisible() || pControl->IsAbsolute() ) continue;
				SIZE szItem = pControl->EstimateSize(szAvailable);
				if( szItem.cy == 0 && pControl->GetFixedHeight() <= 0 )
					return CControlUI::EstimateSize(szAvailable);
			}
		}

		int cxAvailable = szAvailable.cx;
		if( szFixed.cx > 0 ) cxAvailable = szFixed.cx;

		RECT rcPadding = GetPadding();
		int cxContent = cxAvailable - rcPadding.left - rcPadding.right;
		if( cxContent < 0 ) cxContent = 0;
		int iGap = GetGap();

		int curX = 0;
		int curLineHeight = 0;
		int totalHeight = 0;
		bool bFirstInLine = true;

		for( int i = 0; i < m_items.GetSize(); i++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[i]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsAbsolute() ) continue;

			RECT rcMargin = pControl->GetMargin();
			SIZE szItem = pControl->EstimateSize({cxContent, 0});
			if( szItem.cx == 0 ) szItem.cx = cxContent;
			// cy==0：撑满，估算时不占固有行高
			int itemWidth = szItem.cx + rcMargin.left + rcMargin.right;
			int itemHeight = (szItem.cy > 0 ? szItem.cy : 0) + rcMargin.top + rcMargin.bottom;

			if( m_bAutoWrap && !bFirstInLine && curX + itemWidth > cxContent ) {
				totalHeight += curLineHeight + m_iLineSpacing;
				curX = 0;
				curLineHeight = 0;
				bFirstInLine = true;
			}

			curX += itemWidth + (bFirstInLine ? 0 : iGap);
			if( itemHeight > curLineHeight ) curLineHeight = itemHeight;
			bFirstInLine = false;
		}
		totalHeight += curLineHeight;

		SIZE szResult = {0, 0};
		szResult.cx = szFixed.cx > 0 ? szFixed.cx : cxAvailable;
		szResult.cy = szFixed.cy > 0 ? szFixed.cy : totalHeight + rcPadding.top + rcPadding.bottom;
		return szResult;
	}

	void CFlowLayoutUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		rc = m_rcItem;

		// 容器内边距（padding / SetPadding）
		RECT rcPadding = GetPadding();
		rc.left += rcPadding.left;
		rc.top += rcPadding.top;
		rc.right -= rcPadding.right;
		rc.bottom -= rcPadding.bottom;

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
		int cyContent = rc.bottom - rc.top;
		if( cyContent < 0 ) cyContent = 0;
		int iGap = GetGap();
		UINT uAlign = GetJustifyContent();
		UINT uVAlign = GetAlignItems();

		int startY = rc.top;
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() )
			startY -= m_pVerticalScrollBar->GetScrollPos();

		// naturalHeight：固有行高；bStretch：行内有 EstimateSize.cy==0 的撑满项
		struct LineInfo { int startIdx; int endIdx; int naturalHeight; int lineWidth; bool bStretch; };
		CStdValArray lines(sizeof(LineInfo), 16);

		int curX = 0;
		int curNaturalH = 0;
		bool bCurStretch = false;
		int lineStart = 0;
		bool bFirstInLine = true;

		for( int i = 0; i < m_items.GetSize(); i++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[i]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsAbsolute() ) {
				SetAbsolutePos(i);
				continue;
			}

			RECT rcMargin = pControl->GetMargin();
			SIZE szAvail = { cxContent - rcMargin.left - rcMargin.right, cyContent };
			if( szAvail.cx < 0 ) szAvail.cx = 0;
			SIZE szItem = pControl->EstimateSize(szAvail);
			bool bStretchY = (szItem.cy == 0 && pControl->GetFixedHeight() <= 0);
			if( szItem.cx == 0 ) szItem.cx = szAvail.cx;
			if( szItem.cx < pControl->GetMinWidth() ) szItem.cx = pControl->GetMinWidth();
			if( szItem.cx > pControl->GetMaxWidth() ) szItem.cx = pControl->GetMaxWidth();
			if( !bStretchY ) {
				if( szItem.cy < pControl->GetMinHeight() ) szItem.cy = pControl->GetMinHeight();
				if( szItem.cy > pControl->GetMaxHeight() ) szItem.cy = pControl->GetMaxHeight();
			}

			int itemWidth = szItem.cx + rcMargin.left + rcMargin.right;
			// 撑满项：固有高度只计 padding；真正高度在分完剩余空间后确定
			int itemNaturalH = rcMargin.top + rcMargin.bottom + (bStretchY ? 0 : szItem.cy);
			int gap = bFirstInLine ? 0 : iGap;

			if( m_bAutoWrap && !bFirstInLine && curX + gap + itemWidth > cxContent ) {
				LineInfo li = { lineStart, i, curNaturalH, curX, bCurStretch };
				lines.Add(&li);
				curX = 0;
				curNaturalH = 0;
				bCurStretch = false;
				bFirstInLine = true;
				lineStart = i;
				gap = 0;
			}

			curX += gap + itemWidth;
			if( itemNaturalH > curNaturalH ) curNaturalH = itemNaturalH;
			if( bStretchY ) bCurStretch = true;
			bFirstInLine = false;
		}
		if( !bFirstInLine ) {
			LineInfo li = { lineStart, m_items.GetSize(), curNaturalH, curX, bCurStretch };
			lines.Add(&li);
		}

		// 把剩余高度分给带撑满子项的行（与 VerticalLayout 可伸缩子项一致）
		int nStretchLines = 0;
		int cyNatural = 0;
		for( int ln = 0; ln < lines.GetSize(); ln++ ) {
			LineInfo* pLine = (LineInfo*)lines.GetAt(ln);
			cyNatural += pLine->naturalHeight;
			if( pLine->bStretch ) nStretchLines++;
		}
		if( lines.GetSize() > 1 )
			cyNatural += (lines.GetSize() - 1) * m_iLineSpacing;

		int cyRemain = cyContent - cyNatural;
		if( cyRemain < 0 ) cyRemain = 0;
		int cyStretchEach = (nStretchLines > 0) ? (cyRemain / nStretchLines) : 0;
		int cyStretchExtra = (nStretchLines > 0) ? (cyRemain % nStretchLines) : 0;

		int posY = startY;
		int iStretchLine = 0;
		for( int ln = 0; ln < lines.GetSize(); ln++ ) {
			LineInfo* pLine = (LineInfo*)lines.GetAt(ln);
			int lineHeight = pLine->naturalHeight;
			if( pLine->bStretch ) {
				lineHeight += cyStretchEach;
				if( iStretchLine == nStretchLines - 1 )
					lineHeight += cyStretchExtra;
				iStretchLine++;
			}

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
				if( pControl->IsAbsolute() ) continue;

				RECT rcMargin = pControl->GetMargin();
				SIZE szAvail = { cxContent - rcMargin.left - rcMargin.right,
					lineHeight - rcMargin.top - rcMargin.bottom };
				if( szAvail.cx < 0 ) szAvail.cx = 0;
				if( szAvail.cy < 0 ) szAvail.cy = 0;
				SIZE szItem = pControl->EstimateSize(szAvail);
				if( szItem.cx == 0 ) szItem.cx = szAvail.cx;
				if( szItem.cx < pControl->GetMinWidth() ) szItem.cx = pControl->GetMinWidth();
				if( szItem.cx > pControl->GetMaxWidth() ) szItem.cx = pControl->GetMaxWidth();
				// cy==0：撑满本行内容高度
				if( szItem.cy == 0 ) szItem.cy = szAvail.cy;
				if( szItem.cy < pControl->GetMinHeight() ) szItem.cy = pControl->GetMinHeight();
				if( szItem.cy > pControl->GetMaxHeight() ) szItem.cy = pControl->GetMaxHeight();

				if( !bFirst ) posX += iGap;
				bFirst = false;

				int itemTop = posY + rcMargin.top;
				int contentH = lineHeight - rcMargin.top - rcMargin.bottom;
				if( contentH < 0 ) contentH = 0;
				if( uVAlign == DT_VCENTER && szItem.cy < contentH )
					itemTop = posY + rcMargin.top + (contentH - szItem.cy) / 2;
				else if( uVAlign == DT_BOTTOM && szItem.cy < contentH )
					itemTop = posY + rcMargin.top + (contentH - szItem.cy);

				RECT rcPos = {
					posX + rcMargin.left,
					itemTop,
					posX + rcMargin.left + szItem.cx,
					itemTop + szItem.cy
				};
				pControl->SetPos(rcPos, bNeedInvalidate);
				posX = rcPos.right + rcMargin.right;
			}
			posY += lineHeight + m_iLineSpacing;
		}

		int cyNeeded = posY - startY - (lines.GetSize() > 0 ? m_iLineSpacing : 0);
		ProcessScrollBar(rc, 0, cyNeeded);
	}
}
