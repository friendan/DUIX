#ifndef __UIVIRTUALLIST_H__
#define __UIVIRTUALLIST_H__

#pragma once

namespace DuiLib
{
	class CVirtualListUI;

	/// 虚拟列表数据回调：只按可见行取文案 / 可选自绘，避免为每行建控件。
	class IVirtualListCallback
	{
	public:
		virtual ~IVirtualListCallback() {}
		/// 默认绘制用的行文本；可为 NULL
		virtual LPCTSTR GetItemText(CControlUI* pList, int iIndex) = 0;
		/// 返回 true 表示已自绘完整行（跳过默认背景+文字）
		virtual bool PaintItem(CControlUI* pList, IRenderContext& ctx, int iIndex, const RECT& rcItem, UINT uState)
		{
			return false;
		}
	};

	/// 固定行高虚拟列表：滚动范围 = count × height，仅绘制可视行。
	class UILIB_API CVirtualListUI : public CContainerUI
	{
		DECLARE_DUICONTROL(CVirtualListUI)
	public:
		CVirtualListUI();
		~CVirtualListUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;

		void SetItemCount(int nCount);
		int GetItemCount() const;

		void SetItemHeight(int nHeight);
		int GetItemHeight() const;

		void SetCallback(IVirtualListCallback* pCallback);
		IVirtualListCallback* GetCallback() const;

		void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);

		void GetVisibleRange(int& iFirst, int& iLast) const;
		int HitTestItem(POINT pt) const;
		RECT GetItemRect(int iIndex) const;

		int GetCurSel() const;
		bool SelectItem(int iIndex, bool bTakeFocus = false);
		void EnsureVisible(int iIndex);

		int GetHoverIndex() const { return m_iHoverIndex; }

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void SetPos(RECT rc, bool bNeedInvalidate = true);
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl);
		void DoEvent(TEventUI& event);

		// 子项不应通过 Add 塞入；保留容器 API 但不用于虚拟行
		bool Add(CControlUI* pControl);
		bool AddAt(CControlUI* pControl, int iIndex);

	protected:
		RECT GetListClientRect() const;
		int GetScaledItemHeight() const;
		void PaintItemDefault(IRenderContext& ctx, int iIndex, const RECT& rcItem, UINT uState);
		bool PaintItemsAndScrollBars(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl);
		void UpdateHover(POINT pt);
		void SyncScrollLineSize();

	protected:
		int m_nItemCount;
		int m_nItemHeight;
		int m_iCurSel;
		int m_iHoverIndex;
		IVirtualListCallback* m_pCallback;

		int m_iItemFont;
		UINT m_uItemTextStyle;
		RECT m_rcItemPadding;
		DWORD m_dwItemColor;
		DWORD m_dwItemBackgroundColor;
		DWORD m_dwItemHoverColor;
		DWORD m_dwItemHoverBackgroundColor;
		DWORD m_dwItemSelectedColor;
		DWORD m_dwItemSelectedBackgroundColor;
		DWORD m_dwItemDisabledColor;
		DWORD m_dwItemDisabledBackgroundColor;
		DWORD m_dwItemLineColor;
		DWORD m_dwItemAlternateBackgroundColor;
		bool m_bAlternateBk;
		bool m_bShowRowLine;
		bool m_bShowHtml;
	};
}

#endif // __UIVIRTUALLIST_H__
