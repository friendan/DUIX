#include "StdAfx.h"
#include "UIAppGrid.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CAppGridUI)

	static const int kDragThreshold = 5;
	static const int kDotBarPadLogic = 8;   // 圆点条相对直径上下留白
	static const int kDotGapMinLogic = 6;   // 圆点间距下限

	/// 拖影快照：纯 GDI 绘制到 top-down DIB（D2D 离屏不写回 DIB；GenerateBitmap 会上下颠倒）
	static HBITMAP CaptureDragGhostBitmap(CPaintManagerUI* pManager, CControlUI* pItem, const RECT& rcItem)
	{
		if( pManager == NULL || pItem == NULL ) return NULL;
		int cx = rcItem.right - rcItem.left;
		int cy = rcItem.bottom - rcItem.top;
		if( cx < 1 || cy < 1 ) return NULL;

		HDC hRefDC = pManager->GetPaintDC();
		if( hRefDC == NULL ) return NULL;

		BITMAPINFO bmi = {};
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = cx;
		bmi.bmiHeader.biHeight = -cy;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;

		LPVOID pBits = NULL;
		HBITMAP hBmp = ::CreateDIBSection(hRefDC, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
		if( hBmp == NULL ) return NULL;
		if( pBits != NULL )
			::ZeroMemory(pBits, (SIZE_T)cx * cy * 4);

		HDC hMemDC = ::CreateCompatibleDC(hRefDC);
		if( hMemDC == NULL ) {
			::DeleteObject(hBmp);
			return NULL;
		}
		HBITMAP hOldBmp = (HBITMAP)::SelectObject(hMemDC, hBmp);
		{
			CGdiRenderContext ctx(hMemDC, pManager);
			RECT rcLocal = { 0, 0, cx, cy };
			RECT rcSave = pItem->GetPos();
			pItem->SetInternVisible(true);
			pItem->SetPos(rcLocal, false);
			pItem->Paint(ctx, rcLocal, NULL);
			pItem->SetPos(rcSave, false);
		}
		::SelectObject(hMemDC, hOldBmp);
		::DeleteDC(hMemDC);
		return hBmp;
	}

	CAppGridUI::CAppGridUI()
		: m_nPageIndex(0)
		, m_nColumns(1)
		, m_nRows(1)
		, m_nCachedItemCount(0)
		, m_bDraggable(true)
		, m_bShowPageDots(true)
		, m_bScrollMode(false)
		, m_bDragging(false)
		, m_bSuppressChildClick(false)
		, m_nDragSrcIdx(-1)
		, m_nDragHoverIdx(-1)
		, m_nDotSizeMin(6)
		, m_nDotSizeMax(14)
		, m_pfnItemFilter(NULL)
		, m_pFilterUserData(NULL)
		, m_bScrollThumbDragging(false)
		, m_nScrollThumbPos0(0)
		, m_nScrollThumbTrack(0)
		, m_hDragGhostBmp(NULL)
		, m_pDragHideItem(NULL)
		, m_bFinishDragGuard(false)
	{
		m_szDragGhost.cx = m_szDragGhost.cy = 0;
		m_ptDragMouse.x = m_ptDragMouse.y = 0;
		m_ptDragHotspot.x = m_ptDragHotspot.y = 0;
		m_szItem.cx = 72;
		m_szItem.cy = 88;
		m_ptDragDown.x = m_ptDragDown.y = 0;
		m_ptScrollThumbDown.x = m_ptScrollThumbDown.y = 0;
		::ZeroMemory(&m_rcContent, sizeof(m_rcContent));
		::ZeroMemory(&m_rcDots, sizeof(m_rcDots));
		::ZeroMemory(&m_rcDragSourceSlot, sizeof(m_rcDragSourceSlot));
		SetGap(8);
		// 保持 mouse-child，子 AppIcon 才能收到原生 hover；拖拽走 DoCaptureEvent
		SetMouseChildEnabled(true);
		SetShowScrollbar(false);
		// 空白区拖窗（与 TitleBar / html action:title 同一套）；图标 / 圆点见 IsCaptionDragHit
		SetAction(UIACTION_TITLE);
	}

	CAppGridUI::~CAppGridUI()
	{
		if( m_pManager != NULL )
			m_pManager->RemovePreMessageFilter(this);
		EndDragGhost();
		for( int i = 0; i < m_items.GetSize(); ++i )
			UnhookChild(static_cast<CControlUI*>(m_items[i]));
	}

	void CAppGridUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
	{
		if( m_pManager != NULL ) {
			m_pManager->RemovePreMessageFilter(this);
			m_pManager->RemovePostPaint(this);
		}
		CContainerUI::SetManager(pManager, pParent, bInit);
		if( m_pManager != NULL )
			m_pManager->AddPreMessageFilter(this);
	}

	static bool IsTextInputFocus(CControlUI* pFocus)
	{
		if( pFocus == NULL ) return false;
		return pFocus->GetInterface(DUI_CTR_EDIT) != NULL
			|| pFocus->GetInterface(DUI_CTR_RICHEDIT) != NULL;
	}

	LRESULT CAppGridUI::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
	{
		(void)lParam;
		bHandled = false;

		// Toast 等弹窗会夺 capture：左键仍按住则夺回，否则松手消息进不了主窗 → 拖影卡死
		if( uMsg == WM_CAPTURECHANGED ) {
			if( m_bFinishDragGuard ) return 0;
			if( !(m_bDragging || m_nDragSrcIdx >= 0 || m_pDragHideItem != NULL) ) return 0;
			if( (::GetKeyState(VK_LBUTTON) & 0x8000) != 0 ) {
				if( m_pManager != NULL )
					m_pManager->SetCapture();
				return 0;
			}
			POINT pt = m_ptDragMouse;
			QueryDragMouseClient(pt);
			FinishDrag(pt, false);
			return 0;
		}
		if( uMsg == WM_LBUTTONUP ) {
			if( m_bFinishDragGuard ) return 0;
			if( m_bDragging || m_nDragSrcIdx >= 0 || m_pDragHideItem != NULL ) {
				POINT pt = m_ptDragMouse;
				QueryDragMouseClient(pt);
				FinishDrag(pt, true);
			}
			return 0;
		}

		// 捕获期间跟手：用真实光标位置，勿信 WM_MOUSEMOVE 的 lParam（Toast/重绘后可能为 0,0）
		if( uMsg == WM_MOUSEMOVE && (m_bDragging || m_pDragHideItem != NULL) ) {
			if( (::GetKeyState(VK_LBUTTON) & 0x8000) == 0 ) {
				POINT pt = m_ptDragMouse;
				QueryDragMouseClient(pt);
				FinishDrag(pt, true);
				return 0;
			}
			// Toast 夺 capture 后主窗可能收不到 CAPTURECHANGED 时序；移动时主动夺回
			if( m_pManager != NULL ) {
				HWND hWnd = m_pManager->GetPaintWindow();
				if( hWnd != NULL && ::GetCapture() != hWnd )
					m_pManager->SetCapture();
			}
			UpdateDragGhost(m_ptDragMouse);
		}

		if( uMsg != WM_KEYDOWN ) return 0;
		if( !m_bDragging || m_bScrollMode || !IsVisible() || !IsEnabled() ) return 0;
		if( (::GetKeyState(VK_LBUTTON) & 0x8000) == 0 ) return 0;
		if( GetPageCount() <= 1 ) return 0;

		CControlUI* pFocus = (m_pManager != NULL) ? m_pManager->GetFocus() : NULL;
		if( IsTextInputFocus(pFocus) ) return 0;

		switch( wParam ) {
		case VK_LEFT:
		case VK_UP:
		case VK_PRIOR:
		case VK_RIGHT:
		case VK_DOWN:
		case VK_NEXT:
			break;
		default:
			return 0;
		}

		TEventUI event = { 0 };
		event.Type = UIEVENT_KEYDOWN;
		event.chKey = (TCHAR)wParam;
		event.ptMouse = (m_pManager != NULL) ? m_pManager->GetMousePos() : m_ptDragMouse;
		if( HandleDragPageKey(event) ) {
			bHandled = true;
			return 0;
		}
		return 0;
	}

	LPCTSTR CAppGridUI::GetClass() const
	{
		return _T("AppGridUI");
	}

	LPVOID CAppGridUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_APPGRID) == 0 )
			return static_cast<CAppGridUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	UINT CAppGridUI::GetControlFlags() const
	{
		// 无此标志时 WM_SETCURSOR 不会下发 UIEVENT_SETCURSOR，圆点无法变手型
		return CContainerUI::GetControlFlags() | UIFLAG_SETCURSOR;
	}

	bool CAppGridUI::IsCaptionDragHit(POINT pt) const
	{
		UIAction a = GetAction();
		if( a != UIACTION_TITLE && a != UIACTION_MOVEWINDOW ) return false;
		if( !::PtInRect(&m_rcItem, pt) ) return false;
		// 滚动条 / 右侧预留槽须保持 HTCLIENT，否则 action=title 会把滑块拖成拖窗
		if( HitTestScrollBar(pt) != NULL ) return false;
		if( m_bScrollMode && m_pVerticalScrollBar != NULL && pt.x >= m_rcContent.right )
			return false;
		// 分页条整段可点，不拖窗
		if( ShouldPaintDots() && ::PtInRect(&m_rcDots, pt) ) return false;
		// 当前页图标格（含子控件命中）不拖窗
		if( HitTestItem(pt) != NULL ) return false;
		return true;
	}

	SIZE CAppGridUI::EstimateSize(SIZE szAvailable)
	{
		// 与 LinearLayout 相同：只用自身 fixed；0 = 父布局分摊，切勿用单个子项当网格尺寸
		return CControlUI::EstimateSize(szAvailable);
	}

	bool CAppGridUI::Add(CControlUI* pControl)
	{
		if( !CContainerUI::Add(pControl) ) return false;
		HookChild(pControl);
		return true;
	}

	bool CAppGridUI::AddAt(CControlUI* pControl, int iIndex)
	{
		if( !CContainerUI::AddAt(pControl, iIndex) ) return false;
		HookChild(pControl);
		return true;
	}

	bool CAppGridUI::Remove(CControlUI* pControl)
	{
		UnhookChild(pControl);
		return CContainerUI::Remove(pControl);
	}

	void CAppGridUI::RemoveAll()
	{
		for( int i = 0; i < m_items.GetSize(); ++i )
			UnhookChild(static_cast<CControlUI*>(m_items[i]));
		m_bDragging = false;
		m_nDragSrcIdx = -1;
		m_nDragHoverIdx = -1;
		EndDragGhost();
		CContainerUI::RemoveAll();
	}

	void CAppGridUI::HookChild(CControlUI* pControl)
	{
		if( pControl == NULL ) return;
		pControl->OnEvent += MakeDelegate(this, &CAppGridUI::OnChildEvent);
		pControl->OnNotify += MakeDelegate(this, &CAppGridUI::OnChildNotify);
	}

	void CAppGridUI::UnhookChild(CControlUI* pControl)
	{
		if( pControl == NULL ) return;
		pControl->OnEvent -= MakeDelegate(this, &CAppGridUI::OnChildEvent);
		pControl->OnNotify -= MakeDelegate(this, &CAppGridUI::OnChildNotify);
	}

	void CAppGridUI::NotifyItemClick(CControlUI* pItem)
	{
		if( pItem == NULL || m_pManager == NULL ) return;
		int gi = GetGridIndexOf(pItem);
		if( gi < 0 ) return;
		m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMCLICK, (WPARAM)gi, (LPARAM)pItem);
	}

	void CAppGridUI::NotifyItemRClick(CControlUI* pItem)
	{
		if( pItem == NULL || m_pManager == NULL ) return;
		int gi = GetGridIndexOf(pItem);
		if( gi < 0 ) return;
		m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMRCLICK, (WPARAM)gi, (LPARAM)pItem);
	}

	void CAppGridUI::NotifyItemDbClick(CControlUI* pItem)
	{
		if( pItem == NULL || m_pManager == NULL ) return;
		int gi = GetGridIndexOf(pItem);
		if( gi < 0 ) return;
		m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMDBCLICK, (WPARAM)gi, (LPARAM)pItem);
	}

	bool CAppGridUI::OnChildNotify(void* param)
	{
		TNotifyUI* pMsg = static_cast<TNotifyUI*>(param);
		if( pMsg == NULL || pMsg->pSender == NULL ) return true;
		if( pMsg->sType == DUI_MSGTYPE_CLICK ) {
			CControlUI* pItem = ResolveGridItem(pMsg->pSender);
			if( pItem != NULL )
				NotifyItemClick(pItem);
		}
		return true;
	}

	bool CAppGridUI::OnChildEvent(void* param)
	{
		TEventUI* pEvent = static_cast<TEventUI*>(param);
		if( pEvent == NULL ) return true;
		// 拖拽换位后仍走子控件 BUTTONUP 以清 PUSHED/CAPTURED，但把点移出避免 Activate/click
		if( pEvent->Type == UIEVENT_BUTTONUP && m_bSuppressChildClick ) {
			pEvent->ptMouse.x = INT_MIN;
			pEvent->ptMouse.y = INT_MIN;
			m_bSuppressChildClick = false;
			return true;
		}
		if( pEvent->Type == UIEVENT_DBLCLICK ) {
			CControlUI* pItem = ResolveGridItem(pEvent->pSender);
			if( pItem == NULL )
				pItem = HitTestItem(pEvent->ptMouse);
			if( pItem != NULL )
				NotifyItemDbClick(pItem);
			return true;
		}
		if( pEvent->Type == UIEVENT_RBUTTONUP ) {
			CControlUI* pItem = ResolveGridItem(pEvent->pSender);
			if( pItem == NULL )
				pItem = HitTestItem(pEvent->ptMouse);
			if( pItem != NULL )
				NotifyItemRClick(pItem);
		}
		return true;
	}

	int CAppGridUI::ScaleValue(int v) const
	{
		return m_pManager ? m_pManager->GetDPIObj()->Scale(v) : v;
	}

	SIZE CAppGridUI::GetItemSize() const
	{
		if( m_pManager != NULL )
			return m_pManager->GetDPIObj()->Scale(m_szItem);
		return m_szItem;
	}

	void CAppGridUI::SetItemSize(SIZE szItem)
	{
		if( szItem.cx < 1 ) szItem.cx = 1;
		if( szItem.cy < 1 ) szItem.cy = 1;
		if( m_szItem.cx == szItem.cx && m_szItem.cy == szItem.cy ) return;
		m_szItem = szItem;
		NeedUpdate();
	}

	void CAppGridUI::SetDraggable(bool b)
	{
		if( m_bDraggable == b ) return;
		m_bDraggable = b;
		if( !m_bDraggable ) {
			m_bDragging = false;
			m_nDragSrcIdx = -1;
			m_nDragHoverIdx = -1;
			EndDragGhost();
			Invalidate();
		}
	}

	void CAppGridUI::SetShowPageDots(bool b)
	{
		if( m_bShowPageDots == b ) return;
		m_bShowPageDots = b;
		NeedUpdate();
	}

	void CAppGridUI::SetScrollMode(bool b)
	{
		if( m_bScrollMode == b ) return;
		m_bScrollMode = b;
		if( m_bScrollMode ) {
			// 仍用 title：空白可拖窗；滚动条槽由 IsCaptionDragHit 排除
			SetAction(UIACTION_TITLE);
			SetShowScrollbar(true);
			SetFixedScrollbar(true);
			EnableScrollBar(true, false);
			if( m_pVerticalScrollBar != NULL ) {
				m_pVerticalScrollBar->SetShow(true);
				m_pVerticalScrollBar->SetVisible(false); // 由 ProcessScrollBar 按溢出再显示
				if( m_pManager != NULL )
					m_pVerticalScrollBar->SetManager(m_pManager, this, false);
			}
			SIZE szItem = GetItemSize();
			int step = szItem.cy + GetGap();
			if( step > 0 ) SetScrollStepSize(step);
		}
		else {
			SetAction(UIACTION_TITLE);
			SetFixedScrollbar(false);
			EnableScrollBar(false, false);
			SetShowScrollbar(false);
			m_nPageIndex = 0;
			m_bScrollThumbDragging = false;
		}
		NeedUpdate();
	}

	void CAppGridUI::SetDotSizeMin(int n)
	{
		if( n < 2 ) n = 2;
		if( m_nDotSizeMin == n ) return;
		m_nDotSizeMin = n;
		if( m_nDotSizeMax < m_nDotSizeMin ) m_nDotSizeMax = m_nDotSizeMin;
		NeedUpdate();
	}

	void CAppGridUI::SetDotSizeMax(int n)
	{
		if( n < 2 ) n = 2;
		if( m_nDotSizeMax == n ) return;
		m_nDotSizeMax = n;
		if( m_nDotSizeMin > m_nDotSizeMax ) m_nDotSizeMin = m_nDotSizeMax;
		NeedUpdate();
	}

	int CAppGridUI::GetPerPage() const
	{
		int n = m_nColumns * m_nRows;
		return n > 0 ? n : 1;
	}

	int CAppGridUI::GetGridItemCount() const
	{
		int n = 0;
		for( int i = 0; i < m_items.GetSize(); ++i ) {
			CControlUI* p = static_cast<CControlUI*>(m_items[i]);
			if( p == NULL || p->IsAbsolute() ) continue;
			++n;
		}
		return n;
	}

	CControlUI* CAppGridUI::GetGridItemAt(int iGridIndex) const
	{
		if( iGridIndex < 0 ) return NULL;
		int n = 0;
		for( int i = 0; i < m_items.GetSize(); ++i ) {
			CControlUI* p = static_cast<CControlUI*>(m_items[i]);
			if( p == NULL || p->IsAbsolute() ) continue;
			if( n == iGridIndex ) return p;
			++n;
		}
		return NULL;
	}

	int CAppGridUI::GetGridIndexOf(CControlUI* pControl) const
	{
		if( pControl == NULL ) return -1;
		int n = 0;
		for( int i = 0; i < m_items.GetSize(); ++i ) {
			CControlUI* p = static_cast<CControlUI*>(m_items[i]);
			if( p == NULL || p->IsAbsolute() ) continue;
			if( p == pControl ) return n;
			++n;
		}
		return -1;
	}

	bool CAppGridUI::MatchFilterText(CControlUI* pItem, LPCTSTR pstrFilter)
	{
		if( pItem == NULL ) return false;
		if( pstrFilter == NULL || *pstrFilter == _T('\0') ) return true;
		CDuiString needle = pstrFilter;
		needle.MakeLower();
		CDuiString hay = pItem->GetText();
		hay.MakeLower();
		if( hay.Find(needle.GetData()) >= 0 ) return true;
		hay = pItem->GetName();
		hay.MakeLower();
		return hay.Find(needle.GetData()) >= 0;
	}

	bool CAppGridUI::PassesFilter(CControlUI* pItem) const
	{
		if( pItem == NULL || pItem->IsAbsolute() ) return false;
		if( m_pfnItemFilter != NULL && !m_pfnItemFilter(pItem, m_pFilterUserData) )
			return false;
		if( !m_sFilterText.IsEmpty() && !MatchFilterText(pItem, m_sFilterText.GetData()) )
			return false;
		return true;
	}

	int CAppGridUI::GetVisibleItemCount() const
	{
		if( !HasFilter() ) return GetGridItemCount();
		int n = 0;
		for( int i = 0; i < m_items.GetSize(); ++i ) {
			CControlUI* p = static_cast<CControlUI*>(m_items[i]);
			if( p == NULL || p->IsAbsolute() ) continue;
			if( PassesFilter(p) ) ++n;
		}
		return n;
	}

	CControlUI* CAppGridUI::GetVisibleItemAt(int iVisible) const
	{
		if( iVisible < 0 ) return NULL;
		if( !HasFilter() ) return GetGridItemAt(iVisible);
		int n = 0;
		for( int i = 0; i < m_items.GetSize(); ++i ) {
			CControlUI* p = static_cast<CControlUI*>(m_items[i]);
			if( p == NULL || p->IsAbsolute() ) continue;
			if( !PassesFilter(p) ) continue;
			if( n == iVisible ) return p;
			++n;
		}
		return NULL;
	}

	int CAppGridUI::GetVisibleIndexOf(CControlUI* pControl) const
	{
		if( pControl == NULL ) return -1;
		if( !HasFilter() ) return GetGridIndexOf(pControl);
		int n = 0;
		for( int i = 0; i < m_items.GetSize(); ++i ) {
			CControlUI* p = static_cast<CControlUI*>(m_items[i]);
			if( p == NULL || p->IsAbsolute() ) continue;
			if( !PassesFilter(p) ) continue;
			if( p == pControl ) return n;
			++n;
		}
		return -1;
	}

	bool CAppGridUI::HasFilter() const
	{
		return !m_sFilterText.IsEmpty() || m_pfnItemFilter != NULL;
	}

	void CAppGridUI::ApplyFilterChanged()
	{
		m_bDragging = false;
		m_nDragSrcIdx = -1;
		m_nDragHoverIdx = -1;
		EndDragGhost();
		m_nPageIndex = 0;
		NeedUpdate();
		if( m_pManager != NULL ) {
			m_pManager->SendNotify(this, DUI_MSGTYPE_FILTERCHANGED,
				(WPARAM)GetVisibleItemCount(), (LPARAM)GetGridItemCount());
		}
	}

	void CAppGridUI::SetFilterText(LPCTSTR pstrFilter)
	{
		CDuiString s = pstrFilter ? pstrFilter : _T("");
		s.Trim();
		if( m_sFilterText == s ) return;
		m_sFilterText = s;
		ApplyFilterChanged();
	}

	LPCTSTR CAppGridUI::GetFilterText() const
	{
		return m_sFilterText.GetData();
	}

	void CAppGridUI::ClearFilter()
	{
		bool changed = !m_sFilterText.IsEmpty() || m_pfnItemFilter != NULL;
		m_sFilterText.Empty();
		m_pfnItemFilter = NULL;
		m_pFilterUserData = NULL;
		if( changed ) ApplyFilterChanged();
	}

	void CAppGridUI::SetItemFilter(FnItemFilter fn, LPVOID pUserData)
	{
		if( m_pfnItemFilter == fn && m_pFilterUserData == pUserData ) return;
		m_pfnItemFilter = fn;
		m_pFilterUserData = pUserData;
		ApplyFilterChanged();
	}

	void CAppGridUI::ClearItemFilter()
	{
		if( m_pfnItemFilter == NULL && m_pFilterUserData == NULL ) return;
		m_pfnItemFilter = NULL;
		m_pFilterUserData = NULL;
		ApplyFilterChanged();
	}

	CControlUI* CAppGridUI::ResolveGridItem(CControlUI* pFrom) const
	{
		for( CControlUI* p = pFrom; p != NULL && p != (CControlUI*)this; p = p->GetParent() ) {
			// 滚动条挂在 Container 上，父级也是 AppGrid，勿当成网格项
			if( p == (CControlUI*)m_pVerticalScrollBar || p == (CControlUI*)m_pHorizontalScrollBar )
				return NULL;
			if( p->GetParent() == (CControlUI*)this && !p->IsAbsolute() )
				return p;
		}
		return NULL;
	}

	CScrollBarUI* CAppGridUI::HitTestScrollBar(POINT pt) const
	{
		if( m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible()
			&& ::PtInRect(&m_pVerticalScrollBar->GetPos(), pt) )
			return m_pVerticalScrollBar;
		if( m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible()
			&& ::PtInRect(&m_pHorizontalScrollBar->GetPos(), pt) )
			return m_pHorizontalScrollBar;
		return NULL;
	}

	int CAppGridUI::GetPageCount() const
	{
		if( m_bScrollMode ) return 1;
		int nItems = m_nCachedItemCount > 0 ? m_nCachedItemCount : GetVisibleItemCount();
		int per = GetPerPage();
		if( nItems <= 0 ) return 1;
		return (nItems + per - 1) / per;
	}

	int CAppGridUI::DotsBarHeight() const
	{
		if( !ShouldPaintDots() ) return 0;
		// 按最大直径预留，绘制时再在 [min,max] 内收缩
		return ScaleValue(m_nDotSizeMax) + ScaleValue(kDotBarPadLogic);
	}

	bool CAppGridUI::ShouldPaintDots() const
	{
		if( m_bScrollMode || !m_bShowPageDots ) return false;
		return GetPageCount() > 1;
	}

	void CAppGridUI::ResolveDotLayout(int& nDiameter, int& nGap) const
	{
		int nPages = GetPageCount();
		if( nPages < 1 ) nPages = 1;

		int minD = ScaleValue(m_nDotSizeMin);
		int maxD = ScaleValue(m_nDotSizeMax);
		if( minD < 2 ) minD = 2;
		if( maxD < minD ) maxD = minD;
		int minGap = ScaleValue(kDotGapMinLogic);
		if( minGap < 2 ) minGap = 2;

		int availW = m_rcDots.right - m_rcDots.left;
		int availH = m_rcDots.bottom - m_rcDots.top;
		if( availW < 1 ) availW = 1;
		if( availH > 0 && maxD > availH ) maxD = availH;
		if( maxD < minD ) maxD = minD;

		int d = maxD;
		for( ; d >= minD; --d ) {
			int gap = (minGap > d) ? minGap : d;
			int total = nPages * d + (nPages > 1 ? (nPages - 1) * gap : 0);
			if( total <= availW ) break;
		}
		if( d < minD ) d = minD;

		nDiameter = d;
		nGap = (minGap > d) ? minGap : d;
	}

	void CAppGridUI::SetPage(int nPage)
	{
		if( m_bScrollMode ) return;
		int nCount = GetPageCount();
		if( nCount < 1 ) nCount = 1;
		if( nPage < 0 ) nPage = 0;
		if( nPage >= nCount ) nPage = nCount - 1;
		if( m_nPageIndex == nPage ) return;
		int nOld = m_nPageIndex;
		m_nPageIndex = nPage;
		NeedUpdate();
		if( m_pManager != NULL )
			m_pManager->SendNotify(this, DUI_MSGTYPE_PAGECHANGED, (WPARAM)m_nPageIndex, (LPARAM)nOld);
	}

	void CAppGridUI::NextPage()
	{
		if( m_bScrollMode ) return;
		int nCount = GetPageCount();
		if( nCount < 1 || m_nPageIndex + 1 >= nCount ) return;
		SetPage(m_nPageIndex + 1);
	}

	void CAppGridUI::PrevPage()
	{
		if( m_bScrollMode ) return;
		if( m_nPageIndex <= 0 ) return;
		SetPage(m_nPageIndex - 1);
	}

	bool CAppGridUI::EnsureItemVisible(int iGridIndex)
	{
		CControlUI* p = GetGridItemAt(iGridIndex);
		if( p == NULL ) return false;
		int iVis = GetVisibleIndexOf(p);
		if( iVis < 0 ) return false;

		if( m_bScrollMode ) {
			SIZE szItem = GetItemSize();
			int iGap = GetGap();
			int cols = m_nColumns > 0 ? m_nColumns : 1;
			int row = iVis / cols;
			int y = row * (szItem.cy + iGap);
			int viewH = m_rcContent.bottom - m_rcContent.top;
			if( viewH < 1 ) viewH = 1;
			SIZE sp = GetScrollPos();
			if( y < sp.cy )
				sp.cy = y;
			else if( y + szItem.cy > sp.cy + viewH )
				sp.cy = y + szItem.cy - viewH;
			else
				return true;
			if( sp.cy < 0 ) sp.cy = 0;
			SetScrollPos(sp);
			return true;
		}

		int per = GetPerPage();
		if( per < 1 ) per = 1;
		int page = iVis / per;
		if( page == m_nPageIndex ) return true;
		SetPage(page);
		return true;
	}

	void CAppGridUI::RecalcLayoutMetrics(const RECT& rcContent)
	{
		SIZE szItem = GetItemSize();
		int iGap = GetGap();
		int cw = rcContent.right - rcContent.left;
		int ch = rcContent.bottom - rcContent.top;
		if( cw < 1 ) cw = 1;
		if( ch < 1 ) ch = 1;

		int cellW = szItem.cx + iGap;
		int cellH = szItem.cy + iGap;
		if( cellW < 1 ) cellW = 1;
		if( cellH < 1 ) cellH = 1;

		// (content + gap) / (item + gap) ≡ 末格无需尾 gap
		m_nColumns = (cw + iGap) / cellW;
		if( m_nColumns < 1 ) m_nColumns = 1;

		if( m_bScrollMode ) {
			int nVis = m_nCachedItemCount > 0 ? m_nCachedItemCount : GetVisibleItemCount();
			m_nRows = nVis > 0 ? ((nVis + m_nColumns - 1) / m_nColumns) : 1;
			if( m_nRows < 1 ) m_nRows = 1;
		}
		else {
			m_nRows = (ch + iGap) / cellH;
			if( m_nRows < 1 ) m_nRows = 1;
		}
	}

	void CAppGridUI::ApplyPageVisibility()
	{
		if( m_bScrollMode ) {
			for( int i = 0; i < m_items.GetSize(); ++i ) {
				CControlUI* p = static_cast<CControlUI*>(m_items[i]);
				if( p == NULL || p->IsAbsolute() ) continue;
				p->SetVisible(PassesFilter(p));
			}
			return;
		}

		int per = GetPerPage();
		int begin = m_nPageIndex * per;
		int end = begin + per;
		int vi = 0;
		for( int i = 0; i < m_items.GetSize(); ++i ) {
			CControlUI* p = static_cast<CControlUI*>(m_items[i]);
			if( p == NULL || p->IsAbsolute() ) continue;
			if( !PassesFilter(p) ) {
				p->SetVisible(false);
				continue;
			}
			bool onPage = (vi >= begin && vi < end);
			p->SetVisible(onPage);
			++vi;
		}
	}

	void CAppGridUI::LayoutCurrentPage(const RECT& rcContent)
	{
		SIZE szItem = GetItemSize();
		int iGap = GetGap();
		int per = GetPerPage();
		int begin = m_nPageIndex * per;

		for( int slot = 0; slot < per; ++slot ) {
			CControlUI* p = GetVisibleItemAt(begin + slot);
			if( p == NULL ) break;

			int col = slot % m_nColumns;
			int row = slot / m_nColumns;
			RECT rcTile;
			rcTile.left = rcContent.left + col * (szItem.cx + iGap);
			rcTile.top = rcContent.top + row * (szItem.cy + iGap);
			rcTile.right = rcTile.left + szItem.cx;
			rcTile.bottom = rcTile.top + szItem.cy;
			p->SetPos(rcTile);
		}

		// 非本页 / 未通过过滤已 Invisible；绝对定位子项照旧
		for( int i = 0; i < m_items.GetSize(); ++i ) {
			CControlUI* p = static_cast<CControlUI*>(m_items[i]);
			if( p == NULL ) continue;
			if( p->IsAbsolute() )
				SetAbsolutePos(i);
		}
	}

	void CAppGridUI::LayoutScrollContent(const RECT& rcContent, int& cyNeeded)
	{
		SIZE szItem = GetItemSize();
		int iGap = GetGap();
		SIZE szScroll = GetScrollPos();
		int nVis = GetVisibleItemCount();
		int cols = m_nColumns > 0 ? m_nColumns : 1;

		for( int vi = 0; vi < nVis; ++vi ) {
			CControlUI* p = GetVisibleItemAt(vi);
			if( p == NULL ) break;
			int col = vi % cols;
			int row = vi / cols;
			RECT rcTile;
			rcTile.left = rcContent.left + col * (szItem.cx + iGap);
			rcTile.top = rcContent.top + row * (szItem.cy + iGap) - szScroll.cy;
			rcTile.right = rcTile.left + szItem.cx;
			rcTile.bottom = rcTile.top + szItem.cy;
			p->SetPos(rcTile);
		}

		for( int i = 0; i < m_items.GetSize(); ++i ) {
			CControlUI* p = static_cast<CControlUI*>(m_items[i]);
			if( p == NULL ) continue;
			if( p->IsAbsolute() )
				SetAbsolutePos(i);
		}

		if( nVis <= 0 )
			cyNeeded = 0;
		else {
			int rows = (nVis + cols - 1) / cols;
			cyNeeded = rows * szItem.cy + (rows > 0 ? (rows - 1) * iGap : 0);
		}
	}

	RECT CAppGridUI::GetDotsRect() const
	{
		return m_rcDots;
	}

	bool CAppGridUI::PreferClientHit() const
	{
		return CContainerUI::PreferClientHit();
	}

	CControlUI* CAppGridUI::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
	{
		// 右侧槽命中返回 AppGrid 自身，由 DoEvent 直接拖滑块（ScrollBar 鼠标路径在此场景不可靠）
		if( m_bScrollMode && m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible()
			&& (uFlags & UIFIND_HITTEST) != 0 )
		{
			POINT pt = *static_cast<LPPOINT>(pData);
			if( ((uFlags & UIFIND_VISIBLE) == 0 || IsVisible())
				&& IsMouseEnabled()
				&& ::PtInRect(&m_rcItem, pt) && pt.x >= m_rcContent.right )
			{
				return Proc(this, pData);
			}
		}
		return CContainerUI::FindControl(Proc, pData, uFlags);
	}

	void CAppGridUI::SetScrollPos(SIZE szPos, bool bMsg)
	{
		if( !m_bScrollMode ) {
			CContainerUI::SetScrollPos(szPos, bMsg);
			return;
		}
		// 内容坐标已含 scroll 偏移：勿走 Container 的增量挪子项，改为按新 pos 重排
		SIZE szOld = GetScrollPos();
		if( m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible() )
			m_pVerticalScrollBar->SetScrollPos(szPos.cy);
		if( m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible() )
			m_pHorizontalScrollBar->SetScrollPos(szPos.cx);
		SIZE szNew = GetScrollPos();
		if( szOld.cx == szNew.cx && szOld.cy == szNew.cy ) return;

		int cyNeeded = 0;
		LayoutScrollContent(m_rcContent, cyNeeded);
		Invalidate();
		if( bMsg && m_pManager != NULL && m_pVerticalScrollBar != NULL ) {
			int nPage = (m_pVerticalScrollBar->GetScrollPos() + m_pVerticalScrollBar->GetLineSize())
				/ m_pVerticalScrollBar->GetLineSize();
			m_pManager->SendNotify(this, DUI_MSGTYPE_SCROLL, (WPARAM)nPage);
		}
	}

	void CAppGridUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		rc = m_rcItem;

		RECT rcPadding = GetPadding();
		rc.left += rcPadding.left;
		rc.top += rcPadding.top;
		rc.right -= rcPadding.right;
		rc.bottom -= rcPadding.bottom;

		m_nCachedItemCount = GetVisibleItemCount();

		if( m_bScrollMode ) {
			if( m_pVerticalScrollBar == NULL )
				EnableScrollBar(true, false);
			// 预留竖条宽度（含尚未 Visible），避免满宽多列算完「不溢出」导致永远不出条
			if( m_pVerticalScrollBar != NULL )
				rc.right -= m_pVerticalScrollBar->GetFixedWidth();
			m_rcContent = rc;
			m_rcDots = rc;
			m_rcDots.top = m_rcDots.bottom;
			RecalcLayoutMetrics(m_rcContent);
			ApplyPageVisibility();
			int cyNeeded = 0;
			LayoutScrollContent(m_rcContent, cyNeeded);
			ProcessScrollBar(rc, 0, cyNeeded);
			// 与预留槽对齐，避免条 GetPos 偏移导致命中/拖拽异常
			if( m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible() ) {
				RECT rcBar = {
					m_rcContent.right,
					m_rcContent.top,
					m_rcContent.right + m_pVerticalScrollBar->GetFixedWidth(),
					m_rcContent.bottom
				};
				m_pVerticalScrollBar->SetPos(rcBar);
			}
			return;
		}

		// 先按「可能有圆点」估一版，再按实际页数决定是否预留底栏
		RECT rcProbe = rc;
		int dotsH = m_bShowPageDots ? (ScaleValue(m_nDotSizeMax) + ScaleValue(kDotBarPadLogic)) : 0;
		if( dotsH > 0 && (rcProbe.bottom - rcProbe.top) > dotsH )
			rcProbe.bottom -= dotsH;
		RecalcLayoutMetrics(rcProbe);

		int nPages = GetPageCount();
		if( m_nPageIndex >= nPages ) m_nPageIndex = nPages - 1;
		if( m_nPageIndex < 0 ) m_nPageIndex = 0;

		dotsH = DotsBarHeight();
		m_rcContent = rc;
		m_rcDots = rc;
		if( dotsH > 0 ) {
			m_rcContent.bottom = rc.bottom - dotsH;
			m_rcDots.top = m_rcContent.bottom;
		}
		else {
			m_rcDots.top = m_rcDots.bottom;
		}

		RecalcLayoutMetrics(m_rcContent);
		nPages = GetPageCount();
		if( m_nPageIndex >= nPages ) m_nPageIndex = nPages - 1;
		if( m_nPageIndex < 0 ) m_nPageIndex = 0;

		ApplyPageVisibility();
		LayoutCurrentPage(m_rcContent);
		ProcessScrollBar(rc, 0, 0);
	}

	int CAppGridUI::HitTestDot(POINT pt) const
	{
		if( !ShouldPaintDots() ) return -1;
		if( !::PtInRect(&m_rcDots, pt) ) return -1;

		int nPages = GetPageCount();
		int diameter = 0;
		int gap = 0;
		ResolveDotLayout(diameter, gap);
		int r = diameter / 2;
		if( r < 1 ) r = 1;
		int hitSlop = ScaleValue(4);
		int totalW = nPages * diameter + (nPages > 1 ? (nPages - 1) * gap : 0);
		int x0 = (m_rcDots.left + m_rcDots.right - totalW) / 2;
		int y0 = (m_rcDots.top + m_rcDots.bottom) / 2;
		for( int i = 0; i < nPages; ++i ) {
			int cx = x0 + i * (diameter + gap) + r;
			int cy = y0;
			int dx = pt.x - cx;
			int dy = pt.y - cy;
			int rr = r + hitSlop;
			if( dx * dx + dy * dy <= rr * rr )
				return i;
		}
		return -1;
	}

	int CAppGridUI::HitTestCellIndex(POINT pt, const RECT& rcContent) const
	{
		if( !::PtInRect(&rcContent, pt) ) return -1;
		SIZE szItem = GetItemSize();
		int iGap = GetGap();
		int cellW = szItem.cx + iGap;
		int cellH = szItem.cy + iGap;
		if( cellW < 1 || cellH < 1 ) return -1;

		int scrollY = 0;
		if( m_bScrollMode && m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() )
			scrollY = m_pVerticalScrollBar->GetScrollPos();

		int col = (pt.x - rcContent.left) / cellW;
		int row = (pt.y - rcContent.top + scrollY) / cellH;
		if( col < 0 || row < 0 || col >= m_nColumns ) return -1;
		if( !m_bScrollMode && row >= m_nRows ) return -1;

		// 点必须落在 item 矩形内（非 gap）
		int xIn = (pt.x - rcContent.left) % cellW;
		int yIn = (pt.y - rcContent.top + scrollY) % cellH;
		if( xIn >= szItem.cx || yIn >= szItem.cy ) return -1;

		return row * m_nColumns + col;
	}

	CControlUI* CAppGridUI::HitTestItem(POINT pt) const
	{
		CControlUI* pDragHide = m_pDragHideItem;

		if( m_bScrollMode ) {
			for( int i = 0; i < m_items.GetSize(); ++i ) {
				CControlUI* p = static_cast<CControlUI*>(m_items[i]);
				if( p == NULL || p->IsAbsolute() || !p->IsVisible() ) continue;
				if( p == pDragHide ) continue;
				RECT rc = p->GetPos();
				if( ::PtInRect(&rc, pt) ) return p;
			}
			return NULL;
		}
		int slot = HitTestCellIndex(pt, m_rcContent);
		if( slot < 0 ) return NULL;
		int vi = m_nPageIndex * GetPerPage() + slot;
		CControlUI* p = GetVisibleItemAt(vi);
		if( p == NULL || p == pDragHide || !p->IsVisible() ) return NULL;
		return p;
	}

	int CAppGridUI::ResolveDragHoverIndex(POINT pt) const
	{
		// 仅命中真实图标才可换位；空白格 / 间隙 / 滚动条 / 界外均为 -1（松手取消）
		CControlUI* pHit = HitTestItem(pt);
		if( pHit == NULL ) return -1;
		int idx = GetGridIndexOf(pHit);
		if( idx == m_nDragSrcIdx ) return -1;
		return idx;
	}

	void CAppGridUI::UpdateDragHoverAfterPage(POINT pt)
	{
		int hover = ResolveDragHoverIndex(pt);
		if( hover == m_nDragSrcIdx ) hover = -1;
		if( hover != m_nDragHoverIdx ) {
			m_nDragHoverIdx = hover;
			Invalidate();
		}
	}

	bool CAppGridUI::HandleDragPageKey(TEventUI& event)
	{
		if( event.Type != UIEVENT_KEYDOWN || !m_bDragging || m_bScrollMode ) return false;
		int nCount = GetPageCount();
		if( nCount <= 1 ) return false;

		int dir = 0;
		switch( event.chKey ) {
		case VK_LEFT:
		case VK_UP:
		case VK_PRIOR:
			dir = -1;
			break;
		case VK_RIGHT:
		case VK_DOWN:
		case VK_NEXT:
			dir = 1;
			break;
		default:
			return false;
		}
		if( dir < 0 ) {
			if( m_nPageIndex <= 0 ) {
				event.StopPropagation();
				return true;
			}
			PrevPage();
		}
		else {
			if( m_nPageIndex + 1 >= nCount ) {
				event.StopPropagation();
				return true;
			}
			NextPage();
		}
		UpdateDragHoverAfterPage(event.ptMouse);
		UpdateDragGhost(m_ptDragMouse);
		Invalidate();
		event.StopPropagation();
		return true;
	}

	bool CAppGridUI::SwapItems(int iFrom, int iTo, bool bNotify)
	{
		if( iFrom == iTo || iFrom < 0 || iTo < 0 ) return false;
		CControlUI* pFrom = GetGridItemAt(iFrom);
		CControlUI* pTo = GetGridItemAt(iTo);
		if( pFrom == NULL || pTo == NULL ) return false;

		int childFrom = GetItemIndex(pFrom);
		int childTo = GetItemIndex(pTo);
		if( childFrom < 0 || childTo < 0 || childFrom == childTo ) return false;

		m_items.SetAt(childFrom, pTo);
		m_items.SetAt(childTo, pFrom);
		NeedUpdate();
		if( bNotify && m_pManager != NULL )
			m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMMOVED, (WPARAM)iFrom, (LPARAM)iTo);
		return true;
	}

	bool CAppGridUI::MoveItem(int iFrom, int iTo)
	{
		if( iFrom == iTo ) return false;
		int nCount = GetGridItemCount();
		if( iFrom < 0 || iTo < 0 || iFrom >= nCount || iTo >= nCount ) return false;

		CStdPtrArray aGrid;
		for( int i = 0; i < m_items.GetSize(); ++i ) {
			CControlUI* p = static_cast<CControlUI*>(m_items[i]);
			if( p == NULL || p->IsAbsolute() ) continue;
			aGrid.Add(p);
		}
		CControlUI* pFrom = static_cast<CControlUI*>(aGrid.GetAt(iFrom));
		if( pFrom == NULL ) return false;
		aGrid.Remove(iFrom);
		if( !aGrid.InsertAt(iTo, pFrom) ) return false;

		int gi = 0;
		for( int i = 0; i < m_items.GetSize(); ++i ) {
			CControlUI* p = static_cast<CControlUI*>(m_items[i]);
			if( p == NULL || p->IsAbsolute() ) continue;
			m_items.SetAt(i, aGrid.GetAt(gi++));
		}
		NeedUpdate();
		if( m_pManager != NULL )
			m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMMOVED, (WPARAM)iFrom, (LPARAM)iTo);
		return true;
	}

	bool CAppGridUI::RemoveGridItemAt(int iGridIndex)
	{
		CControlUI* p = GetGridItemAt(iGridIndex);
		if( p == NULL ) return false;
		return Remove(p);
	}

	int CAppGridUI::HitTestItemIndex(POINT pt) const
	{
		return GetGridIndexOf(HitTestItem(pt));
	}

	DWORD CAppGridUI::ResolvePrimaryColor() const
	{
		CTheme* th = NULL;
		CThemeManager* tm = CThemeManager::GetInstance();
		if( tm != NULL ) {
			CDuiString mode;
			tm->ResolveEffectiveTheme(const_cast<CAppGridUI*>(this), mode, &th);
			if( th == NULL ) th = tm->GetCurrentTheme();
			if( th == NULL ) th = tm->FindTheme(tm->GetDefaultThemeId());
		}
		return th ? th->GetToken(_T("color-primary"), 0x0D6EFDFF) : 0x0D6EFDFF;
	}

	DWORD CAppGridUI::ResolveMutedColor() const
	{
		CTheme* th = NULL;
		CThemeManager* tm = CThemeManager::GetInstance();
		if( tm != NULL ) {
			CDuiString mode;
			tm->ResolveEffectiveTheme(const_cast<CAppGridUI*>(this), mode, &th);
			if( th == NULL ) th = tm->GetCurrentTheme();
			if( th == NULL ) th = tm->FindTheme(tm->GetDefaultThemeId());
		}
		// 未选中：次要文字色更易在深/浅底上辨认；无则退回边框色
		if( th != NULL ) {
			DWORD muted = th->GetToken(_T("color-text-secondary"), 0);
			if( muted != 0 ) return muted;
			return th->GetToken(_T("color-border"), 0xD9D9D9FF);
		}
		return 0xD9D9D9FF;
	}

	void CAppGridUI::PaintPageDots(IRenderContext& ctx)
	{
		if( !ShouldPaintDots() ) return;
		int nPages = GetPageCount();
		int diameter = 0;
		int gap = 0;
		ResolveDotLayout(diameter, gap);
		int r = diameter / 2;
		if( r < 1 ) r = 1;
		int totalW = nPages * diameter + (nPages > 1 ? (nPages - 1) * gap : 0);
		int x0 = (m_rcDots.left + m_rcDots.right - totalW) / 2;
		int y0 = (m_rcDots.top + m_rcDots.bottom) / 2;
		DWORD clrOn = ResolvePrimaryColor();
		DWORD clrOff = ResolveMutedColor();
		for( int i = 0; i < nPages; ++i ) {
			int cx = x0 + i * (diameter + gap) + r;
			int cy = y0;
			RECT rcDot = { cx - r, cy - r, cx + r, cy + r };
			DWORD clr = (i == m_nPageIndex) ? clrOn : clrOff;
			ctx.FillRoundRect(rcDot, r, r, GetAdjustColor(clr));
		}
	}

	void CAppGridUI::PaintDragHint(IRenderContext& ctx)
	{
		if( !m_bDragging || m_nDragHoverIdx < 0 ) return;
		CControlUI* pHover = GetGridItemAt(m_nDragHoverIdx);
		if( pHover == NULL || !pHover->IsVisible() ) return;

		RECT rc;
		if( m_bScrollMode ) {
			rc = pHover->GetPos();
		}
		else {
			int vis = GetVisibleIndexOf(pHover);
			if( vis < 0 ) return;
			int per = GetPerPage();
			int begin = m_nPageIndex * per;
			int slot = vis - begin;
			if( slot < 0 || slot >= per ) return;
			SIZE szItem = GetItemSize();
			int iGap = GetGap();
			int col = slot % m_nColumns;
			int row = slot / m_nColumns;
			rc.left = m_rcContent.left + col * (szItem.cx + iGap);
			rc.top = m_rcContent.top + row * (szItem.cy + iGap);
			rc.right = rc.left + szItem.cx;
			rc.bottom = rc.top + szItem.cy;
		}
		DWORD primary = ResolvePrimaryColor();
		DWORD fill = (primary & 0xFFFFFF00) | 0x28;
		int rad = ScaleValue(12);
		ctx.FillRoundRect(rc, rad, rad, GetAdjustColor(fill));
		ctx.DrawRoundRect(rc, ScaleValue(2), rad, rad, GetAdjustColor(primary), PS_SOLID);
	}

	void CAppGridUI::BeginDragGhost(CControlUI* pItem, POINT ptMouse)
	{
		EndDragGhost();
		if( pItem == NULL ) return;

		RECT rc = pItem->GetPos();
		m_szDragGhost.cx = rc.right - rc.left;
		m_szDragGhost.cy = rc.bottom - rc.top;
		if( m_szDragGhost.cx < 1 || m_szDragGhost.cy < 1 ) {
			SIZE szItem = GetItemSize();
			m_szDragGhost = szItem;
		}
		m_ptDragHotspot.x = ptMouse.x - rc.left;
		m_ptDragHotspot.y = ptMouse.y - rc.top;
		if( m_ptDragHotspot.x < 0 ) m_ptDragHotspot.x = 0;
		if( m_ptDragHotspot.y < 0 ) m_ptDragHotspot.y = 0;
		if( m_ptDragHotspot.x > m_szDragGhost.cx ) m_ptDragHotspot.x = m_szDragGhost.cx / 2;
		if( m_ptDragHotspot.y > m_szDragGhost.cy ) m_ptDragHotspot.y = m_szDragGhost.cy / 2;
		m_ptDragMouse = ptMouse;

		if( m_pManager != NULL )
			m_hDragGhostBmp = CaptureDragGhostBitmap(m_pManager, pItem, rc);

		pItem->SetInternVisible(true);
		if( PassesFilter(pItem) )
			pItem->SetVisible(true);

		m_rcDragSourceSlot = rc;
		m_pDragHideItem = pItem;
		if( m_pManager != NULL ) {
			RECT rcHide = rc;
			::InflateRect(&rcHide, 2, 2);
			m_pManager->Invalidate(rcHide);
		}
		if( m_pManager != NULL && !m_pManager->IsPostPaint(this) )
			m_pManager->AddPostPaint(this);
		InvalidateDragGhost();
	}

	RECT CAppGridUI::GetDragGhostRect() const
	{
		RECT rc = { 0 };
		if( m_szDragGhost.cx < 1 || m_szDragGhost.cy < 1 ) return rc;
		rc.left = m_ptDragMouse.x - m_ptDragHotspot.x;
		rc.top = m_ptDragMouse.y - m_ptDragHotspot.y;
		rc.right = rc.left + m_szDragGhost.cx;
		rc.bottom = rc.top + m_szDragGhost.cy;
		return rc;
	}

	bool CAppGridUI::QueryDragMouseClient(POINT& pt) const
	{
		if( m_pManager == NULL ) return false;
		HWND hWnd = m_pManager->GetPaintWindow();
		if( hWnd == NULL ) return false;
		POINT ptScreen = { 0 };
		if( !::GetCursorPos(&ptScreen) ) return false;
		pt = ptScreen;
		if( !::ScreenToClient(hWnd, &pt) ) return false;
		return true;
	}

	void CAppGridUI::InvalidateDragGhost() const
	{
		if( !m_bDragging || m_pManager == NULL ) return;
		if( m_szDragGhost.cx < 1 || m_szDragGhost.cy < 1 ) return;
		RECT rc = GetDragGhostRect();
		if( rc.right <= rc.left || rc.bottom <= rc.top ) return;
		::InflateRect(&rc, 2, 2);
		m_pManager->Invalidate(rc);
	}

	void CAppGridUI::UpdateDragGhost(POINT ptMouse)
	{
		if( !m_bDragging ) return;
		POINT pt = ptMouse;
		POINT ptCursor = { 0 };
		if( QueryDragMouseClient(ptCursor) )
			pt = ptCursor;
		if( m_ptDragMouse.x == pt.x && m_ptDragMouse.y == pt.y ) return;
		RECT rcOld = GetDragGhostRect();
		m_ptDragMouse = pt;
		if( m_pManager != NULL ) {
			::InflateRect(&rcOld, 2, 2);
			m_pManager->Invalidate(rcOld);
		}
		InvalidateDragGhost();
	}

	void CAppGridUI::EndDragGhost(int iSrcIdx)
	{
		RECT rcGhost = GetDragGhostRect();
		RECT rcSrc = m_rcDragSourceSlot;
		CControlUI* pSrc = m_pDragHideItem;
		if( pSrc == NULL && iSrcIdx >= 0 )
			pSrc = GetGridItemAt(iSrcIdx);
		if( pSrc == NULL && iSrcIdx < 0 )
			iSrcIdx = m_nDragSrcIdx;
		if( pSrc == NULL && iSrcIdx >= 0 )
			pSrc = GetGridItemAt(iSrcIdx);
		if( pSrc != NULL ) {
			RECT rcPos = pSrc->GetPos();
			if( rcPos.right > rcPos.left && rcPos.bottom > rcPos.top )
				rcSrc = rcPos;
			pSrc->SetInternVisible(true);
			if( PassesFilter(pSrc) )
				pSrc->SetVisible(true);
		}

		if( m_pManager != NULL )
			m_pManager->RemovePostPaint(this);

		m_pDragHideItem = NULL;
		if( m_hDragGhostBmp != NULL ) {
			::DeleteObject(m_hDragGhostBmp);
			m_hDragGhostBmp = NULL;
		}
		m_szDragGhost.cx = m_szDragGhost.cy = 0;
		::ZeroMemory(&m_rcDragSourceSlot, sizeof(m_rcDragSourceSlot));

		if( m_pManager != NULL ) {
			if( rcGhost.right > rcGhost.left && rcGhost.bottom > rcGhost.top ) {
				::InflateRect(&rcGhost, 2, 2);
				m_pManager->Invalidate(rcGhost);
			}
			if( rcSrc.right > rcSrc.left && rcSrc.bottom > rcSrc.top ) {
				::InflateRect(&rcSrc, 2, 2);
				m_pManager->Invalidate(rcSrc);
			}
		}
	}

	void CAppGridUI::SyncLayoutAfterDrag(int iRestoredIdx)
	{
		RECT rcSelf = GetPos();
		if( rcSelf.right <= rcSelf.left || rcSelf.bottom <= rcSelf.top ) return;
		SetPos(rcSelf, true);

		CControlUI* pRestored = iRestoredIdx >= 0 ? GetGridItemAt(iRestoredIdx) : NULL;
		if( pRestored == NULL || m_pManager == NULL ) return;

		pRestored->Invalidate();
		RECT rcItem = pRestored->GetPos();
		if( rcItem.right <= rcItem.left || rcItem.bottom <= rcItem.top ) return;
		::InflateRect(&rcItem, 2, 2);
		m_pManager->Invalidate(rcItem);
		if( m_bScrollMode ) {
			RECT rcPaint = rcItem;
			if( ::IntersectRect(&rcPaint, &rcPaint, &m_rcContent) )
				m_pManager->Invalidate(rcPaint);
		}
	}

	bool CAppGridUI::ShouldPaintGridChild(CControlUI* pControl) const
	{
		if( pControl == NULL ) return false;
		if( pControl == m_pDragHideItem ) return false;
		return pControl->IsVisible();
	}

	bool CAppGridUI::DoPaintContent(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		RECT rcTemp = { 0 };
		CControlUI::DoPaint(ctx, rcPaint, pStopControl);

		if( m_items.GetSize() > 0 ) {
			RECT rcPadding = GetPadding();
			RECT rc = m_rcItem;
			rc.left += rcPadding.left;
			rc.top += rcPadding.top;
			rc.right -= rcPadding.right;
			rc.bottom -= rcPadding.bottom;
			if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
			if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

			if( !::IntersectRect(&rcTemp, &rcPaint, &rc) ) {
				for( int it = 0; it < m_items.GetSize(); it++ ) {
					CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
					if( pControl == pStopControl ) return false;
					if( !ShouldPaintGridChild(pControl) ) continue;
					if( !::IntersectRect(&rcTemp, &rcPaint, &pControl->GetPos()) ) continue;
					if( pControl->IsAbsolute() ) {
						if( !::IntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos()) ) continue;
						if( !pControl->Paint(ctx, rcPaint, pStopControl) ) return false;
					}
				}
			}
			else {
				CRenderClipScope childClip(ctx, rcTemp);
				for( int it = 0; it < m_items.GetSize(); it++ ) {
					CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
					if( pControl == pStopControl ) return false;
					if( !ShouldPaintGridChild(pControl) ) continue;
					if( !::IntersectRect(&rcTemp, &rcPaint, &pControl->GetPos()) ) continue;
					if( pControl->IsAbsolute() ) {
						if( !::IntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos()) ) continue;
						ctx.SuspendClip();
						if( !pControl->Paint(ctx, rcPaint, pStopControl) ) return false;
						ctx.ResumeClip();
					}
					else {
						if( !::IntersectRect(&rcTemp, &rc, &pControl->GetPos()) ) continue;
						if( !pControl->Paint(ctx, rcPaint, pStopControl) ) return false;
					}
				}
			}
		}

		if( m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible() ) {
			if( m_pVerticalScrollBar == pStopControl ) return false;
			if( ::IntersectRect(&rcTemp, &rcPaint, &m_pVerticalScrollBar->GetPos()) ) {
				if( !m_pVerticalScrollBar->Paint(ctx, rcPaint, pStopControl) ) return false;
			}
		}

		if( m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible() ) {
			if( m_pHorizontalScrollBar == pStopControl ) return false;
			if( ::IntersectRect(&rcTemp, &rcPaint, &m_pHorizontalScrollBar->GetPos()) ) {
				if( !m_pHorizontalScrollBar->Paint(ctx, rcPaint, pStopControl) ) return false;
			}
		}
		return true;
	}

	void CAppGridUI::ForceRepaintAfterDrag(CControlUI* pItem)
	{
		if( m_pManager == NULL ) return;
		// 兜底：收尾路径偶发漏掉 RemovePostPaint / hide 指针
		if( m_pManager->IsPostPaint(this) )
			m_pManager->RemovePostPaint(this);
		m_pDragHideItem = NULL;

		if( pItem != NULL ) {
			pItem->SetInternVisible(true);
			if( PassesFilter(pItem) )
				pItem->SetVisible(true);
			pItem->Invalidate();
			RECT rcItem = pItem->GetPos();
			if( rcItem.right > rcItem.left && rcItem.bottom > rcItem.top ) {
				::InflateRect(&rcItem, 4, 4);
				m_pManager->Invalidate(rcItem);
			}
		}
		RECT rcGrid = m_rcItem;
		if( rcGrid.right > rcGrid.left && rcGrid.bottom > rcGrid.top ) {
			::InflateRect(&rcGrid, 2, 2);
			m_pManager->Invalidate(rcGrid);
		}
		else if( m_rcContent.right > m_rcContent.left && m_rcContent.bottom > m_rcContent.top ) {
			RECT rc = m_rcContent;
			::InflateRect(&rc, 2, 2);
			m_pManager->Invalidate(rc);
		}
		Invalidate();

		HWND hWnd = m_pManager->GetPaintWindow();
		if( hWnd != NULL && ::IsWindow(hWnd) ) {
			::UpdateWindow(hWnd);
			// Toast/异步 notify 可能紧跟覆盖脏区：再投一次异步 Invalidate，等下一拍绘制
			RECT rcAsync = m_rcItem;
			if( rcAsync.right > rcAsync.left && rcAsync.bottom > rcAsync.top ) {
				::InflateRect(&rcAsync, 2, 2);
				::InvalidateRect(hWnd, &rcAsync, FALSE);
			}
		}
	}

	void CAppGridUI::FinishDrag(POINT pt, bool bAllowSwap)
	{
		if( m_bFinishDragGuard ) return;
		if( !m_bDragging && m_nDragSrcIdx < 0 ) {
			CControlUI* pLeft = m_pDragHideItem;
			if( pLeft != NULL || (m_pManager != NULL && m_pManager->IsPostPaint(this))
				|| m_hDragGhostBmp != NULL )
			{
				EndDragGhost(-1);
				ForceRepaintAfterDrag(pLeft);
			}
			return;
		}

		m_bFinishDragGuard = true;

		int iFrom = m_nDragSrcIdx;
		int iTo = -1;
		bool wasDragging = m_bDragging;
		bool didSwap = false;
		CControlUI* pDragItem = m_pDragHideItem;
		if( pDragItem == NULL && iFrom >= 0 )
			pDragItem = GetGridItemAt(iFrom);
		RECT rcGhostSnap = GetDragGhostRect();
		// 仅落在「其它图标」上才换位；空白 / 滚动条 / 圆点条 / 界外 → 取消
		if( bAllowSwap && wasDragging && iFrom >= 0 ) {
			if( HitTestScrollBar(pt) == NULL && HitTestDot(pt) < 0 )
				iTo = ResolveDragHoverIndex(pt);
			if( iTo == iFrom ) iTo = -1;
		}

		m_bDragging = false;
		m_nDragHoverIdx = -1;
		m_nDragSrcIdx = -1;
		EndDragGhost(iFrom);

		if( bAllowSwap && wasDragging && iTo >= 0 && iTo != iFrom )
			didSwap = SwapItems(iFrom, iTo, false);

		SyncLayoutAfterDrag(iFrom);
		if( didSwap ) {
			SyncLayoutAfterDrag(iTo);
			if( pDragItem == NULL && iFrom >= 0 )
				pDragItem = GetGridItemAt(iFrom);
		}

		if( wasDragging )
			m_bSuppressChildClick = true;

		ForceRepaintAfterDrag(pDragItem);
		if( m_pManager != NULL
			&& rcGhostSnap.right > rcGhostSnap.left && rcGhostSnap.bottom > rcGhostSnap.top )
		{
			::InflateRect(&rcGhostSnap, 4, 4);
			m_pManager->Invalidate(rcGhostSnap);
		}

		if( m_pManager != NULL ) {
			if( didSwap )
				m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMMOVED, (WPARAM)iFrom, (LPARAM)iTo, true);
			if( wasDragging )
				m_pManager->SendNotify(this, DUI_MSGTYPE_DRAGEND, (WPARAM)iFrom, (LPARAM)iTo, true);
		}
		m_bFinishDragGuard = false;
	}

	void CAppGridUI::PaintDragGhost(IRenderContext& ctx)
	{
		if( !m_bDragging || m_nDragSrcIdx < 0 ) return;
		if( m_szDragGhost.cx < 1 || m_szDragGhost.cy < 1 ) return;

		RECT rcGhost = GetDragGhostRect();
		if( rcGhost.right <= rcGhost.left || rcGhost.bottom <= rcGhost.top ) return;

		if( m_hDragGhostBmp != NULL ) {
			RECT rcBmp = { 0, 0, m_szDragGhost.cx, m_szDragGhost.cy };
			RECT rcEmpty = { 0, 0, 0, 0 };
			ctx.DrawImage(m_hDragGhostBmp, rcGhost, rcGhost, rcBmp, rcEmpty, true, 255);
		}
	}

	bool CAppGridUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		RECT rcTemp = { 0 };
		if( !::IntersectRect(&rcTemp, &rcPaint, &m_rcItem) ) return true;

		SIZE cxyRound = GetBorderRadius();
		bool ok = true;
		if( cxyRound.cx > 0 || cxyRound.cy > 0 ) {
			CRenderClipScope roundClip(ctx, rcTemp, m_rcItem, cxyRound.cx, cxyRound.cy);
			ok = DoPaintContent(ctx, rcPaint, pStopControl);
			PaintDragHint(ctx);
			PaintPageDots(ctx);
		}
		else {
			CRenderClipScope clip(ctx, rcTemp);
			ok = DoPaintContent(ctx, rcPaint, pStopControl);
			PaintDragHint(ctx);
			PaintPageDots(ctx);
		}
		// 拖影改 DoPostPaint，可超出 AppGrid 边界（容器外 / 窗口内其它区域）
		return ok;
	}

	void CAppGridUI::DoPostPaint(IRenderContext& ctx, const RECT& rcPaint)
	{
		if( !m_bDragging || m_nDragSrcIdx < 0 ) return;
		RECT rcGhost = GetDragGhostRect();
		RECT rcTemp = { 0 };
		if( !::IntersectRect(&rcTemp, &rcPaint, &rcGhost) ) return;
		PaintDragGhost(ctx);
	}

	void CAppGridUI::DoCaptureEvent(TEventUI& event)
	{
		if( !m_bDraggable ) return;
		if( m_bScrollThumbDragging ) return;

		// 分页拖拽中：方向键 / PageUp·PageDown（capture 兜底，焦点在子项时）
		if( HandleDragPageKey(event) )
			return;

		// 拖拽中松手：必须先收尾（勿因落在滚动条 / 空白提前 return 导致拖影残留）
		if( event.Type == UIEVENT_BUTTONUP ) {
			FinishDrag(event.ptMouse, true);
			return;
		}

		// 拖拽中移动：始终跟手（含移到滚动条 / 网格外）
		if( event.Type == UIEVENT_MOUSEMOVE && (m_bDragging || m_nDragSrcIdx >= 0) ) {
			if( m_nDragSrcIdx < 0 ) return;
			if( m_bDragging && (::GetKeyState(VK_LBUTTON) & 0x8000) == 0 ) {
				FinishDrag(event.ptMouse, true);
				return;
			}
			if( !m_bDragging ) {
				int dx = event.ptMouse.x - m_ptDragDown.x;
				int dy = event.ptMouse.y - m_ptDragDown.y;
				if( dx < 0 ) dx = -dx;
				if( dy < 0 ) dy = -dy;
				if( dx >= kDragThreshold || dy >= kDragThreshold ) {
					SetFocus();
					if( m_pManager != NULL )
						m_pManager->SetCapture();
					m_bDragging = true;
					BeginDragGhost(GetGridItemAt(m_nDragSrcIdx), event.ptMouse);
					if( m_pManager != NULL )
						m_pManager->SendNotify(this, DUI_MSGTYPE_DRAGBEGIN, (WPARAM)m_nDragSrcIdx, (LPARAM)GetGridItemAt(m_nDragSrcIdx));
				}
			}
			if( m_bDragging ) {
				UpdateDragGhost(event.ptMouse);
				if( !m_bScrollMode ) {
					int dot = HitTestDot(event.ptMouse);
					if( dot >= 0 && dot != m_nPageIndex ) {
						SetPage(dot);
						UpdateDragHoverAfterPage(event.ptMouse);
						InvalidateDragGhost();
					}
					else {
						int hover = ResolveDragHoverIndex(event.ptMouse);
						if( hover != m_nDragHoverIdx ) {
							m_nDragHoverIdx = hover;
							Invalidate();
						}
					}
				}
				else {
					int hover = ResolveDragHoverIndex(event.ptMouse);
					if( hover != m_nDragHoverIdx ) {
						m_nDragHoverIdx = hover;
						Invalidate();
					}
				}
			}
			return;
		}

		// 以下仅启动阶段：滚动条 / 条槽上不启拖
		if( m_bScrollMode && m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible()
			&& event.ptMouse.x >= m_rcContent.right )
			return;
		if( event.pSender == (CControlUI*)m_pVerticalScrollBar
			|| event.pSender == (CControlUI*)m_pHorizontalScrollBar )
			return;
		if( HitTestScrollBar(event.ptMouse) != NULL )
			return;

		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK ) {
			// 上次拖拽若因丢 capture 未收尾，先清拖影再开新拖
			if( m_bDragging || m_pDragHideItem != NULL
				|| (m_pManager != NULL && m_pManager->IsPostPaint(this)) )
			{
				POINT pt = event.ptMouse;
				FinishDrag(pt, false);
			}
			if( HitTestDot(event.ptMouse) >= 0 ) {
				m_nDragSrcIdx = -1;
				m_bDragging = false;
				m_nDragHoverIdx = -1;
				return;
			}
			CControlUI* pItem = ResolveGridItem(event.pSender);
			if( pItem == NULL )
				pItem = HitTestItem(event.ptMouse);
			m_bDragging = false;
			m_bSuppressChildClick = false;
			m_nDragHoverIdx = -1;
			m_nDragSrcIdx = -1;
			if( pItem != NULL ) {
				m_nDragSrcIdx = GetGridIndexOf(pItem);
				m_ptDragDown = event.ptMouse;
			}
			return;
		}
	}

	void CAppGridUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			BubbleEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETCURSOR ) {
			// 整条分页条（含圆点间隙）用手型，命中更稳
			if( ShouldPaintDots() && ::PtInRect(&m_rcDots, event.ptMouse) ) {
				::SetCursor(::LoadCursor(NULL, IDC_HAND));
				return;
			}
		}

		if( HandleDragPageKey(event) )
			return;

		// 拖拽中松手：DoEvent 兜底（空白区按下路径可能不进 capture 收尾）
		if( event.Type == UIEVENT_BUTTONUP && (m_bDragging || m_nDragSrcIdx >= 0) ) {
			FinishDrag(event.ptMouse, true);
			return;
		}

		if( event.Type == UIEVENT_SCROLLWHEEL ) {
			if( !m_bScrollMode && GetPageCount() > 1 ) {
				if( LOWORD(event.wParam) == SB_LINEDOWN )
					NextPage();
				else
					PrevPage();
				return;
			}
			// scroll 模式交给 Container 滚内容
		}

		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK ) {
			int dot = HitTestDot(event.ptMouse);
			if( dot >= 0 ) {
				SetPage(dot);
				m_nDragSrcIdx = -1;
				m_bDragging = false;
				m_nDragHoverIdx = -1;
				m_bScrollThumbDragging = false;
				return;
			}
			// 自行处理竖条：点滑块拖动，点轨道翻页
			if( m_bScrollMode && m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible()
				&& event.ptMouse.x >= m_rcContent.right && ::PtInRect(&m_rcItem, event.ptMouse) )
			{
				RECT rcThumb = m_pVerticalScrollBar->GetThumbRect();
				RECT rcBar = m_pVerticalScrollBar->GetPos();
				if( ::PtInRect(&rcThumb, event.ptMouse) ) {
					m_bScrollThumbDragging = true;
					m_ptScrollThumbDown = event.ptMouse;
					m_nScrollThumbPos0 = m_pVerticalScrollBar->GetScrollPos();
					m_nScrollThumbTrack = (rcBar.bottom - rcBar.top) - (rcThumb.bottom - rcThumb.top);
					if( m_nScrollThumbTrack < 1 ) m_nScrollThumbTrack = 1;
				}
				else if( event.ptMouse.y < rcThumb.top ) {
					PageUp();
				}
				else if( event.ptMouse.y > rcThumb.bottom ) {
					PageDown();
				}
				return;
			}
		}
		if( event.Type == UIEVENT_MOUSEMOVE && m_bScrollThumbDragging ) {
			if( (::GetKeyState(VK_LBUTTON) & 0x8000) == 0 ) {
				m_bScrollThumbDragging = false;
			}
			else if( m_pVerticalScrollBar != NULL ) {
				int dy = event.ptMouse.y - m_ptScrollThumbDown.y;
				int range = m_pVerticalScrollBar->GetScrollRange();
				int newPos = m_nScrollThumbPos0
					+ (int)((__int64)dy * (__int64)range / (__int64)m_nScrollThumbTrack);
				SIZE sz = GetScrollPos();
				sz.cy = newPos;
				SetScrollPos(sz);
			}
			return;
		}
		if( event.Type == UIEVENT_BUTTONUP && m_bScrollThumbDragging ) {
			m_bScrollThumbDragging = false;
			return;
		}

		CContainerUI::DoEvent(event);
	}

	void CAppGridUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("item-size")) == 0 ) {
			SIZE szItem = { 0 };
			LPTSTR pstr = NULL;
			szItem.cx = _tcstol(pstrValue, &pstr, 10);
			ASSERT(pstr);
			szItem.cy = _tcstol(pstr + 1, &pstr, 10);
			ASSERT(pstr);
			SetItemSize(szItem);
		}
		else if( _tcsicmp(pstrName, _T("page")) == 0 ) {
			SetPage(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("draggable")) == 0 ) {
			SetDraggable(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("show-page-dots")) == 0 ) {
			SetShowPageDots(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("scroll")) == 0 ) {
			SetScrollMode(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("dot-size-min")) == 0 ) {
			SetDotSizeMin(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("dot-size-max")) == 0 ) {
			SetDotSizeMax(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("filter")) == 0 || _tcsicmp(pstrName, _T("filter-text")) == 0 ) {
			SetFilterText(pstrValue);
		}
		else {
			CContainerUI::SetAttribute(pstrName, pstrValue);
		}
	}
}
