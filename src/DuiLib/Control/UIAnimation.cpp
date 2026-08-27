#include "StdAfx.h"
#include "UIAnimation.h"
#include <vector>
#include <algorithm>

namespace
{
	VOID CALLBACK AnimationQueueTimerProc(PVOID lpParameter, BOOLEAN /*TimerOrWaitFired*/)
	{
		DuiLib::CAnimationData* pData = static_cast<DuiLib::CAnimationData*>(lpParameter);
		if( pData == NULL || pData->m_pOwner == NULL ) return;
		DuiLib::CPaintManagerUI* pm = pData->m_pOwner->GetManager();
		if( pm == NULL ) return;
		HWND hWnd = pm->GetPaintWindow();
		if( hWnd == NULL || !::IsWindow(hWnd) ) return;
		::PostMessage(hWnd, DuiLib::UIMSG_ANIMATION_TICK,
			(WPARAM)pData->m_pOwner, (LPARAM)pData->m_nAnimationID);
	}
}

namespace DuiLib {
	struct CUIAnimation::Imp
	{
		std::vector<CAnimationData*> m_arAnimations;
	};

	CUIAnimation::CUIAnimation():m_pImp(new CUIAnimation::Imp())
	{
		m_pControl = NULL;
	}

	CUIAnimation:: ~CUIAnimation()
	{
		StopAnimation(0);
		if(m_pImp)
		{
			delete m_pImp;
			m_pImp = NULL;
		}
	}

	void CUIAnimation::Attach(CControlUI* pOwner)
	{
		m_pControl = pOwner;
	}

	void CUIAnimation::StopAnimationTimer(CAnimationData* pData)
	{
		if( pData != NULL && pData->m_hQueueTimer != NULL ) {
			::DeleteTimerQueueTimer(NULL, pData->m_hQueueTimer, INVALID_HANDLE_VALUE);
			pData->m_hQueueTimer = NULL;
		}
	}

	void CUIAnimation::StartAnimationTimer(CAnimationData* pData)
	{
		if( pData == NULL || m_pControl == NULL ) return;
		StopAnimationTimer(pData);
		pData->m_pOwner = m_pControl;
		if( pData->m_nElapse <= 0 ) return;
		HANDLE hTimer = NULL;
		if( ::CreateTimerQueueTimer(&hTimer, NULL, AnimationQueueTimerProc,
			reinterpret_cast<PVOID>(pData),
			(UINT)pData->m_nElapse, (UINT)pData->m_nElapse, WT_EXECUTEDEFAULT) ) {
			pData->m_hQueueTimer = hTimer;
		}
	}

	BOOL CUIAnimation::StartAnimation(int nElapse, int nTotalFrame, int nAnimationID /*= 0*/, BOOL bLoop/* = FALSE*/)
	{
		CAnimationData* pData = GetAnimationDataByID(nAnimationID);
		if( NULL != pData 
			|| nElapse <= 0
			|| nTotalFrame <= 0
			|| NULL == m_pControl )
		{
			ASSERT(FALSE);
			return FALSE;
		}

		CAnimationData* pAnimation = new CAnimationData(nElapse, nTotalFrame, nAnimationID, bLoop);
		if( NULL == pAnimation ) return FALSE;
		pAnimation->m_pOwner = m_pControl;

		StartAnimationTimer(pAnimation);
		if( pAnimation->m_hQueueTimer != NULL )
		{
			m_pImp->m_arAnimations.push_back(pAnimation);
			return TRUE;
		}
		delete pAnimation;
		return FALSE;
	}

	void CUIAnimation::StopAnimation(int nAnimationID /*= 0*/)
	{
		if(m_pControl == NULL || m_pImp == NULL) return;

		if(nAnimationID  != 0)
		{
			CAnimationData* pData = GetAnimationDataByID(nAnimationID);
			if( NULL != pData )
			{
				StopAnimationTimer(pData);
				m_pImp->m_arAnimations.erase(std::remove(m_pImp->m_arAnimations.begin(), m_pImp->m_arAnimations.end(), pData), m_pImp->m_arAnimations.end());
				delete pData;
				return;
			}
		}
		else
		{
			int nCount = (int)m_pImp->m_arAnimations.size();
			for(int i=0; i<nCount; ++i)
			{
				CAnimationData* pData = m_pImp->m_arAnimations[i];
				if(pData) {
					StopAnimationTimer(pData);
					delete pData;
				}
			}
			m_pImp->m_arAnimations.clear();
		}
	}

	BOOL CUIAnimation::IsAnimationRunning(int nAnimationID)
	{
		CAnimationData* pData = GetAnimationDataByID(nAnimationID);
		return NULL != pData;
	}

	int CUIAnimation::GetCurrentFrame(int nAnimationID/* = 0*/)
	{
		CAnimationData* pData = GetAnimationDataByID(nAnimationID);
		if( NULL == pData )
		{
			ASSERT(FALSE);
			return -1;
		}
		return pData->m_nCurFrame;
	}

	BOOL CUIAnimation::SetCurrentFrame(int nFrame, int nAnimationID/* = 0*/)
	{
		CAnimationData* pData = GetAnimationDataByID(nAnimationID);
		if( NULL == pData)
		{
			ASSERT(FALSE);
			return FALSE;
		}

		if(nFrame >= 0 && nFrame <= pData->m_nTotalFrame)
		{
			pData->m_nCurFrame = nFrame;
			return TRUE;
		}
		else
		{
			ASSERT(FALSE);
		}
		return FALSE;
	}

	void CUIAnimation::OnAnimationElapse(int nAnimationID)
	{
		if(m_pControl == NULL) return;

		CAnimationData* pData = GetAnimationDataByID(nAnimationID);
		if( NULL == pData ) return;

		int nCurFrame = pData->m_nCurFrame;
		if(nCurFrame == 0)
		{
			OnAnimationStart(nAnimationID, pData->m_bFirstLoop);
			pData->m_bFirstLoop = FALSE;
		}

		OnAnimationStep(pData->m_nTotalFrame, nCurFrame, nAnimationID);

		if(nCurFrame >= pData->m_nTotalFrame)
		{
			OnAnimationStop(nAnimationID);
			if(pData->m_bLoop)
			{
				pData->m_nCurFrame = 0;
			}
			else
			{
				StopAnimationTimer(pData);
				m_pImp->m_arAnimations.erase(std::remove(m_pImp->m_arAnimations.begin(), m_pImp->m_arAnimations.end(), pData), m_pImp->m_arAnimations.end());
				delete pData;
				pData = NULL;
			}
		}

		if( NULL != pData )
		{
			++(pData->m_nCurFrame);
		}
	}

	CAnimationData* CUIAnimation::GetAnimationDataByID(int nAnimationID)
	{
		CAnimationData* pRet = NULL;
		int nCount = (int)m_pImp->m_arAnimations.size();
		for(int i=0; i<nCount; ++i)
		{
			if(m_pImp->m_arAnimations[i]->m_nAnimationID == nAnimationID)
			{
				pRet = m_pImp->m_arAnimations[i];
				break;
			}
		}

		return pRet;
	}

} // namespace DuiLib
