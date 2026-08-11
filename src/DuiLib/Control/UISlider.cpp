#include "StdAfx.h"
#include "UISlider.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CSliderUI)
		CSliderUI::CSliderUI() : m_uButtonState(0), m_nStep(1),m_bSendMove(false)
	{
		m_uTextStyle = DT_SINGLELINE | DT_CENTER;
		m_szThumb.cx = m_szThumb.cy = 10;
	}

	LPCTSTR CSliderUI::GetClass() const
	{
		return _T("SliderUI");
	}

	UINT CSliderUI::GetControlFlags() const
	{
		if( IsEnabled() ) return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
		else return 0;
	}

	LPVOID CSliderUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_SLIDER) == 0 ) return static_cast<CSliderUI*>(this);
		return CProgressUI::GetInterface(pstrName);
	}

	void CSliderUI::SetEnabled(bool bEnable)
	{
		CControlUI::SetEnabled(bEnable);
		if( !IsEnabled() ) {
			m_uButtonState = 0;
		}
	}

	int CSliderUI::GetChangeStep()
	{
		return m_nStep;
	}

	void CSliderUI::SetChangeStep(int step)
	{
		m_nStep = (step > 0) ? step : 1;
	}

	void CSliderUI::SetThumbSize(SIZE szXY)
	{
		m_szThumb = szXY;
	}

	RECT CSliderUI::GetThumbRect() const
	{
		RECT rcThumb = {0};
		SIZE m_szThumb = CSliderUI::m_szThumb;
		if (GetManager() != NULL) {
			GetManager()->GetDPIObj()->Scale(&m_szThumb);
		}
		if( m_bHorizontal ) {
			int left = m_rcItem.left + (m_rcItem.right - m_rcItem.left - m_szThumb.cx) * (m_nValue - m_nMin) / (m_nMax - m_nMin);
			int top = (m_rcItem.bottom + m_rcItem.top - m_szThumb.cy) / 2;
			rcThumb = CDuiRect(left, top, left + m_szThumb.cx, top + m_szThumb.cy); 
		}
		else {
			int left = (m_rcItem.right + m_rcItem.left - m_szThumb.cx) / 2;
			int top = m_rcItem.bottom - m_szThumb.cy - (m_rcItem.bottom - m_rcItem.top - m_szThumb.cy) * (m_nValue - m_nMin) / (m_nMax - m_nMin);
			rcThumb = CDuiRect(left, top, left + m_szThumb.cx, top + m_szThumb.cy); 
		}
		if(m_pManager != NULL) {
			//m_pManager->GetDPIObj()->Scale(&rcThumb);
		}
		return rcThumb;
	}

	LPCTSTR CSliderUI::GetThumbImage() const
	{
		return m_sThumbImage.GetData();
	}

	void CSliderUI::SetThumbImage(LPCTSTR pStrImage)
	{
		m_sThumbImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CSliderUI::GetThumbHoverImage() const
	{
		return m_sThumbHoverImage.GetData();
	}

	void CSliderUI::SetThumbHoverImage(LPCTSTR pStrImage)
	{
		m_sThumbHoverImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CSliderUI::GetThumbActiveImage() const
	{
		return m_sThumbActiveImage.GetData();
	}

	void CSliderUI::SetThumbActiveImage(LPCTSTR pStrImage)
	{
		m_sThumbActiveImage = pStrImage;
		Invalidate();
	}

	void CSliderUI::SetValue(int nValue)
	{
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) return;
		CProgressUI::SetValue(nValue);
	}

	void CSliderUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( !BubbleEvent(event) ) CProgressUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK ) {
			if( IsEnabled() ) {
				SetFocus();
				m_uButtonState |= UISTATE_CAPTURED;

				int nValue;
				if( m_bHorizontal ) {
					if( event.ptMouse.x >= m_rcItem.right - m_szThumb.cx / 2 ) nValue = m_nMax;
					else if( event.ptMouse.x <= m_rcItem.left + m_szThumb.cx / 2 ) nValue = m_nMin;
					else nValue = m_nMin + 1.0f * (m_nMax - m_nMin) * (event.ptMouse.x - m_rcItem.left - m_szThumb.cx / 2 ) / (m_rcItem.right - m_rcItem.left - m_szThumb.cx);
				}
				else {
					if( event.ptMouse.y >= m_rcItem.bottom - m_szThumb.cy / 2 ) nValue = m_nMin;
					else if( event.ptMouse.y <= m_rcItem.top + m_szThumb.cy / 2  ) nValue = m_nMax;
					else nValue = m_nMin + 1.0f * (m_nMax - m_nMin) * (m_rcItem.bottom - event.ptMouse.y - m_szThumb.cy / 2 ) / (m_rcItem.bottom - m_rcItem.top - m_szThumb.cy);
				}
				if(m_nValue != nValue && nValue >= m_nMin && nValue <= m_nMax) {
					m_nValue = nValue;
					Invalidate();
				}
				UpdateText();
			}
			return;
		}

		if( event.Type == UIEVENT_KEYDOWN && IsEnabled() ) {
			const int nOld = GetValue();
			const int nStep = GetChangeStep() > 0 ? GetChangeStep() : 1;
			int nNew = nOld;
			bool bHandled = false;
			if( event.chKey == VK_HOME ) {
				nNew = m_nMin;
				bHandled = true;
			}
			else if( event.chKey == VK_END ) {
				nNew = m_nMax;
				bHandled = true;
			}
			else if( m_bHorizontal ) {
				if( event.chKey == VK_LEFT || event.chKey == VK_DOWN ) {
					nNew = nOld - nStep;
					bHandled = true;
				}
				else if( event.chKey == VK_RIGHT || event.chKey == VK_UP ) {
					nNew = nOld + nStep;
					bHandled = true;
				}
			}
			else {
				// 竖向：上增下减（与常见音量条一致）
				if( event.chKey == VK_DOWN || event.chKey == VK_LEFT ) {
					nNew = nOld - nStep;
					bHandled = true;
				}
				else if( event.chKey == VK_UP || event.chKey == VK_RIGHT ) {
					nNew = nOld + nStep;
					bHandled = true;
				}
			}
			if( bHandled ) {
				if( nNew < m_nMin ) nNew = m_nMin;
				if( nNew > m_nMax ) nNew = m_nMax;
				if( nNew != nOld ) {
					// 勿走 SetValue：捕获态会吞掉；键盘路径直接改值
					m_nValue = nNew;
					UpdateText();
					Invalidate();
					if( m_pManager != NULL )
						m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
				}
				return;
			}
		}

		if( event.Type == UIEVENT_BUTTONUP || event.Type == UIEVENT_RBUTTONUP) {
			if( IsEnabled() ) {
				int nValue = 0;
				if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
					m_uButtonState &= ~UISTATE_CAPTURED;
				}
				if( m_bHorizontal ) {
					if( event.ptMouse.x >= m_rcItem.right - m_szThumb.cx / 2 ) nValue = m_nMax;
					else if( event.ptMouse.x <= m_rcItem.left + m_szThumb.cx / 2 ) nValue = m_nMin;
					else nValue = m_nMin + (m_nMax - m_nMin) * (event.ptMouse.x - m_rcItem.left - m_szThumb.cx / 2 ) / (m_rcItem.right - m_rcItem.left - m_szThumb.cx);
				}
				else {
					if( event.ptMouse.y >= m_rcItem.bottom - m_szThumb.cy / 2 ) nValue = m_nMin;
					else if( event.ptMouse.y <= m_rcItem.top + m_szThumb.cy / 2  ) nValue = m_nMax;
					else nValue = m_nMin + (m_nMax - m_nMin) * (m_rcItem.bottom - event.ptMouse.y - m_szThumb.cy / 2 ) / (m_rcItem.bottom - m_rcItem.top - m_szThumb.cy);
				}
				if(nValue >= m_nMin && nValue <= m_nMax) {
					m_nValue =nValue;
					m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
					Invalidate();
				}
				UpdateText();
				return;
			}
		}
		if( event.Type == UIEVENT_CONTEXTMENU )
		{
			return;
		}
		if( event.Type == UIEVENT_SCROLLWHEEL ) 
		{
			if( IsEnabled() ) {
				switch( LOWORD(event.wParam) ) {
				case SB_LINEUP:
					SetValue(GetValue() + GetChangeStep());
					m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
					return;
				case SB_LINEDOWN:
					SetValue(GetValue() - GetChangeStep());
					m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
					return;
				}
			}
		}
		if( event.Type == UIEVENT_MOUSEMOVE ) {
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
				if( m_bHorizontal ) {
					if( event.ptMouse.x >= m_rcItem.right - m_szThumb.cx / 2 ) m_nValue = m_nMax;
					else if( event.ptMouse.x <= m_rcItem.left + m_szThumb.cx / 2 ) m_nValue = m_nMin;
					else m_nValue = m_nMin + 1.0f * (m_nMax - m_nMin) * (event.ptMouse.x - m_rcItem.left - m_szThumb.cx / 2 ) / (m_rcItem.right - m_rcItem.left - m_szThumb.cx);
				}
				else {
					if( event.ptMouse.y >= m_rcItem.bottom - m_szThumb.cy / 2 ) m_nValue = m_nMin;
					else if( event.ptMouse.y <= m_rcItem.top + m_szThumb.cy / 2  ) m_nValue = m_nMax;
					else m_nValue = m_nMin + 1.0f * (m_nMax - m_nMin) * (m_rcItem.bottom - event.ptMouse.y - m_szThumb.cy / 2 ) / (m_rcItem.bottom - m_rcItem.top - m_szThumb.cy);
				}
				if (m_bSendMove) {
					UpdateText();
					m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED_MOVE);
				}
				Invalidate();
			}

			POINT pt = event.ptMouse;
			RECT rcThumb = GetThumbRect();
			if( IsEnabled() && ::PtInRect(&rcThumb, event.ptMouse) ) {
				m_uButtonState |= UISTATE_HOT;
				Invalidate();
			}
			else {
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_SETCURSOR )
		{
			RECT rcThumb = GetThumbRect();
			if( IsEnabled()) {
				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
				return;
			}
		}
		if( event.Type == UIEVENT_MOUSELEAVE )
		{
			if( IsEnabled() ) {
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		CControlUI::DoEvent(event);
	}

	void CSliderUI::SetCanSendMove(bool bCanSend)
	{
		m_bSendMove = bCanSend;
	}
	bool CSliderUI::GetCanSendMove() const
	{
		return m_bSendMove;
	}

	void CSliderUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("thumb-image")) == 0 ) SetThumbImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("thumb-image-hover")) == 0 ) SetThumbHoverImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("thumb-image-active")) == 0 ) SetThumbActiveImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("thumb-size")) == 0 ) {
			SIZE szXY = {0};
			LPTSTR pstr = NULL;
			szXY.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
			szXY.cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr); 
			SetThumbSize(szXY);
		}
		else if( _tcsicmp(pstrName, _T("step")) == 0 || _tcsicmp(pstrName, _T("change-step")) == 0 ) {
			SetChangeStep(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("send-move")) == 0 ) {
			SetCanSendMove(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else CProgressUI::SetAttribute(pstrName, pstrValue);
	}

	void CSliderUI::PaintForegroundImage(IRenderContext& ctx)
	{
		CProgressUI::PaintForegroundImage(ctx);

		RECT rcThumb = GetThumbRect();
		rcThumb.left -= m_rcItem.left;
		rcThumb.top -= m_rcItem.top;
		rcThumb.right -= m_rcItem.left;
		rcThumb.bottom -= m_rcItem.top;

		GetManager()->GetDPIObj()->ScaleBack(&rcThumb);

		if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
			if( !m_sThumbActiveImage.IsEmpty() ) {
				m_sImageModify.Empty();
				m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
				if( !DrawImage(ctx, m_sThumbActiveImage.GetData(), m_sImageModify.GetData()) ) {}
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if( !m_sThumbHoverImage.IsEmpty() ) {
				m_sImageModify.Empty();
				m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
				if( !DrawImage(ctx, m_sThumbHoverImage.GetData(), m_sImageModify.GetData()) ) {}
				else return;
			}
		}

		if( !m_sThumbImage.IsEmpty() ) {
			m_sImageModify.Empty();
			m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
			if( !DrawImage(ctx, m_sThumbImage.GetData(), m_sImageModify.GetData()) ) {}
			else return;
		}
	}
}
