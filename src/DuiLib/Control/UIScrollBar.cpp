#include "StdAfx.h"
#include "UIScrollBar.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CScrollBarUI)

	CScrollBarUI::CScrollBarUI() : m_bHorizontal(false), m_nRange(0), m_nScrollPos(0), m_nLineSize(8), 
		m_nThumbMinSize(DEFAULT_THUMB_MIN_SIZE),
		m_pOwner(NULL), m_nLastScrollPos(0), m_nLastScrollOffset(0), m_nScrollRepeatDelay(0), m_uButtonPrevState(0), \
		m_uButtonNextState(0), m_uThumbState(0), m_bShowButtonPrev(false), m_bShowButtonNext(false), m_bShow(true),
		m_dwThumbColor(0), m_dwThumbHoverColor(0), m_dwThumbActiveColor(0), m_dwThumbDisabledColor(0)
	{
		m_cxyFixed.cx = DEFAULT_SCROLLBAR_SIZE;
		m_ptLastMouse.x = m_ptLastMouse.y = 0;
		::ZeroMemory(&m_rcThumb, sizeof(m_rcThumb));
		::ZeroMemory(&m_rcButtonPrev, sizeof(m_rcButtonPrev));
		::ZeroMemory(&m_rcButtonNext, sizeof(m_rcButtonNext));
		// 浅灰轨道；无图时配合实心圆角滑块
		SetBackgroundColor(0xEDEDF0FF);
	}

	LPCTSTR CScrollBarUI::GetClass() const
	{
		return _T("ScrollBarUI");
	}

	LPVOID CScrollBarUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_SCROLLBAR) == 0 ) return static_cast<CScrollBarUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	bool CScrollBarUI::PreferClientHit() const
	{
		return IsEnabled();
	}

	CContainerUI* CScrollBarUI::GetOwner() const
	{
		return m_pOwner;
	}

	void CScrollBarUI::SetOwner(CContainerUI* pOwner)
	{
		m_pOwner = pOwner;
	}

	void CScrollBarUI::SetVisible(bool bVisible)
	{
		if( m_bVisible == bVisible ) return;

		bool v = IsVisible();
		m_bVisible = bVisible;
		if( m_bFocused ) m_bFocused = false;

	}

	void CScrollBarUI::SetEnabled(bool bEnable)
	{
		CControlUI::SetEnabled(bEnable);
		if( !IsEnabled() ) {
			m_uButtonPrevState = 0;
			m_uButtonNextState = 0;
			m_uThumbState = 0;
		}
	}

	void CScrollBarUI::SetFocus()
	{
		if( m_pOwner != NULL ) m_pOwner->SetFocus();
		else CControlUI::SetFocus();
	}

	bool CScrollBarUI::IsHorizontal()
	{
		return m_bHorizontal;
	}

	void CScrollBarUI::SetHorizontal(bool bHorizontal)
	{
		if( m_bHorizontal == bHorizontal ) return;

		m_bHorizontal = bHorizontal;
		if( m_bHorizontal ) {
			if( m_cxyFixed.cy == 0 ) {
				m_cxyFixed.cx = 0;
				m_cxyFixed.cy = DEFAULT_SCROLLBAR_SIZE;
			}
		}
		else {
			if( m_cxyFixed.cx == 0 ) {
				m_cxyFixed.cx = DEFAULT_SCROLLBAR_SIZE;
				m_cxyFixed.cy = 0;
			}
		}

		if( m_pOwner != NULL ) m_pOwner->NeedUpdate(); else NeedParentUpdate();
	}

	int CScrollBarUI::GetScrollRange() const
	{
		return m_nRange;
	}

	void CScrollBarUI::SetScrollRange(int nRange)
	{
		if( m_nRange == nRange ) return;

		m_nRange = nRange;
		if( m_nRange < 0 ) m_nRange = 0;
		if( m_nScrollPos > m_nRange ) m_nScrollPos = m_nRange;
		SetPos(m_rcItem);
	}

	int CScrollBarUI::GetScrollPos() const
	{
		return m_nScrollPos;
	}

	void CScrollBarUI::SetScrollPos(int nPos)
	{
		if( m_nScrollPos == nPos ) return;

		m_nScrollPos = nPos;
		if( m_nScrollPos < 0 ) m_nScrollPos = 0;
		if( m_nScrollPos > m_nRange ) m_nScrollPos = m_nRange;
		SetPos(m_rcItem);
	}

	int CScrollBarUI::GetLineSize() const
	{
		return m_nLineSize;
	}

	void CScrollBarUI::SetLineSize(int nSize)
	{
		m_nLineSize = nSize;
	}

	bool CScrollBarUI::GetShowButtonPrev()
	{
		return m_bShowButtonPrev;
	}

	void CScrollBarUI::SetShowButtonPrev(bool bShow)
	{
		m_bShowButtonPrev = bShow;
		SetPos(m_rcItem);
	}

	LPCTSTR CScrollBarUI::GetButtonPrevNormalImage()
	{
		return m_sButtonPrevNormalImage.GetData();
	}

	void CScrollBarUI::SetButtonPrevNormalImage(LPCTSTR pStrImage)
	{
		m_sButtonPrevNormalImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CScrollBarUI::GetButtonPrevHoverImage()
	{
		return m_sButtonPrevHoverImage.GetData();
	}

	void CScrollBarUI::SetButtonPrevHoverImage(LPCTSTR pStrImage)
	{
		m_sButtonPrevHoverImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CScrollBarUI::GetButtonPrevActiveImage()
	{
		return m_sButtonPrevActiveImage.GetData();
	}

	void CScrollBarUI::SetButtonPrevActiveImage(LPCTSTR pStrImage)
	{
		m_sButtonPrevActiveImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CScrollBarUI::GetButtonPrevDisabledImage()
	{
		return m_sButtonPrevDisabledImage.GetData();
	}

	void CScrollBarUI::SetButtonPrevDisabledImage(LPCTSTR pStrImage)
	{
		m_sButtonPrevDisabledImage = pStrImage;
		Invalidate();
	}

	bool CScrollBarUI::GetShowButtonNext()
	{
		return m_bShowButtonNext;
	}

	void CScrollBarUI::SetShowButtonNext(bool bShow)
	{
		m_bShowButtonNext = bShow;
		SetPos(m_rcItem);
	}

	LPCTSTR CScrollBarUI::GetButtonNextNormalImage()
	{
		return m_sButtonNextNormalImage.GetData();
	}

	void CScrollBarUI::SetButtonNextNormalImage(LPCTSTR pStrImage)
	{
		m_sButtonNextNormalImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CScrollBarUI::GetButtonNextHoverImage()
	{
		return m_sButtonNextHoverImage.GetData();
	}

	void CScrollBarUI::SetButtonNextHoverImage(LPCTSTR pStrImage)
	{
		m_sButtonNextHoverImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CScrollBarUI::GetButtonNextActiveImage()
	{
		return m_sButtonNextActiveImage.GetData();
	}

	void CScrollBarUI::SetButtonNextActiveImage(LPCTSTR pStrImage)
	{
		m_sButtonNextActiveImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CScrollBarUI::GetButtonNextDisabledImage()
	{
		return m_sButtonNextDisabledImage.GetData();
	}

	void CScrollBarUI::SetButtonNextDisabledImage(LPCTSTR pStrImage)
	{
		m_sButtonNextDisabledImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CScrollBarUI::GetThumbNormalImage()
	{
		return m_sThumbNormalImage.GetData();
	}

	void CScrollBarUI::SetThumbNormalImage(LPCTSTR pStrImage)
	{
		m_sThumbNormalImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CScrollBarUI::GetThumbHoverImage()
	{
		return m_sThumbHoverImage.GetData();
	}

	void CScrollBarUI::SetThumbHoverImage(LPCTSTR pStrImage)
	{
		m_sThumbHoverImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CScrollBarUI::GetThumbActiveImage()
	{
		return m_sThumbActiveImage.GetData();
	}

	void CScrollBarUI::SetThumbActiveImage(LPCTSTR pStrImage)
	{
		m_sThumbActiveImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CScrollBarUI::GetThumbDisabledImage()
	{
		return m_sThumbDisabledImage.GetData();
	}

	void CScrollBarUI::SetThumbDisabledImage(LPCTSTR pStrImage)
	{
		m_sThumbDisabledImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CScrollBarUI::GetRailNormalImage()
	{
		return m_sRailNormalImage.GetData();
	}

	void CScrollBarUI::SetRailNormalImage(LPCTSTR pStrImage)
	{
		m_sRailNormalImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CScrollBarUI::GetRailHoverImage()
	{
		return m_sRailHoverImage.GetData();
	}

	void CScrollBarUI::SetRailHoverImage(LPCTSTR pStrImage)
	{
		m_sRailHoverImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CScrollBarUI::GetRailActiveImage()
	{
		return m_sRailActiveImage.GetData();
	}

	void CScrollBarUI::SetRailActiveImage(LPCTSTR pStrImage)
	{
		m_sRailActiveImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CScrollBarUI::GetRailDisabledImage()
	{
		return m_sRailDisabledImage.GetData();
	}

	void CScrollBarUI::SetRailDisabledImage(LPCTSTR pStrImage)
	{
		m_sRailDisabledImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CScrollBarUI::GetBkNormalImage()
	{
		return m_sBkNormalImage.GetData();
	}

	void CScrollBarUI::SetBkNormalImage(LPCTSTR pStrImage)
	{
		m_sBkNormalImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CScrollBarUI::GetBkHoverImage()
	{
		return m_sBkHoverImage.GetData();
	}

	void CScrollBarUI::SetBkHoverImage(LPCTSTR pStrImage)
	{
		m_sBkHoverImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CScrollBarUI::GetBkActiveImage()
	{
		return m_sBkActiveImage.GetData();
	}

	void CScrollBarUI::SetBkActiveImage(LPCTSTR pStrImage)
	{
		m_sBkActiveImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CScrollBarUI::GetBkDisabledImage()
	{
		return m_sBkDisabledImage.GetData();
	}

	void CScrollBarUI::SetBkDisabledImage(LPCTSTR pStrImage)
	{
		m_sBkDisabledImage = pStrImage;
		Invalidate();
	}

	bool CScrollBarUI::GetShow()
	{
		return m_bShow;
	}

	void CScrollBarUI::SetShow(bool bShow)
	{
		m_bShow = bShow;
		Invalidate();
	}

	int CScrollBarUI::GetThumbMinSize() const
	{
		if( m_pManager != NULL ) return m_pManager->GetDPIObj()->Scale(m_nThumbMinSize);
		return m_nThumbMinSize;
	}

	void CScrollBarUI::SetThumbMinSize(int nSize)
	{
		if( nSize < 0 ) nSize = 0;
		if( m_nThumbMinSize == nSize ) return;
		m_nThumbMinSize = nSize;
		if( !::IsRectEmpty(&m_rcItem) ) SetPos(m_rcItem);
		else Invalidate();
	}

	void CScrollBarUI::SetThumbColor(DWORD dwColor)
	{
		if( m_dwThumbColor == dwColor ) return;
		m_dwThumbColor = dwColor;
		Invalidate();
	}

	DWORD CScrollBarUI::GetThumbColor() const
	{
		return m_dwThumbColor;
	}

	void CScrollBarUI::SetThumbHoverColor(DWORD dwColor)
	{
		if( m_dwThumbHoverColor == dwColor ) return;
		m_dwThumbHoverColor = dwColor;
		Invalidate();
	}

	DWORD CScrollBarUI::GetThumbHoverColor() const
	{
		return m_dwThumbHoverColor;
	}

	void CScrollBarUI::SetThumbActiveColor(DWORD dwColor)
	{
		if( m_dwThumbActiveColor == dwColor ) return;
		m_dwThumbActiveColor = dwColor;
		Invalidate();
	}

	DWORD CScrollBarUI::GetThumbActiveColor() const
	{
		return m_dwThumbActiveColor;
	}

	void CScrollBarUI::SetThumbDisabledColor(DWORD dwColor)
	{
		if( m_dwThumbDisabledColor == dwColor ) return;
		m_dwThumbDisabledColor = dwColor;
		Invalidate();
	}

	DWORD CScrollBarUI::GetThumbDisabledColor() const
	{
		return m_dwThumbDisabledColor;
	}

	void CScrollBarUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		SIZE cxyFixed = m_cxyFixed;
		if (m_pManager != NULL) {
			GetManager()->GetDPIObj()->Scale(&cxyFixed);
		}
		rc = m_rcItem;
		if( m_bHorizontal ) {
			int cx = rc.right - rc.left;
			if( m_bShowButtonPrev ) cx -= cxyFixed.cy;
			if( m_bShowButtonNext ) cx -= cxyFixed.cy;
			if( cx > cxyFixed.cy ) {
				m_rcButtonPrev.left = rc.left;
				m_rcButtonPrev.top = rc.top;
				if( m_bShowButtonPrev ) {
					m_rcButtonPrev.right = rc.left + cxyFixed.cy;
					m_rcButtonPrev.bottom = rc.top + cxyFixed.cy;
				}
				else {
					m_rcButtonPrev.right = m_rcButtonPrev.left;
					m_rcButtonPrev.bottom = m_rcButtonPrev.top;
				}

				m_rcButtonNext.top = rc.top;
				m_rcButtonNext.right = rc.right;
				if( m_bShowButtonNext ) {
					m_rcButtonNext.left = rc.right - cxyFixed.cy;
					m_rcButtonNext.bottom = rc.top + cxyFixed.cy;
				}
				else {
					m_rcButtonNext.left = m_rcButtonNext.right;
					m_rcButtonNext.bottom = m_rcButtonNext.top;
				}

				m_rcThumb.top = rc.top;
				m_rcThumb.bottom = rc.top + cxyFixed.cy;
				if( m_nRange > 0 ) {
					int cxThumb = cx * (rc.right - rc.left) / (m_nRange + rc.right - rc.left);
					int nMinThumb = GetThumbMinSize();
					if( nMinThumb < cxyFixed.cy ) nMinThumb = cxyFixed.cy;
					if( cxThumb < nMinThumb ) cxThumb = nMinThumb;
					if( cxThumb > cx ) cxThumb = cx;

					m_rcThumb.left = m_nScrollPos * (cx - cxThumb) / m_nRange + m_rcButtonPrev.right;
					m_rcThumb.right = m_rcThumb.left + cxThumb;
					if( m_rcThumb.right > m_rcButtonNext.left ) {
						m_rcThumb.left = m_rcButtonNext.left - cxThumb;
						m_rcThumb.right = m_rcButtonNext.left;
					}
				}
				else {
					m_rcThumb.left = m_rcButtonPrev.right;
					m_rcThumb.right = m_rcButtonNext.left;
				}
			}
			else {
				int cxButton = (rc.right - rc.left) / 2;
				if( cxButton > cxyFixed.cy ) cxButton = cxyFixed.cy;
				m_rcButtonPrev.left = rc.left;
				m_rcButtonPrev.top = rc.top;
				if( m_bShowButtonPrev ) {
					m_rcButtonPrev.right = rc.left + cxButton;
					m_rcButtonPrev.bottom = rc.top + cxyFixed.cy;
				}
				else {
					m_rcButtonPrev.right = m_rcButtonPrev.left;
					m_rcButtonPrev.bottom = m_rcButtonPrev.top;
				}

				m_rcButtonNext.top = rc.top;
				m_rcButtonNext.right = rc.right;
				if( m_bShowButtonNext ) {
					m_rcButtonNext.left = rc.right - cxButton;
					m_rcButtonNext.bottom = rc.top + cxyFixed.cy;
				}
				else {
					m_rcButtonNext.left = m_rcButtonNext.right;
					m_rcButtonNext.bottom = m_rcButtonNext.top;
				}

				::ZeroMemory(&m_rcThumb, sizeof(m_rcThumb));
			}
		}
		else {
			int cy = rc.bottom - rc.top;
			if( m_bShowButtonPrev ) cy -= cxyFixed.cx;
			if( m_bShowButtonNext ) cy -= cxyFixed.cx;
			if( cy > cxyFixed.cx ) {
				m_rcButtonPrev.left = rc.left;
				m_rcButtonPrev.top = rc.top;
				if( m_bShowButtonPrev ) {
					m_rcButtonPrev.right = rc.left + cxyFixed.cx;
					m_rcButtonPrev.bottom = rc.top + cxyFixed.cx;
				}
				else {
					m_rcButtonPrev.right = m_rcButtonPrev.left;
					m_rcButtonPrev.bottom = m_rcButtonPrev.top;
				}

				m_rcButtonNext.left = rc.left;
				m_rcButtonNext.bottom = rc.bottom;
				if( m_bShowButtonNext ) {
					m_rcButtonNext.top = rc.bottom - cxyFixed.cx;
					m_rcButtonNext.right = rc.left + cxyFixed.cx;
				}
				else {
					m_rcButtonNext.top = m_rcButtonNext.bottom;
					m_rcButtonNext.right = m_rcButtonNext.left;
				}

				m_rcThumb.left = rc.left;
				m_rcThumb.right = rc.left + cxyFixed.cx;
				if( m_nRange > 0 ) {
					int cyThumb = cy * (rc.bottom - rc.top) / (m_nRange + rc.bottom - rc.top);
					int nMinThumb = GetThumbMinSize();
					if( nMinThumb < cxyFixed.cx ) nMinThumb = cxyFixed.cx;
					if( cyThumb < nMinThumb ) cyThumb = nMinThumb;
					if( cyThumb > cy ) cyThumb = cy;

					m_rcThumb.top = (m_nScrollPos * 1.0f / m_nRange) * (cy - cyThumb) + m_rcButtonPrev.bottom;
					m_rcThumb.bottom = m_rcThumb.top + cyThumb;
					if( m_rcThumb.bottom > m_rcButtonNext.top ) {
						m_rcThumb.top = m_rcButtonNext.top - cyThumb;
						m_rcThumb.bottom = m_rcButtonNext.top;
					}
				}
				else {
					m_rcThumb.top = m_rcButtonPrev.bottom;
					m_rcThumb.bottom = m_rcButtonNext.top;
				}
			}
			else {
				int cyButton = (rc.bottom - rc.top) / 2;
				if( cyButton > cxyFixed.cx ) cyButton = cxyFixed.cx;
				m_rcButtonPrev.left = rc.left;
				m_rcButtonPrev.top = rc.top;
				if( m_bShowButtonPrev ) {
					m_rcButtonPrev.right = rc.left + cxyFixed.cx;
					m_rcButtonPrev.bottom = rc.top + cyButton;
				}
				else {
					m_rcButtonPrev.right = m_rcButtonPrev.left;
					m_rcButtonPrev.bottom = m_rcButtonPrev.top;
				}

				m_rcButtonNext.left = rc.left;
				m_rcButtonNext.bottom = rc.bottom;
				if( m_bShowButtonNext ) {
					m_rcButtonNext.top = rc.bottom - cyButton;
					m_rcButtonNext.right = rc.left + cxyFixed.cx;
				}
				else {
					m_rcButtonNext.top = m_rcButtonNext.bottom;
					m_rcButtonNext.right = m_rcButtonNext.left;
				}

				::ZeroMemory(&m_rcThumb, sizeof(m_rcThumb));
			}
		}
	}

	void CScrollBarUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
			else CControlUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETFOCUS ) 
		{
			return;
		}
		if( event.Type == UIEVENT_KILLFOCUS ) 
		{
			return;
		}
		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
		{
			if( !IsEnabled() ) return;

			m_nLastScrollOffset = 0;
			m_nScrollRepeatDelay = 0;

			if( ::PtInRect(&m_rcButtonPrev, event.ptMouse) ) {
				m_uButtonPrevState |= UISTATE_PUSHED;
				if( !m_bHorizontal ) {
					if( m_pOwner != NULL ) m_pOwner->LineUp(); 
					else SetScrollPos(m_nScrollPos - m_nLineSize);
				}
				else {
					if( m_pOwner != NULL ) m_pOwner->LineLeft(); 
					else SetScrollPos(m_nScrollPos - m_nLineSize);
				}
			}
			else if( ::PtInRect(&m_rcButtonNext, event.ptMouse) ) {
				m_uButtonNextState |= UISTATE_PUSHED;
				if( !m_bHorizontal ) {
					if( m_pOwner != NULL ) m_pOwner->LineDown(); 
					else SetScrollPos(m_nScrollPos + m_nLineSize);
				}
				else {
					if( m_pOwner != NULL ) m_pOwner->LineRight(); 
					else SetScrollPos(m_nScrollPos + m_nLineSize);
				}
			}
			else if( ::PtInRect(&m_rcThumb, event.ptMouse) ) {
				m_uThumbState |= UISTATE_CAPTURED | UISTATE_PUSHED;
				m_ptLastMouse = event.ptMouse;
				m_nLastScrollPos = m_nScrollPos;
				
				m_pManager->SetTimer(this, DEFAULT_TIMERID, 50U);
			}
			else {
				if( !m_bHorizontal ) {
					if( event.ptMouse.y < m_rcThumb.top ) {
						if( m_pOwner != NULL ) m_pOwner->PageUp(); 
						else SetScrollPos(m_nScrollPos + m_rcItem.top - m_rcItem.bottom);
					}
					else if ( event.ptMouse.y > m_rcThumb.bottom ){
						if( m_pOwner != NULL ) m_pOwner->PageDown(); 
						else SetScrollPos(m_nScrollPos - m_rcItem.top + m_rcItem.bottom);                    
					}
				}
				else {
					if( event.ptMouse.x < m_rcThumb.left ) {
						if( m_pOwner != NULL ) m_pOwner->PageLeft(); 
						else SetScrollPos(m_nScrollPos + m_rcItem.left - m_rcItem.right);
					}
					else if ( event.ptMouse.x > m_rcThumb.right ){
						if( m_pOwner != NULL ) m_pOwner->PageRight(); 
						else SetScrollPos(m_nScrollPos - m_rcItem.left + m_rcItem.right);                    
					}
				}
			}
			if( m_pManager != NULL) m_pManager->SendNotify(this, DUI_MSGTYPE_SCROLL);
			return;
		}
		if( event.Type == UIEVENT_BUTTONUP )
		{
			m_nScrollRepeatDelay = 0;
			m_nLastScrollOffset = 0;
			m_pManager->KillTimer(this, DEFAULT_TIMERID);

			if( (m_uThumbState & UISTATE_CAPTURED) != 0 ) {
				m_uThumbState &= ~( UISTATE_CAPTURED | UISTATE_PUSHED );
				Invalidate();
			}
			else if( (m_uButtonPrevState & UISTATE_PUSHED) != 0 ) {
				m_uButtonPrevState &= ~UISTATE_PUSHED;
				Invalidate();
			}
			else if( (m_uButtonNextState & UISTATE_PUSHED) != 0 ) {
				m_uButtonNextState &= ~UISTATE_PUSHED;
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEMOVE )
		{
			if( (m_uThumbState & UISTATE_CAPTURED) != 0 ) {
				if( !m_bHorizontal ) {
					__int64 fMouseRange = (event.ptMouse.y - m_ptLastMouse.y) * m_nRange;
					int nBtnSize = 0;
					if(GetShowButtonPrev()) nBtnSize += m_cxyFixed.cx;
					if(GetShowButtonNext()) nBtnSize += m_cxyFixed.cx;
					int vRange = m_rcItem.bottom - m_rcItem.top - (m_rcThumb.bottom - m_rcThumb.top) - nBtnSize;
					if (vRange != 0){
						m_nLastScrollOffset = fMouseRange / abs(vRange);
					}
				}
				else {
					__int64 fMouseRange = (event.ptMouse.x - m_ptLastMouse.x) * m_nRange;
					int nBtnSize = 0;
					if(GetShowButtonPrev()) nBtnSize += m_cxyFixed.cy;
					if(GetShowButtonNext()) nBtnSize += m_cxyFixed.cy;
					int hRange = m_rcItem.right - m_rcItem.left - m_rcThumb.right + m_rcThumb.left - nBtnSize;
					if (hRange != 0) m_nLastScrollOffset = fMouseRange / abs(hRange);
				}
			}
			else {
				if( (m_uThumbState & UISTATE_HOT) != 0 ) {
					if( !::PtInRect(&m_rcThumb, event.ptMouse) ) {
						m_uThumbState &= ~UISTATE_HOT;
						Invalidate();
					}
				}
				else {
					if( !IsEnabled() ) return;
					if( ::PtInRect(&m_rcThumb, event.ptMouse) ) {
						m_uThumbState |= UISTATE_HOT;
						Invalidate();
					}
				}
			}
			return;
		}
		if( event.Type == UIEVENT_CONTEXTMENU )
		{
			return;
		}
		if( event.Type == UIEVENT_TIMER && event.wParam == DEFAULT_TIMERID )
		{
			++m_nScrollRepeatDelay;
			if( (m_uThumbState & UISTATE_CAPTURED) != 0 ) {
				if( !m_bHorizontal ) {
					if( m_pOwner != NULL ) m_pOwner->SetScrollPos(CDuiSize(m_pOwner->GetScrollPos().cx, \
						m_nLastScrollPos + m_nLastScrollOffset)); 
					else SetScrollPos(m_nLastScrollPos + m_nLastScrollOffset);
				}
				else {
					if( m_pOwner != NULL ) m_pOwner->SetScrollPos(CDuiSize(m_nLastScrollPos + m_nLastScrollOffset, \
						m_pOwner->GetScrollPos().cy)); 
					else SetScrollPos(m_nLastScrollPos + m_nLastScrollOffset);
				}
				Invalidate();
			}
			else if( (m_uButtonPrevState & UISTATE_PUSHED) != 0 ) {
				if( m_nScrollRepeatDelay <= 5 ) return;
				if( !m_bHorizontal ) {
					if( m_pOwner != NULL ) m_pOwner->LineUp(); 
					else SetScrollPos(m_nScrollPos - m_nLineSize);
				}
				else {
					if( m_pOwner != NULL ) m_pOwner->LineLeft(); 
					else SetScrollPos(m_nScrollPos - m_nLineSize);
				}
			}
			else if( (m_uButtonNextState & UISTATE_PUSHED) != 0 ) {
				if( m_nScrollRepeatDelay <= 5 ) return;
				if( !m_bHorizontal ) {
					if( m_pOwner != NULL ) m_pOwner->LineDown(); 
					else SetScrollPos(m_nScrollPos + m_nLineSize);
				}
				else {
					if( m_pOwner != NULL ) m_pOwner->LineRight(); 
					else SetScrollPos(m_nScrollPos + m_nLineSize);
				}
			}
			else {
				if( m_nScrollRepeatDelay <= 5 ) return;
				POINT pt = { 0 };
				::GetCursorPos(&pt);
				::ScreenToClient(m_pManager->GetPaintWindow(), &pt);
				if( !m_bHorizontal ) {
					if( pt.y < m_rcThumb.top ) {
						if( m_pOwner != NULL ) m_pOwner->PageUp(); 
						else SetScrollPos(m_nScrollPos + m_rcItem.top - m_rcItem.bottom);
					}
					else if ( pt.y > m_rcThumb.bottom ){
						if( m_pOwner != NULL ) m_pOwner->PageDown(); 
						else SetScrollPos(m_nScrollPos - m_rcItem.top + m_rcItem.bottom);                    
					}
				}
				else {
					if( pt.x < m_rcThumb.left ) {
						if( m_pOwner != NULL ) m_pOwner->PageLeft(); 
						else SetScrollPos(m_nScrollPos + m_rcItem.left - m_rcItem.right);
					}
					else if ( pt.x > m_rcThumb.right ){
						if( m_pOwner != NULL ) m_pOwner->PageRight(); 
						else SetScrollPos(m_nScrollPos - m_rcItem.left + m_rcItem.right);                    
					}
				}
			}
			if( m_pManager != NULL ) m_pManager->SendNotify(this, DUI_MSGTYPE_SCROLL);
			return;
		}
		if( event.Type == UIEVENT_MOUSEENTER )
		{
			if( IsEnabled() ) {
				m_uButtonPrevState |= UISTATE_HOT;
				m_uButtonNextState |= UISTATE_HOT;
				if( ::PtInRect(&m_rcThumb, event.ptMouse) ) m_uThumbState |= UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSELEAVE )
		{
			if( IsEnabled() ) {
				m_uButtonPrevState &= ~UISTATE_HOT;
				m_uButtonNextState &= ~UISTATE_HOT;
				m_uThumbState &= ~UISTATE_HOT;
				Invalidate();
			}
			return;
		}

		if( m_pOwner != NULL ) m_pOwner->DoEvent(event); else CControlUI::DoEvent(event);
	}

	void CScrollBarUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("button-prev-image")) == 0 ) SetButtonPrevNormalImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("button-prev-image-hover")) == 0 ) SetButtonPrevHoverImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("button-prev-image-active")) == 0 ) SetButtonPrevActiveImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("button-prev-image-disabled")) == 0 ) SetButtonPrevDisabledImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("button-next-image")) == 0 ) SetButtonNextNormalImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("button-next-image-hover")) == 0 ) SetButtonNextHoverImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("button-next-image-active")) == 0 ) SetButtonNextActiveImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("button-next-image-disabled")) == 0 ) SetButtonNextDisabledImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("thumb-image")) == 0 ) SetThumbNormalImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("thumb-image-hover")) == 0 ) SetThumbHoverImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("thumb-image-active")) == 0 ) SetThumbActiveImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("thumb-image-disabled")) == 0 ) SetThumbDisabledImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("rail-image")) == 0 ) SetRailNormalImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("rail-image-hover")) == 0 ) SetRailHoverImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("rail-image-active")) == 0 ) SetRailActiveImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("rail-image-disabled")) == 0 ) SetRailDisabledImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("background-image")) == 0 ) SetBkNormalImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("background-image-hover")) == 0 ) SetBkHoverImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("background-image-active")) == 0 ) SetBkActiveImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("background-image-disabled")) == 0 ) SetBkDisabledImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("horizontal")) == 0 ) SetHorizontal(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("line-size")) == 0 ) SetLineSize(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("range")) == 0 ) SetScrollRange(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("value")) == 0 ) SetScrollPos(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("show-button-prev")) == 0 ) SetShowButtonPrev(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("show-button-next")) == 0 ) SetShowButtonNext(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("thumb-min-size")) == 0 ) SetThumbMinSize(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("thumb-color")) == 0 ) {
			DWORD c = 0; if( ParseColorString(pstrValue, c) ) SetThumbColor(c);
		}
		else if( _tcsicmp(pstrName, _T("thumb-color-hover")) == 0 ) {
			DWORD c = 0; if( ParseColorString(pstrValue, c) ) SetThumbHoverColor(c);
		}
		else if( _tcsicmp(pstrName, _T("thumb-color-active")) == 0 ) {
			DWORD c = 0; if( ParseColorString(pstrValue, c) ) SetThumbActiveColor(c);
		}
		else if( _tcsicmp(pstrName, _T("thumb-color-disabled")) == 0 ) {
			DWORD c = 0; if( ParseColorString(pstrValue, c) ) SetThumbDisabledColor(c);
		}
		else CControlUI::SetAttribute(pstrName, pstrValue);
	}

	bool CScrollBarUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		if(!GetShow()) return true;

		PaintBackgroundColor(ctx);
		PaintBackgroundImage(ctx);
		PaintBk(ctx);
		PaintButtonPrev(ctx);
		PaintButtonNext(ctx);
		PaintThumb(ctx);
		PaintRail(ctx);
		PaintBorder(ctx);
		return true;
	}

	void CScrollBarUI::PaintBk(IRenderContext& ctx)
	{
		if( !IsEnabled() ) m_uThumbState |= UISTATE_DISABLED;
		else m_uThumbState &= ~ UISTATE_DISABLED;

		if( (m_uThumbState & UISTATE_DISABLED) != 0 ) {
			if( !m_sBkDisabledImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sBkDisabledImage.GetData()) ) {}
				else return;
			}
		}
		else if( (m_uThumbState & UISTATE_PUSHED) != 0 ) {
			if( !m_sBkActiveImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sBkActiveImage.GetData()) ) {}
				else return;
			}
		}
		else if( (m_uThumbState & UISTATE_HOT) != 0 ) {
			if( !m_sBkHoverImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sBkHoverImage.GetData()) ) {}
				else return;
			}
		}

		if( !m_sBkNormalImage.IsEmpty() ) {
			if( !DrawImage(ctx, m_sBkNormalImage.GetData()) ) {}
			else return;
		}
	}

	void CScrollBarUI::PaintButtonPrev(IRenderContext& ctx)
	{
		if( !m_bShowButtonPrev ) return;

		if( !IsEnabled() ) m_uButtonPrevState |= UISTATE_DISABLED;
		else m_uButtonPrevState &= ~ UISTATE_DISABLED;

		int d1 = MulDiv(m_rcButtonPrev.left - m_rcItem.left, 100, GetManager()->GetDPIObj()->GetScale());
		int d2 = MulDiv(m_rcButtonPrev.top - m_rcItem.top, 100, GetManager()->GetDPIObj()->GetScale());
		int d3 = MulDiv(m_rcButtonPrev.right - m_rcItem.left, 100, GetManager()->GetDPIObj()->GetScale());
		int d4 = MulDiv(m_rcButtonPrev.bottom - m_rcItem.top, 100, GetManager()->GetDPIObj()->GetScale());
		m_sImageModify.Empty();
		m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), d1, d2, d3, d4);

		if( (m_uButtonPrevState & UISTATE_DISABLED) != 0 ) {
			if( !m_sButtonPrevDisabledImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sButtonPrevDisabledImage.GetData(), m_sImageModify.GetData()) ) {}
				else return;
			}
		}
		else if( (m_uButtonPrevState & UISTATE_PUSHED) != 0 ) {
			if( !m_sButtonPrevActiveImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sButtonPrevActiveImage.GetData(), m_sImageModify.GetData()) ) {}
				else return;
			}
		}
		else if( (m_uButtonPrevState & UISTATE_HOT) != 0 ) {
			if( !m_sButtonPrevHoverImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sButtonPrevHoverImage.GetData(), m_sImageModify.GetData()) ) {}
				else return;
			}
		}

		if( !m_sButtonPrevNormalImage.IsEmpty() ) {
			if( !DrawImage(ctx, m_sButtonPrevNormalImage.GetData(), m_sImageModify.GetData()) ) {}
			else return;
		}

		// 无图：跟 thumb chrome 色（未设时回退浅灰）
		DWORD dwColor = (m_dwThumbColor != 0) ? m_dwThumbColor : 0xB8B8C0FF;
		if( (m_uButtonPrevState & UISTATE_DISABLED) != 0 )
			dwColor = (m_dwThumbDisabledColor != 0) ? m_dwThumbDisabledColor : 0xD8D8DCFF;
		else if( (m_uButtonPrevState & UISTATE_PUSHED) != 0 )
			dwColor = (m_dwThumbActiveColor != 0) ? m_dwThumbActiveColor : 0x808088FF;
		else if( (m_uButtonPrevState & UISTATE_HOT) != 0 )
			dwColor = (m_dwThumbHoverColor != 0) ? m_dwThumbHoverColor : 0x9A9AA2FF;
		RECT rc = m_rcButtonPrev;
		int pad = 3;
		rc.left += pad; rc.top += pad; rc.right -= pad; rc.bottom -= pad;
		if( rc.right > rc.left && rc.bottom > rc.top )
			ctx.FillRoundRect(rc, 2, 2, dwColor);
	}

	void CScrollBarUI::PaintButtonNext(IRenderContext& ctx)
	{
		if( !m_bShowButtonNext ) return;

		if( !IsEnabled() ) m_uButtonNextState |= UISTATE_DISABLED;
		else m_uButtonNextState &= ~ UISTATE_DISABLED;
		int d1 = MulDiv(m_rcButtonNext.left - m_rcItem.left, 100, GetManager()->GetDPIObj()->GetScale());
		int d2 = MulDiv(m_rcButtonNext.top - m_rcItem.top, 100, GetManager()->GetDPIObj()->GetScale());
		int d3 = MulDiv(m_rcButtonNext.right - m_rcItem.left, 100, GetManager()->GetDPIObj()->GetScale());
		int d4 = MulDiv(m_rcButtonNext.bottom - m_rcItem.top, 100, GetManager()->GetDPIObj()->GetScale());
		m_sImageModify.Empty();
		m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"),d1 ,d2 ,d3 ,d4 );

		if( (m_uButtonNextState & UISTATE_DISABLED) != 0 ) {
			if( !m_sButtonNextDisabledImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sButtonNextDisabledImage.GetData(), m_sImageModify.GetData()) ) {}
				else return;
			}
		}
		else if( (m_uButtonNextState & UISTATE_PUSHED) != 0 ) {
			if( !m_sButtonNextActiveImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sButtonNextActiveImage.GetData(), m_sImageModify.GetData()) ) {}
				else return;
			}
		}
		else if( (m_uButtonNextState & UISTATE_HOT) != 0 ) {
			if( !m_sButtonNextHoverImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sButtonNextHoverImage.GetData(), m_sImageModify.GetData()) ) {}
				else return;
			}
		}

		if( !m_sButtonNextNormalImage.IsEmpty() ) {
			if( !DrawImage(ctx, m_sButtonNextNormalImage.GetData(), m_sImageModify.GetData()) ) {}
			else return;
		}

		DWORD dwColor = (m_dwThumbColor != 0) ? m_dwThumbColor : 0xB8B8C0FF;
		if( (m_uButtonNextState & UISTATE_DISABLED) != 0 )
			dwColor = (m_dwThumbDisabledColor != 0) ? m_dwThumbDisabledColor : 0xD8D8DCFF;
		else if( (m_uButtonNextState & UISTATE_PUSHED) != 0 )
			dwColor = (m_dwThumbActiveColor != 0) ? m_dwThumbActiveColor : 0x808088FF;
		else if( (m_uButtonNextState & UISTATE_HOT) != 0 )
			dwColor = (m_dwThumbHoverColor != 0) ? m_dwThumbHoverColor : 0x9A9AA2FF;
		RECT rc = m_rcButtonNext;
		int pad = 3;
		rc.left += pad; rc.top += pad; rc.right -= pad; rc.bottom -= pad;
		if( rc.right > rc.left && rc.bottom > rc.top )
			ctx.FillRoundRect(rc, 2, 2, dwColor);
	}

	void CScrollBarUI::PaintThumb(IRenderContext& ctx)
	{
		if( m_rcThumb.left == 0 && m_rcThumb.top == 0 && m_rcThumb.right == 0 && m_rcThumb.bottom == 0 ) return;
		if( !IsEnabled() ) m_uThumbState |= UISTATE_DISABLED;
		else m_uThumbState &= ~ UISTATE_DISABLED;
		int d1 = MulDiv(m_rcThumb.left - m_rcItem.left, 100, GetManager()->GetDPIObj()->GetScale());
		int d2 = MulDiv(m_rcThumb.top - m_rcItem.top, 100, GetManager()->GetDPIObj()->GetScale());
		int d3 = MulDiv(m_rcThumb.right - m_rcItem.left, 100, GetManager()->GetDPIObj()->GetScale());
		int d4 = MulDiv(m_rcThumb.bottom - m_rcItem.top, 100, GetManager()->GetDPIObj()->GetScale());
		m_sImageModify.Empty();
		m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), d1, d2, d3, d4);

		if( (m_uThumbState & UISTATE_DISABLED) != 0 ) {
			if( !m_sThumbDisabledImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sThumbDisabledImage.GetData(), m_sImageModify.GetData()) ) {}
				else return;
			}
		}
		else if( (m_uThumbState & UISTATE_PUSHED) != 0 ) {
			if( !m_sThumbActiveImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sThumbActiveImage.GetData(), m_sImageModify.GetData()) ) {}
				else return;
			}
		}
		else if( (m_uThumbState & UISTATE_HOT) != 0 ) {
			if( !m_sThumbHoverImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sThumbHoverImage.GetData(), m_sImageModify.GetData()) ) {}
				else return;
			}
		}

		if( !m_sThumbNormalImage.IsEmpty() ) {
			if( !DrawImage(ctx, m_sThumbNormalImage.GetData(), m_sImageModify.GetData()) ) {}
			else return;
		}

		// 无图：实心圆角滑块（原先 DrawRect 空心青框）
		DWORD dwColor = (m_dwThumbColor != 0) ? m_dwThumbColor : 0xC0C0C6FF;
		if( (m_uThumbState & UISTATE_DISABLED) != 0 )
			dwColor = (m_dwThumbDisabledColor != 0) ? m_dwThumbDisabledColor : 0xD8D8DCFF;
		else if( (m_uThumbState & UISTATE_PUSHED) != 0 )
			dwColor = (m_dwThumbActiveColor != 0) ? m_dwThumbActiveColor : 0x8A8A92FF;
		else if( (m_uThumbState & UISTATE_HOT) != 0 )
			dwColor = (m_dwThumbHoverColor != 0) ? m_dwThumbHoverColor : 0xA6A6AEFF;

		RECT rc = m_rcThumb;
		if( rc.right <= rc.left || rc.bottom <= rc.top ) return;

		int round = m_bHorizontal ? (rc.bottom - rc.top) : (rc.right - rc.left);
		if( round < 2 ) round = 2;
		round /= 2; // CSS 半径：胶囊形 ≈ 短边一半
		if( round < 1 ) round = 1;
		ctx.FillRoundRect(rc, round, round, dwColor);
	}

	void CScrollBarUI::PaintRail(IRenderContext& ctx)
	{
		if( m_rcThumb.left == 0 && m_rcThumb.top == 0 && m_rcThumb.right == 0 && m_rcThumb.bottom == 0 ) return;
		if( !IsEnabled() ) m_uThumbState |= UISTATE_DISABLED;
		else m_uThumbState &= ~ UISTATE_DISABLED;

		m_sImageModify.Empty();
		if( !m_bHorizontal ) {
			int d1 = MulDiv(m_rcThumb.left - m_rcItem.left, 100, GetManager()->GetDPIObj()->GetScale());
			int d2 = MulDiv((m_rcThumb.top + m_rcThumb.bottom) / 2 - m_rcItem.top - m_cxyFixed.cx / 2, 100, GetManager()->GetDPIObj()->GetScale());
			int d3 = MulDiv(m_rcThumb.right - m_rcItem.left, 100, GetManager()->GetDPIObj()->GetScale());
			int d4 = MulDiv((m_rcThumb.top + m_rcThumb.bottom) / 2 - m_rcItem.top + m_cxyFixed.cx - m_cxyFixed.cx / 2, 100, GetManager()->GetDPIObj()->GetScale());
			m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), d1, d2, d3,d4);
		}
		else {
			int d1 = MulDiv((m_rcThumb.left + m_rcThumb.right) / 2 - m_rcItem.left - m_cxyFixed.cy / 2, 100, GetManager()->GetDPIObj()->GetScale());
			int d2 = MulDiv(m_rcThumb.top - m_rcItem.top, 100, GetManager()->GetDPIObj()->GetScale());
			int d3 = MulDiv((m_rcThumb.left + m_rcThumb.right) / 2 - m_rcItem.left + m_cxyFixed.cy - m_cxyFixed.cy / 2, 100, GetManager()->GetDPIObj()->GetScale());
			int d4 = MulDiv(m_rcThumb.bottom - m_rcItem.top, 100, GetManager()->GetDPIObj()->GetScale());
			m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), d1,d2, d3, d4);
		}

		if( (m_uThumbState & UISTATE_DISABLED) != 0 ) {
			if( !m_sRailDisabledImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sRailDisabledImage.GetData(), m_sImageModify.GetData()) ) {}
				else return;
			}
		}
		else if( (m_uThumbState & UISTATE_PUSHED) != 0 ) {
			if( !m_sRailActiveImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sRailActiveImage.GetData(), m_sImageModify.GetData()) ) {}
				else return;
			}
		}
		else if( (m_uThumbState & UISTATE_HOT) != 0 ) {
			if( !m_sRailHoverImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sRailHoverImage.GetData(), m_sImageModify.GetData()) ) {}
				else return;
			}
		}

		if( !m_sRailNormalImage.IsEmpty() ) {
			if( !DrawImage(ctx, m_sRailNormalImage.GetData(), m_sImageModify.GetData()) ) {}
			else return;
		}
	}
}
