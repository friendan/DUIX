#ifndef __UISCROLLBAR_H__
#define __UISCROLLBAR_H__

#pragma once

namespace DuiLib
{
	class UILIB_API CScrollBarUI : public CControlUI
	{
		DECLARE_DUICONTROL(CScrollBarUI)
	public:
		CScrollBarUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		/// 可交互：不继承 html/父级 action=title 的 HTCAPTION，否则滑块无法拖
		bool PreferClientHit() const;

		CContainerUI* GetOwner() const;
		void SetOwner(CContainerUI* pOwner);

		void SetVisible(bool bVisible = true);
		void SetEnabled(bool bEnable = true);
		void SetFocus();

		bool IsHorizontal();
		void SetHorizontal(bool bHorizontal = true);
		int GetScrollRange() const;
		void SetScrollRange(int nRange);
		int GetScrollPos() const;
		void SetScrollPos(int nPos);
		int GetLineSize() const;
		void SetLineSize(int nSize);

		bool GetShowButtonPrev();
		void SetShowButtonPrev(bool bShow);
		LPCTSTR GetButtonPrevNormalImage();
		void SetButtonPrevNormalImage(LPCTSTR pStrImage);
		LPCTSTR GetButtonPrevHoverImage();
		void SetButtonPrevHoverImage(LPCTSTR pStrImage);
		LPCTSTR GetButtonPrevActiveImage();
		void SetButtonPrevActiveImage(LPCTSTR pStrImage);
		LPCTSTR GetButtonPrevDisabledImage();
		void SetButtonPrevDisabledImage(LPCTSTR pStrImage);

		bool GetShowButtonNext();
		void SetShowButtonNext(bool bShow);
		LPCTSTR GetButtonNextNormalImage();
		void SetButtonNextNormalImage(LPCTSTR pStrImage);
		LPCTSTR GetButtonNextHoverImage();
		void SetButtonNextHoverImage(LPCTSTR pStrImage);
		LPCTSTR GetButtonNextActiveImage();
		void SetButtonNextActiveImage(LPCTSTR pStrImage);
		LPCTSTR GetButtonNextDisabledImage();
		void SetButtonNextDisabledImage(LPCTSTR pStrImage);

		LPCTSTR GetThumbNormalImage();
		void SetThumbNormalImage(LPCTSTR pStrImage);
		LPCTSTR GetThumbHoverImage();
		void SetThumbHoverImage(LPCTSTR pStrImage);
		LPCTSTR GetThumbActiveImage();
		void SetThumbActiveImage(LPCTSTR pStrImage);
		LPCTSTR GetThumbDisabledImage();
		void SetThumbDisabledImage(LPCTSTR pStrImage);

		LPCTSTR GetRailNormalImage();
		void SetRailNormalImage(LPCTSTR pStrImage);
		LPCTSTR GetRailHoverImage();
		void SetRailHoverImage(LPCTSTR pStrImage);
		LPCTSTR GetRailActiveImage();
		void SetRailActiveImage(LPCTSTR pStrImage);
		LPCTSTR GetRailDisabledImage();
		void SetRailDisabledImage(LPCTSTR pStrImage);

		LPCTSTR GetBkNormalImage();
		void SetBkNormalImage(LPCTSTR pStrImage);
		LPCTSTR GetBkHoverImage();
		void SetBkHoverImage(LPCTSTR pStrImage);
		LPCTSTR GetBkActiveImage();
		void SetBkActiveImage(LPCTSTR pStrImage);
		LPCTSTR GetBkDisabledImage();
		void SetBkDisabledImage(LPCTSTR pStrImage);

		bool GetShow();
		void SetShow(bool bShow);

		int GetThumbMinSize() const;
		void SetThumbMinSize(int nSize);

		void SetThumbColor(DWORD dwColor);
		DWORD GetThumbColor() const;
		void SetThumbHoverColor(DWORD dwColor);
		DWORD GetThumbHoverColor() const;
		void SetThumbActiveColor(DWORD dwColor);
		DWORD GetThumbActiveColor() const;
		void SetThumbDisabledColor(DWORD dwColor);
		DWORD GetThumbDisabledColor() const;

		void SetPos(RECT rc, bool bNeedInvalidate = true);
		void DoEvent(TEventUI& event);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl);

		void PaintBk(IRenderContext& ctx);
		void PaintButtonPrev(IRenderContext& ctx);
		void PaintButtonNext(IRenderContext& ctx);
		void PaintThumb(IRenderContext& ctx);
		void PaintRail(IRenderContext& ctx);

	protected:
		enum
		{ 
			DEFAULT_SCROLLBAR_SIZE = 14,
			DEFAULT_THUMB_MIN_SIZE = 50,
			DEFAULT_TIMERID = 10,
		};
		bool m_bShow;
		bool m_bHorizontal;
		__int64 m_nRange;
		__int64 m_nScrollPos;
		int m_nLineSize;
		int m_nThumbMinSize;
		CContainerUI* m_pOwner;
		POINT m_ptLastMouse;
		int m_nLastScrollPos;
		int m_nLastScrollOffset;
		int m_nScrollRepeatDelay;

		CDuiString m_sBkNormalImage;
		CDuiString m_sBkHoverImage;
		CDuiString m_sBkActiveImage;
		CDuiString m_sBkDisabledImage;

		bool m_bShowButtonPrev;
		RECT m_rcButtonPrev;
		UINT m_uButtonPrevState;
		CDuiString m_sButtonPrevNormalImage;
		CDuiString m_sButtonPrevHoverImage;
		CDuiString m_sButtonPrevActiveImage;
		CDuiString m_sButtonPrevDisabledImage;

		bool m_bShowButtonNext;
		RECT m_rcButtonNext;
		UINT m_uButtonNextState;
		CDuiString m_sButtonNextNormalImage;
		CDuiString m_sButtonNextHoverImage;
		CDuiString m_sButtonNextActiveImage;
		CDuiString m_sButtonNextDisabledImage;

		RECT m_rcThumb;
		UINT m_uThumbState;
		CDuiString m_sThumbNormalImage;
		CDuiString m_sThumbHoverImage;
		CDuiString m_sThumbActiveImage;
		CDuiString m_sThumbDisabledImage;

		CDuiString m_sRailNormalImage;
		CDuiString m_sRailHoverImage;
		CDuiString m_sRailActiveImage;
		CDuiString m_sRailDisabledImage;

		CDuiString m_sImageModify;

		DWORD m_dwThumbColor;
		DWORD m_dwThumbHoverColor;
		DWORD m_dwThumbActiveColor;
		DWORD m_dwThumbDisabledColor;
	};
}

#endif // __UISCROLLBAR_H__