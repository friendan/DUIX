#ifndef __UISPLITLAYOUT_H__
#define __UISPLITLAYOUT_H__

#pragma once

#include "UILinearLayout.h"

namespace DuiLib
{
	/// 一维分割布局：子项沿主轴排列，相邻项之间可拖分隔条互抢空间。
	/// 水平 = 分栏（竖线）；垂直 = 分行（横线）。行列组合靠嵌套另一个 SplitLayout。
	class UILIB_API CSplitLayoutUI : public CContainerUI
	{
		DECLARE_DUICONTROL(CSplitLayoutUI)
	public:
		CSplitLayoutUI();
		explicit CSplitLayoutUI(LayoutDirection eDirection);

		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;
		UINT GetControlFlags() const override;
		bool PreferClientHit() const override;

		LayoutDirection GetOrientation() const;
		void SetOrientation(LayoutDirection eDirection);

		void SetSepSize(int iSize);
		int GetSepSize() const;
		void SetSepImmMode(bool bImmediately);
		bool IsSepImmMode() const;
		void SetSepColor(DWORD dwColor);
		DWORD GetSepColor() const;
		void SetSepHoverColor(DWORD dwColor);
		DWORD GetSepHoverColor() const;
		void SetSepActiveColor(DWORD dwColor);
		DWORD GetSepActiveColor() const;

		SIZE EstimateSize(SIZE szAvailable) override;
		void SetPos(RECT rc, bool bNeedInvalidate = true) override;
		void DoEvent(TEventUI& event) override;
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl) override;
		CControlUI* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags) override;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;

	protected:
		void CollectPanes(CStdPtrArray& aPanes) const;
		int GetSepSizePx() const;
		int HitSep(POINT pt) const;
		RECT GetSepRect(int iSep) const;
		int CtrlMainMin(CControlUI* p) const;
		int CtrlMainMax(CControlUI* p) const;
		int MainSize(const RECT& rc) const;
		void FreezeMain(CControlUI* p, int nPixel);
		void ApplySepSizes(int iSep, int nPixelA, int nPixelB);
		void PaintSeparators(IRenderContext& ctx);
		void UpdateHotSep(POINT pt);
		DWORD ResolveThemeColor(LPCTSTR pstrToken, DWORD dwFallback) const;
		DWORD GetSepPaintColor(int iSep) const;
		int GetSepLinePx(bool bEmphasis) const;
		bool ParseSepColorAttr(LPCTSTR pstrName, LPCTSTR pstrValue, DWORD& dwColor);

		LONG& RcMainStart(RECT& r) const         { return ((LONG*)&r)[m_iMainAxis]; }
		LONG& RcMainEnd(RECT& r) const           { return ((LONG*)&r)[m_iMainAxis + 2]; }
		LONG& RcCrossStart(RECT& r) const        { return ((LONG*)&r)[1 - m_iMainAxis]; }
		LONG& RcCrossEnd(RECT& r) const          { return ((LONG*)&r)[3 - m_iMainAxis]; }
		LONG  RcMainStart(const RECT& r) const    { return ((const LONG*)&r)[m_iMainAxis]; }
		LONG  RcMainEnd(const RECT& r) const      { return ((const LONG*)&r)[m_iMainAxis + 2]; }
		LONG  RcCrossStart(const RECT& r) const    { return ((const LONG*)&r)[1 - m_iMainAxis]; }
		LONG  RcCrossEnd(const RECT& r) const      { return ((const LONG*)&r)[3 - m_iMainAxis]; }
		LONG& SzMain(SIZE& s) const               { return ((LONG*)&s)[m_iMainAxis]; }
		LONG& SzCross(SIZE& s) const              { return ((LONG*)&s)[1 - m_iMainAxis]; }
		LONG  SzMain(const SIZE& s) const         { return ((const LONG*)&s)[m_iMainAxis]; }
		LONG  SzCross(const SIZE& s) const        { return ((const LONG*)&s)[1 - m_iMainAxis]; }
		LONG  PtMain(const POINT& p) const        { return ((const LONG*)&p)[m_iMainAxis]; }

		LayoutDirection m_eDirection;
		int m_iMainAxis;
		int m_iSepSize;
		UINT m_uButtonState;
		int m_iActiveSep;
		int m_iHotSep;
		int m_nDragSizeA;
		int m_nDragSizeB;
		POINT m_ptDown;
		bool m_bImmMode;
		DWORD m_dwSepColor;
		DWORD m_dwSepHoverColor;
		DWORD m_dwSepActiveColor;
	};

	class UILIB_API CHSplitLayoutUI : public CSplitLayoutUI
	{
		DECLARE_DUICONTROL(CHSplitLayoutUI)
	public:
		CHSplitLayoutUI();
		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;
	};

	class UILIB_API CVSplitLayoutUI : public CSplitLayoutUI
	{
		DECLARE_DUICONTROL(CVSplitLayoutUI)
	public:
		CVSplitLayoutUI();
		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;
	};
}

#endif // __UISPLITLAYOUT_H__
