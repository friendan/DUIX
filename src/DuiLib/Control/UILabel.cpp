#include "StdAfx.h"
#include "UILabel.h"

#include <atlconv.h>
namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CLabelUI)

		CLabelUI::CLabelUI() : m_uTextStyle(DT_VCENTER | DT_SINGLELINE), m_dwColor(0), 
		m_dwDisabledColor(0),
		m_dwHoverColor(0),
		m_dwActiveColor(0),
		m_dwFocusedColor(0),
		m_iFont(-1),
		m_nFontSize(0),
		m_bFontBold(false),
		m_bFontItalic(false),
		m_bFontUnderline(false),
		m_bFontStrikeout(false),
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

	bool CLabelUI::PreferClientHit() const
	{
		if( m_bClickable ) return true;
		if( HasTextStateColor() ) return true;
		return CControlUI::PreferClientHit();
	}

	bool CLabelUI::HasTextStateColor() const
	{
		return m_dwHoverColor != 0 || m_dwActiveColor != 0 || m_dwFocusedColor != 0;
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

	void CLabelUI::SetColor(DWORD dwColor)
	{
		m_dwColor = dwColor;
		Invalidate();
	}

	DWORD CLabelUI::GetColor() const
	{
		return m_dwColor;
	}

	void CLabelUI::SetDisabledColor(DWORD dwColor)
	{
		m_dwDisabledColor = dwColor;
		Invalidate();
	}

	DWORD CLabelUI::GetDisabledColor() const
	{
		return m_dwDisabledColor;
	}

	void CLabelUI::SetHoverColor(DWORD dwColor)
	{
		m_dwHoverColor = dwColor;
		Invalidate();
	}

	DWORD CLabelUI::GetHoverColor() const
	{
		return m_dwHoverColor;
	}

	void CLabelUI::SetActiveColor(DWORD dwColor)
	{
		m_dwActiveColor = dwColor;
		Invalidate();
	}

	DWORD CLabelUI::GetActiveColor() const
	{
		return m_dwActiveColor;
	}

	void CLabelUI::SetFocusedColor(DWORD dwColor)
	{
		m_dwFocusedColor = dwColor;
		Invalidate();
	}

	DWORD CLabelUI::GetFocusedColor() const
	{
		return m_dwFocusedColor;
	}

	void CLabelUI::DoInit()
	{
		CControlUI::DoInit();
		ResolveCssFont();
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

	void CLabelUI::ResolveCssFont()
	{
		if( m_pManager == NULL ) return;
		if( m_sFontFamily.IsEmpty() && m_nFontSize <= 0
			&& !m_bFontBold && !m_bFontItalic && !m_bFontUnderline && !m_bFontStrikeout )
			return;

		CDuiString sFamily = m_sFontFamily;
		int nSize = m_nFontSize;
		TFontInfo* pInfo = m_pManager->GetFontInfo(m_iFont);
		if( pInfo == NULL ) pInfo = m_pManager->GetDefaultFontInfo();
		if( pInfo != NULL ) {
			if( sFamily.IsEmpty() ) sFamily = pInfo->sFontName;
			if( nSize <= 0 ) nSize = pInfo->iSize;
		}
		if( sFamily.IsEmpty() ) sFamily = _T("Microsoft YaHei UI");
		if( nSize <= 0 ) nSize = 12;

		int id = m_pManager->EnsureFont(sFamily, nSize, m_bFontBold, m_bFontUnderline, m_bFontItalic, m_bFontStrikeout);
		if( id >= 0 ) SetFont(id);
	}

	void CLabelUI::SetFontFamily(LPCTSTR pstrFamily)
	{
		m_sFontFamily = pstrFamily ? pstrFamily : _T("");
		ResolveCssFont();
	}

	LPCTSTR CLabelUI::GetFontFamily() const
	{
		return m_sFontFamily;
	}

	void CLabelUI::SetFontSize(int nSize)
	{
		m_nFontSize = nSize > 0 ? nSize : 0;
		ResolveCssFont();
	}

	int CLabelUI::GetFontSize() const
	{
		return m_nFontSize;
	}

	void CLabelUI::SetFontBold(bool bBold)
	{
		if( m_bFontBold == bBold ) return;
		m_bFontBold = bBold;
		ResolveCssFont();
	}

	void CLabelUI::SetFontItalic(bool bItalic)
	{
		if( m_bFontItalic == bItalic ) return;
		m_bFontItalic = bItalic;
		ResolveCssFont();
	}

	void CLabelUI::SetFontUnderline(bool bUnderline)
	{
		if( m_bFontUnderline == bUnderline ) return;
		m_bFontUnderline = bUnderline;
		ResolveCssFont();
	}

	void CLabelUI::SetFontStrikeout(bool bStrikeout)
	{
		if( m_bFontStrikeout == bStrikeout ) return;
		m_bFontStrikeout = bStrikeout;
		ResolveCssFont();
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
		RECT rcPadding = GetPadding();
		const int padL = rcPadding.left + rcTextPadding.left;
		const int padR = rcPadding.right + rcTextPadding.right;
		const int padT = rcPadding.top + rcTextPadding.top;
		const int padB = rcPadding.bottom + rcTextPadding.bottom;
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
					m_cxyFixedLast.cy += padT + padB;
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
						m_cxyFixedLast.cx = rcText.right - rcText.left + padL + padR;
					}
				}
			}
			// 自动计算高度
			else if(m_cxyFixedLast.cy == 0) {
				if(m_bAutoCalcHeight) {
					RECT rcText = { 0, 0, m_cxyFixedLast.cx, 9999 };
					rcText.left += padL;
					rcText.right -= padR;
					UINT uStyle = DT_CALCRECT | m_uTextStyle & ~DT_RIGHT & ~DT_CENTER;
					if( m_bShowHtml )
						RenderMeasureHtmlText(m_pManager, rcText, sText, 0, m_iFont, uStyle);
					else
						RenderMeasureText(m_pManager, rcText, sText, 0, m_iFont, uStyle);
					m_cxyFixedLast.cy = rcText.bottom - rcText.top + padT + padB;
				}
			}

		}
		return m_cxyFixedLast;
	}

	void CLabelUI::DoEvent(TEventUI& event)
	{
		if( m_bClickable && IsMouseEnabled() ) {
			if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK ) {
				if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) {
					m_bLButtonDown = true;
					if( HasTextStateColor() ) {
						m_uControlState |= UISTATE_PUSHED | UISTATE_CAPTURED;
						Invalidate();
					}
				}
				return;
			}
			if( event.Type == UIEVENT_BUTTONUP ) {
				if( m_bLButtonDown && IsEnabled() && ::PtInRect(&m_rcItem, event.ptMouse) && m_pManager != NULL )
					m_pManager->SendNotify(this, DUI_MSGTYPE_CLICK);
				m_bLButtonDown = false;
				if( HasTextStateColor() ) {
					m_uControlState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
					Invalidate();
				}
				return;
			}
			if( event.Type == UIEVENT_MOUSELEAVE )
				m_bLButtonDown = false;
		}
		if( HasTextStateColor() && IsMouseEnabled() ) {
			if( event.Type == UIEVENT_MOUSEENTER ) {
				if( IsEnabled() ) {
					m_uControlState |= UISTATE_HOT;
					Invalidate();
				}
				return;
			}
			if( event.Type == UIEVENT_MOUSELEAVE ) {
				m_uControlState &= ~UISTATE_HOT;
				Invalidate();
				return;
			}
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
		if( _tcsicmp(pstrName, _T("text-align")) == 0 || _tcsicmp(pstrName, _T("align")) == 0 ) {
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
		else if( _tcsicmp(pstrName, _T("vertical-align")) == 0 || _tcsicmp(pstrName, _T("valign")) == 0 ) {
			if( _tcsstr(pstrValue, _T("top")) != NULL ) {
				m_uTextStyle &= ~(DT_BOTTOM | DT_VCENTER | DT_WORDBREAK);
				m_uTextStyle |= (DT_TOP | DT_SINGLELINE);
			}
			if( _tcsstr(pstrValue, _T("vcenter")) != NULL || _tcsstr(pstrValue, _T("middle")) != NULL ) {
				m_uTextStyle &= ~(DT_TOP | DT_BOTTOM | DT_WORDBREAK);            
				m_uTextStyle |= (DT_VCENTER | DT_SINGLELINE);
			}
			if( _tcsstr(pstrValue, _T("bottom")) != NULL ) {
				m_uTextStyle &= ~(DT_TOP | DT_VCENTER | DT_WORDBREAK);
				m_uTextStyle |= (DT_BOTTOM | DT_SINGLELINE);
			}
		}
		else if( _tcsicmp(pstrName, _T("text-overflow")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("ellipsis")) == 0 ) m_uTextStyle |= DT_END_ELLIPSIS;
			else m_uTextStyle &= ~DT_END_ELLIPSIS;
		}   
		else if( _tcsicmp(pstrName, _T("word-break")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("break-word")) == 0 || _tcsicmp(pstrValue, _T("break-all")) == 0 ) {
				m_uTextStyle &= ~DT_SINGLELINE;
				m_uTextStyle |= DT_WORDBREAK | DT_EDITCONTROL;
			}
			else {
				m_uTextStyle &= ~DT_WORDBREAK & ~DT_EDITCONTROL;
				m_uTextStyle |= DT_SINGLELINE;
			}
		}
		else if( _tcsicmp(pstrName, _T("white-space")) == 0 ) {
			// nowrap → 单行；normal / pre-wrap / pre-line → 换行（同 word-break）
			if( _tcsicmp(pstrValue, _T("nowrap")) == 0 ) {
				m_uTextStyle &= ~DT_WORDBREAK & ~DT_EDITCONTROL;
				m_uTextStyle |= DT_SINGLELINE;
			}
			else {
				m_uTextStyle &= ~DT_SINGLELINE;
				m_uTextStyle |= DT_WORDBREAK | DT_EDITCONTROL;
			}
		}
		else if( _tcsicmp(pstrName, _T("no-prefix")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("true")) == 0)
			{
				m_uTextStyle |= DT_NOPREFIX;
			}
			else
			{
				m_uTextStyle = m_uTextStyle & ~DT_NOPREFIX;
			}
		}
		else if( _tcsicmp(pstrName, _T("font-family")) == 0 ) SetFontFamily(pstrValue);
		else if( _tcsicmp(pstrName, _T("font-size")) == 0 ) {
			LPTSTR pEnd = NULL;
			long v = _tcstol(pstrValue, &pEnd, 10);
			if( pEnd != pstrValue && v > 0 ) SetFontSize((int)v);
		}
		else if( _tcsicmp(pstrName, _T("font-weight")) == 0 ) {
			bool bBold = false;
			if( ParseCssFontWeightBold(pstrValue, bBold) ) SetFontBold(bBold);
		}
		else if( _tcsicmp(pstrName, _T("font-style")) == 0 ) {
			bool bItalic = false;
			if( ParseCssFontStyleItalic(pstrValue, bItalic) ) SetFontItalic(bItalic);
		}
		else if( _tcsicmp(pstrName, _T("text-decoration")) == 0 ) {
			bool u = m_bFontUnderline, s = m_bFontStrikeout;
			if( ParseCssTextDecoration(pstrValue, u, s) ) {
				SetFontUnderline(u);
				SetFontStrikeout(s);
			}
		}
		else if( _tcsicmp(pstrName, _T("color")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("color-disabled")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetDisabledColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("color-hover")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetHoverColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("color-active")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetActiveColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("color-focus")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetFocusedColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("showhtml")) == 0 ) SetShowHtml(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("width")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("auto")) == 0 || _tcsicmp(pstrValue, _T("fit-content")) == 0 ) {
				SetAutoCalcWidth(true);
			}
			else {
				SetAutoCalcWidth(false);
				CControlUI::SetAttribute(pstrName, pstrValue);
			}
		}
		else if( _tcsicmp(pstrName, _T("height")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("auto")) == 0 || _tcsicmp(pstrValue, _T("fit-content")) == 0 ) {
				SetAutoCalcHeight(true);
			}
			else {
				SetAutoCalcHeight(false);
				CControlUI::SetAttribute(pstrName, pstrValue);
			}
		}
		else if( _tcsicmp(pstrName, _T("clickable")) == 0 ) {
			SetClickable(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else CControlUI::SetAttribute(pstrName, pstrValue);
	}

	void CLabelUI::PaintText(IRenderContext& ctx)
	{
		if( m_dwColor == 0 ) m_dwColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledColor == 0 ) m_dwDisabledColor = m_pManager->GetDefaultDisabledColor();

		RECT rc = m_rcItem;
		RECT rcPadding = GetPadding();
		RECT rcTextPadding = GetTextPadding();
		rc.left += rcPadding.left + rcTextPadding.left;
		rc.right -= rcPadding.right + rcTextPadding.right;
		rc.top += rcPadding.top + rcTextPadding.top;
		rc.bottom -= rcPadding.bottom + rcTextPadding.bottom;

		CDuiString sText = GetText();
		if( sText.IsEmpty() ) return;


		int nLinks = 0;
		DWORD clrColor = IsEnabled() ? m_dwColor : m_dwDisabledColor;
		if( IsEnabled() ) {
			if( (m_uControlState & UISTATE_PUSHED) != 0 && m_dwActiveColor != 0 )
				clrColor = m_dwActiveColor;
			else if( (m_uControlState & UISTATE_HOT) != 0 && m_dwHoverColor != 0 )
				clrColor = m_dwHoverColor;
			else if( IsFocused() && m_dwFocusedColor != 0 )
				clrColor = m_dwFocusedColor;
		}
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
