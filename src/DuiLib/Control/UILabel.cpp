#include "StdAfx.h"
#include "UILabel.h"

#include <atlconv.h>
namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CLabelUI)

		CLabelUI::CLabelUI() : m_uTextStyle(DT_VCENTER | DT_SINGLELINE), m_dwTextColor(0), 
		m_dwDisabledTextColor(0),
		m_iFont(-1),
		m_bShowHtml(false),
		m_bAutoCalcWidth(false),
		m_bAutoCalcHeight(false),
		m_bClickable(false),
		m_bLButtonDown(false),
		m_bNeedEstimateSize(false)
	{
		m_cxyFixedLast.cx = m_cxyFixedLast.cy = 0;
		m_szAvailableLast.cx = m_szAvailableLast.cy = 0;
		::ZeroMemory(&m_rcTextPadding, sizeof(m_rcTextPadding));
	}

	CLabelUI::~CLabelUI()
	{
	}

	LPCTSTR CLabelUI::GetClass() const
	{
		return _T("LabelUI");
	}

	LPVOID CLabelUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, _T("Label")) == 0 ) return static_cast<CLabelUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	UINT CLabelUI::GetControlFlags() const
	{
		return IsEnabled() ? UIFLAG_SETCURSOR : 0;
	}

	void CLabelUI::SetClickable(bool bClickable)
	{
		m_bClickable = bClickable;
		if( !m_bClickable ) m_bLButtonDown = false;
	}

	void CLabelUI::SetTextStyle(UINT uStyle)
	{
		m_uTextStyle = uStyle;
		Invalidate();
	}

	UINT CLabelUI::GetTextStyle() const
	{
		return m_uTextStyle;
	}

	void CLabelUI::SetTextColor(DWORD dwTextColor)
	{
		m_dwTextColor = dwTextColor;
		Invalidate();
	}

	DWORD CLabelUI::GetTextColor() const
	{
		return m_dwTextColor;
	}

	void CLabelUI::SetDisabledTextColor(DWORD dwTextColor)
	{
		m_dwDisabledTextColor = dwTextColor;
		Invalidate();
	}

	DWORD CLabelUI::GetDisabledTextColor() const
	{
		return m_dwDisabledTextColor;
	}

	void CLabelUI::SetFont(int index)
	{
		m_iFont = index;
		m_bNeedEstimateSize = true;
		Invalidate();
	}

	int CLabelUI::GetFont() const
	{
		return m_iFont;
	}

	RECT CLabelUI::GetTextPadding() const
	{
		RECT rcTextPadding = m_rcTextPadding;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&rcTextPadding);
		return rcTextPadding;
	}

	void CLabelUI::SetTextPadding(RECT rc)
	{
		m_rcTextPadding = rc;
		m_bNeedEstimateSize = true;
		Invalidate();
	}

	bool CLabelUI::IsShowHtml()
	{
		return m_bShowHtml;
	}

	void CLabelUI::SetShowHtml(bool bShowHtml)
	{
		if( m_bShowHtml == bShowHtml ) return;

		m_bShowHtml = bShowHtml;
		m_bNeedEstimateSize = true;
		Invalidate();
	}

	SIZE CLabelUI::EstimateSize(SIZE szAvailable)
	{
		RECT rcTextPadding = GetTextPadding();
		if (m_cxyFixed.cx > 0 && m_cxyFixed.cy > 0) {
			return GetFixedSize();
		}

		if ((szAvailable.cx != m_szAvailableLast.cx || szAvailable.cy != m_szAvailableLast.cy)) {
			m_bNeedEstimateSize = true;
		}

		if (m_bNeedEstimateSize) {
			CDuiString sText = GetText();
			m_bNeedEstimateSize = false;
			m_szAvailableLast = szAvailable;
			m_cxyFixedLast = GetFixedSize();
			// 自动计算宽度
			if ((m_uTextStyle & DT_SINGLELINE) != 0) {
				// 高度
				if (m_cxyFixedLast.cy == 0) {
					m_cxyFixedLast.cy = m_pManager->GetFontInfo(m_iFont)->tm.tmHeight + 8;
					m_cxyFixedLast.cy += rcTextPadding.top + rcTextPadding.bottom;
				}
				// 宽度
				if (m_cxyFixedLast.cx == 0) {
					if(m_bAutoCalcWidth) {
						RECT rcText = { 0, 0, 9999, m_cxyFixedLast.cy };
						UINT uStyle = DT_CALCRECT | m_uTextStyle & ~DT_RIGHT & ~DT_CENTER;
						if( m_bShowHtml )
							RenderMeasureHtmlText(m_pManager, rcText, sText, 0, m_iFont, uStyle);
						else
							RenderMeasureText(m_pManager, rcText, sText, 0, m_iFont, uStyle);
						m_cxyFixedLast.cx = rcText.right - rcText.left + GetManager()->GetDPIObj()->Scale(m_rcTextPadding.left + m_rcTextPadding.right);
					}
				}
			}
			// 自动计算高度
			else if(m_cxyFixedLast.cy == 0) {
				if(m_bAutoCalcHeight) {
					RECT rcText = { 0, 0, m_cxyFixedLast.cx, 9999 };
					rcText.left += rcTextPadding.left;
					rcText.right -= rcTextPadding.right;
					UINT uStyle = DT_CALCRECT | m_uTextStyle & ~DT_RIGHT & ~DT_CENTER;
					if( m_bShowHtml )
						RenderMeasureHtmlText(m_pManager, rcText, sText, 0, m_iFont, uStyle);
					else
						RenderMeasureText(m_pManager, rcText, sText, 0, m_iFont, uStyle);
					m_cxyFixedLast.cy = rcText.bottom - rcText.top + rcTextPadding.top + rcTextPadding.bottom;
				}
			}

		}
		return m_cxyFixedLast;
	}

	void CLabelUI::DoEvent(TEventUI& event)
	{
		if( m_bClickable && IsMouseEnabled() ) {
			if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK ) {
				if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() )
					m_bLButtonDown = true;
				return;
			}
			if( event.Type == UIEVENT_BUTTONUP ) {
				if( m_bLButtonDown && IsEnabled() && ::PtInRect(&m_rcItem, event.ptMouse) && m_pManager != NULL )
					m_pManager->SendNotify(this, DUI_MSGTYPE_CLICK);
				m_bLButtonDown = false;
				return;
			}
			if( event.Type == UIEVENT_MOUSELEAVE )
				m_bLButtonDown = false;
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
		CControlUI::DoEvent(event);
	}

	void CLabelUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("align")) == 0 ) {
			if( _tcsstr(pstrValue, _T("left")) != NULL ) {
				m_uTextStyle &= ~(DT_CENTER | DT_RIGHT);
				m_uTextStyle |= DT_LEFT;
			}
			if( _tcsstr(pstrValue, _T("center")) != NULL ) {
				m_uTextStyle &= ~(DT_LEFT | DT_RIGHT );
				m_uTextStyle |= DT_CENTER;
			}
			if( _tcsstr(pstrValue, _T("right")) != NULL ) {
				m_uTextStyle &= ~(DT_LEFT | DT_CENTER);
				m_uTextStyle |= DT_RIGHT;
			}
		}
		else if( _tcsicmp(pstrName, _T("valign")) == 0 ) {
			if( _tcsstr(pstrValue, _T("top")) != NULL ) {
				m_uTextStyle &= ~(DT_BOTTOM | DT_VCENTER | DT_WORDBREAK);
				m_uTextStyle |= (DT_TOP | DT_SINGLELINE);
			}
			if( _tcsstr(pstrValue, _T("vcenter")) != NULL ) {
				m_uTextStyle &= ~(DT_TOP | DT_BOTTOM | DT_WORDBREAK);            
				m_uTextStyle |= (DT_VCENTER | DT_SINGLELINE);
			}
			if( _tcsstr(pstrValue, _T("bottom")) != NULL ) {
				m_uTextStyle &= ~(DT_TOP | DT_VCENTER | DT_WORDBREAK);
				m_uTextStyle |= (DT_BOTTOM | DT_SINGLELINE);
			}
		}
		else if( _tcsicmp(pstrName, _T("endellipsis")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("true")) == 0 ) m_uTextStyle |= DT_END_ELLIPSIS;
			else m_uTextStyle &= ~DT_END_ELLIPSIS;
		}   
		else if( _tcsicmp(pstrName, _T("wordbreak")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("true")) == 0 ) {
				m_uTextStyle &= ~DT_SINGLELINE;
				m_uTextStyle |= DT_WORDBREAK | DT_EDITCONTROL;
			}
			else {
				m_uTextStyle &= ~DT_WORDBREAK & ~DT_EDITCONTROL;
				m_uTextStyle |= DT_SINGLELINE;
			}
		}
		else if( _tcsicmp(pstrName, _T("noprefix")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("true")) == 0)
			{
				m_uTextStyle |= DT_NOPREFIX;
			}
			else
			{
				m_uTextStyle = m_uTextStyle & ~DT_NOPREFIX;
			}
		}
		else if( _tcsicmp(pstrName, _T("font")) == 0 ) SetFont(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("textcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetTextColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("disabledtextcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetDisabledTextColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("textpadding")) == 0 ) {
			RECT rcTextPadding = { 0 };
			LPTSTR pstr = NULL;
			rcTextPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
			rcTextPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
			rcTextPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
			rcTextPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
			SetTextPadding(rcTextPadding);
		}
		else if( _tcsicmp(pstrName, _T("showhtml")) == 0 ) SetShowHtml(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("autocalcwidth")) == 0 ) {
			SetAutoCalcWidth(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("autocalcheight")) == 0 ) {
			SetAutoCalcHeight(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("clickable")) == 0 || _tcsicmp(pstrName, _T("enableclick")) == 0 ) {
			SetClickable(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else CControlUI::SetAttribute(pstrName, pstrValue);
	}

	void CLabelUI::PaintText(IRenderContext& ctx)
	{
		if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

		RECT rc = m_rcItem;
		RECT m_rcTextPadding = CLabelUI::m_rcTextPadding;
		GetManager()->GetDPIObj()->Scale(&m_rcTextPadding);
		rc.left += m_rcTextPadding.left;
		rc.right -= m_rcTextPadding.right;
		rc.top += m_rcTextPadding.top;
		rc.bottom -= m_rcTextPadding.bottom;

		CDuiString sText = GetText();
		if( sText.IsEmpty() ) return;


		int nLinks = 0;
		DWORD clrColor = IsEnabled() ? m_dwTextColor : m_dwDisabledTextColor;
		if( m_bShowHtml )
			ctx.DrawHtmlText(rc, sText, clrColor, NULL, NULL, nLinks, m_iFont, m_uTextStyle);
		else
			ctx.DrawText(rc, sText, clrColor, m_iFont, m_uTextStyle);
	}

	bool CLabelUI::GetAutoCalcWidth() const
	{
		return m_bAutoCalcWidth;
	}

	void CLabelUI::SetAutoCalcWidth(bool bAutoCalcWidth)
	{
		m_bAutoCalcWidth = bAutoCalcWidth;
	}

	bool CLabelUI::GetAutoCalcHeight() const
	{
		return m_bAutoCalcHeight;
	}

	void CLabelUI::SetAutoCalcHeight(bool bAutoCalcHeight)
	{
		m_bAutoCalcHeight = bAutoCalcHeight;
	}

	void CLabelUI::SetText( LPCTSTR pstrText )
	{
		CControlUI::SetText(pstrText);
		if(GetAutoCalcWidth() || GetAutoCalcHeight()) {
			NeedParentUpdate();
		}
	}
}
