#include "StdAfx.h"
#include "UIRing.h"

namespace
{
	VOID CALLBACK RingQueueTimerProc(PVOID lpParameter, BOOLEAN /*TimerOrWaitFired*/)
	{
		DuiLib::CRingUI* pSelf = static_cast<DuiLib::CRingUI*>(lpParameter);
		if( pSelf == NULL ) return;
		DuiLib::CPaintManagerUI* pm = pSelf->GetManager();
		if( pm == NULL ) return;
		HWND hWnd = pm->GetPaintWindow();
		if( hWnd == NULL || !::IsWindow(hWnd) ) return;
		::PostMessage(hWnd, DuiLib::UIMSG_RING_TICK, (WPARAM)pSelf, 0);
	}
}

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CRingUI)

	CRingUI::CRingUI()
		: m_fCurAngle(0.0f)
		, m_pBkimage(NULL)
		, m_hQueueTimer(NULL)
	{
	}

	CRingUI::~CRingUI()
	{
		StopQueueTimer();
		DeleteImage();
	}

	LPCTSTR CRingUI::GetClass() const
	{
		return _T("RingUI");
	}

	LPVOID CRingUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, _T("Ring")) == 0 ) return static_cast<CRingUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	void CRingUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcscmp(pstrName, _T("background-image")) == 0 ) SetBackgroundImage(pstrValue);
		else CLabelUI::SetAttribute(pstrName, pstrValue);
	}

	void CRingUI::SetBackgroundImage(LPCTSTR pStrImage)
	{
		if( m_sBackgroundImage == pStrImage ) return;
		m_sBackgroundImage = pStrImage;
		DeleteImage();
		Invalidate();
	}

	void CRingUI::SetVisible(bool bVisible)
	{
		CLabelUI::SetVisible(bVisible);
		if( bVisible ) TryStartAnim();
		else StopQueueTimer();
	}

	void CRingUI::SetInternVisible(bool bVisible)
	{
		CLabelUI::SetInternVisible(bVisible);
		if( bVisible && IsVisible() ) TryStartAnim();
		else StopQueueTimer();
	}

	void CRingUI::PaintBackgroundImage(IRenderContext& ctx)
	{
		if( m_pBkimage == NULL )
			InitImage();

		if( m_pBkimage != NULL )
			ctx.DrawGdiplusImageRotated(m_pBkimage, m_rcItem, m_fCurAngle);
	}

	void CRingUI::DoEvent(TEventUI& event)
	{
		CLabelUI::DoEvent(event);
	}

	void CRingUI::OnAnimTick()
	{
		if( m_pBkimage == NULL || !IsVisible() ) return;
		if( m_fCurAngle > 359.0f )
			m_fCurAngle = 0.0f;
		m_fCurAngle += 36.0f;
		Invalidate();
	}

	void CRingUI::StopQueueTimer()
	{
		if( m_hQueueTimer != NULL ) {
			::DeleteTimerQueueTimer(NULL, m_hQueueTimer, INVALID_HANDLE_VALUE);
			m_hQueueTimer = NULL;
		}
	}

	void CRingUI::StartQueueTimer()
	{
		StopQueueTimer();
		if( m_pBkimage == NULL || !IsVisible() || m_pManager == NULL ) return;
		HANDLE hTimer = NULL;
		if( ::CreateTimerQueueTimer(&hTimer, NULL, RingQueueTimerProc,
			reinterpret_cast<PVOID>(this),
			kRingTickMs, kRingTickMs, WT_EXECUTEDEFAULT) ) {
			m_hQueueTimer = hTimer;
		}
	}

	void CRingUI::TryStartAnim()
	{
		if( m_pBkimage != NULL )
			StartQueueTimer();
	}

	void CRingUI::InitImage()
	{
		TImageInfo* pImageInfo = GetRenderDevice()->GdiplusLoadImage(GetBackgroundImage());
		if( pImageInfo != NULL ) {
			m_pBkimage = pImageInfo->pImage;
			delete pImageInfo;
			pImageInfo = NULL;
			TryStartAnim();
		}
	}

	void CRingUI::DeleteImage()
	{
		StopQueueTimer();
		if( m_pBkimage != NULL ) {
			delete m_pBkimage;
			m_pBkimage = NULL;
		}
	}

	void DuiLib_RingOnQueueTick(CRingUI* pRing)
	{
		if( pRing != NULL )
			pRing->OnAnimTick();
	}
}
