#include "StdAfx.h"
#include "UIFadeButton.h"

namespace DuiLib {
	IMPLEMENT_DUICONTROL(CFadeButtonUI)

	CFadeButtonUI::CFadeButtonUI(): m_bMouseHove( FALSE ), m_bMouseLeave( FALSE )
	{
		Attach(this);
	}

	CFadeButtonUI::~CFadeButtonUI()
	{
		StopAnimation();
	}

	LPCTSTR CFadeButtonUI::GetClass() const
	{
		return _T("FadeButtonUI");
	}

	LPVOID CFadeButtonUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, _T("FadeButton")) == 0 ) 
			return static_cast<CFadeButtonUI*>(this);
		return CButtonUI::GetInterface(pstrName);
	}

	void CFadeButtonUI::SetImage(LPCTSTR pStrImage)
	{
		m_sImage = pStrImage;
		m_sLastImage = m_sImage;
	}

	void CFadeButtonUI::DoEvent(TEventUI& event)
	{
		if( event.Type == UIEVENT_TIMER ) {
			OnTimer(  event.wParam );
		}
		else if(IsEnabled()) {
			if( event.Type == UIEVENT_MOUSEENTER && !IsAnimationRunning( FADE_IN_ID ) )
			{
				m_bFadeAlpha = 0;
				m_bMouseHove = TRUE;
				StopAnimation(FADE_OUT_ID);
				StartAnimation(FADE_ELLAPSE, FADE_FRAME_COUNT, FADE_IN_ID);
				Invalidate();
				return;
			}
			else if( event.Type == UIEVENT_MOUSELEAVE && !IsAnimationRunning( FADE_OUT_ID ) )
			{
				m_bFadeAlpha = 0;
				m_bMouseLeave = TRUE;
				StopAnimation(FADE_IN_ID);
				StartAnimation(FADE_ELLAPSE, FADE_FRAME_COUNT, FADE_OUT_ID);
				Invalidate();
				return;
			}
		}
		CButtonUI::DoEvent( event );
	}

	void CFadeButtonUI::OnTimer( int nTimerID )
	{
		OnAnimationElapse( nTimerID );
	}

	void CFadeButtonUI::PaintStatusImage(IRenderContext& ctx)
	{
		if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~ UISTATE_FOCUSED;
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~ UISTATE_DISABLED;

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
		else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
			if( !m_sFocusImage.IsEmpty() ) {
				if( !DrawImage(ctx, (LPCTSTR)m_sFocusImage) ) {}
				else return;
			}
		}
		if( !m_sImage.IsEmpty() ) {
			if( m_bMouseHove ) {
				m_bMouseHove = FALSE;
				m_sLastImage = m_sHoverImage;
			}

			if( m_bMouseLeave ) {
				m_bMouseLeave = FALSE;
				m_sLastImage = m_sImage;
			}

			if( IsAnimationRunning(FADE_IN_ID) || IsAnimationRunning(FADE_OUT_ID)) {
				m_sOldImage = m_sImage;
				m_sNewImage = m_sHoverImage;
				if( IsAnimationRunning(FADE_OUT_ID) ) {
					m_sOldImage = m_sHoverImage;
					m_sNewImage = m_sImage;
				}
				CDuiString sFadeOut, sFadeIn;
				sFadeOut.Format(_T("fade='%d'"), 255 - m_bFadeAlpha);
				sFadeIn.Format(_T("fade='%d'"), m_bFadeAlpha);
				if( !DrawImage(ctx, (LPCTSTR)m_sOldImage, sFadeOut) ) {}
				if( !DrawImage(ctx, (LPCTSTR)m_sNewImage, sFadeIn) ) {}
				return;
			}
			else {
				if(m_sLastImage.IsEmpty()) m_sLastImage = m_sImage;
				if( !DrawImage(ctx, (LPCTSTR)m_sLastImage) ) {}
				return;
			}
		}
	}

	void CFadeButtonUI::OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID)
	{
		m_bFadeAlpha = (BYTE)((nCurFrame / (double)nTotalFrame) * 255);
		m_bFadeAlpha = m_bFadeAlpha == 0 ? 10 : m_bFadeAlpha;
		Invalidate();
	}

} // namespace DuiLib