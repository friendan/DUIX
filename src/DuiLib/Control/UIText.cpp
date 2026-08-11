#include "StdAfx.h"
#include "UIText.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CTextUI)

	CTextUI::CTextUI() : m_nLinks(0), m_nHoverLink(-1)
	{
		m_uTextStyle = DT_WORDBREAK;
		m_rcTextPadding.left = 2;
		m_rcTextPadding.right = 2;
		::ZeroMemory(m_rcLinks, sizeof(m_rcLinks));
	}

	CTextUI::~CTextUI()
	{
	}

	LPCTSTR CTextUI::GetClass() const
	{
		return _T("TextUI");
	}

	LPVOID CTextUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_TEXT) == 0 ) return static_cast<CTextUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	UINT CTextUI::GetControlFlags() const
	{
		if( IsEnabled() && m_nLinks > 0 ) return UIFLAG_SETCURSOR;
		else return 0;
	}

	CDuiString* CTextUI::GetLinkContent(int iIndex)
	{
		if( iIndex >= 0 && iIndex < m_nLinks ) return &m_sLinks[iIndex];
		return NULL;
	}

	void CTextUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( !BubbleEvent(event) ) CLabelUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETCURSOR ) {
			for( int i = 0; i < m_nLinks; i++ ) {
				if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
					::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
					return;
				}
			}
		}
		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK && IsEnabled() ) {
			for( int i = 0; i < m_nLinks; i++ ) {
				if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
					Invalidate();
					return;
				}
			}
		}
		if( event.Type == UIEVENT_BUTTONUP && IsEnabled() ) {
			for( int i = 0; i < m_nLinks; i++ ) {
				if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
					m_pManager->SendNotify(this, DUI_MSGTYPE_LINK, i);
					return;
				}
			}
		}
		if( event.Type == UIEVENT_CONTEXTMENU )
		{
			return;
		}
		// When you move over a link
		if( m_nLinks > 0 && event.Type == UIEVENT_MOUSEMOVE && IsEnabled() ) {
			int nHoverLink = -1;
			for( int i = 0; i < m_nLinks; i++ ) {
				if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
					nHoverLink = i;
					break;
				}
			}

			if(m_nHoverLink != nHoverLink) {
				m_nHoverLink = nHoverLink;
				Invalidate();
				return;
			}      
		}
		if( event.Type == UIEVENT_MOUSELEAVE ) {
			if( m_nLinks > 0 && IsEnabled() ) {
				if(m_nHoverLink != -1) {
					m_nHoverLink = -1;
					Invalidate();
					return;
				}
			}
		}

		CLabelUI::DoEvent(event);
	}

	SIZE CTextUI::EstimateSize(SIZE szAvailable)
	{
		CDuiString sText = GetText();
		RECT rcTextPadding = GetTextPadding();
		RECT rcPadding = GetPadding();
		const int padL = rcPadding.left + rcTextPadding.left;
		const int padR = rcPadding.right + rcTextPadding.right;
		const int padT = rcPadding.top + rcTextPadding.top;
		const int padB = rcPadding.bottom + rcTextPadding.bottom;

		RECT rcText = { 0, 0, m_bAutoCalcWidth ? 9999 : GetManager()->GetDPIObj()->Scale(m_cxyFixed.cx), 9999 };
		rcText.left += padL;
		rcText.right -= padR;

		if( m_bShowHtml ) {
			RenderMeasureHtmlText(m_pManager, rcText, sText.GetData(), m_dwColor, m_iFont, DT_CALCRECT | m_uTextStyle);
		}
		else {
			RenderMeasureText(m_pManager, rcText, sText.GetData(), m_dwColor, m_iFont, DT_CALCRECT | m_uTextStyle);
		}
		SIZE cXY = {rcText.right - rcText.left + padL + padR,
			rcText.bottom - rcText.top + padT + padB};
		
		if (m_bAutoCalcWidth)
		{
			m_cxyFixed.cx = MulDiv(cXY.cx, 100.0, GetManager()->GetDPIObj()->GetScale());
		}

		return CControlUI::EstimateSize(szAvailable);
	}

	void CTextUI::PaintText(IRenderContext& ctx)
	{
		CDuiString sText = GetText();
		if( sText.IsEmpty() ) {
			m_nLinks = 0;
			return;
		}

		if( m_dwColor == 0 ) m_dwColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledColor == 0 ) m_dwDisabledColor = m_pManager->GetDefaultDisabledColor();

		m_nLinks = lengthof(m_rcLinks);
		RECT rc = m_rcItem;
		RECT rcPadding = GetPadding();
		RECT rcTextPadding = GetTextPadding();
		rc.left += rcPadding.left + rcTextPadding.left;
		rc.right -= rcPadding.right + rcTextPadding.right;
		rc.top += rcPadding.top + rcTextPadding.top;
		rc.bottom -= rcPadding.bottom + rcTextPadding.bottom;

		DWORD clrColor = IsEnabled() ? m_dwColor : m_dwDisabledColor;
		if( m_bShowHtml )
			ctx.DrawHtmlText(rc, sText.GetData(), GetAdjustColor(clrColor), m_rcLinks, m_sLinks, m_nLinks, m_iFont, m_uTextStyle);
		else
			ctx.DrawText(rc, sText.GetData(), GetAdjustColor(clrColor), m_iFont, m_uTextStyle);
	}
}
