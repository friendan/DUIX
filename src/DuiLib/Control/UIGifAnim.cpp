#include "StdAfx.h"
#include "UIGifAnim.h"

namespace
{
	VOID CALLBACK GifAnimQueueTimerProc(PVOID lpParameter, BOOLEAN /*TimerOrWaitFired*/)
	{
		DuiLib::CGifAnimUI* pSelf = static_cast<DuiLib::CGifAnimUI*>(lpParameter);
		if( pSelf == NULL ) return;
		DuiLib::CPaintManagerUI* pm = pSelf->GetManager();
		if( pm == NULL ) return;
		HWND hWnd = pm->GetPaintWindow();
		if( hWnd == NULL || !::IsWindow(hWnd) ) return;
		::PostMessage(hWnd, DuiLib::UIMSG_GIFANIM_TICK, (WPARAM)pSelf, 0);
	}
}

///////////////////////////////////////////////////////////////////////////////////////
namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CGifAnimUI)

		CGifAnimUI::CGifAnimUI(void)
	{
		m_pGifImage			=	NULL;
		m_pPropertyItem		=	NULL;
		m_nFrameCount		=	0;	
		m_nFramePosition	=	0;	
		m_bIsAutoPlay		=	true;
		m_bIsAutoSize		=	false;
		m_bIsPlaying		=	false;
		m_hQueueTimer		=	NULL;
	}


	CGifAnimUI::~CGifAnimUI(void)
	{
		StopQueueTimer();
		DeleteGif();
	}

	LPCTSTR CGifAnimUI::GetClass() const
	{
		return _T("GifAnimUI");
	}

	LPVOID CGifAnimUI::GetInterface( LPCTSTR pstrName )
	{
		if( _tcsicmp(pstrName, DUI_CTR_GIFANIM) == 0 ) return static_cast<CGifAnimUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	void CGifAnimUI::DoInit()
	{
		InitGifImage();
	}

	bool CGifAnimUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		if( !::IntersectRect( &m_rcPaint, &rcPaint, &m_rcItem ) ) return true;
		if ( NULL == m_pGifImage )
		{		
			InitGifImage();
		}
		DrawFrame( ctx );
		return true;
	}

	void CGifAnimUI::DoEvent( TEventUI& event )
	{
		CControlUI::DoEvent(event);
	}

	void CGifAnimUI::SetVisible(bool bVisible /* = true */)
	{
		CControlUI::SetVisible(bVisible);
		if (bVisible)
			PlayGif();
		else
			StopGif();
	}

	void CGifAnimUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("background-image")) == 0 ) SetBackgroundImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("auto-play")) == 0 ) {
			SetAutoPlay(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("auto-size")) == 0 ) {
			SetAutoSize(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else
			CControlUI::SetAttribute(pstrName, pstrValue);
	}

	void CGifAnimUI::SetBackgroundImage(LPCTSTR pStrImage)
	{
		if( m_sBackgroundImage == pStrImage || NULL == pStrImage) return;

		m_sBackgroundImage = pStrImage;

		StopGif();
		DeleteGif();

		Invalidate();

	}

	LPCTSTR CGifAnimUI::GetBackgroundImage()
	{
		return m_sBackgroundImage.GetData();
	}

	void CGifAnimUI::SetAutoPlay(bool bIsAuto)
	{
		m_bIsAutoPlay = bIsAuto;
	}

	bool CGifAnimUI::IsAutoPlay() const
	{
		return m_bIsAutoPlay;
	}

	void CGifAnimUI::SetAutoSize(bool bIsAuto)
	{
		m_bIsAutoSize = bIsAuto;
	}

	bool CGifAnimUI::IsAutoSize() const
	{
		return m_bIsAutoSize;
	}

	UINT CGifAnimUI::GetFrameDelayMs() const
	{
		if( m_pPropertyItem == NULL || m_nFrameCount <= 1 ) return 100;
		long lPause = ((long*)m_pPropertyItem->value)[m_nFramePosition] * 10;
		if( lPause == 0 ) lPause = 100;
		return (UINT)lPause;
	}

	void CGifAnimUI::StopQueueTimer()
	{
		if( m_hQueueTimer != NULL ) {
			::DeleteTimerQueueTimer(NULL, m_hQueueTimer, INVALID_HANDLE_VALUE);
			m_hQueueTimer = NULL;
		}
	}

	void CGifAnimUI::StartQueueTimer(UINT uElapse)
	{
		StopQueueTimer();
		if( !m_bIsPlaying || m_pGifImage == NULL || m_pManager == NULL ) return;
		if( uElapse == 0 ) uElapse = 100;
		HANDLE hTimer = NULL;
		if( ::CreateTimerQueueTimer(&hTimer, NULL, GifAnimQueueTimerProc,
			reinterpret_cast<PVOID>(this), uElapse, 0, WT_EXECUTEONLYONCE) ) {
			m_hQueueTimer = hTimer;
		}
	}

	void CGifAnimUI::PlayGif()
	{
		if (m_bIsPlaying || m_pGifImage == NULL || m_nFrameCount <= 1)
		{
			return;
		}

		m_bIsPlaying = true;
		StartQueueTimer(GetFrameDelayMs());
	}

	void CGifAnimUI::PauseGif()
	{
		if (!m_bIsPlaying || m_pGifImage == NULL)
		{
			return;
		}

		StopQueueTimer();
		this->Invalidate();
		m_bIsPlaying = false;
	}

	void CGifAnimUI::StopGif()
	{
		if (!m_bIsPlaying)
		{
			return;
		}

		StopQueueTimer();
		m_nFramePosition = 0;
		this->Invalidate();
		m_bIsPlaying = false;
	}

	void CGifAnimUI::InitGifImage()
	{
		TImageInfo* pImageInfo = GetRenderDevice()->GdiplusLoadImage(GetBackgroundImage());
		if(pImageInfo != NULL) {
			m_pGifImage = pImageInfo->pImage;

			delete pImageInfo;
			pImageInfo = NULL;
		}
		if ( NULL == m_pGifImage ) return;

		UINT nCount	= 0;
		nCount	=	m_pGifImage->GetFrameDimensionsCount();
		GUID* pDimensionIDs	=	new GUID[ nCount ];
		m_pGifImage->GetFrameDimensionsList( pDimensionIDs, nCount );
		m_nFrameCount	=	m_pGifImage->GetFrameCount( &pDimensionIDs[0] );
		if (m_nFrameCount > 1)
		{
			int nSize = m_pGifImage->GetPropertyItemSize(PropertyTagFrameDelay);
			m_pPropertyItem = (Gdiplus::PropertyItem*) malloc(nSize);
			m_pGifImage->GetPropertyItem(PropertyTagFrameDelay, nSize, m_pPropertyItem);
		}
		delete[]  pDimensionIDs;
		pDimensionIDs = NULL;

		if (m_bIsAutoSize)
		{
			SetFixedWidth(m_pGifImage->GetWidth());
			SetFixedHeight(m_pGifImage->GetHeight());
		}
		if (m_bIsAutoPlay)
		{
			PlayGif();
		}
	}

	void CGifAnimUI::DeleteGif()
	{
		StopQueueTimer();
		if ( m_pGifImage != NULL )
		{
			delete m_pGifImage;
			m_pGifImage = NULL;
		}

		if ( m_pPropertyItem != NULL )
		{
			free( m_pPropertyItem );
			m_pPropertyItem = NULL;
		}
		m_nFrameCount		=	0;	
		m_nFramePosition	=	0;	
		m_bIsPlaying		=	false;
	}

	void CGifAnimUI::OnAnimTick()
	{
		if( !m_bIsPlaying || m_pGifImage == NULL || m_nFrameCount <= 1 ) return;
		StopQueueTimer();
		Invalidate();

		m_nFramePosition = (++m_nFramePosition) % m_nFrameCount;
		StartQueueTimer(GetFrameDelayMs());
	}

	void CGifAnimUI::DrawFrame( IRenderContext& ctx )
	{
		if( NULL == m_pGifImage ) return;
		GUID pageGuid = Gdiplus::FrameDimensionTime;
		ctx.DrawGdiplusImage(m_pGifImage, (INT)m_rcItem.left, (INT)m_rcItem.top, (INT)(m_rcItem.right-m_rcItem.left), (INT)(m_rcItem.bottom-m_rcItem.top));
		m_pGifImage->SelectActiveFrame( &pageGuid, m_nFramePosition );
	}

	void DuiLib_GifAnimOnQueueTick(CGifAnimUI* pGif)
	{
		if( pGif != NULL )
			pGif->OnAnimTick();
	}
}
