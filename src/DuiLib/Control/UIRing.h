#ifndef __UIROTATE_H__
#define __UIROTATE_H__

#pragma once

namespace DuiLib
{
	class CRingUI : public CLabelUI
	{
		enum
		{
			kRingTickMs = 100,
		};
		DECLARE_DUICONTROL(CRingUI)
	public:
		CRingUI();
		~CRingUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void SetBackgroundImage(LPCTSTR pStrImage);
		void SetVisible(bool bVisible = true) override;
		void SetInternVisible(bool bVisible = true) override;
		void DoEvent(TEventUI& event) override;
		void PaintBackgroundImage(IRenderContext& ctx) override;
		/// TimerQueue → UIMSG_RING_TICK 回调
		void OnAnimTick();

	private:
		void InitImage();
		void DeleteImage();
		void StartQueueTimer();
		void StopQueueTimer();
		void TryStartAnim();

	public:
		float m_fCurAngle;
		Gdiplus::Image* m_pBkimage;

	private:
		HANDLE m_hQueueTimer;
	};

	void DuiLib_RingOnQueueTick(CRingUI* pRing);
}

#endif // __UIROTATE_H__
