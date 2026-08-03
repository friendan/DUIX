#ifndef __UITABBAR_H__
#define __UITABBAR_H__

#pragma once

#include <vector>

namespace DuiLib
{
	class CTabButtonUI;
	class CButtonUI;
	class CSvgBoxUI;
	class CTabLayoutUI;
	class CMenuUI;

	// 标签栏：水平标签、内置「+」、左右滚动/滚轮溢出、关闭、中键/Ctrl+W、拖拽排序；可 bind-tab-layout-name
	class UILIB_API CTabBarUI : public CHorizontalLayoutUI, public IMessageFilterUI
	{
		DECLARE_DUICONTROL(CTabBarUI)
	public:
		CTabBarUI();
		~CTabBarUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const override;
		void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true) override;
		LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

		CTabButtonUI* AddTab(LPCTSTR pstrTitle);
		CTabButtonUI* InsertTab(int iIndex, LPCTSTR pstrTitle);
		void RemoveTab(int iIndex);
		void RemoveTab(CTabButtonUI* pTab);
		void RemoveAllTabs();
		void RemoveUnlockedTabs();
		void RemoveOtherTabs(int iKeepIndex);
		void RemoveTabsToLeft(int iIndex);
		void RemoveTabsToRight(int iIndex);

		// 锁定标签固定在左侧；拖拽时钉住区与普通区互不跨越
		int GetPinnedCount() const;
		void EnsurePinnedTabsLeft();
		void OnTabLockChanged(CTabButtonUI* pTab);

		// 右键菜单：默认内置（纯代码，无需皮肤 XML）；contextmenu="false" 关闭；
		// 也可指定自定义 XML 文件路径或内联 XML 字符串（以 '<' 开头）
		void SetContextMenuEnabled(bool bEnable);
		bool IsContextMenuEnabled() const { return m_bContextMenuEnabled; }
		void SetContextMenuXml(LPCTSTR pstrXml);
		LPCTSTR GetContextMenuXml() const { return m_sContextMenuXml; }

		int GetTabCount() const;
		CTabButtonUI* GetTab(int iIndex) const;
		int GetTabIndex(CTabButtonUI* pTab) const;
		int FindTabByTitle(LPCTSTR pstrTitle) const;

		bool SetActiveTab(int iIndex, bool bCheckAllow = true);
		int GetActiveTab() const { return m_iActive; }
		CTabButtonUI* GetActiveTabButton() const;
		bool ActivateNextTab(bool bCheckAllow = true);
		bool ActivatePrevTab(bool bCheckAllow = true);

		// tab-width>0 为固定宽度；flexible=true 时均分可视区，受 tab-min/max-width 约束（0=该侧无限制）
		void SetTabWidth(int nWidth);
		int GetTabWidth() const { return m_nTabWidth; }
		void SetFlexibleTabWidth(bool bFlexible);
		bool IsFlexibleTabWidth() const { return m_bFlexibleWidth; }
		void SetTabMinWidth(int nWidth);
		int GetTabMinWidth() const { return m_nTabMinWidth; }
		void SetTabMaxWidth(int nWidth);
		int GetTabMaxWidth() const { return m_nTabMaxWidth; }

		void SetScrollBtnWidth(int nWidth);
		int GetScrollBtnWidth() const { return m_nScrollBtnWidth; }

		void SetShowAdd(bool bShow);
		bool IsShowAdd() const { return m_bShowAdd; }
		void SetAddBtnWidth(int nWidth);
		int GetAddBtnWidth() const { return m_nAddBtnWidth; }

