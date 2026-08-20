#ifndef __UIACCORDION_H__
#define __UIACCORDION_H__

#pragma once

namespace DuiLib
{
	class CAccordionItemUI;
	class CLabelUI;
	class CHorizontalLayoutUI;

	class UILIB_API CAccordionUI : public CVerticalLayoutUI
	{
		DECLARE_DUICONTROL(CAccordionUI)
	public:
		CAccordionUI();
		~CAccordionUI();

		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;

		void SetMode(bool bMultiple);
		bool IsMultipleMode() const { return m_bMultiple; }
		void SetDefaultHeaderHeight(int nHeight);
		int GetDefaultHeaderHeight() const { return m_nDefaultHeaderHeight; }
		// fill=true：手风琴撑满父级剩余高度；展开项 FixedHeight=0 再分给可伸缩子控件（如带滚动的 RichEdit）
		void SetFill(bool bFill);
		bool IsFill() const { return m_bFill; }

		void ToggleItem(CAccordionItemUI* pItem);
		CAccordionItemUI* GetActiveItem();

		bool Add(CControlUI* pControl) override;
		bool AddAt(CControlUI* pControl, int iIndex) override;
		SIZE EstimateSize(SIZE szAvailable) override;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;

	protected:
		void RefreshItemHeights();

		bool m_bMultiple;
		bool m_bFill;
		int m_nDefaultHeaderHeight;
	};

	class UILIB_API CAccordionItemUI : public CVerticalLayoutUI
	{
		DECLARE_DUICONTROL(CAccordionItemUI)
		friend class CAccordionUI;
	public:
		CAccordionItemUI();
		~CAccordionItemUI();

		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;

		void SetTitle(LPCTSTR pstrText);
		CDuiString GetTitle() const;
		bool IsActive() const { return m_bActive; }
		bool IsDisabled() const { return m_bDisabled; }
		void SetActive(bool bActive, bool bNotify = true);
		void SetDisabled(bool bDisabled);
		void ApplyDefaultHeaderHeight(int nHeight);
		void OnHeaderClick();
		void OnHeaderHoverChanged(bool bHot);

		bool Add(CControlUI* pControl) override;
		bool AddAt(CControlUI* pControl, int iIndex) override;
		SIZE EstimateSize(SIZE szAvailable) override;
		void SetPos(RECT rc, bool bNeedInvalidate = true) override;
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl) override;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
		void DoInit() override;

	protected:
		void EnsureHeader();
		void ApplyContentPadding(CControlUI* pControl);
		void SyncContentVisibility();
		void SyncHeaderChrome();
		void UpdateFixedHeight();
		void RequestAncestorLayout();
		CAccordionUI* GetOwnerAccordion() const;
		static DWORD ParseColorValue(LPCTSTR pstrValue);

		CHorizontalLayoutUI* m_pHeader;
		CLabelUI* m_pTitle;
		CLabelUI* m_pChevron;
		bool m_bActive;
		bool m_bDisabled;
		bool m_bHeaderHover;
		bool m_bHeaderHeightExplicit;
		int m_nHeaderHeight;
		CDuiBox m_rcContentPadding;
		DWORD m_dwHeaderBk;
		DWORD m_dwHeaderHoverBk;
		DWORD m_dwHeaderActiveBk;
		DWORD m_dwHeaderActiveHoverBk;
	};
}

#endif // __UIACCORDION_H__
