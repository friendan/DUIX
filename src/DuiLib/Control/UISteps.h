#ifndef __UISTEPS_H__
#define __UISTEPS_H__

#pragma once

namespace DuiLib
{
	/// 步骤项（数据节点，由父 Steps 绘制）。
	class UILIB_API CStepItemUI : public CControlUI
	{
		DECLARE_DUICONTROL(CStepItemUI)
	public:
		enum Status { StatusAuto = 0, StatusWait, StatusProcess, StatusFinish, StatusError };

		CStepItemUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetDescription(LPCTSTR pstr);
		LPCTSTR GetDescription() const;
		void SetStatus(Status e);
		Status GetStatus() const;

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl);
		SIZE EstimateSize(SIZE szAvailable);

	protected:
		CDuiString m_sDescription;
		Status m_eStatus;
	};

	/// 步骤条：水平 / 垂直；current 之前为完成、当前进行、之后等待。
	class UILIB_API CStepsUI : public CContainerUI
	{
		DECLARE_DUICONTROL(CStepsUI)
	public:
		enum Direction { Horizontal = 0, Vertical };

		CStepsUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;
		bool PreferClientHit() const;

		void SetCurrent(int nIndex);
		int GetCurrent() const;
		void SetDirection(Direction e);
		Direction GetDirection() const;
		void SetClickable(bool b);
		bool IsClickable() const;
		void SetItems(LPCTSTR pstrItems); // title|title|…

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void SetPos(RECT rc, bool bNeedInvalidate = true);
		SIZE EstimateSize(SIZE szAvailable);
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl);
		void DoEvent(TEventUI& event);

	protected:
		int ScaleValue(int v) const;
		void EnsureFromItemsAttr();
		void LayoutHitRects();
		int HitTest(POINT pt) const;
		CStepItemUI::Status ResolveStatus(int index, CStepItemUI* pItem) const;
		void PaintHorizontal(IRenderContext& ctx);
		void PaintVertical(IRenderContext& ctx);

	protected:
		int m_nCurrent;
		Direction m_eDirection;
		bool m_bClickable;
		CDuiString m_sItemsAttr;
		bool m_bItemsApplied;
		int m_nDotSize;
		int m_nHoverIndex;
		CDuiRect m_rcHits[64];
		int m_nHitCount;

		DWORD m_dwFinishColor;
		DWORD m_dwProcessColor;
		DWORD m_dwWaitColor;
		DWORD m_dwErrorColor;
		DWORD m_dwTitleColor;
		DWORD m_dwDescColor;
		DWORD m_dwWaitTitleColor;
	};
}

#endif // __UISTEPS_H__