		// 标签皮肤（未单独配置时 TabButton 使用这些默认色）
		void SetTabBackgroundColor(DWORD dwColor);
		DWORD GetTabBackgroundColor() const { return m_dwTabBackgroundColor; }
		void SetTabHoverBackgroundColor(DWORD dwColor);
		DWORD GetTabHoverBackgroundColor() const { return m_dwTabHoverBackgroundColor; }
		void SetTabSelectedBackgroundColor(DWORD dwColor);
		DWORD GetTabSelectedBackgroundColor() const { return m_dwTabSelectedBackgroundColor; }
		void SetTabColor(DWORD dwColor);
		DWORD GetTabColor() const { return m_dwTabColor; }
		void SetTabHoverColor(DWORD dwColor);
		DWORD GetTabHoverColor() const { return m_dwTabHoverColor; }
		void SetTabSelectedColor(DWORD dwColor);
		DWORD GetTabSelectedColor() const { return m_dwTabSelectedColor; }
		void SetTabBorderColor(DWORD dwColor);
		DWORD GetTabBorderColor() const { return m_dwTabBorderColor; }
		void SetTabSelectedBorderColor(DWORD dwColor);
		DWORD GetTabSelectedBorderColor() const { return m_dwTabSelectedBorderColor; }
		void SetTabBorderWidth(int nSize);
		int GetTabBorderWidth() const { return m_nTabBorderWidth; }
		void SetTabSelectedBorderWidth(int nSize);
		int GetTabSelectedBorderWidth() const { return m_nTabSelectedBorderWidth; }
		void SetShowTabSeparator(bool bShow);
		bool IsShowTabSeparator() const { return m_bShowTabSeparator; }
		void SetTabSeparatorColor(DWORD dwColor);
		DWORD GetTabSeparatorColor() const { return m_dwTabSeparatorColor; }
		void SetCloseColor(DWORD dwColor);
		DWORD GetCloseColor() const { return m_dwCloseColor; }
		void SetCloseHoverBackgroundColor(DWORD dwColor);
		DWORD GetCloseHoverBackgroundColor() const { return m_dwCloseHoverBackgroundColor; }
		void SetCloseHoverColor(DWORD dwColor);
		DWORD GetCloseHoverColor() const { return m_dwCloseHoverColor; }
		void RefreshTabStyles();

		// 将 iFrom 处标签移动到最终下标 iTo（插入语义，非互换）
		void MoveTab(int iFrom, int iTo);

		void BindTabLayoutName(LPCTSTR pstrName);
		LPCTSTR GetBindTabLayoutName() const { return m_sBindTabLayoutName; }

		// 在 tabclosing / tabselecting 通知处理里调用，可取消本次操作
		void CancelNotify();
		void ResetNotifyCancel();
		bool IsNotifyCanceled() const { return m_bNotifyCancel; }

		bool Add(CControlUI* pControl) override;
		bool AddAt(CControlUI* pControl, int iIndex) override;
		bool Remove(CControlUI* pControl) override;
		void RemoveAll() override;

