#ifndef __UICAROUSEL_H__
#define __UICAROUSEL_H__

#pragma once

namespace DuiLib
{
	class CCarouselItemUI;
	class CButtonUI;
	class CLabelUI;
	class CHorizontalLayoutUI;
	class CVerticalLayoutUI;
	class CSpacerUI;

	/// 轮播容器：CarouselItem 显隐切换 + 可选自动播放 + 底部控制栏
	class UILIB_API CCarouselUI : public CVerticalLayoutUI
	{
		DECLARE_DUICONTROL(CCarouselUI)
	public:
		CCarouselUI();
		~CCarouselUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void Next();
		void Prev();
		void GoTo(int index);
		void Play();
		void Pause();

		int GetCurrentIndex() const { return m_nCurrentIndex; }
		int GetItemCount() const;

		bool Add(CControlUI* pControl) override;
		bool AddAt(CControlUI* pControl, int iIndex) override;
		void DoEvent(TEventUI& event) override;
		void DoInit() override;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;

	protected:
		enum { TIMER_ID = 0xC401 };

		void EnsureControlBar();
		void ShowItem(int index);
		void UpdatePageLabel();
		void UpdateIndicators();
		void StartTimer();
		void StopTimer();
		bool IsCursorInside() const;
		CButtonUI* AddControlButton(LPCTSTR pstrText, LPCTSTR pstrName);
		CControlUI* AddGap(int nWidth);
		void ApplyControlsKind(LPCTSTR pstrKind);
		bool OnControlClick(void* param);
		int FindControlBarIndex() const;

		int m_nCurrentIndex;
		int m_nInterval;
		bool m_bWrap;
		bool m_bPauseOnHover;
		bool m_bShowControls;
		bool m_bRidePending;
		bool m_bTimerActive;
		int m_nControlsGap;
		int m_nPageGap;
		int m_nPageWidth;
		CDuiString m_sControlsKind;

		CHorizontalLayoutUI* m_pControlBar;
		CLabelUI* m_pPageLabel;
		CControlUI* m_pGapBeforePage;
		CControlUI* m_pGapAfterPage;
		CControlUI* m_pGapAfterFirst;
		CControlUI* m_pGapBeforeLast;
	};

	/// 轮播项：可选 caption 条；内容由皮肤自由布局
	class UILIB_API CCarouselItemUI : public CVerticalLayoutUI
	{
		DECLARE_DUICONTROL(CCarouselItemUI)
		friend class CCarouselUI;
	public:
		CCarouselItemUI();
		~CCarouselItemUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		bool Add(CControlUI* pControl) override;
		bool AddAt(CControlUI* pControl, int iIndex) override;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;

	protected:
		void EnsureCaptionBar();
		void ApplyCaptionAlign(LPCTSTR pstrAlign);
		void ApplyCaptionKind(LPCTSTR pstrKind);
		int FindCaptionBarIndex() const;

		CHorizontalLayoutUI* m_pCaptionBar;
		CLabelUI* m_pCaptionTitle;
		CLabelUI* m_pCaptionText;
	};
}

#endif // __UICAROUSEL_H__
