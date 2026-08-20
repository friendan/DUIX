#include "StdAfx.h"
#include "UIGroupBox.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CGroupBoxUI)

	//////////////////////////////////////////////////////////////////////////
	//
	CGroupBoxUI::CGroupBoxUI(): m_dwColor(0), 
		m_dwDisabledColor(0), m_iFont(-1), m_uTextStyle(DT_SINGLELINE | DT_VCENTER | DT_CENTER)
	{
		SetPadding(CDuiBox(25, 20, 20, 20));
	}

	CGroupBoxUI::~CGroupBoxUI()
	{
	}

	LPCTSTR CGroupBoxUI::GetClass() const
	{
		return _T("GroupBoxUI");
	}

	LPVOID CGroupBoxUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, _T("GroupBox")) == 0 ) return static_cast<CGroupBoxUI*>(this);
		return CVerticalLayoutUI::GetInterface(pstrName);
	}
	void CGroupBoxUI::SetColor(DWORD dwColor)
	{
		m_dwColor = dwColor;
		Invalidate();
	}

	DWORD CGroupBoxUI::GetColor() const
	{
		return m_dwColor;
	}
	void CGroupBoxUI::SetDisabledColor(DWORD dwColor)
	{
		m_dwDisabledColor = dwColor;
		Invalidate();
	}

	DWORD CGroupBoxUI::GetDisabledColor() const
	{
		return m_dwDisabledColor;
	}
	void CGroupBoxUI::SetFont(int index)
	{
		m_iFont = index;
		Invalidate();
	}

	int CGroupBoxUI::GetFont() const
	{
		return m_iFont;
	}
	void CGroupBoxUI::PaintText(IRenderContext& ctx)
	{
		CDuiString sText = GetText();
		if( sText.IsEmpty() ) {
			return;
		}

		if( m_dwColor == 0 ) m_dwColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledColor == 0 ) m_dwDisabledColor = m_pManager->GetDefaultDisabledColor();
		if( sText.IsEmpty() ) return;

		CDuiRect rcText = m_rcItem;
		rcText.Deflate(5,5);
		SIZE szAvailable = { rcText.right - rcText.left, rcText.bottom - rcText.top };
		SIZE sz = CalcrectSize(szAvailable);

		//计算文字区域
		rcText.left = rcText.left + 15;
		rcText.top = rcText.top - 5;
		rcText.right = rcText.left + sz.cx;
		rcText.bottom = rcText.top + sz.cy;

		DWORD dwColor = m_dwColor;
		if(!IsEnabled()) dwColor = m_dwDisabledColor;
		ctx.DrawText(rcText, sText.GetData(), GetAdjustColor(dwColor), m_iFont, m_uTextStyle, GetAdjustColor(m_dwBackColor));
	}
	void CGroupBoxUI::PaintBorder(IRenderContext& ctx)
	{
		int nBorderWidth;
		SIZE cxyBorderRadius;
		RECT rcBorderWidth;
		if (m_pManager) {
			nBorderWidth = GetManager()->GetDPIObj()->Scale(m_nBorderWidth);
			cxyBorderRadius = GetManager()->GetDPIObj()->Scale(m_cxyBorderRadius);
			rcBorderWidth = GetManager()->GetDPIObj()->Scale(m_rcBorderWidth);
		}
		else {
			nBorderWidth = m_nBorderWidth;
			cxyBorderRadius = m_cxyBorderRadius;
			rcBorderWidth = m_rcBorderWidth;
		}

		if( nBorderWidth > 0 )
		{
			CDuiRect rcItem = m_rcItem;
			rcItem.Deflate(5, 5);

			if( cxyBorderRadius.cx > 0 || cxyBorderRadius.cy > 0 )//画圆角边框
			{
				if (IsFocused() && m_dwFocusBorderColor != 0)
					ctx.DrawRoundRect(rcItem, nBorderWidth, cxyBorderRadius.cx, cxyBorderRadius.cy, GetAdjustColor(m_dwFocusBorderColor));
				else
					ctx.DrawRoundRect(rcItem, nBorderWidth, cxyBorderRadius.cx, cxyBorderRadius.cy, GetAdjustColor(m_dwBorderColor));
			}
			else
			{
				if (IsFocused() && m_dwFocusBorderColor != 0)
					ctx.DrawRect(rcItem, nBorderWidth, GetAdjustColor(m_dwFocusBorderColor));
				else
					ctx.DrawRect(rcItem, nBorderWidth, GetAdjustColor(m_dwBorderColor));
			}
		}

		PaintText(ctx);
	}

	SIZE CGroupBoxUI::CalcrectSize(SIZE szAvailable)
	{
		SIZE cxyFixed = GetFixedXY();
		RECT rcText = { 0, 0, MAX(szAvailable.cx, cxyFixed.cx), 20 };
		
		CDuiString sText = GetText();

		RenderMeasureText(m_pManager, rcText, sText.GetData(), m_dwColor, m_iFont, DT_CALCRECT | m_uTextStyle);
		SIZE cXY = {rcText.right - rcText.left, rcText.bottom - rcText.top};
		return cXY;
	}
	void CGroupBoxUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("color")) == 0 ) 
		{
			DWORD clrColor = 0;
			if( !ParseColorString(pstrValue, clrColor) ) clrColor = 0;
			SetColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("color-disabled")) == 0 ) 
		{
			DWORD clrColor = 0;
			if( !ParseColorString(pstrValue, clrColor) ) clrColor = 0;
			SetDisabledColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("font-family")) == 0 || _tcsicmp(pstrName, _T("font-size")) == 0 )
		{
			CDuiString sFamily;
			int nSize = 0;
			if( _tcsicmp(pstrName, _T("font-family")) == 0 ) sFamily = pstrValue ? pstrValue : _T("");
			else {
				LPTSTR pEnd = NULL;
				long v = _tcstol(pstrValue, &pEnd, 10);
				if( pEnd != pstrValue && v > 0 ) nSize = (int)v;
			}
			if( m_pManager != NULL ) {
				TFontInfo* pInfo = m_pManager->GetFontInfo(m_iFont);
				if( pInfo == NULL ) pInfo = m_pManager->GetDefaultFontInfo();
				if( pInfo != NULL ) {
					if( sFamily.IsEmpty() ) sFamily = pInfo->sFontName;
					if( nSize <= 0 ) nSize = pInfo->iSize;
				}
				if( sFamily.IsEmpty() ) sFamily = _T("Microsoft YaHei UI");
				if( nSize <= 0 ) nSize = 12;
				int id = m_pManager->EnsureFont(sFamily.GetData(), nSize, false, false, false, false);
				if( id >= 0 ) SetFont(id);
			}
		}

		CVerticalLayoutUI::SetAttribute(pstrName, pstrValue);
	}
}
