#ifndef __UIBUTTON_H__
#define __UIBUTTON_H__

#pragma once

namespace DuiLib
{
	class UILIB_API CButtonUI : public CLabelUI
	{
		DECLARE_DUICONTROL(CButtonUI)

	public:
		CButtonUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;
		bool PreferClientHit() const;

		bool Activate();
		void SetEnabled(bool bEnable = true);
		void DoEvent(TEventUI& event);

		virtual LPCTSTR GetImage();
		virtual void SetImage(LPCTSTR pStrImage);
		virtual LPCTSTR GetHoverImage();
		virtual void SetHoverImage(LPCTSTR pStrImage);
		virtual LPCTSTR GetActiveImage();
		virtual void SetActiveImage(LPCTSTR pStrImage);
		virtual LPCTSTR GetFocusImage();
		virtual void SetFocusImage(LPCTSTR pStrImage);
		virtual LPCTSTR GetDisabledImage();
		virtual void SetDisabledImage(LPCTSTR pStrImage);
		virtual LPCTSTR GetHoverForegroundImage();
		virtual void SetHoverForegroundImage(LPCTSTR pStrImage);
		virtual LPCTSTR GetActiveForegroundImage();
		virtual void SetActiveForegroundImage(LPCTSTR pStrImage);
		void SetStateCount(int nCount);
		int GetStateCount() const;
		virtual LPCTSTR GetStateImage();
		virtual void SetStateImage(LPCTSTR pStrImage);

		void BindTabIndex(int _BindTabIndex);
		void BindTabLayoutName(LPCTSTR _TabLayoutName);
		void BindTriggerTabSel(int _SetSelectIndex = -1);
		void RemoveBindTabIndex();
		int	 GetBindTabLayoutIndex();
		LPCTSTR GetBindTabLayoutName();

		void SetHoverFont(int index);
		int GetHoverFont() const;
		void SetActiveFont(int index);
		int GetActiveFont() const;
		void SetFocusedFont(int index);
		int GetFocusedFont() const;

		void SetFocusedColor(DWORD dwColor);
		DWORD GetFocusedColor() const;

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void SetKind(ControlKind kind) override;
		void SetOutline(bool bOutline) override;

		void PaintText(IRenderContext& ctx);

		void PaintBackgroundColor(IRenderContext& ctx);
		void PaintStatusImage(IRenderContext& ctx);
		void PaintBorder(IRenderContext& ctx);
		void PaintForegroundImage(IRenderContext& ctx);
		void PaintBackgroundImage(IRenderContext& ctx);

	protected:
		void SyncControlStateFromButton();

		UINT m_uButtonState;

		int		m_iHoverFont;
		int		m_iActiveFont;
		int		m_iFocusedFont;

		DWORD m_dwFocusedColor;

		CDuiString m_sImage;
		CDuiString m_sHoverImage;
		CDuiString m_sHoverForegroundImage;
		CDuiString m_sActiveImage;
		CDuiString m_sActiveForegroundImage;
		CDuiString m_sFocusImage;
		CDuiString m_sDisabledImage;
		int m_nStateCount;
		CDuiString m_sStateImage;

		int			m_iBindTabIndex;
		CDuiString	m_sBindTabLayoutName;
	};

}	// namespace DuiLib

#endif // __UIBUTTON_H__
