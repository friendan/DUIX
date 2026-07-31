#ifndef __UILINEARLAYOUT_H__
#define __UILINEARLAYOUT_H__

#pragma once

namespace DuiLib
{
	enum LayoutDirection { LAYOUT_VERTICAL, LAYOUT_HORIZONTAL };

	class UILIB_API CLinearLayoutUI : public CContainerUI
	{
	public:
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;

		LayoutDirection GetDirection() const;
		void SetSepSize(int iSize);
		int GetSepSize() const;
		void SetSepImmMode(bool bImmediately);
		bool IsSepImmMode() const;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		SIZE MeasureContent(SIZE szAvailable);
		SIZE EstimateSize(SIZE szAvailable);
		void SetPos(RECT rc, bool bNeedInvalidate = true);
		void DoEvent(TEventUI& event);
		void DoPostPaint(IRenderContext& ctx, const RECT& rcPaint);
		RECT GetThumbRect(bool bUseNew = false) const;

	protected:
		CLinearLayoutUI(LayoutDirection eDirection);

		// RECT: {left=0, top=1, right=2, bottom=3}  SIZE: {cx=0, cy=1}
		// m_iMainAxis: 1 → Y (Vertical), 0 → X (Horizontal)
		LONG& RcMainStart(RECT& r) const         { return ((LONG*)&r)[m_iMainAxis]; }
		LONG& RcMainEnd(RECT& r) const           { return ((LONG*)&r)[m_iMainAxis + 2]; }
		LONG& RcCrossStart(RECT& r) const        { return ((LONG*)&r)[1 - m_iMainAxis]; }
		LONG& RcCrossEnd(RECT& r) const          { return ((LONG*)&r)[3 - m_iMainAxis]; }
		LONG  RcMainStart(const RECT& r) const    { return ((const LONG*)&r)[m_iMainAxis]; }
		LONG  RcMainEnd(const RECT& r) const      { return ((const LONG*)&r)[m_iMainAxis + 2]; }
		LONG  RcCrossStart(const RECT& r) const   { return ((const LONG*)&r)[1 - m_iMainAxis]; }
		LONG  RcCrossEnd(const RECT& r) const     { return ((const LONG*)&r)[3 - m_iMainAxis]; }
		LONG& SzMain(SIZE& s) const               { return ((LONG*)&s)[m_iMainAxis]; }
		LONG& SzCross(SIZE& s) const              { return ((LONG*)&s)[1 - m_iMainAxis]; }
		LONG  SzMain(const SIZE& s) const         { return ((const LONG*)&s)[m_iMainAxis]; }
		LONG  SzCross(const SIZE& s) const        { return ((const LONG*)&s)[1 - m_iMainAxis]; }
		LONG  PtMain(const POINT& p) const        { return ((const LONG*)&p)[m_iMainAxis]; }

		CScrollBarUI* MainScrollBar() const;
		CScrollBarUI* CrossScrollBar() const;
		UINT MainChildAlign() const;
		UINT CrossChildAlign() const;
		UINT MainAlignCenter() const;
		UINT MainAlignEnd() const;
		UINT CrossAlignCenter() const;
		UINT CrossAlignEnd() const;
		virtual UINT ResolveCrossAlign(CControlUI* pControl) const;
		void PositionChildCrossAxis(CControlUI* pControl, UINT iCrossAlign,
			const RECT& rc, const RECT& rcPadding, int iMainPos,
			int szMainChild, int szCrossChild, CScrollBarUI* pCrossScroll);
		int CtrlMainMin(CControlUI* p) const;
		int CtrlMainMax(CControlUI* p) const;
		int CtrlCrossMin(CControlUI* p) const;
		int CtrlCrossMax(CControlUI* p) const;
		int SelfMainMin() const;
		int SelfMainMax() const;

	protected:
		LayoutDirection m_eDirection;
		int m_iMainAxis;
		int m_iSepSize;
		UINT m_uButtonState;
		POINT ptLastMouse;
		RECT m_rcNewPos;
		bool m_bImmMode;
	};
}

#endif // __UILINEARLAYOUT_H__