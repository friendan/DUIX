#include "StdAfx.h"
#include "UIButton.h"
#include "UISvgBox.h"
#include "UILoading.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CButtonUI)

	CButtonUI::CButtonUI()
		: m_uButtonState(0)
		, m_iHoverFont(-1)
		, m_iActiveFont(-1)
		, m_iFocusedFont(-1)
		, m_dwFocusedColor(0)
		, m_nStateCount(0)
		, m_iBindTabIndex(-1)
		, m_pIcon(NULL)
		, m_pRasterIcon(NULL)
		, m_pLoading(NULL)
		, m_eIconKind(IconNone)
		, m_hRasterTint(NULL)
		, m_dwRasterTintColor(0)
		, m_nRasterTintW(0)
		, m_nRasterTintH(0)
		, m_sLoadingType(_T("css"))
		, m_bLoading(false)
		, m_bLoadingDisable(true)
		, m_bEnabledBeforeLoading(true)
		, m_nIconSize(16)
		, m_nIconGap(4)
		, m_sIconPos(_T("left"))
		, m_dwIconTint(0)
		, m_dwIconTintHover(0)
		, m_dwIconTintActive(0)
		, m_dwIconTintDisabled(0)
		, m_dwIconTintFocus(0)
		, m_bIconTint(false)
		, m_bIconTintAuto(false)
	{
		m_uTextStyle = DT_SINGLELINE | DT_VCENTER | DT_CENTER;
		SetKind(CONTROLKIND_DEFAULT);
		// 可点按钮默认手型光标；皮肤可用 cursor="arrow" 覆盖
		SetCursor(DUI_HAND);
	}

	CButtonUI::~CButtonUI()
	{
		if( m_pIcon != NULL ) {
			delete m_pIcon;
			m_pIcon = NULL;
		}
		if( m_pRasterIcon != NULL ) {
			delete m_pRasterIcon;
			m_pRasterIcon = NULL;
		}
		ClearRasterTintCache();
		if( m_pLoading != NULL ) {
			if( m_pManager != NULL ) m_pLoading->Stop();
			delete m_pLoading;
			m_pLoading = NULL;
		}
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
		if( HasIcon() ) return true;
		if( m_dwFocusedColor != 0 ) return true;
		if( !m_sHoverImage.IsEmpty() || !m_sHoverForegroundImage.IsEmpty() ) return true;
		if( !m_sActiveImage.IsEmpty() || !m_sActiveForegroundImage.IsEmpty() ) return true;
		return CLabelUI::PreferClientHit();
	}

	void CButtonUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
	{
		CLabelUI::SetManager(pManager, pParent, bInit);
		if( m_pIcon != NULL )
			m_pIcon->SetManager(pManager, this, bInit);
		if( m_pRasterIcon != NULL )
			m_pRasterIcon->SetManager(pManager, this, bInit);
		if( m_pLoading != NULL )
			m_pLoading->SetManager(pManager, this, bInit);
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
		return m_sImage.GetData();
	}

	void CButtonUI::SetImage(LPCTSTR pStrImage)
	{
		m_sImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CButtonUI::GetHoverImage()
	{
		return m_sHoverImage.GetData();
	}

	void CButtonUI::SetHoverImage(LPCTSTR pStrImage)
	{
		m_sHoverImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CButtonUI::GetActiveImage()
	{
		return m_sActiveImage.GetData();
	}

	void CButtonUI::SetActiveImage(LPCTSTR pStrImage)
	{
		m_sActiveImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CButtonUI::GetFocusImage()
	{
		return m_sFocusImage.GetData();
	}

	void CButtonUI::SetFocusImage(LPCTSTR pStrImage)
	{
		m_sFocusImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CButtonUI::GetDisabledImage()
	{
		return m_sDisabledImage.GetData();
	}

	void CButtonUI::SetDisabledImage(LPCTSTR pStrImage)
	{
		m_sDisabledImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CButtonUI::GetHoverForegroundImage()
	{
		return m_sHoverForegroundImage.GetData();
	}

	void CButtonUI::SetHoverForegroundImage( LPCTSTR pStrImage )
	{
		m_sHoverForegroundImage = pStrImage;
		Invalidate();
	}

    LPCTSTR CButtonUI::GetActiveForegroundImage()
    {
        return m_sActiveForegroundImage.GetData();
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
		return m_sStateImage.GetData();
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
		return m_sBindTabLayoutName.GetData();
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
		else if( _tcsicmp(pstrName, _T("icon-size")) == 0 ) {
			SetIconSize(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("icon-gap")) == 0 ) {
			SetIconGap(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("icon-position")) == 0
			|| _tcsicmp(pstrName, _T("icon-pos")) == 0 ) {
			SetIconPosition(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("icon-tint")) == 0
			|| _tcsicmp(pstrName, _T("icon-color")) == 0 ) {
			if( pstrValue == NULL || *pstrValue == _T('\0')
				|| _tcsicmp(pstrValue, _T("none")) == 0
				|| _tcsicmp(pstrValue, _T("false")) == 0
				|| _tcsicmp(pstrValue, _T("original")) == 0 ) {
				SetIconTintAuto(false);
				SetIconTint(0);
			}
			else if( _tcsicmp(pstrValue, _T("auto")) == 0
				|| _tcsicmp(pstrValue, _T("true")) == 0 ) {
				SetIconTintAuto(true);
			}
			else {
				DWORD clr = 0;
				if( ParseColorString(pstrValue, clr) ) SetIconTint(clr);
			}
		}
		else if( _tcsicmp(pstrName, _T("icon-tint-hover")) == 0
			|| _tcsicmp(pstrName, _T("icon-color-hover")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) SetIconTintHover(clr);
		}
		else if( _tcsicmp(pstrName, _T("icon-tint-active")) == 0
			|| _tcsicmp(pstrName, _T("icon-color-active")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) SetIconTintActive(clr);
		}
		else if( _tcsicmp(pstrName, _T("icon-tint-disabled")) == 0
			|| _tcsicmp(pstrName, _T("icon-color-disabled")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) SetIconTintDisabled(clr);
		}
		else if( _tcsicmp(pstrName, _T("icon-tint-focus")) == 0
			|| _tcsicmp(pstrName, _T("icon-color-focus")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) SetIconTintFocus(clr);
		}
		else if( _tcsicmp(pstrName, _T("loading")) == 0 ) {
			SetLoading(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("loading-type")) == 0 ) {
			SetLoadingType(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("loading-disable")) == 0 ) {
			SetLoadingDisable(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0
				|| _tcsicmp(pstrValue, _T("yes")) == 0);
		}
		else if( IsIconAttr(pstrName) ) {
			if( pstrValue == NULL || *pstrValue == _T('\0') ) {
				ClearIcon();
				return;
			}
			if( _tcsicmp(pstrName, _T("icon-src")) == 0 || _tcsicmp(pstrName, _T("icon")) == 0 )
				SetIconSrc(pstrValue);
			else
				SetIconLib(pstrName, pstrValue);
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

		RECT rcPadding = GetPadding();
		RECT rcTextPadding = GetTextPadding();
		GetManager()->GetDPIObj()->Scale(&rcTextPadding);
		RECT rcContent = m_rcItem;
		rcContent.left += rcPadding.left + rcTextPadding.left;
		rcContent.right -= rcPadding.right + rcTextPadding.right;
		rcContent.top += rcPadding.top + rcTextPadding.top;
		rcContent.bottom -= rcPadding.bottom + rcTextPadding.bottom;

		RECT rcText = rcContent;
		if( HasIcon() ) {
			SyncIconAppearance();
			RECT rcIcon = { 0 };
			if( LayoutIconAndText(rcContent, rcIcon, rcText) ) {
				if( IsLoading() && m_pLoading != NULL ) {
					m_pLoading->SetPos(rcIcon, false);
					m_pLoading->Paint(ctx, m_rcPaint, NULL);
				}
				else if( m_eIconKind == IconRaster && m_pRasterIcon != NULL && m_pRasterIcon->IsVisible() ) {
					PaintRasterIcon(ctx, rcIcon);
				}
				else if( m_pIcon != NULL && m_pIcon->IsVisible() ) {
					m_pIcon->SetPos(rcIcon, false);
					// 只贴图标位图，避免 SvgBox 完整 DoPaint；脏区一律由本按钮负责
					m_pIcon->PaintIcon(ctx, m_rcPaint);
				}
			}
		}

		if( sText.IsEmpty() ) return;

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

		int nLinks = 0;
		UINT uStyle = m_uTextStyle;
		if( HasIcon() ) {
			// 图标+文字已作为一组居中；左右排布文字左对齐，上下排布文字水平居中
			uStyle &= ~(DT_CENTER | DT_RIGHT | DT_LEFT);
			const bool bVertical = (m_sIconPos.CompareNoCase(_T("top")) == 0
				|| m_sIconPos.CompareNoCase(_T("bottom")) == 0);
			if( bVertical )
				uStyle |= DT_CENTER;
			else
				uStyle |= DT_LEFT;
			if( (uStyle & (DT_VCENTER | DT_BOTTOM | DT_TOP)) == 0 )
				uStyle |= DT_VCENTER;
		}

		if( m_bShowHtml )
			ctx.DrawHtmlText(rcText, sText.GetData(), GetAdjustColor(clrColor), NULL, NULL, nLinks, iFont, uStyle);
		else
			ctx.DrawText(rcText, sText.GetData(), GetAdjustColor(clrColor), iFont, uStyle);
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
			info.Parse(m_sStateImage.GetData(), _T(""), m_pManager);
			const TImageInfo* pImage = m_pManager->GetImageEx(info.sImageName.GetData(), info.sResType.GetData(), info.dwMask, info.bHSL, info.bGdiplus);
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
				if( !DrawImage(ctx, m_sDisabledImage.GetData()) ) {}
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
			if( !m_sActiveImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sActiveImage.GetData()) ) {}
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if( !m_sHoverImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sHoverImage.GetData()) ) {}
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
			if( !m_sFocusImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sFocusImage.GetData()) ) {}
				else return;
			}
		}

		if( !m_sImage.IsEmpty() ) {
			if( !DrawImage(ctx, m_sImage.GetData()) ) {}
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
				if( !DrawImage(ctx, m_sActiveForegroundImage.GetData()) ) {}
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if( !m_sHoverForegroundImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sHoverForegroundImage.GetData()) ) {}
				else return;
			}
		}
		if(!m_sForegroundImage.IsEmpty() ) {
			if( !DrawImage(ctx, m_sForegroundImage.GetData()) ) {}
		}
	}

	void CButtonUI::SetKind(ControlKind kind)
	{
		InitKindColors();
		CControlUI::SetKind(kind);

		if (kind == CONTROLKIND_NONE) {
			// 清掉 ctor/上一 kind 残留的悬停·按下色，避免 none 仍带 default 灰底
			SetHoverBackgroundColor(0);
			SetHoverBorderColor(0);
			SetActiveBackgroundColor(0);
			SetActiveBorderColor(0);
			SetDisabledBackgroundColor(0);
			SetDisabledBorderColor(0);
			return;
		}
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

	void CButtonUI::EnsureIcon()
	{
		if( m_pIcon != NULL ) return;
		m_pIcon = new CSvgBoxUI;
		m_pIcon->SetMouseEnabled(false);
		m_pIcon->SetVisible(false);
		if( m_pManager != NULL )
			m_pIcon->SetManager(m_pManager, this, false);
	}

	void CButtonUI::EnsureRasterIcon()
	{
		if( m_pRasterIcon != NULL ) return;
		m_pRasterIcon = new CControlUI;
		m_pRasterIcon->SetMouseEnabled(false);
		m_pRasterIcon->SetVisible(false);
		if( m_pManager != NULL )
			m_pRasterIcon->SetManager(m_pManager, this, false);
	}

	void CButtonUI::EnsureLoading()
	{
		if( m_pLoading != NULL ) return;
		m_pLoading = new CLoadingUI;
		m_pLoading->SetMouseEnabled(false);
		m_pLoading->SetVisible(false);
		m_pLoading->SetAttribute(_T("type"), m_sLoadingType.IsEmpty() ? _T("css") : m_sLoadingType.GetData());
		m_pLoading->Stop();
		if( m_pManager != NULL )
			m_pLoading->SetManager(m_pManager, this, false);
	}

	bool CButtonUI::IsIconAttr(LPCTSTR pstrName) const
	{
		return _tcsicmp(pstrName, _T("bsicon")) == 0
			|| _tcsicmp(pstrName, _T("iconpark")) == 0
			|| _tcsicmp(pstrName, _T("lucide")) == 0
			|| _tcsicmp(pstrName, _T("tabler-outline")) == 0
			|| _tcsicmp(pstrName, _T("tabler-filled")) == 0
			|| _tcsicmp(pstrName, _T("remixicon")) == 0
			|| _tcsicmp(pstrName, _T("twicon")) == 0
			|| _tcsicmp(pstrName, _T("icon-src")) == 0
			|| _tcsicmp(pstrName, _T("icon")) == 0;
	}

	bool CButtonUI::IsRasterImagePath(LPCTSTR pstrPath)
	{
		if( pstrPath == NULL || *pstrPath == _T('\0') ) return false;
		CDuiString s(pstrPath);
		s.MakeLower();
		LPCTSTR pExt = NULL;
		for( LPCTSTR p = s.GetData(); *p != _T('\0'); ++p ) {
			if( *p == _T('.') ) pExt = p;
			else if( *p == _T('\'') || *p == _T('"') || *p == _T(' ') || *p == _T('\t') ) {
				if( pExt != NULL ) break;
			}
		}
		if( pExt == NULL ) return false;
		return _tcsncmp(pExt, _T(".bmp"), 4) == 0
			|| _tcsncmp(pExt, _T(".png"), 4) == 0
			|| _tcsncmp(pExt, _T(".jpg"), 4) == 0
			|| _tcsncmp(pExt, _T(".jpeg"), 5) == 0;
	}

	void CButtonUI::RefreshRasterIconImage()
	{
		if( m_pRasterIcon == NULL || m_sRasterPath.IsEmpty() ) return;
		if( !m_pRasterIcon->IsVisible() ) return;
		int nSize = m_nIconSize;
		if( m_pManager != NULL )
			nSize = m_pManager->GetDPIObj()->Scale(m_nIconSize);
		CDuiString sImg = m_sRasterPath;
		if( sImg.Find(_T("file=")) < 0 && sImg.Find(_T("res=")) < 0
			&& sImg.Find(_T("url(")) < 0 ) {
			CDuiString sFmt;
			sFmt.Format(_T("file='%s' dest='0,0,%d,%d'"), m_sRasterPath.GetData(), nSize, nSize);
			sImg = sFmt;
		}
		else if( sImg.Find(_T("dest=")) < 0 ) {
			CDuiString sFmt;
			sFmt.Format(_T("%s dest='0,0,%d,%d'"), m_sRasterPath.GetData(), nSize, nSize);
			sImg = sFmt;
		}
		m_pRasterIcon->SetBackgroundImage(sImg.GetData());
		ClearRasterTintCache();
	}

	void CButtonUI::ClearRasterTintCache()
	{
		if( m_hRasterTint != NULL ) {
			IRenderDevice* pDev = GetRenderDevice();
			if( pDev != NULL ) pDev->InvalidateBitmapGpu(m_hRasterTint);
			::DeleteObject(m_hRasterTint);
			m_hRasterTint = NULL;
		}
		m_dwRasterTintColor = 0;
		m_nRasterTintW = 0;
		m_nRasterTintH = 0;
	}

	bool CButtonUI::EnsureRasterTintCache(DWORD dwColor)
	{
		if( m_pManager == NULL || m_sRasterPath.IsEmpty() || dwColor == 0 )
			return false;

		int nSize = m_nIconSize;
		nSize = m_pManager->GetDPIObj()->Scale(m_nIconSize);
		if( nSize <= 0 ) return false;

		if( m_hRasterTint != NULL && m_dwRasterTintColor == dwColor
			&& m_nRasterTintW == nSize && m_nRasterTintH == nSize )
			return true;

		ClearRasterTintCache();

		CDuiString sName = m_sRasterPath;
		// file='xxx.png' … → 裸路径给 GetImageEx
		const int nFile = sName.Find(_T("file='"));
		if( nFile >= 0 ) {
			sName = sName.Mid(nFile + 6);
			const int nEnd = sName.Find(_T('\''));
			if( nEnd >= 0 ) sName = sName.Left(nEnd);
		}
		else {
			const int nUrl = sName.Find(_T("url("));
			if( nUrl >= 0 ) {
				CDuiString sPath;
				if( ParseCssUrlImage(m_sRasterPath.GetData(), sPath) )
					sName = sPath;
			}
		}

		const TImageInfo* pSrc = m_pManager->GetImageEx(sName.GetData());
		if( pSrc == NULL || pSrc->hBitmap == NULL || pSrc->nX <= 0 || pSrc->nY <= 0 )
			return false;

		BITMAP bm = { 0 };
		if( !::GetObject(pSrc->hBitmap, sizeof(bm), &bm) || bm.bmWidth <= 0 || bm.bmHeight <= 0 )
			return false;

		LPBYTE pSrcBits = NULL;
		BYTE* pTempBits = NULL;
		if( bm.bmBits != NULL ) {
			pSrcBits = (LPBYTE)bm.bmBits;
		}
		else if( pSrc->pBits != NULL ) {
			pSrcBits = pSrc->pBits;
		}
		else {
			pTempBits = new BYTE[pSrc->nX * pSrc->nY * 4];
			BITMAPINFO bmi = {};
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = pSrc->nX;
			bmi.bmiHeader.biHeight = -pSrc->nY;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;
			HDC hScreen = ::GetDC(NULL);
			int nCopied = ::GetDIBits(hScreen, pSrc->hBitmap, 0, pSrc->nY, pTempBits, &bmi, DIB_RGB_COLORS);
			::ReleaseDC(NULL, hScreen);
			if( nCopied == 0 ) {
				delete[] pTempBits;
				return false;
			}
			pSrcBits = pTempBits;
		}

		BITMAPINFO bmiOut = {};
		bmiOut.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmiOut.bmiHeader.biWidth = nSize;
		bmiOut.bmiHeader.biHeight = -nSize;
		bmiOut.bmiHeader.biPlanes = 1;
		bmiOut.bmiHeader.biBitCount = 32;
		bmiOut.bmiHeader.biCompression = BI_RGB;
		LPBYTE pDest = NULL;
		HBITMAP hTint = ::CreateDIBSection(NULL, &bmiOut, DIB_RGB_COLORS, (void**)&pDest, NULL, 0);
		if( hTint == NULL || pDest == NULL ) {
			delete[] pTempBits;
			return false;
		}

		const BYTE tR = DuiColorR(dwColor);
		const BYTE tG = DuiColorG(dwColor);
		const BYTE tB = DuiColorB(dwColor);
		const int srcW = pSrc->nX;
		const int srcH = pSrc->nY;

		// 最近邻缩放到 icon-size，并按 alpha（无）重着色为 tint（预乘）
		for( int y = 0; y < nSize; ++y ) {
			const int sy = y * srcH / nSize;
			for( int x = 0; x < nSize; ++x ) {
				const int sx = x * srcW / nSize;
				const BYTE* pS = pSrcBits + (sy * srcW + sx) * 4;
				BYTE* pD = pDest + (y * nSize + x) * 4;
				BYTE a = pS[3];
				if( !pSrc->bAlpha ) {
					// 无 alpha（如 JPG）：用暗度作蒙版
					const int lum = (pS[2] * 30 + pS[1] * 59 + pS[0] * 11) / 100;
					a = (BYTE)(255 - lum);
				}
				pD[0] = (BYTE)((DWORD)tB * a / 255);
				pD[1] = (BYTE)((DWORD)tG * a / 255);
				pD[2] = (BYTE)((DWORD)tR * a / 255);
				pD[3] = a;
			}
		}

		delete[] pTempBits;
		m_hRasterTint = hTint;
		m_dwRasterTintColor = dwColor;
		m_nRasterTintW = nSize;
		m_nRasterTintH = nSize;
		return true;
	}

	void CButtonUI::PaintRasterIcon(IRenderContext& ctx, const RECT& rcIcon)
	{
		if( ShouldTintRasterIcon() ) {
			const DWORD paint = ResolvePaintIconColor();
			if( EnsureRasterTintCache(paint) && m_hRasterTint != NULL ) {
				RECT rcBmp = { 0, 0, m_nRasterTintW, m_nRasterTintH };
				RECT rcCorners = { 0, 0, 0, 0 };
				ctx.DrawImage(m_hRasterTint, rcIcon, m_rcPaint, rcBmp, rcCorners, true, ScaleImageFade());
				return;
			}
		}
		// 未请求着色 / 着色失败 → 原图
		if( m_pRasterIcon != NULL ) {
			m_pRasterIcon->SetPos(rcIcon, false);
			m_pRasterIcon->Paint(ctx, m_rcPaint, NULL);
		}
	}

	bool CButtonUI::ShouldTintRasterIcon() const
	{
		if( m_eIconKind != IconRaster ) return false;
		if( m_bIconTintAuto || m_bIconTint ) return true;
		if( !IsEnabled() || (m_uButtonState & UISTATE_DISABLED) != 0 )
			return m_dwIconTintDisabled != 0;
		if( (m_uButtonState & UISTATE_PUSHED) != 0 )
			return m_dwIconTintActive != 0;
		if( (m_uButtonState & UISTATE_HOT) != 0 )
			return m_dwIconTintHover != 0;
		if( (m_uButtonState & UISTATE_FOCUSED) != 0 )
			return m_dwIconTintFocus != 0;
		return false;
	}

	void CButtonUI::ShowSvgIcon()
	{
		if( m_pRasterIcon != NULL ) {
			m_pRasterIcon->SetVisible(false);
			m_pRasterIcon->SetBackgroundImage(_T(""));
		}
		m_sRasterPath.Empty();
		ClearRasterTintCache();
		m_eIconKind = IconSvg;
		if( m_pIcon != NULL && !m_bLoading )
			m_pIcon->SetVisible(true);
	}

	void CButtonUI::ShowRasterIcon(LPCTSTR pstrPath)
	{
		EnsureRasterIcon();
		if( m_pIcon != NULL )
			m_pIcon->SetVisible(false);
		m_sRasterPath = pstrPath ? pstrPath : _T("");
		m_eIconKind = IconRaster;
		if( m_pRasterIcon != NULL ) {
			m_pRasterIcon->SetVisible(!m_bLoading);
			if( !m_bLoading )
				RefreshRasterIconImage();
		}
	}

	void CButtonUI::SetIconLib(LPCTSTR pstrLib, LPCTSTR pstrName)
	{
		if( pstrLib == NULL || *pstrLib == _T('\0')
			|| pstrName == NULL || *pstrName == _T('\0')
			|| !IsIconAttr(pstrLib) ) {
			ClearIcon();
			return;
		}
		EnsureIcon();
		m_pIcon->SetAttribute(pstrLib, pstrName);
		ShowSvgIcon();
		m_bNeedEstimateSize = true;
		Invalidate();
	}

	void CButtonUI::SetIconSrc(LPCTSTR pstrPath)
	{
		if( pstrPath == NULL || *pstrPath == _T('\0') ) {
			ClearIcon();
			return;
		}
		if( IsRasterImagePath(pstrPath) ) {
			ShowRasterIcon(pstrPath);
		}
		else {
			EnsureIcon();
			m_pIcon->SetAttribute(_T("src"), pstrPath);
			ShowSvgIcon();
		}
		m_bNeedEstimateSize = true;
		Invalidate();
	}

	void CButtonUI::ClearIcon()
	{
		m_eIconKind = IconNone;
		m_sRasterPath.Empty();
		ClearRasterTintCache();
		if( m_pIcon != NULL ) {
			m_pIcon->SetVisible(false);
			m_pIcon->LoadFromUtf8Data("");
		}
		if( m_pRasterIcon != NULL ) {
			m_pRasterIcon->SetVisible(false);
			m_pRasterIcon->SetBackgroundImage(_T(""));
		}
		m_bNeedEstimateSize = true;
		Invalidate();
	}

	bool CButtonUI::HasIcon() const
	{
		if( m_bLoading ) return true;
		if( m_eIconKind == IconSvg && m_pIcon != NULL && m_pIcon->IsVisible() ) return true;
		if( m_eIconKind == IconRaster && m_pRasterIcon != NULL && m_pRasterIcon->IsVisible() ) return true;
		return false;
	}

	void CButtonUI::SetLoading(bool bLoading)
	{
		if( m_bLoading == bLoading ) return;
		EnsureLoading();
		if( bLoading ) {
			m_bEnabledBeforeLoading = IsEnabled();
			m_bLoading = true;
			if( m_pIcon != NULL ) m_pIcon->SetVisible(false);
			if( m_pRasterIcon != NULL ) m_pRasterIcon->SetVisible(false);
			SyncLoadingAppearance();
			m_pLoading->SetVisible(true);
			m_pLoading->Start();
			if( m_bLoadingDisable )
				SetEnabled(false);
		}
		else {
			m_bLoading = false;
			if( m_pLoading != NULL ) {
				m_pLoading->Stop();
				m_pLoading->SetVisible(false);
			}
			RestoreIconAfterLoading();
			if( m_bLoadingDisable )
				SetEnabled(m_bEnabledBeforeLoading);
		}
		m_bNeedEstimateSize = true;
		Invalidate();
	}

	bool CButtonUI::IsLoading() const
	{
		return m_bLoading && m_pLoading != NULL && m_pLoading->IsVisible();
	}

	void CButtonUI::SetLoadingType(LPCTSTR pstrType)
	{
		m_sLoadingType = pstrType ? pstrType : _T("css");
		if( m_sLoadingType.IsEmpty() ) m_sLoadingType = _T("css");
		if( IsLoading() )
			SyncLoadingAppearance();
		Invalidate();
	}

	void CButtonUI::SetLoadingDisable(bool bDisable)
	{
		if( m_bLoadingDisable == bDisable ) return;
		m_bLoadingDisable = bDisable;
		if( m_bLoading ) {
			if( bDisable )
				SetEnabled(false);
			else
				SetEnabled(m_bEnabledBeforeLoading);
		}
	}

	void CButtonUI::RestoreIconAfterLoading()
	{
		if( m_eIconKind == IconSvg && m_pIcon != NULL )
			m_pIcon->SetVisible(true);
		else if( m_eIconKind == IconRaster && m_pRasterIcon != NULL ) {
			m_pRasterIcon->SetVisible(true);
			RefreshRasterIconImage();
		}
	}

	void CButtonUI::SyncLoadingAppearance()
	{
		if( m_pLoading == NULL || !m_bLoading ) return;
		LPCTSTR pType = m_sLoadingType.IsEmpty() ? _T("css") : m_sLoadingType.GetData();
		m_pLoading->SetAttribute(_T("type"), pType);
		DWORD dwColor = ResolvePaintIconColor();
		if( dwColor == 0 ) dwColor = 0x1677FFFF;
		CDuiString sClr;
		sClr.Format(_T("#%08X"), dwColor);
		m_pLoading->SetAttribute(_T("color"), sClr.GetData());
	}

	void CButtonUI::SetIconSize(int nSize)
	{
		if( nSize < 8 ) nSize = 8;
		if( nSize > 64 ) nSize = 64;
		if( m_nIconSize == nSize ) return;
		m_nIconSize = nSize;
		if( m_eIconKind == IconRaster ) {
			ClearRasterTintCache();
			RefreshRasterIconImage();
		}
		m_bNeedEstimateSize = true;
		Invalidate();
	}

	void CButtonUI::SetIconGap(int nGap)
	{
		if( nGap < 0 ) nGap = 0;
		if( m_nIconGap == nGap ) return;
		m_nIconGap = nGap;
		m_bNeedEstimateSize = true;
		Invalidate();
	}

	void CButtonUI::SetIconPosition(LPCTSTR pstrPos)
	{
		CDuiString s = pstrPos ? pstrPos : _T("left");
		if( s.CompareNoCase(_T("right")) != 0
			&& s.CompareNoCase(_T("top")) != 0
			&& s.CompareNoCase(_T("bottom")) != 0 )
			s = _T("left");
		if( m_sIconPos == s ) return;
		m_sIconPos = s;
		m_bNeedEstimateSize = true;
		Invalidate();
	}

	void CButtonUI::SetIconTint(DWORD dwColor)
	{
		m_bIconTint = (dwColor != 0);
		m_dwIconTint = dwColor;
		if( m_bIconTint ) m_bIconTintAuto = false;
		ClearRasterTintCache();
		if( m_bLoading ) SyncLoadingAppearance();
		Invalidate();
	}

	void CButtonUI::SetIconTintAuto(bool bAuto)
	{
		const bool bClearExplicit = bAuto && m_bIconTint;
		if( m_bIconTintAuto == bAuto && !bClearExplicit ) return;
		m_bIconTintAuto = bAuto;
		if( bAuto ) {
			m_bIconTint = false;
			m_dwIconTint = 0;
		}
		ClearRasterTintCache();
		if( m_bLoading ) SyncLoadingAppearance();
		Invalidate();
	}

	void CButtonUI::SetIconTintHover(DWORD dwColor)
	{
		m_dwIconTintHover = dwColor;
		ClearRasterTintCache();
		if( m_bLoading ) SyncLoadingAppearance();
		Invalidate();
	}

	void CButtonUI::SetIconTintActive(DWORD dwColor)
	{
		m_dwIconTintActive = dwColor;
		ClearRasterTintCache();
		if( m_bLoading ) SyncLoadingAppearance();
		Invalidate();
	}

	void CButtonUI::SetIconTintDisabled(DWORD dwColor)
	{
		m_dwIconTintDisabled = dwColor;
		ClearRasterTintCache();
		if( m_bLoading ) SyncLoadingAppearance();
		Invalidate();
	}

	void CButtonUI::SetIconTintFocus(DWORD dwColor)
	{
		m_dwIconTintFocus = dwColor;
		ClearRasterTintCache();
		if( m_bLoading ) SyncLoadingAppearance();
		Invalidate();
	}

	DWORD CButtonUI::ResolveIconColor() const
	{
		if( m_bIconTint && m_dwIconTint != 0 )
			return m_dwIconTint;
		if( m_dwColor != 0 ) return m_dwColor;
		if( m_pManager != NULL ) return m_pManager->GetDefaultFontColor();
		return 0x000000E0;
	}

	DWORD CButtonUI::ResolvePaintIconColor() const
	{
		DWORD clr = ResolveIconColor();
		DWORD clrHover = m_dwIconTintHover != 0 ? m_dwIconTintHover
			: (GetHoverColor() != 0 ? GetHoverColor() : clr);
		DWORD clrActive = m_dwIconTintActive != 0 ? m_dwIconTintActive
			: (GetActiveColor() != 0 ? GetActiveColor() : clr);
		DWORD clrDisabled = m_dwIconTintDisabled != 0 ? m_dwIconTintDisabled
			: (m_dwDisabledColor != 0 ? m_dwDisabledColor : clr);
		DWORD clrFocus = m_dwIconTintFocus != 0 ? m_dwIconTintFocus
			: (GetFocusedColor() != 0 ? GetFocusedColor() : clr);

		if( !IsEnabled() || (m_uButtonState & UISTATE_DISABLED) != 0 )
			return clrDisabled;
		if( (m_uButtonState & UISTATE_PUSHED) != 0 )
			return clrActive;
		if( (m_uButtonState & UISTATE_HOT) != 0 )
			return clrHover;
		if( (m_uButtonState & UISTATE_FOCUSED) != 0 )
			return clrFocus;
		return clr;
	}

	void CButtonUI::SyncIconAppearance()
	{
		if( m_pIcon == NULL || m_eIconKind != IconSvg ) return;
		m_pIcon->SetEnabled(IsEnabled());
		DWORD paint = ResolvePaintIconColor();
		// 绘制路径内勿 Invalidate：父 Button 已整控件刷新；只脏图标矩形会在圆角按钮上露出白角
		m_pIcon->SetColor(paint, false);
		m_pIcon->SetHoverColor(0, false);
		m_pIcon->SetActiveColor(0, false);
		m_pIcon->SetDisabledColor(0, false);
	}

	SIZE CButtonUI::EstimateSize(SIZE szAvailable)
	{
		SIZE sz = CLabelUI::EstimateSize(szAvailable);
		if( !HasIcon() ) return sz;
		// 宽高都写死时不改
		if( m_cxyFixed.cx > 0 && m_cxyFixed.cy > 0 ) return sz;

		int nSize = m_nIconSize;
		int nGap = m_nIconGap;
		if( m_pManager != NULL ) {
			nSize = m_pManager->GetDPIObj()->Scale(m_nIconSize);
			nGap = m_pManager->GetDPIObj()->Scale(m_nIconGap);
		}

		RECT rcTextPadding = GetTextPadding();
		RECT rcPadding = GetPadding();
		const int padL = rcPadding.left + rcTextPadding.left;
		const int padR = rcPadding.right + rcTextPadding.right;
		const int padT = rcPadding.top + rcTextPadding.top;
		const int padB = rcPadding.bottom + rcTextPadding.bottom;

		const bool bHasText = !GetText().IsEmpty();
		const bool bVertical = (m_sIconPos.CompareNoCase(_T("top")) == 0
			|| m_sIconPos.CompareNoCase(_T("bottom")) == 0);

		if( m_cxyFixed.cx == 0 ) {
			if( !bHasText ) {
				sz.cx = nSize + padL + padR;
			}
			else if( bVertical ) {
				const int minW = nSize + padL + padR;
				if( sz.cx < minW ) sz.cx = minW;
			}
			else {
				sz.cx += nSize + nGap;
			}
		}
		if( m_cxyFixed.cy == 0 ) {
			if( !bHasText ) {
				sz.cy = nSize + padT + padB;
			}
			else if( bVertical ) {
				sz.cy += nSize + nGap;
			}
			else {
				const int minH = nSize + padT + padB;
				if( sz.cy < minH ) sz.cy = minH;
			}
		}

		m_cxyFixedLast = sz;
		return sz;
	}

	bool CButtonUI::LayoutIconAndText(const RECT& rcContent, RECT& rcIcon, RECT& rcText) const
	{
		rcText = rcContent;
		::ZeroMemory(&rcIcon, sizeof(rcIcon));
		if( !HasIcon() ) return false;

		int nSize = m_nIconSize;
		int nGap = m_nIconGap;
		if( m_pManager != NULL ) {
			nSize = m_pManager->GetDPIObj()->Scale(m_nIconSize);
			nGap = m_pManager->GetDPIObj()->Scale(m_nIconGap);
		}
		const int cw = rcContent.right - rcContent.left;
		const int ch = rcContent.bottom - rcContent.top;
		if( cw <= 0 || ch <= 0 ) return false;
		if( nSize > cw ) nSize = cw;

		const bool bHasText = !GetText().IsEmpty();
		const bool bTop = (m_sIconPos.CompareNoCase(_T("top")) == 0);
		const bool bBottom = (m_sIconPos.CompareNoCase(_T("bottom")) == 0);
		const bool bRight = (m_sIconPos.CompareNoCase(_T("right")) == 0);

		if( !bHasText ) {
			if( nSize > ch ) nSize = ch;
			// 纯图标：居中
			rcIcon.left = rcContent.left + (cw - nSize) / 2;
			rcIcon.top = rcContent.top + (ch - nSize) / 2;
			rcIcon.right = rcIcon.left + nSize;
			rcIcon.bottom = rcIcon.top + nSize;
			rcText = rcContent;
			return true;
		}

		// 测量文字，使「图标 + gap + 文字」整体在内容区居中
		int iFont = GetFont();
		if( ((m_uButtonState & UISTATE_PUSHED) != 0) && (GetActiveFont() != -1) )
			iFont = GetActiveFont();
		else if( ((m_uButtonState & UISTATE_HOT) != 0) && (GetHoverFont() != -1) )
			iFont = GetHoverFont();
		else if( ((m_uButtonState & UISTATE_FOCUSED) != 0) && (GetFocusedFont() != -1) )
			iFont = GetFocusedFont();

		SIZE szText = { 0, 0 };
		if( m_pManager != NULL ) {
			CDuiString sText = GetText();
			UINT uMeas = DT_SINGLELINE | DT_LEFT | DT_TOP | DT_CALCRECT;
			szText = RenderMeasureTextSize(const_cast<CPaintManagerUI*>(m_pManager),
				sText.GetData(), iFont, uMeas);
		}
		if( szText.cx < 0 ) szText.cx = 0;
		if( szText.cy < 0 ) szText.cy = 0;

		if( bTop || bBottom ) {
			// 给文字留高度，避免图标占满导致文字被裁
			const int nTextReserve = szText.cy + nGap;
			if( nSize > ch - nTextReserve && ch > nTextReserve )
				nSize = ch - nTextReserve;
			else if( nSize > ch )
				nSize = ch;

			int blockH = nSize + nGap + szText.cy;
			if( blockH > ch ) blockH = ch;
			int y = rcContent.top + (ch - blockH) / 2;
			const int yEnd = (y + blockH > rcContent.bottom) ? rcContent.bottom : (y + blockH);
			rcIcon.left = rcContent.left + (cw - nSize) / 2;
			rcIcon.right = rcIcon.left + nSize;
			rcText.left = rcContent.left;
			rcText.right = rcContent.right;
			if( bBottom ) {
				// 文字在上、图标在下
				rcText.top = y;
				rcText.bottom = y + szText.cy;
				if( rcText.bottom > yEnd ) rcText.bottom = yEnd;
				if( rcText.top > rcText.bottom ) rcText.top = rcText.bottom;
				rcIcon.top = rcText.bottom + nGap;
				rcIcon.bottom = rcIcon.top + nSize;
				if( rcIcon.bottom > yEnd ) {
					rcIcon.bottom = yEnd;
					rcIcon.top = rcIcon.bottom - nSize;
					if( rcIcon.top < rcText.bottom ) rcIcon.top = rcText.bottom;
				}
			}
			else {
				// 图标在上、文字在下
				rcIcon.top = y;
				rcIcon.bottom = rcIcon.top + nSize;
				rcText.top = rcIcon.bottom + nGap;
				rcText.bottom = yEnd;
				if( rcText.top > rcText.bottom ) rcText.top = rcText.bottom;
			}
			return true;
		}

		if( nSize > ch ) nSize = ch;

		int blockW = nSize + nGap + szText.cx;
		if( blockW > cw ) {
			// 内容过宽：贴边排布，文字吃剩余空间
			if( bRight ) {
				rcIcon.right = rcContent.right;
				rcIcon.left = rcIcon.right - nSize;
				rcIcon.top = rcContent.top + (ch - nSize) / 2;
				rcIcon.bottom = rcIcon.top + nSize;
				rcText.left = rcContent.left;
				rcText.right = rcIcon.left - nGap;
				if( rcText.right < rcText.left ) rcText.right = rcText.left;
			}
			else {
				rcIcon.left = rcContent.left;
				rcIcon.right = rcIcon.left + nSize;
				rcIcon.top = rcContent.top + (ch - nSize) / 2;
				rcIcon.bottom = rcIcon.top + nSize;
				rcText.left = rcIcon.right + nGap;
				rcText.right = rcContent.right;
				if( rcText.left > rcText.right ) rcText.left = rcText.right;
			}
			rcText.top = rcContent.top;
			rcText.bottom = rcContent.bottom;
			return true;
		}

		int x = rcContent.left + (cw - blockW) / 2;
		if( bRight ) {
			rcText.left = x;
			rcText.right = x + szText.cx;
			rcIcon.left = rcText.right + nGap;
			rcIcon.right = rcIcon.left + nSize;
		}
		else {
			rcIcon.left = x;
			rcIcon.right = x + nSize;
			rcText.left = rcIcon.right + nGap;
			rcText.right = rcText.left + szText.cx;
		}
		rcIcon.top = rcContent.top + (ch - nSize) / 2;
		rcIcon.bottom = rcIcon.top + nSize;
		rcText.top = rcContent.top;
		rcText.bottom = rcContent.bottom;
		return true;
	}

}