#include "StdAfx.h"
#include "UIRing.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CRingUI)

		CRingUI::CRingUI() : m_fCurAngle(0.0f), m_pBkimage(NULL)
	{
	}

	CRingUI::~CRingUI()
	{
		if(m_pManager) m_pManager->KillTimer(this, RING_TIMERID);

		DeleteImage();
	}

	LPCTSTR CRingUI::GetClass() const
	{
		return _T("RingUI");
	}

	LPVOID CRingUI::GetInterface( LPCTSTR pstrName )
	{
		if( _tcscmp(pstrName, _T("Ring")) == 0 ) return static_cast<CRingUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	void CRingUI::SetAttribute( LPCTSTR pstrName, LPCTSTR pstrValue )
	{
		if( _tcscmp(pstrName, _T("bkimage")) == 0 ) SetBkImage(pstrValue);
		else CLabelUI::SetAttribute(pstrName, pstrValue);
	}

	void CRingUI::SetBkImage( LPCTSTR pStrImage )
	{
		if (m_sBkImage == pStrImage) return;
		m_sBkImage = pStrImage;
		DeleteImage();
		Invalidate();
	}

	void CRingUI::PaintBkImage(IRenderContext& ctx)
	{
		if(m_pBkimage == NULL) {
			InitImage();
		}

		if(m_pBkimage != NULL) {
			ctx.DrawGdiplusImageRotated(m_pBkimage, m_rcItem, m_fCurAngle);
		}
	}

	void CRingUI::DoEvent( TEventUI& event )
	{
		if( event.Type == UIEVENT_TIMER && event.wParam == RING_TIMERID ) {
			if(m_fCurAngle > 359) {
				m_fCurAngle = 0;
			}
			m_fCurAngle += 36.0;
			//Invalidate();
			NeedParentUpdate();
		}
		else {
			CLabelUI::DoEvent(event);
		}
	}

	void CRingUI::InitImage()
	{
		TImageInfo* pImageInfo = GetRenderDevice()->GdiplusLoadImage(GetBkImage());
		if(pImageInfo != NULL) {
			m_pBkimage = pImageInfo->pImage;

			delete pImageInfo;
			pImageInfo = NULL;

			if(m_pManager != NULL && m_pBkimage != NULL) {
				m_pManager->SetTimer(this, RING_TIMERID, 100);
			}
		}

	}

	void CRingUI::DeleteImage()
	{
		if ( m_pBkimage != NULL )
		{
			delete m_pBkimage;
			m_pBkimage = NULL;
		}
	}
}
