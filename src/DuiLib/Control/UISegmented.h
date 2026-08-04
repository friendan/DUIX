#ifndef __UISEGMENTED_H__
#define __UISEGMENTED_H__

#pragma once

namespace DuiLib
{
	/// 分段项（数据节点，由父 Segmented 绘制）。
	class UILIB_API CSegmentItemUI : public CControlUI
	{
		DECLARE_DUICONTROL(CSegmentItemUI)
	public:
		CSegmentItemUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetValue(LPCTSTR pstr);
		LPCTSTR GetValue() const;

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl);
		SIZE EstimateSize(SIZE szAvailable);

	protected:
		CDuiString m_sValue;
	};

	/// 分段选择器：工具栏常见互斥选项（日/周/月、列表/卡片…）。
	class UILIB_API CSegmentedUI : public CContainerUI
	{
		DECLARE_DUICONTROL(CSegmentedUI)
	public:
		CSegmentedUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;
		bool PreferClientHit() const;

		void SetSelected(int nIndex, bool bNotify = true);
		int GetSelected() const;
		void SetSelectedValue(LPCTSTR pstrValue, bool bNotify = true);
		LPCTSTR GetSelectedValue() const;

		void SetOptions(LPCTSTR pstrOptions); // text|text|… 或 text:value|…
		int GetItemCount() const;
		CSegmentItemUI* GetSegment(int nIndex) const;

		void SetBlock(bool bBlock);
		bool IsBlock() const;
		void SetItemPadding(int nPad);
		int GetItemPadding() const;

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void SetPos(RECT rc, bool bNeedInvalidate = true);
		SIZE EstimateSize(SIZE szAvailable);
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl);
		void DoEvent(TEventUI& event);

	protected:
		int ScaleValue(int v) const;
		void EnsureFromOptionsAttr();
		void LayoutHitRects();
		int HitTest(POINT pt) const;
		CSegmentItemUI* ItemAt(int i) const;
		CDuiString ItemText(int i) const;
		CDuiString ItemValue(int i) const;
		int MeasureItemWidth(int i) const;
		void PaintTrack(IRenderContext& ctx);
		void PaintSegment(IRenderContext& ctx, int i);

	protected:
		int m_nSelected;
		int m_nHover;
		bool m_bBlock;
		int m_nItemPad;
		int m_nInset;
		CDuiString m_sOptionsAttr;
		bool m_bOptionsApplied;
		CDuiRect m_rcHits[32];
		int m_nHitCount;

		DWORD m_dwTrackColor;
		DWORD m_dwSelectedBk;
		DWORD m_dwSelectedColor;
		DWORD m_dwNormalColor;
		DWORD m_dwHoverColor;
		DWORD m_dwDisabledColor;
	};
}

#endif // __UISEGMENTED_H__
