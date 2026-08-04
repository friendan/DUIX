#ifndef __UITIMELINE_H__
#define __UITIMELINE_H__

#pragma once

namespace DuiLib
{
	/// 时间线条目（数据节点，由父 Timeline 绘制）。
	class UILIB_API CTimelineItemUI : public CControlUI
	{
		DECLARE_DUICONTROL(CTimelineItemUI)
	public:
		enum Status { StatusFinish = 0, StatusProcess, StatusWait };

		CTimelineItemUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetTimeText(LPCTSTR pstr);
		LPCTSTR GetTimeText() const;
		void SetDescription(LPCTSTR pstr);
		LPCTSTR GetDescription() const;
		void SetStatus(Status e);
		Status GetStatus() const;
		void SetDotColor(DWORD clr);
		DWORD GetDotColor() const;

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl);
		SIZE EstimateSize(SIZE szAvailable);

	protected:
		CDuiString m_sTime;
		CDuiString m_sDescription;
		Status m_eStatus;
		DWORD m_dwDotColor; // 0 = 用父级按 status
	};

	/// 垂直时间线。
	class UILIB_API CTimelineUI : public CContainerUI
	{
		DECLARE_DUICONTROL(CTimelineUI)
	public:
		CTimelineUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetItems(LPCTSTR pstrItems); // time title|… 或 title|title
		void SetPending(bool bPending);   // 末尾虚线「进行中」感
		bool IsPending() const;

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void SetPos(RECT rc, bool bNeedInvalidate = true);
		SIZE EstimateSize(SIZE szAvailable);
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl);

	protected:
		int ScaleValue(int v) const;
		void EnsureFromItemsAttr();
		void PaintTimeline(IRenderContext& ctx);

	protected:
		CDuiString m_sItemsAttr;
		bool m_bItemsApplied;
		bool m_bPending;
		int m_nDotSize;
		int m_nItemGap;

		DWORD m_dwFinishColor;
		DWORD m_dwProcessColor;
		DWORD m_dwWaitColor;
		DWORD m_dwLineColor;
		DWORD m_dwTitleColor;
		DWORD m_dwTimeColor;
		DWORD m_dwDescColor;
	};
}

#endif // __UITIMELINE_H__
