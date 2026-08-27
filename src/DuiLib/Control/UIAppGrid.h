#ifndef __UIAPPGRID_H__
#define __UIAPPGRID_H__

#pragma once

namespace DuiLib
{
	/// 自适应列×行的应用图标网格：按页切片、底部分页圆点、拖拽换位（拖到圆点 / 方向键跨页）。
	/// 子节点任意 CControlUI（Demo 用 AppIcon）；不做文件夹 / 长按菜单。
	class UILIB_API CAppGridUI : public CContainerUI, public IMessageFilterUI
	{
		DECLARE_DUICONTROL(CAppGridUI)
	public:
		CAppGridUI();
		~CAppGridUI();

		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;
		UINT GetControlFlags() const override;
		void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true) override;
		LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;
		/// action=title/move：空白区可拖窗；图标格与分页条保持客户区
		bool IsCaptionDragHit(POINT pt) const override;
		bool PreferClientHit() const override;

		bool Add(CControlUI* pControl) override;
		bool AddAt(CControlUI* pControl, int iIndex) override;
		bool Remove(CControlUI* pControl) override;
		void RemoveAll() override;

		void SetPos(RECT rc, bool bNeedInvalidate = true) override;
		CControlUI* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags) override;
		void SetScrollPos(SIZE szPos, bool bMsg = true) override;
		void DoCaptureEvent(TEventUI& event) override;
		void DoEvent(TEventUI& event) override;
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl) override;
		bool DoPaintContent(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl);
		void DoPostPaint(IRenderContext& ctx, const RECT& rcPaint) override;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;

		/// 不按子项估尺寸；宽/高为 0 表示由父布局撑满（避免只剩一格 AppIcon 大小）
		SIZE EstimateSize(SIZE szAvailable) override;

		SIZE GetItemSize() const;
		void SetItemSize(SIZE szItem);

		int GetPage() const { return m_nPageIndex; }
		void SetPage(int nPage);
		int GetPageCount() const;
		void NextPage();
		void PrevPage();

		/// 将含该网格下标的页设为当前页（已在当前页则无操作）
		bool EnsureItemVisible(int iGridIndex);
		bool SetPageByItem(int iGridIndex) { return EnsureItemVisible(iGridIndex); }

		bool IsDraggable() const { return m_bDraggable; }
		void SetDraggable(bool b);

		bool IsShowPageDots() const { return m_bShowPageDots; }
		void SetShowPageDots(bool b);

		/// 连续滚动（像列表）：铺开全部可见项 + 竖滚动条；关闭分页圆点与按页切片
		bool IsScrollMode() const { return m_bScrollMode; }
		void SetScrollMode(bool b);

		/// 分页圆点直径（逻辑像素）；绘制时按可用宽度在 [min,max] 内取尽量大的值
		void SetDotSizeMin(int n);
		int GetDotSizeMin() const { return m_nDotSizeMin; }
		void SetDotSizeMax(int n);
		int GetDotSizeMax() const { return m_nDotSizeMax; }

		int GetColumns() const { return m_nColumns; }
		int GetRows() const { return m_nRows; }
		int GetPerPage() const;

		/// 网格项（跳过 absolute）；下标用于顺序持久化 / EnsureItemVisible（不过滤）
		int GetGridItemCount() const;
		CControlUI* GetGridItemAt(int iGridIndex) const;
		int GetGridIndexOf(CControlUI* pControl) const;
		/// 互换两个网格下标；成功则发 itemmoved（wParam=from, lParam=to）
		bool SwapItems(int iFrom, int iTo, bool bNotify = true);
		/// 将 from 插入到 to 位置（最终下标为 to）；成功发 itemmoved
		bool MoveItem(int iFrom, int iTo);
		/// 按网格下标删除（跳过 absolute）；走 Remove 生命周期
		bool RemoveGridItemAt(int iGridIndex);
		/// 命中当前页格子 → 全局网格下标；未命中 -1
		int HitTestItemIndex(POINT pt) const;

		/// 显示过滤：空串=显示全部；非空时对 text / name 做不区分大小写子串匹配（可叠加回调）
		void SetFilterText(LPCTSTR pstrFilter);
		LPCTSTR GetFilterText() const;
		void ClearFilter();
		bool HasFilter() const;

		/// 自定义谓词；返回 false 则该项不参与分页布局。与 filter-text 同时生效时为 AND
		typedef bool (*FnItemFilter)(CControlUI* pItem, LPVOID pUserData);
		void SetItemFilter(FnItemFilter fn, LPVOID pUserData = NULL);
		void ClearItemFilter();

		bool PassesFilter(CControlUI* pItem) const;
		/// 当前过滤后的可见项（分页 / 命中基于此）
		int GetVisibleItemCount() const;
		CControlUI* GetVisibleItemAt(int iVisible) const;
		int GetVisibleIndexOf(CControlUI* pControl) const;

	protected:
		CControlUI* ResolveGridItem(CControlUI* pFrom) const;
		void RecalcLayoutMetrics(const RECT& rcContent);
		void ApplyPageVisibility();
		void LayoutCurrentPage(const RECT& rcContent);
		void LayoutScrollContent(const RECT& rcContent, int& cyNeeded);
		/// 拖拽结束同步重排并刷新源格（scroll 模式子项脏区易漏）
		void SyncLayoutAfterDrag(int iRestoredIdx);
		void ApplyFilterChanged();
		static bool MatchFilterText(CControlUI* pItem, LPCTSTR pstrFilter);
		RECT GetDotsRect() const;
		void ResolveDotLayout(int& nDiameter, int& nGap) const;
		int HitTestDot(POINT pt) const;
		/// 命中竖/横滚动条（含 thumb）；用于排除拖窗 / 图标拖拽
		CScrollBarUI* HitTestScrollBar(POINT pt) const;
		int HitTestCellIndex(POINT pt, const RECT& rcContent) const;
		CControlUI* HitTestItem(POINT pt) const;
		int ResolveDragHoverIndex(POINT pt) const;
		void UpdateDragHoverAfterPage(POINT pt);
		/// 拖拽中方向键 / PageUp·PageDown 翻页；已处理返回 true
		bool HandleDragPageKey(TEventUI& event);
		void HookChild(CControlUI* pControl);
		void UnhookChild(CControlUI* pControl);
		bool OnChildEvent(void* param);
		bool OnChildNotify(void* param);
		void NotifyItemClick(CControlUI* pItem);
		void NotifyItemRClick(CControlUI* pItem);
		void NotifyItemDbClick(CControlUI* pItem);
		void BeginDragGhost(CControlUI* pItem, POINT ptMouse);
		void UpdateDragGhost(POINT ptMouse);
		void EndDragGhost(int iSrcIdx = -1);
		void ForceRepaintAfterDrag(CControlUI* pItem);
		/// 结束拖拽：仅落在其它图标上才换位；空白 / 滚动条 / 界外均取消并还原
		void FinishDrag(POINT pt, bool bAllowSwap);
		bool QueryDragMouseClient(POINT& pt) const;
		RECT GetDragGhostRect() const;
		void InvalidateDragGhost() const;
		bool ShouldPaintGridChild(CControlUI* pControl) const;
		void PaintPageDots(IRenderContext& ctx);
		void PaintDragHint(IRenderContext& ctx);
		void PaintDragGhost(IRenderContext& ctx);
		DWORD ResolvePrimaryColor() const;
		DWORD ResolveMutedColor() const;
		int ScaleValue(int v) const;
		int DotsBarHeight() const;
		bool ShouldPaintDots() const;

	protected:
		SIZE m_szItem;
		int m_nPageIndex;
		int m_nColumns;
		int m_nRows;
		int m_nCachedItemCount;
		bool m_bDraggable;
		bool m_bShowPageDots;
		bool m_bScrollMode;
		bool m_bDragging;
		bool m_bSuppressChildClick;
		int m_nDragSrcIdx;
		int m_nDragHoverIdx;
		int m_nDotSizeMin;
		int m_nDotSizeMax;
		POINT m_ptDragDown;
		RECT m_rcContent;
		RECT m_rcDots;
		CDuiString m_sFilterText;
		FnItemFilter m_pfnItemFilter;
		LPVOID m_pFilterUserData;
		/// AppGrid 自行拖滑块（不依赖 ScrollBar 定时器路径）
		bool m_bScrollThumbDragging;
		POINT m_ptScrollThumbDown;
		int m_nScrollThumbPos0;
		int m_nScrollThumbTrack;
		/// 拖拽跟手：热点 / 尺寸；BeginDragGhost 时 GDI 快照，翻页后仍可用
		POINT m_ptDragMouse;
		POINT m_ptDragHotspot;
		SIZE m_szDragGhost;
		HBITMAP m_hDragGhostBmp;
		CControlUI* m_pDragHideItem;
		bool m_bFinishDragGuard;
		RECT m_rcDragSourceSlot;
	};
}

#endif // __UIAPPGRID_H__
