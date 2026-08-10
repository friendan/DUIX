#ifndef __UILABEL_H__
#define __UILABEL_H__

#pragma once

namespace DuiLib
{
	class UILIB_API CLabelUI : public CControlUI
	{
		DECLARE_DUICONTROL(CLabelUI)
	public:
		CLabelUI();
		~CLabelUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;
		bool PreferClientHit() const;

		void SetTextStyle(UINT uStyle);
		UINT GetTextStyle() const;
		void SetColor(DWORD dwColor);
		DWORD GetColor() const;
		void SetDisabledColor(DWORD dwColor);
		DWORD GetDisabledColor() const;
		void SetHoverColor(DWORD dwColor);
		DWORD GetHoverColor() const;
		void SetActiveColor(DWORD dwColor);
		DWORD GetActiveColor() const;
		void SetFocusedColor(DWORD dwColor);
		DWORD GetFocusedColor() const;
		void SetFont(int index);
		int GetFont() const;
		void SetFontFamily(LPCTSTR pstrFamily);
		LPCTSTR GetFontFamily() const;
		void SetFontSize(int nSize);
		int GetFontSize() const;
		void SetFontBold(bool bBold);
		void SetFontItalic(bool bItalic);
		void SetFontUnderline(bool bUnderline);
		void SetFontStrikeout(bool bStrikeout);
		RECT GetTextPadding() const;
		void SetTextPadding(RECT rc);
		bool IsShowHtml();
		void SetShowHtml(bool bShowHtml = true);

		SIZE EstimateSize(SIZE szAvailable);
		void DoEvent(TEventUI& event);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void SetClickable(bool bClickable);
		bool IsClickable() const { return m_bClickable; }

		void PaintText(IRenderContext& ctx);

		virtual void SetText(LPCTSTR pstrText);
		virtual void DoInit();

	protected:
		void ResolveCssFont();
		bool HasTextStateColor() const;
		DWORD	m_dwColor;
		DWORD	m_dwDisabledColor;
		DWORD	m_dwHoverColor;
		DWORD	m_dwActiveColor;
		DWORD	m_dwFocusedColor;
		int		m_iFont;
		CDuiString m_sFontFamily;
		int		m_nFontSize; // 0=未用 font-size；与 font-family 一起经 EnsureFont 解析
		bool	m_bFontBold;
		bool	m_bFontItalic;
		bool	m_bFontUnderline;
		bool	m_bFontStrikeout;
		UINT	m_uTextStyle;
		RECT	m_rcTextPadding;
		bool	m_bShowHtml;
		bool	m_bClickable;
		bool	m_bLButtonDown;

		SIZE    m_szAvailableLast;
		SIZE    m_cxyFixedLast;
		bool    m_bNeedEstimateSize;
	};
}

#endif // __UILABEL_H__
