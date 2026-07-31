#ifndef __UITABBUTTON_H__
#define __UITABBUTTON_H__

#pragma once

namespace DuiLib
{
	class CLabelUI;
	class CSvgBoxUI;
	class CTabBarUI;

	// 标签页按钮：可选图标 + 标题 + 关闭钮；鼠标事件由父 TabBar 统一处理
	class UILIB_API CTabButtonUI : public CHorizontalLayoutUI
	{
		DECLARE_DUICONTROL(CTabButtonUI)
	public:
		CTabButtonUI();
		~CTabButtonUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetTabTitle(LPCTSTR pstrTitle);
		CDuiString GetTabTitle() const;

		void SetActive(bool bActive);
		bool IsActive() const { return m_bActive; }

		void SetLocked(bool bLocked);
		bool IsLocked() const { return m_bLocked; }

		void SetCloseHover(bool bHover);
		RECT GetCloseRect() const;

		void SetUrl(LPCTSTR pstrUrl);
		CDuiString GetUrl() const { return m_sUrl; }
		void SetDir(LPCTSTR pstrDir);
		CDuiString GetDir() const { return m_sDir; }

		void SetButtonWidth(int nWidth);
		int GetButtonWidth() const;

		void SetIconSize(int nSize);
		int GetIconSize() const { return m_nIconSize; }
		void ClearTabIcon();
		bool HasTabIcon() const;

		void ApplyHoverStyle(bool bHover);
		void UpdateStyle();
		void PaintBorder(IRenderContext& ctx) override;
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl) override;
		bool IsCloseFullyVisible() const;

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
		void DoInit() override;

	protected:
		void EnsureChildren();
		CTabBarUI* GetOwnerBar() const;
		void ApplyIconSize();
		bool IsIconAttr(LPCTSTR pstrName) const;

		CControlUI* m_pLeftPad;
		CSvgBoxUI* m_pIcon;
		CControlUI* m_pIconGap;
		CLabelUI* m_pTitle;
		CLabelUI* m_pClose;
		bool m_bActive;
		bool m_bLocked;
		bool m_bCloseHovered;
		bool m_bHover;
		int m_nIconSize;
		CDuiString m_sUrl;
		CDuiString m_sDir;
	};
}

#endif // __UITABBUTTON_H__