		void SetPos(RECT rc, bool bNeedInvalidate = true) override;
		void DoEvent(TEventUI& event) override;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl) override;
		// action=title 时仅空白区拖窗；标签 / + / 滚动钮保持可点
		bool IsCaptionDragHit(POINT pt) const override;

		// 标签可视区（‹ › / + 之间）
		RECT GetTabViewportRect() const;
		// 未锁定标签的滚动可视区（钉住区右侧）
		RECT GetScrollViewportRect() const;
		int GetPinnedTabsWidth() const;
		bool IsTabFullyInViewport(const CTabButtonUI* pTab) const;

	protected:
		void EnsureScrollButtons();
		void EnsureAddButton();
		void UpdateScrollInfo();
		void UpdateScrollButtonsPos();
		void UpdateAddButtonPos();
		void UpdateScrollButtonState();
		void SetScrollOffset(int nOffset);
		void ApplyScrollOffset();
		void OffsetUnpinnedTabs(int delta);
		void ScrollToTab(int iIndex);
		void RequestScrollToTab(int iIndex);
		void ScrollByStep(int nDirection);
		int GetScrollStep() const;
		int CalcSnappedScrollOffset(int nTarget, int nDirection = 0) const;
		bool IsTabFullyVisibleAtOffset(int iIndex, int nOffset) const;
		void FlushScrollToLeadingEdge();
		void StretchLastVisibleTab();
		void ShowTabContextMenu(int iTabIndex, POINT ptScreen);
		void BuildBuiltinContextMenu(CMenuUI* pMenu);
		void PrepareContextMenuItems(CMenuUI* pMenu, int iTabIndex);
		bool HandleContextMenuClick(WPARAM wParam);
		CControlUI* FindMenuItemByName(CMenuUI* pMenu, LPCTSTR pstrName) const;
		int GetAddReserveWidth() const;
		DWORD GetChromeBackgroundColor() const;
		bool QueryAllowNotify(LPCTSTR pstrMsg, WPARAM wParam, LPARAM lParam);
		void SyncBoundTabLayout();
		void SyncBoundTabLayoutMove(int iFrom, int iTo);
		void SyncBoundTabLayoutRemove(int iIndex);
		CTabLayoutUI* GetBoundTabLayout() const;
		CTabButtonUI* HitTestTab(POINT pt) const;
		int HitTestScrollButton(POINT pt) const; // -1 left, +1 right, 0 none
		bool HitTestAddButton(POINT pt) const;
		bool IsPointInClose(CTabButtonUI* pTab, POINT pt) const;
		bool IsMouseOverBar() const;
		bool IsFocusInside(CControlUI* pFocus) const;
		void RebuildTabListFromChildren();
		int GetTabAppendChildIndex() const;
		int EstimateTabsWidth() const;
		int CalcFlexibleTabWidth(int fullView) const;
		void ApplyTabWidths(int fullView = -1);
		void PlaceChromeFloat(CControlUI* pCtrl, const RECT& rc);
		void DestroyChromeIcons();
		void NotifyAddTab();
		void PaintChromeBackplates(IRenderContext& ctx, const RECT& rcPaint);
		void BeginDragGhost(CTabButtonUI* pTab, POINT ptClient);
		void EndDragGhost();
		void UpdateDragGhost(POINT ptClient);
		void PaintDragGhost(IRenderContext& ctx);
		void PaintDragDropIndicator(IRenderContext& ctx);
		int ClampMoveTarget(int iFrom, int iTo) const;

		std::vector<CTabButtonUI*> m_tabs;
		std::vector<bool> m_tabHover;
		int m_iActive;
		int m_nScrollOffset;
		int m_nMaxScrollOffset;
		int m_nTabWidth;
		int m_nTabMinWidth;
		int m_nTabMaxWidth;
		bool m_bFlexibleWidth;
		int m_nScrollBtnWidth;
		int m_nAddBtnWidth;
		int m_nDragSrcIdx;
		int m_iDragHoverIdx;
		int m_iPendingScrollTab;
		POINT m_ptDragDown;
		POINT m_ptDragMouse;
		POINT m_ptDragHotspot;
		SIZE m_szDragGhost;
		bool m_bDragging;
		CDuiString m_sBindTabLayoutName;
		CDuiString m_sContextMenuXml;
		int m_iContextMenuTab;
		bool m_bContextMenuPending;
		bool m_bContextMenuEnabled;
		bool m_bApplyingScroll;
		bool m_bOverflow;
		bool m_bRelayouting;
		bool m_bNotifyCancel;
		bool m_bShowAdd;
		bool m_bAddHover;
		bool m_bEnsuringPinned;
		int m_nScrollHover; // -1 left, +1 right, 0 none
		CButtonUI* m_pBtnLeft;
		CButtonUI* m_pBtnRight;
		CButtonUI* m_pBtnAdd;
		CSvgBoxUI* m_pIconLeft;
		CSvgBoxUI* m_pIconRight;
		CSvgBoxUI* m_pIconAdd;

		DWORD m_dwTabBackgroundColor;
		DWORD m_dwTabHoverBackgroundColor;
		DWORD m_dwTabSelectedBackgroundColor;
		DWORD m_dwTabColor;
		DWORD m_dwTabHoverColor;
		DWORD m_dwTabSelectedColor;
		DWORD m_dwTabBorderColor;
		DWORD m_dwTabSelectedBorderColor;
		DWORD m_dwTabSeparatorColor;
		int m_nTabBorderWidth;
		int m_nTabSelectedBorderWidth;
		bool m_bShowTabSeparator;
		DWORD m_dwCloseColor;
		DWORD m_dwCloseHoverBackgroundColor;
		DWORD m_dwCloseHoverColor;
	};
}

#endif // __UITABBAR_H__
