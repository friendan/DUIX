#ifndef __UIOPTION_H__
#define __UIOPTION_H__

#pragma once

namespace DuiLib
{
	class UILIB_API COptionUI : public CButtonUI
	{
		DECLARE_DUICONTROL(COptionUI)
	public:
		COptionUI();
		virtual ~COptionUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);

		bool Activate();

		LPCTSTR GetSelectedImage();
		void SetSelectedImage(LPCTSTR pStrImage);

		LPCTSTR GetSelectedHoverImage();
		void SetSelectedHoverImage(LPCTSTR pStrImage);

		LPCTSTR GetSelectedActiveImage();
		void SetSelectedActiveImage(LPCTSTR pStrImage);

		void SetSelectedColor(DWORD dwColor);
		DWORD GetSelectedColor();

		void SetSelectedBackgroundColor(DWORD dwBackgroundColor);
		DWORD GetSelectedBackgroundColor();

		LPCTSTR GetSelectedForegroundImage();
		void SetSelectedForegroundImage(LPCTSTR pStrImage);

		void SetSelectedStateCount(int nCount);
		int GetSelectedStateCount() const;
		virtual LPCTSTR GetSelectedStateImage();
		virtual void SetSelectedStateImage(LPCTSTR pStrImage);

		void SetSelectedFont(int index);
		int GetSelectedFont() const;

		LPCTSTR GetGroup() const;
		void SetGroup(LPCTSTR pStrGroupName = NULL);
		LPCTSTR GetGroupType() const;
		void SetGroupType(LPCTSTR pStrGroupType = NULL);

		bool IsSelected() const;
		virtual void Selected(bool bSelected, bool bMsg = true);

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void PaintBackgroundColor(IRenderContext& ctx);
		void PaintStatusImage(IRenderContext& ctx);
		void PaintForegroundImage(IRenderContext& ctx);
		void PaintText(IRenderContext& ctx);

	protected:
		bool			m_bSelected;
		CDuiString		m_sGroupName;
		CDuiString		m_sGroupType;

		int				m_iSelectedFont;

		DWORD			m_dwSelectedBackgroundColor;
		DWORD			m_dwSelectedColor;

		CDuiString		m_sSelectedImage;
		CDuiString		m_sSelectedHoverImage;
		CDuiString		m_sSelectedActiveImage;
		CDuiString		m_sSelectedForegroundImage;

		int m_nSelectedStateCount;
		CDuiString m_sSelectedStateImage;
	};

	class UILIB_API CCheckBoxUI : public COptionUI
	{
		DECLARE_DUICONTROL(CCheckBoxUI)
	public:
		CCheckBoxUI();

		virtual LPCTSTR GetClass() const;
		virtual LPVOID GetInterface(LPCTSTR pstrName);

		void SetCheck(bool bCheck);
		bool GetCheck() const;

		void SetAutoCheck(bool bEnable);
		bool IsAutoCheck() const { return m_bAutoCheck; }

		void SetBoxSize(SIZE sz);
		SIZE GetBoxSize() const;
		void SetBoxGap(int nGap);
		int GetBoxGap() const;

		virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		virtual void DoEvent(TEventUI& event);
		virtual void Selected(bool bSelected, bool bMsg = true);

		virtual SIZE EstimateSize(SIZE szAvailable);
		virtual void PaintBackgroundColor(IRenderContext& ctx);
		virtual void PaintStatusImage(IRenderContext& ctx);
		virtual void PaintBorder(IRenderContext& ctx);
		virtual void PaintText(IRenderContext& ctx);

	protected:
		bool IsNativeCheckStyle() const;
		RECT GetCheckBoxRect() const;
		void PaintNativeCheckBox(IRenderContext& ctx);

	protected:
		bool m_bAutoCheck;
		SIZE m_szBox;
		int m_nBoxGap;
		int m_nBoxBorderWidth;
		SIZE m_szBoxRound;
		DWORD m_dwBoxBackgroundColor;
		DWORD m_dwBoxBorderColor;
		DWORD m_dwBoxHoverBackgroundColor;
		DWORD m_dwBoxHoverBorderColor;
		DWORD m_dwSelectedBoxBackgroundColor;
		DWORD m_dwSelectedBoxBorderColor;
		DWORD m_dwSelectedBoxHoverBackgroundColor;
		DWORD m_dwSelectedBoxHoverBorderColor;
		DWORD m_dwCheckMarkColor;
		DWORD m_dwDisabledBoxBackgroundColor;
		DWORD m_dwDisabledBoxBorderColor;
	};
} // namespace DuiLib

#endif // __UIOPTION_H__