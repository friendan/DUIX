#include "StdAfx.h"
#include "UIButton.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CButtonUI)

	CButtonUI::CButtonUI()
		: m_uButtonState(0)
		, m_iHoverFont(-1)
		, m_iActiveFont(-1)
		, m_iFocusedFont(-1)
		, m_dwFocusedColor(0)
		, m_iBindTabIndex(-1)
		, m_nStateCount(0)
	{
		m_uTextStyle = DT_SINGLELINE | DT_VCENTER | DT_CENTER;
		SetKind(CONTROLKIND_DEFAULT);
	}

	LPCTSTR CButtonUI::GetClass() const
	{
		return _T("ButtonUI");
	}

	LPVOID CButtonUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_BUTTON) == 0 ) return static_cast<CButtonUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	UINT CButtonUI::GetControlFlags() const
	{
		return (IsKeyboardEnabled() ? UIFLAG_TABSTOP : 0) | (IsEnabled() ? UIFLAG_SETCURSOR : 0);
	}

	bool CButtonUI::PreferClientHit() const
	{
		if( !IsEnabled() ) return false;
		if( m_dwFocusedColor != 0 ) return true;
		if( !m_sHoverImage.IsEmpty() || !m_sHoverForegroundImage.IsEmpty() ) return true;
		if( !m_sActiveImage.IsEmpty() || !m_sActiveForegroundImage.IsEmpty() ) return true;
		return CLabelUI::PreferClientHit();
	}

	void CButtonUI::SyncControlStateFromButton()
	{
		m_uControlState = m_uButtonState;
	}

	void CButtonUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( !BubbleEvent(event) ) CLabelUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETFOCUS ) 
		{
			Invalidate();
		}
		if( event.Type == UIEVENT_KILLFOCUS ) 
		{
			Invalidate();
		}
		if( event.Type == UIEVENT_KEYDOWN )
		{
			if (IsKeyboardEnabled()) {
				if( event.chKey == VK_SPACE || event.chKey == VK_RETURN ) {
					Activate();
					return;
				}
			}
		}		
		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK)
		{
			if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) {
				m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
				SyncControlStateFromButton();
				Invalidate();
				if(IsRichEvent()) m_pManager->SendNotify(this, DUI_MSGTYPE_BUTTONDOWN);
			}
			return;
		}	
		if( event.Type == UIEVENT_MOUSEMOVE )
		{
            if ((m_uButtonState & UISTATE_CAPTURED) != 0)
            {
                if (::PtInRect(&m_rcItem, event.ptMouse))
                    m_uButtonState |= UISTATE_PUSHED;
                else m_uButtonState &= ~UISTATE_PUSHED;
				SyncControlStateFromButton();
                Invalidate();
            }

			return;
		}
		if( event.Type == UIEVENT_BUTTONUP)
		{
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
				m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
				SyncControlStateFromButton();
				Invalidate();
				if( ::PtInRect(&m_rcItem, event.ptMouse) ) Activate();				
			}
			return;
		}
		if( event.Type == UIEVENT_CONTEXTMENU )
		{
			if( IsContextMenuUsed() ) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_MENU, event.wParam, event.lParam);
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEENTER )
		{
			if( IsEnabled() ) {
				m_uButtonState |= UISTATE_HOT;
				SyncControlStateFromButton();
				Invalidate();

				if(IsRichEvent()) m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSEENTER);
			}
		}
		if( event.Type == UIEVENT_MOUSELEAVE )
		{
			if( IsEnabled() ) {
				m_uButtonState &= ~UISTATE_HOT;
				SyncControlStateFromButton();
				Invalidate();

				if(IsRichEvent()) m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSELEAVE);
			}
		}
		CLabelUI::DoEvent(event);
	}

	bool CButtonUI::Activate()
	{
		if( !CControlUI::Activate() ) return false;
		if( m_pManager != NULL )
		{
			m_pManager->SendNotify(this, DUI_MSGTYPE_CLICK);
			BindTriggerTabSel();
		}
		return true;
	}

	void CButtonUI::SetEnabled(bool bEnable)
	{
		CControlUI::SetEnabled(bEnable);
		if (!IsEnabled()) {
			m_uButtonState |= UISTATE_DISABLED;
		}
		else {
			m_uButtonState &= ~UISTATE_DISABLED;
		}
		SyncControlStateFromButton();
	}

	
	void CButtonUI::SetHoverFont(int index)
	{
		m_iHoverFont = index;
		Invalidate();
	}

	int CButtonUI::GetHoverFont() const
	{
		return m_iHoverFont;
	}

	void CButtonUI::SetActiveFont(int index)
	{
		m_iActiveFont = index;
		Invalidate();
	}

	int CButtonUI::GetActiveFont() const
	{
		return m_iActiveFont;
	}

	void CButtonUI::SetFocusedFont(int index)
	{
		m_iFocusedFont = index;
		Invalidate();
	}

	int CButtonUI::GetFocusedFont() const
	{
		return m_iFocusedFont;
	}

	void CButtonUI::SetFocusedColor(DWORD dwColor)
	{
		m_dwFocusedColor = dwColor;
		Invalidate();
	}

	DWORD CButtonUI::GetFocusedColor() const
	{
		return m_dwFocusedColor;
	}

	LPCTSTR CButtonUI::GetImage()
	{
		return m_sImage;
	}

	void CButtonUI::SetImage(LPCTSTR pStrImage)
	{
		m_sImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CButtonUI::GetHoverImage()
	{
		return m_sHoverImage;
	}

	void CButtonUI::SetHoverImage(LPCTSTR pStrImage)
	{
		m_sHoverImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CButtonUI::GetActiveImage()
	{
		return m_sActiveImage;
	}

	void CButtonUI::SetActiveImage(LPCTSTR pStrImage)
	{
		m_sActiveImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CButtonUI::GetFocusImage()
	{
		return m_sFocusImage;
	}

	void CButtonUI::SetFocusImage(LPCTSTR pStrImage)
	{
		m_sFocusImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CButtonUI::GetDisabledImage()
	{
		return m_sDisabledImage;
	}

	void CButtonUI::SetDisabledImage(LPCTSTR pStrImage)
	{
		m_sDisabledImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CButtonUI::GetHoverForegroundImage()
	{
		return m_sHoverForegroundImage;
	}

	void CButtonUI::SetHoverForegroundImage( LPCTSTR pStrImage )
	{
		m_sHoverForegroundImage = pStrImage;
		Invalidate();
	}

    LPCTSTR CButtonUI::GetActiveForegroundImage()
    {
        return m_sActiveForegroundImage;
    }

    void CButtonUI::SetActiveForegroundImage(LPCTSTR pStrImage)
    {
        m_sActiveForegroundImage = pStrImage;
        Invalidate();
    }

	void CButtonUI::SetStateCount(int nCount)
	{
		m_nStateCount = nCount;
		Invalidate();
	}

	int CButtonUI::GetStateCount() const
	{
		return m_nStateCount;
	}

	LPCTSTR CButtonUI::GetStateImage()
	{
		return m_sStateImage;
	}

	void CButtonUI::SetStateImage( LPCTSTR pStrImage )
	{
		m_sImage.Empty();
		m_sStateImage = pStrImage;
		Invalidate();
	}

	void CButtonUI::BindTabIndex(int _BindTabIndex )
	{
		if( _BindTabIndex >= 0)
			m_iBindTabIndex	= _BindTabIndex;
	}

	void CButtonUI::BindTabLayoutName( LPCTSTR _TabLayoutName )
	{
		if(_TabLayoutName)
			m_sBindTabLayoutName = _TabLayoutName;
	}

	void CButtonUI::BindTriggerTabSel( int _SetSelectIndex /*= -1*/ )
	{
		LPCTSTR pstrName = GetBindTabLayoutName();
		if(pstrName == NULL || (GetBindTabLayoutIndex() < 0 && _SetSelectIndex < 0))
			return;

		CTabLayoutUI* pTabLayout = static_cast<CTabLayoutUI*>(GetManager()->FindControl(pstrName));
		if(!pTabLayout) return;
		pTabLayout->SelectItem(_SetSelectIndex >=0?_SetSelectIndex:GetBindTabLayoutIndex());
	}

	void CButtonUI::RemoveBindTabIndex()
	{
		m_iBindTabIndex	= -1;
		m_sBindTabLayoutName.Empty();
	}

	int CButtonUI::GetBindTabLayoutIndex()
	{
		return m_iBindTabIndex;
	}

	LPCTSTR CButtonUI::GetBindTabLayoutName()
	{
		return m_sBindTabLayoutName;
	}

	void CButtonUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("image")) == 0 ) SetImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("image-hover")) == 0 ) SetHoverImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("image-active")) == 0 ) SetActiveImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("image-focus")) == 0 ) SetFocusImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("image-disabled")) == 0 ) SetDisabledImage(pstrValue);
		else if (_tcsicmp(pstrName, _T("foreground-image-hover")) == 0) SetHoverForegroundImage(pstrValue);
		else if (_tcsicmp(pstrName, _T("foreground-image-active")) == 0) SetActiveForegroundImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("state-image")) == 0 ) SetStateImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("state-count")) == 0 ) SetStateCount(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("bind-tab-index")) == 0 ) BindTabIndex(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("bind-tab-layout-name")) == 0 ) BindTabLayoutName(pstrValue);
		else if( _tcsicmp(pstrName, _T("color-focus")) == 0 )
		{
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetFocusedColor(clrColor);
		}
		else CLabelUI::SetAttribute(pstrName, pstrValue);
	}

	void CButtonUI::PaintText(IRenderContext& ctx)
	{
		if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~ UISTATE_FOCUSED;
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~ UISTATE_DISABLED;
		SyncControlStateFromButton();

		if( m_dwColor == 0 ) m_dwColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledColor == 0 ) m_dwDisabledColor = m_pManager->GetDefaultDisabledColor();
		
		CDuiString sText = GetText();
		if( sText.IsEmpty() ) return;

		RECT rcPadding = GetPadding();
		RECT rcTextPadding = GetTextPadding();
		GetManager()->GetDPIObj()->Scale(&rcTextPadding);
		int nLinks = 0;
		RECT rc = m_rcItem;
		rc.left += rcPadding.left + rcTextPadding.left;
		rc.right -= rcPadding.right + rcTextPadding.right;
		rc.top += rcPadding.top + rcTextPadding.top;
		rc.bottom -= rcPadding.bottom + rcTextPadding.bottom;

		DWORD clrColor = IsEnabled()?m_dwColor:m_dwDisabledColor;
		
		if( ((m_uButtonState & UISTATE_PUSHED) != 0) && (GetActiveColor() != 0) )
			clrColor = GetActiveColor();
		else if( ((m_uButtonState & UISTATE_HOT) != 0) && (GetHoverColor() != 0) )
			clrColor = GetHoverColor();
		else if( ((m_uButtonState & UISTATE_FOCUSED) != 0) && (GetFocusedColor() != 0) )
			clrColor = GetFocusedColor();

		int iFont = GetFont();
		if( ((m_uButtonState & UISTATE_PUSHED) != 0) && (GetActiveFont() != -1) )
			iFont = GetActiveFont();
		else if( ((m_uButtonState & UISTATE_HOT) != 0) && (GetHoverFont() != -1) )
			iFont = GetHoverFont();
		else if( ((m_uButtonState & UISTATE_FOCUSED) != 0) && (GetFocusedFont() != -1) )
			iFont = GetFocusedFont();


		if( m_bShowHtml )
			ctx.DrawHtmlText(rc, sText, clrColor, NULL, NULL, nLinks, iFont, m_uTextStyle);
		else
			ctx.DrawText(rc, sText, clrColor, iFont, m_uTextStyle);
	}

	void CButtonUI::PaintBackgroundColor(IRenderContext& ctx)
	{
		SyncControlStateFromButton();
		CControlUI::PaintBackgroundColor(ctx);
	}

	void CButtonUI::PaintBackgroundImage(IRenderContext& ctx)
	{
		SyncControlStateFromButton();
		CControlUI::PaintBackgroundImage(ctx);
	}

	void CButtonUI::PaintStatusImage(IRenderContext& ctx)
	{
		if(!m_sStateImage.IsEmpty() && m_nStateCount > 0)
		{
			TDrawInfo info;
			info.Parse(m_sStateImage, _T(""), m_pManager);
			const TImageInfo* pImage = m_pManager->GetImageEx(info.sImageName, info.sResType, info.dwMask, info.bHSL, info.bGdiplus);
			if(m_sImage.IsEmpty() && pImage != NULL)
			{
				SIZE szImage = {pImage->nX, pImage->nY};
				SIZE szStatus = {pImage->nX / m_nStateCount, pImage->nY};
				if( szImage.cx > 0 && szImage.cy > 0 )
				{
					RECT rcSrc = {0, 0, szImage.cx, szImage.cy};
					if(m_nStateCount > 0) {
						int iLeft = rcSrc.left + 0 * szStatus.cx;
						int iRight = iLeft + szStatus.cx;
						int iTop = rcSrc.top;
						int iBottom = iTop + szStatus.cy;
						m_sImage.Format(_T("res='%s' restype='%s' dest='%d,%d,%d,%d' source='%d,%d,%d,%d'"), info.sImageName.GetData(), info.sResType.GetData(), info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
					}
					if(m_nStateCount > 1) {
						int iLeft = rcSrc.left + 1 * szStatus.cx;
						int iRight = iLeft + szStatus.cx;
						int iTop = rcSrc.top;
						int iBottom = iTop + szStatus.cy;
						m_sHoverImage.Format(_T("res='%s' restype='%s' dest='%d,%d,%d,%d' source='%d,%d,%d,%d'"), info.sImageName.GetData(), info.sResType.GetData(), info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
						m_sActiveImage.Format(_T("res='%s' restype='%s' dest='%d,%d,%d,%d' source='%d,%d,%d,%d'"), info.sImageName.GetData(), info.sResType.GetData(), info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
					}
					if(m_nStateCount > 2) {
						int iLeft = rcSrc.left + 2 * szStatus.cx;
						int iRight = iLeft + szStatus.cx;
						int iTop = rcSrc.top;
						int iBottom = iTop + szStatus.cy;
						m_sActiveImage.Format(_T("res='%s' restype='%s' dest='%d,%d,%d,%d' source='%d,%d,%d,%d'"), info.sImageName.GetData(), info.sResType.GetData(), info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
					}
					if(m_nStateCount > 3) {
						int iLeft = rcSrc.left + 3 * szStatus.cx;
						int iRight = iLeft + szStatus.cx;
						int iTop = rcSrc.top;
						int iBottom = iTop + szStatus.cy;
						m_sDisabledImage.Format(_T("res='%s' restype='%s' dest='%d,%d,%d,%d' source='%d,%d,%d,%d'"), info.sImageName.GetData(), info.sResType.GetData(), info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
					}
				}
			}
		}

		if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~ UISTATE_FOCUSED;
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~ UISTATE_DISABLED;
		if(!::IsWindowEnabled(m_pManager->GetPaintWindow())) {
			m_uButtonState &= UISTATE_DISABLED;
		}
		SyncControlStateFromButton();
		if( (m_uButtonState & UISTATE_DISABLED) != 0 ) {
			if( !m_sDisabledImage.IsEmpty() ) {
				if( !DrawImage(ctx, (LPCTSTR)m_sDisabledImage) ) {}
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
			if( !m_sActiveImage.IsEmpty() ) {
				if( !DrawImage(ctx, (LPCTSTR)m_sActiveImage) ) {}
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if( !m_sHoverImage.IsEmpty() ) {
				if( !DrawImage(ctx, (LPCTSTR)m_sHoverImage) ) {}
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
			if( !m_sFocusImage.IsEmpty() ) {
				if( !DrawImage(ctx, (LPCTSTR)m_sFocusImage) ) {}
				else return;
			}
		}

		if( !m_sImage.IsEmpty() ) {
			if( !DrawImage(ctx, (LPCTSTR)m_sImage) ) {}
		}
	}

	void CButtonUI::PaintBorder(IRenderContext& ctx)
	{
		SyncControlStateFromButton();
		CControlUI::PaintBorder(ctx);
	}

	void CButtonUI::PaintForegroundImage(IRenderContext& ctx)
	{
		if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
			if( !m_sActiveForegroundImage.IsEmpty() ) {
				if( !DrawImage(ctx, (LPCTSTR)m_sActiveForegroundImage) ) {}
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if( !m_sHoverForegroundImage.IsEmpty() ) {
				if( !DrawImage(ctx, (LPCTSTR)m_sHoverForegroundImage) ) {}
				else return;
			}
		}
		if(!m_sForegroundImage.IsEmpty() ) {
			if( !DrawImage(ctx, (LPCTSTR)m_sForegroundImage) ) {}
		}
	}

	void CButtonUI::SetKind(ControlKind kind)
	{
		InitKindColors();
		CControlUI::SetKind(kind);

		if (kind == CONTROLKIND_NONE) return;
		if (m_bOutline) return;

		int idx = (int)kind;
		const KindStateColors& normal = g_kindColors[idx].Normal;
		const KindStateColors& hover = g_kindColors[idx].Hover;
		const KindStateColors& active = g_kindColors[idx].Active;

		SetColor(normal.dwColor);
		SetHoverBackgroundColor(hover.dwBackgroundColor);
		SetHoverColor(hover.dwColor);
		SetHoverBorderColor(hover.dwBorderColor);
		SetActiveBackgroundColor(active.dwBackgroundColor);
		SetActiveColor(active.dwColor);
		SetActiveBorderColor(active.dwBorderColor);

		if (kind == CONTROLKIND_LINK) {
			SetCursor(DUI_HAND);
		}
	}

	void CButtonUI::SetOutline(bool bOutline)
	{
		InitKindColors();
		m_bOutline = bOutline;
		int idx = (int)m_controlKind;

		if (bOutline && m_controlKind != CONTROLKIND_NONE) {
			DWORD outlineColor = g_kindColors[idx].Normal.dwBackgroundColor;
			if (outlineColor == 0) outlineColor = g_kindColors[idx].Normal.dwColor;
			if (m_controlKind == CONTROLKIND_LIGHT) outlineColor = 0x212529FF;

			SetBackgroundColor(0);
			SetForeColor(outlineColor);
			SetBorderColor(outlineColor);
			SetBorderWidth(1);
			SIZE round = {6, 6};
			SetBorderRadius(round);

			SetHoverBackgroundColor(g_kindColors[idx].Hover.dwBackgroundColor);
			SetHoverColor(0xFFFFFFFF);
			SetHoverBorderColor(g_kindColors[idx].Hover.dwBorderColor);
			SetActiveBackgroundColor(g_kindColors[idx].Active.dwBackgroundColor);
			SetActiveColor(0xFFFFFFFF);
			SetActiveBorderColor(g_kindColors[idx].Active.dwBorderColor);
		}
		else {
			SetKind(m_controlKind);
			return;
		}

		Invalidate();
	}

}