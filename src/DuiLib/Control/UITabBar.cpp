#include "StdAfx.h"
#include "UITabBar.h"
#include "UITabButton.h"
#include "UITabLayout.h"
#include "UIButton.h"
#include "UISvgBox.h"
#include "UIMenu.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CTabBarUI)

	CTabBarUI::CTabBarUI()
		: m_iActive(-1)
		, m_nScrollOffset(0)
		, m_nMaxScrollOffset(0)
		, m_nTabWidth(150)
		, m_nTabMinWidth(100)
		, m_nTabMaxWidth(300)
		, m_bFlexibleWidth(false)
		, m_nScrollBtnWidth(36)
		, m_nAddBtnWidth(28)
		, m_nDragSrcIdx(-1)
		, m_iDragHoverIdx(-1)
		, m_iPendingScrollTab(-1)
		, m_bDragging(false)
		, m_iContextMenuTab(-1)
		, m_bContextMenuPending(false)
		, m_bContextMenuEnabled(true)
		, m_bApplyingScroll(false)
		, m_bOverflow(false)
		, m_bRelayouting(false)
		, m_bNotifyCancel(false)
		, m_bShowAdd(true)
		, m_bAddHover(false)
		, m_bEnsuringPinned(false)
		, m_nScrollHover(0)
		, m_pBtnLeft(NULL)
		, m_pBtnRight(NULL)
		, m_pBtnAdd(NULL)
		, m_pIconLeft(NULL)
		, m_pIconRight(NULL)
		, m_pIconAdd(NULL)
		, m_dwTabBkColor(0)
		, m_dwTabHotBkColor(0xFFD6EBFF)
		, m_dwTabSelectedBkColor(0xFFBAE0FF)
		, m_dwTabTextColor(0xFF8C8C8C)
		, m_dwTabHotTextColor(0xFF1677FF)
		, m_dwTabSelectedTextColor(0xFF1677FF)
		, m_dwTabBorderColor(0)
		, m_dwTabSelectedBorderColor(0xFF1677FF)
		, m_dwTabSeparatorColor(0xFFD9D9D9)
		, m_nTabBorderSize(0)
		, m_nTabSelectedBorderSize(2)
		, m_bShowTabSeparator(true)
		, m_dwCloseTextColor(0xFF8C8C8C)
		, m_dwCloseHotBkColor(0xFFDC3C3C)
		, m_dwCloseHotTextColor(0xFFFFFFFF)
	{
		SetBorderSize(0);
		SetBorderColor(0);
		SetBkColor(0xFFF5F5F5);
		SetMouseChildEnabled(false);
		m_ptDragDown.x = m_ptDragDown.y = 0;
		m_ptDragMouse.x = m_ptDragMouse.y = 0;
		m_ptDragHotspot.x = m_ptDragHotspot.y = 0;
		m_szDragGhost.cx = m_szDragGhost.cy = 0;
	}

	CTabBarUI::~CTabBarUI()
	{
		EndDragGhost();
		if( m_pManager != NULL ) {
			m_pManager->RemoveMessageFilter(this);
			m_pManager->RemovePreMessageFilter(this);
		}
		DestroyChromeIcons();
	}

	void CTabBarUI::DestroyChromeIcons()
	{
		if( m_pIconLeft != NULL ) { delete m_pIconLeft; m_pIconLeft = NULL; }
		if( m_pIconRight != NULL ) { delete m_pIconRight; m_pIconRight = NULL; }
		if( m_pIconAdd != NULL ) { delete m_pIconAdd; m_pIconAdd = NULL; }
	}

	LPCTSTR CTabBarUI::GetClass() const
	{
		return _T("TabBarUI");
	}

	LPVOID CTabBarUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_TABBAR) == 0 ) return static_cast<CTabBarUI*>(this);
		return CHorizontalLayoutUI::GetInterface(pstrName);
	}

	void CTabBarUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
	{
		if( m_pManager != NULL ) {
			m_pManager->RemoveMessageFilter(this);
			m_pManager->RemovePreMessageFilter(this);
		}
		CHorizontalLayoutUI::SetManager(pManager, pParent, bInit);
		if( m_pIconLeft != NULL ) m_pIconLeft->SetManager(pManager, this, false);
		if( m_pIconRight != NULL ) m_pIconRight->SetManager(pManager, this, false);
		if( m_pIconAdd != NULL ) m_pIconAdd->SetManager(pManager, this, false);
		if( m_pManager != NULL ) {
			m_pManager->AddMessageFilter(this);
			m_pManager->AddPreMessageFilter(this);
		}
	}

	static bool IsTextInputFocus(CControlUI* pFocus)
	{
		if( pFocus == NULL ) return false;
		return pFocus->GetInterface(DUI_CTR_EDIT) != NULL
			|| pFocus->GetInterface(DUI_CTR_RICHEDIT) != NULL;
	}

	bool CTabBarUI::IsMouseOverBar() const
	{
		if( m_pManager == NULL ) return false;
		POINT pt = m_pManager->GetMousePos();
		return ::PtInRect(&m_rcItem, pt) != FALSE;
	}

	bool CTabBarUI::IsFocusInside(CControlUI* pFocus) const
	{
		const CControlUI* pSelf = this;
		for( CControlUI* p = pFocus; p != NULL; p = p->GetParent() ) {
			if( p == pSelf ) return true;
		}
		return false;
	}

	LRESULT CTabBarUI::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
	{
		(void)lParam;
		bHandled = false;

		if( uMsg == WM_MENUCLICK ) {
			if( HandleContextMenuClick(wParam) ) {
				bHandled = true;
				return 0;
			}
			return 0;
		}

		if( uMsg != WM_KEYDOWN ) return 0;
		if( !IsVisible() || !IsEnabled() ) return 0;

		CControlUI* pFocus = (m_pManager != NULL) ? m_pManager->GetFocus() : NULL;
		if( IsTextInputFocus(pFocus) ) return 0;

		bool bCtrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
		bool bShift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;

		// Ctrl+W 关闭当前
		if( (wParam == _T('W') || wParam == _T('w')) && bCtrl ) {
			if( m_iActive >= 0 ) {
				RemoveTab(m_iActive);
				bHandled = true;
			}
			return 0;
		}

		// Ctrl+Tab / Ctrl+Shift+Tab：全局切换
		// Tab / Shift+Tab：焦点在栏内，或鼠标正停在标签栏上时切换
		// （仅点选标签时焦点不一定落在 TabBar，故不能只判断 pFocus==this）
		if( wParam == VK_TAB ) {
			bool bCycleTabs = bCtrl || IsFocusInside(pFocus) || IsMouseOverBar();
			if( !bCycleTabs ) return 0;
			if( GetTabCount() <= 0 ) return 0;
			if( bShift )
				ActivatePrevTab(true);
			else
				ActivateNextTab(true);
			bHandled = true;
			return 0;
		}

		return 0;
	}

	void CTabBarUI::BindTabLayoutName(LPCTSTR pstrName)
	{
		m_sBindTabLayoutName = pstrName ? pstrName : _T("");
	}

	CTabLayoutUI* CTabBarUI::GetBoundTabLayout() const
	{
		if( m_sBindTabLayoutName.IsEmpty() || m_pManager == NULL ) return NULL;
		CControlUI* p = m_pManager->FindControl(m_sBindTabLayoutName);
		if( p == NULL ) return NULL;
		return static_cast<CTabLayoutUI*>(p->GetInterface(DUI_CTR_TABLAYOUT));
	}

	void CTabBarUI::SyncBoundTabLayout()
	{
		CTabLayoutUI* pTab = GetBoundTabLayout();
		if( pTab == NULL ) return;
		if( m_iActive >= 0 )
			pTab->SelectItem(m_iActive);
	}

	void CTabBarUI::SyncBoundTabLayoutMove(int iFrom, int iTo)
	{
		CTabLayoutUI* pTab = GetBoundTabLayout();
		if( pTab == NULL ) return;
		pTab->MoveItem(iFrom, iTo);
	}

	void CTabBarUI::SyncBoundTabLayoutRemove(int iIndex)
	{
		CTabLayoutUI* pTab = GetBoundTabLayout();
		if( pTab == NULL || iIndex < 0 ) return;
		CControlUI* pPage = pTab->GetItemAt(iIndex);
		if( pPage != NULL )
			pTab->Remove(pPage);
	}

	UINT CTabBarUI::GetControlFlags() const
	{
		return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
	}

	void CTabBarUI::EnsureScrollButtons()
	{
		if( m_pBtnLeft != NULL && m_pBtnRight != NULL &&
			m_pIconLeft != NULL && m_pIconRight != NULL ) {
			EnsureAddButton();
			return;
		}

		if( m_pBtnLeft == NULL ) {
			m_pBtnLeft = new CButtonUI;
			m_pBtnLeft->SetFloat(true);
			m_pBtnLeft->SetText(_T(""));
			m_pBtnLeft->SetFixedWidth(m_nScrollBtnWidth);
			m_pBtnLeft->SetKind(CONTROLKIND_NONE);
			m_pBtnLeft->SetBkColor(0xFFF5F5F5);
			m_pBtnLeft->SetHotBkColor(0xFFECECEC);
			m_pBtnLeft->SetPushedBkColor(0xFFE0E0E0);
			m_pBtnLeft->SetMouseEnabled(false);
			m_pBtnLeft->SetVisible(false);
			CHorizontalLayoutUI::Add(m_pBtnLeft);
		}
		if( m_pBtnRight == NULL ) {
			m_pBtnRight = new CButtonUI;
			m_pBtnRight->SetFloat(true);
			m_pBtnRight->SetText(_T(""));
			m_pBtnRight->SetFixedWidth(m_nScrollBtnWidth);
			m_pBtnRight->SetKind(CONTROLKIND_NONE);
			m_pBtnRight->SetBkColor(0xFFF5F5F5);
			m_pBtnRight->SetHotBkColor(0xFFECECEC);
			m_pBtnRight->SetPushedBkColor(0xFFE0E0E0);
			m_pBtnRight->SetMouseEnabled(false);
			m_pBtnRight->SetVisible(false);
			CHorizontalLayoutUI::Add(m_pBtnRight);
		}
		if( m_pIconLeft == NULL ) {
			m_pIconLeft = new CSvgBoxUI;
			m_pIconLeft->SetMouseEnabled(false);
			m_pIconLeft->SetAttribute(_T("tabler-outline"), _T("chevron-left"));
			m_pIconLeft->SetTintColor(0xFF333333);
			m_pIconLeft->SetVisible(false);
			// 不加入子树：避免 Container/SetFloatPos 与 backplates 各画一次形成「<<」
			if( m_pManager != NULL ) m_pIconLeft->SetManager(m_pManager, this, false);
		}
		if( m_pIconRight == NULL ) {
			m_pIconRight = new CSvgBoxUI;
			m_pIconRight->SetMouseEnabled(false);
			m_pIconRight->SetAttribute(_T("tabler-outline"), _T("chevron-right"));
			m_pIconRight->SetTintColor(0xFF333333);
			m_pIconRight->SetVisible(false);
			if( m_pManager != NULL ) m_pIconRight->SetManager(m_pManager, this, false);
		}
		EnsureAddButton();
	}

	void CTabBarUI::EnsureAddButton()
	{
		if( !m_bShowAdd ) return;
		if( m_pBtnAdd != NULL && m_pIconAdd != NULL ) return;

		if( m_pBtnAdd == NULL ) {
			m_pBtnAdd = new CButtonUI;
			m_pBtnAdd->SetFloat(true);
			m_pBtnAdd->SetText(_T(""));
			m_pBtnAdd->SetFixedWidth(m_nAddBtnWidth);
			m_pBtnAdd->SetKind(CONTROLKIND_NONE);
			m_pBtnAdd->SetBkColor(GetChromeBkColor());
			m_pBtnAdd->SetHotBkColor(0xFFECECEC);
			m_pBtnAdd->SetPushedBkColor(0xFFE0E0E0);
			m_pBtnAdd->SetMouseEnabled(false);
			m_pBtnAdd->SetVisible(false);
			CHorizontalLayoutUI::Add(m_pBtnAdd);
		}
		if( m_pIconAdd == NULL ) {
			m_pIconAdd = new CSvgBoxUI;
			m_pIconAdd->SetMouseEnabled(false);
			m_pIconAdd->SetAttribute(_T("tabler-outline"), _T("plus"));
			m_pIconAdd->SetTintColor(0xFF595959);
			m_pIconAdd->SetVisible(false);
			if( m_pManager != NULL ) m_pIconAdd->SetManager(m_pManager, this, false);
		}
	}

	int CTabBarUI::GetAddReserveWidth() const
	{
		return (m_bShowAdd ? m_nAddBtnWidth : 0);
	}

	DWORD CTabBarUI::GetChromeBkColor() const
	{
		DWORD dwBk = GetBkColor();
		return (dwBk != 0) ? dwBk : 0xFFF5F5F5;
	}

	void CTabBarUI::SetShowAdd(bool bShow)
	{
		if( m_bShowAdd == bShow ) return;
		m_bShowAdd = bShow;
		if( m_bShowAdd )
			EnsureAddButton();
		else {
			if( m_pBtnAdd != NULL ) m_pBtnAdd->SetVisible(false);
			if( m_pIconAdd != NULL ) m_pIconAdd->SetVisible(false);
			m_bAddHover = false;
		}
		NeedUpdate();
	}

	void CTabBarUI::SetAddBtnWidth(int nWidth)
	{
		if( nWidth < 20 ) nWidth = 20;
		if( nWidth > 48 ) nWidth = 48;
		m_nAddBtnWidth = nWidth;
		if( m_pBtnAdd != NULL ) m_pBtnAdd->SetFixedWidth(m_nAddBtnWidth);
		NeedUpdate();
	}

	void CTabBarUI::NotifyAddTab()
	{
		if( m_pManager != NULL )
			m_pManager->SendNotify(this, DUI_MSGTYPE_TABADD, 0, 0);
	}

	int CTabBarUI::GetTabAppendChildIndex() const
	{
		if( m_pBtnLeft != NULL ) {
			int idx = GetItemIndex(m_pBtnLeft);
			if( idx >= 0 ) return idx;
		}
		return GetCount();
	}

	int CTabBarUI::EstimateTabsWidth() const
	{
		int totalWidth = 0;
		for( size_t i = 0; i < m_tabs.size(); ++i ) {
			if( m_tabs[i] == NULL ) continue;
			int w = m_tabs[i]->GetFixedWidth();
			if( w <= 0 ) {
				RECT rc = m_tabs[i]->GetPos();
				w = rc.right - rc.left;
			}
			if( w <= 0 ) w = m_bFlexibleWidth
				? (m_nTabMinWidth > 0 ? m_nTabMinWidth : 80)
				: (m_nTabWidth > 0 ? m_nTabWidth : 150);
			totalWidth += w;
		}
		return totalWidth;
	}

	int CTabBarUI::CalcFlexibleTabWidth(int fullView) const
	{
		int nCount = GetTabCount();
		if( nCount <= 0 ) return m_nTabMinWidth > 0 ? m_nTabMinWidth : 80;

		int addReserve = GetAddReserveWidth();
		int minW = m_nTabMinWidth; // 0 = 无下限
		int maxW = m_nTabMaxWidth; // 0 = 无上限
		auto clampW = [&](int w) -> int {
			if( minW > 0 && w < minW ) w = minW;
			if( maxW > 0 && w > maxW ) w = maxW;
			if( w < 1 ) w = 1;
			return w;
		};

		int avail = fullView - addReserve;
		if( avail < 0 ) avail = 0;
		int w = clampW(nCount > 0 ? avail / nCount : avail);

		// 触达下限后仍装不下 → 预留滚动钮再算一次
		if( fullView > 0 && (w * nCount + addReserve > fullView) ) {
			int avail2 = fullView - addReserve - m_nScrollBtnWidth * 2;
			if( avail2 < 0 ) avail2 = 0;
			w = clampW(nCount > 0 ? avail2 / nCount : avail2);
			// 按「能完整放下的个数」均分视口，避免右侧空半截
			if( w > 0 && avail2 > 0 ) {
				int nFit = avail2 / w;
				if( nFit > 0 ) {
					int wFill = avail2 / nFit;
					w = clampW(wFill);
				}
			}
		}
		return w;
	}

	void CTabBarUI::ApplyTabWidths(int fullView)
	{
		if( m_tabs.empty() ) return;

		if( fullView < 0 ) {
			RECT rcInset = GetInset();
			fullView = (m_rcItem.right - m_rcItem.left) - rcInset.left - rcInset.right;
		}

		int w = 0;
		if( m_bFlexibleWidth ) {
			if( fullView <= 0 ) {
				w = m_nTabMinWidth > 0 ? m_nTabMinWidth : (m_nTabWidth > 0 ? m_nTabWidth : 80);
				if( m_nTabMaxWidth > 0 && w > m_nTabMaxWidth ) w = m_nTabMaxWidth;
			}
			else {
				w = CalcFlexibleTabWidth(fullView);
			}
		}
		else {
			w = m_nTabWidth > 0 ? m_nTabWidth : 150;
		}

		for( size_t i = 0; i < m_tabs.size(); ++i ) {
			if( m_tabs[i] != NULL )
				m_tabs[i]->SetButtonWidth(w);
		}
	}

	RECT CTabBarUI::GetTabViewportRect() const
	{
		RECT rcInset = GetInset();
		RECT rc = m_rcItem;
		rc.left += rcInset.left;
		rc.top += rcInset.top;
		rc.right -= rcInset.right;
		rc.bottom -= rcInset.bottom;
		if( m_bOverflow )
			rc.left += m_nScrollBtnWidth;
		rc.right -= GetAddReserveWidth();
		if( m_bOverflow )
			rc.right -= m_nScrollBtnWidth;
		if( rc.right < rc.left ) rc.right = rc.left;
		if( rc.bottom < rc.top ) rc.bottom = rc.top;
		return rc;
	}

	int CTabBarUI::GetPinnedTabsWidth() const
	{
		int nPinned = GetPinnedCount();
		int w = 0;
		for( int i = 0; i < nPinned; ++i ) {
			if( m_tabs[i] == NULL ) continue;
			RECT rc = m_tabs[i]->GetPos();
			int tw = rc.right - rc.left;
			if( tw <= 0 ) tw = m_tabs[i]->GetFixedWidth();
			if( tw > 0 ) w += tw;
		}
		return w;
	}

	RECT CTabBarUI::GetScrollViewportRect() const
	{
		RECT rc = GetTabViewportRect();
		rc.left += GetPinnedTabsWidth();
		if( rc.right < rc.left ) rc.right = rc.left;
		return rc;
	}

	bool CTabBarUI::IsTabFullyInViewport(const CTabButtonUI* pTab) const
	{
		if( pTab == NULL ) return false;
		RECT rcTab = pTab->GetPos();
		if( rcTab.right <= rcTab.left ) return false;

		int idx = -1;
		for( size_t i = 0; i < m_tabs.size(); ++i ) {
			if( m_tabs[i] == pTab ) { idx = (int)i; break; }
		}
		RECT rcView = (idx >= 0 && idx < GetPinnedCount())
			? GetTabViewportRect() : GetScrollViewportRect();
		// 钉住标签：完整落在总视口左段（其自身布局区）
		if( idx >= 0 && idx < GetPinnedCount() ) {
			RECT rcPin = GetTabViewportRect();
			rcPin.right = rcPin.left + GetPinnedTabsWidth();
			if( rcPin.right > GetTabViewportRect().right )
				rcPin.right = GetTabViewportRect().right;
			return rcTab.left >= rcPin.left && rcTab.right <= rcPin.right + 1
				&& rcTab.top >= rcPin.top && rcTab.bottom <= rcPin.bottom;
		}
		return rcTab.left >= rcView.left && rcTab.right <= rcView.right
			&& rcTab.top >= rcView.top && rcTab.bottom <= rcView.bottom;
	}

	void CTabBarUI::PlaceChromeFloat(CControlUI* pCtrl, const RECT& rc)
	{
		if( pCtrl == NULL ) return;
		int w = rc.right - rc.left;
		int h = rc.bottom - rc.top;
		if( w < 0 ) w = 0;
		if( h < 0 ) h = 0;
		SIZE xy = { rc.left - m_rcItem.left, rc.top - m_rcItem.top };

		// 同步 FixedXY/宽高，避免父布局 SetFloatPos 把右侧钮叠回 (0,0)
		RECT rcOld = pCtrl->GetPos();
		if( rcOld.left != rc.left || rcOld.top != rc.top
			|| rcOld.right != rc.right || rcOld.bottom != rc.bottom ) {
			pCtrl->SetFixedXY(xy);
			pCtrl->SetFixedWidth(w);
			if( h > 0 ) pCtrl->SetFixedHeight(h);
		}
		pCtrl->SetPos(rc, false);
	}

	int CTabBarUI::GetScrollStep() const
	{
		int nPinned = GetPinnedCount();
		if( nPinned < (int)m_tabs.size() && m_tabs[nPinned] != NULL ) {
			int w = m_tabs[nPinned]->GetFixedWidth();
			if( w > 0 ) return w;
		}
		if( !m_tabs.empty() && m_tabs[0] != NULL ) {
			int w = m_tabs[0]->GetFixedWidth();
			if( w > 0 ) return w;
		}
		if( m_bFlexibleWidth )
			return m_nTabMinWidth > 0 ? m_nTabMinWidth : 80;
		return m_nTabWidth > 0 ? m_nTabWidth : 80;
	}

	bool CTabBarUI::IsTabFullyVisibleAtOffset(int iIndex, int nOffset) const
	{
		if( iIndex < 0 || iIndex >= (int)m_tabs.size() || m_tabs[iIndex] == NULL )
			return false;
		if( iIndex < GetPinnedCount() )
			return IsTabFullyInViewport(m_tabs[iIndex]);

		RECT rcView = GetScrollViewportRect();
		RECT rc = m_tabs[iIndex]->GetPos();
		int contentLeft = rc.left - m_nScrollOffset;
		int contentRight = rc.right - m_nScrollOffset;
		int left = contentLeft + nOffset;
		int right = contentRight + nOffset;
		return left >= rcView.left && right <= rcView.right;
	}

	int CTabBarUI::CalcSnappedScrollOffset(int nTarget, int nDirection) const
	{
		if( !m_bOverflow || m_tabs.empty() ) return nTarget;
		if( nTarget > 0 ) nTarget = 0;
		if( nTarget < m_nMaxScrollOffset ) nTarget = m_nMaxScrollOffset;
		if( m_nMaxScrollOffset >= 0 ) return 0;

		RECT rcView = GetScrollViewportRect();
		int viewLeft = rcView.left;
		int nPinned = GetPinnedCount();

		int best = nTarget;
		int bestDist = 0x7FFFFFFF;
		bool bFound = false;

		auto consider = [&](int candidate) {
			if( candidate > 0 ) candidate = 0;
			if( candidate < m_nMaxScrollOffset ) candidate = m_nMaxScrollOffset;
			int dist = candidate - nTarget;
			if( dist < 0 ) dist = -dist;

			if( nDirection < 0 ) {
				if( candidate < nTarget ) return;
			}
			else if( nDirection > 0 ) {
				if( candidate > nTarget ) return;
			}

			if( !bFound || dist < bestDist || (dist == bestDist &&
				((nDirection < 0 && candidate < best) || (nDirection > 0 && candidate > best))) ) {
				best = candidate;
				bestDist = dist;
				bFound = true;
			}
		};

		for( size_t i = (size_t)nPinned; i < m_tabs.size(); ++i ) {
			if( m_tabs[i] == NULL ) continue;
			RECT rc = m_tabs[i]->GetPos();
			int contentLeft = rc.left - m_nScrollOffset;
			consider(viewLeft - contentLeft);
		}
		consider(0);

		return bFound ? best : nTarget;
	}

	void CTabBarUI::OffsetUnpinnedTabs(int delta)
	{
		if( delta == 0 ) return;
		int nPinned = GetPinnedCount();
		for( size_t i = (size_t)nPinned; i < m_tabs.size(); ++i ) {
			CTabButtonUI* pTab = m_tabs[i];
			if( pTab == NULL ) continue;
			RECT rc = pTab->GetPos();
			::OffsetRect(&rc, delta, 0);
			pTab->SetPos(rc, false);
		}
	}

	void CTabBarUI::FlushScrollToLeadingEdge()
	{
		if( !m_bOverflow || m_tabs.empty() ) return;
		if( m_bApplyingScroll ) return;

		RECT rcView = GetScrollViewportRect();
		int viewW = rcView.right - rcView.left;
		if( viewW <= 0 ) return;

		int nPinned = GetPinnedCount();
		int iLead = -1;
		for( int i = nPinned; i < (int)m_tabs.size(); ++i ) {
			if( m_tabs[i] == NULL ) continue;
			RECT rc = m_tabs[i]->GetPos();
			if( rc.right <= rcView.left ) continue;
			iLead = i;
			if( rc.left < rcView.left && i + 1 < (int)m_tabs.size() && m_tabs[i + 1] != NULL )
				iLead = i + 1;
			break;
		}
		if( iLead < 0 || m_tabs[iLead] == NULL ) return;

		RECT rcLead = m_tabs[iLead]->GetPos();
		int tabW = rcLead.right - rcLead.left;
		if( tabW <= 0 || tabW > viewW ) return;

		if( rcLead.left >= rcView.left - 1 && rcLead.left <= rcView.left + 1
			&& rcLead.right <= rcView.right + 1 )
			return;

		int contentLeft = rcLead.left - m_nScrollOffset;
		int nNew = rcView.left - contentLeft;
		if( nNew > 0 ) nNew = 0;
		if( nNew < m_nMaxScrollOffset ) nNew = m_nMaxScrollOffset;
		if( nNew == m_nScrollOffset ) return;

		int delta = nNew - m_nScrollOffset;
		m_nScrollOffset = nNew;
		m_bApplyingScroll = true;
		OffsetUnpinnedTabs(delta);
		m_bApplyingScroll = false;
		UpdateScrollButtonState();
	}

	void CTabBarUI::StretchLastVisibleTab()
	{
		if( !m_bOverflow || m_tabs.empty() ) return;
		if( m_bApplyingScroll ) return;

		RECT rcView = GetScrollViewportRect();
		if( rcView.right <= rcView.left ) return;

		int nPinned = GetPinnedCount();
		// 先收回未锁定标签上次撑开的宽度
		for( size_t i = (size_t)nPinned; i < m_tabs.size(); ++i ) {
			CTabButtonUI* pTab = m_tabs[i];
			if( pTab == NULL ) continue;
			int baseW = pTab->GetFixedWidth();
			if( baseW <= 0 ) continue;
			RECT rc = pTab->GetPos();
			if( rc.right - rc.left == baseW ) continue;
			rc.right = rc.left + baseW;
			pTab->SetPos(rc, false);
		}

		CTabButtonUI* pLast = NULL;
		for( size_t i = (size_t)nPinned; i < m_tabs.size(); ++i ) {
			CTabButtonUI* pTab = m_tabs[i];
			if( pTab == NULL || !pTab->IsVisible() ) continue;
			if( IsTabFullyInViewport(pTab) )
				pLast = pTab;
		}
		if( pLast == NULL ) return;

		RECT rc = pLast->GetPos();
		if( rc.right >= rcView.right ) return;
		rc.right = rcView.right;
		pLast->SetPos(rc, false);
	}

	void CTabBarUI::ScrollByStep(int nDirection)
	{
		int nStep = GetScrollStep();
		int nTarget = m_nScrollOffset;
		if( nDirection < 0 )
			nTarget = m_nScrollOffset + nStep;
		else if( nDirection > 0 )
			nTarget = m_nScrollOffset - nStep;
		else
			return;
		SetScrollOffset(CalcSnappedScrollOffset(nTarget, nDirection));
	}

	CTabButtonUI* CTabBarUI::AddTab(LPCTSTR pstrTitle)
	{
		return InsertTab(GetTabCount(), pstrTitle);
	}

	CTabButtonUI* CTabBarUI::InsertTab(int iIndex, LPCTSTR pstrTitle)
	{
		if( iIndex < 0 ) iIndex = 0;
		if( iIndex > GetTabCount() ) iIndex = GetTabCount();

		CTabButtonUI* pTab = new CTabButtonUI;
		pTab->SetTabTitle(pstrTitle ? pstrTitle : _T(""));
		if( m_bFlexibleWidth ) {
			int w = m_nTabMinWidth > 0 ? m_nTabMinWidth : (m_nTabWidth > 0 ? m_nTabWidth : 80);
			if( m_nTabMaxWidth > 0 && w > m_nTabMaxWidth ) w = m_nTabMaxWidth;
			pTab->SetButtonWidth(w);
		}
		else {
			pTab->SetButtonWidth(m_nTabWidth > 0 ? m_nTabWidth : 150);
		}
		if( !AddAt(pTab, iIndex) ) {
			delete pTab;
			return NULL;
		}
		SetActiveTab(iIndex);
		// 立即重排并滚到新标签（否则要等下一帧布局，且 ScrollToTab 时偏移仍是旧的）
		if( m_rcItem.right > m_rcItem.left && m_rcItem.bottom > m_rcItem.top )
			SetPos(m_rcItem);
		return pTab;
	}

	void CTabBarUI::RemoveTab(int iIndex)
	{
		CTabButtonUI* pTab = GetTab(iIndex);
		if( pTab != NULL ) RemoveTab(pTab);
	}

	void CTabBarUI::CancelNotify()
	{
		m_bNotifyCancel = true;
	}

	void CTabBarUI::ResetNotifyCancel()
	{
		m_bNotifyCancel = false;
	}

	bool CTabBarUI::QueryAllowNotify(LPCTSTR pstrMsg, WPARAM wParam, LPARAM lParam)
	{
		m_bNotifyCancel = false;
		if( m_pManager != NULL )
			m_pManager->SendNotify(this, pstrMsg, wParam, lParam, false);
		return !m_bNotifyCancel;
	}

	void CTabBarUI::RemoveTab(CTabButtonUI* pTab)
	{
		if( pTab == NULL || pTab->IsLocked() ) return;
		int tabIdx = GetTabIndex(pTab);
		if( tabIdx < 0 ) return;

		if( !QueryAllowNotify(DUI_MSGTYPE_TABCLOSING, (WPARAM)tabIdx, 0) )
			return;

		int iOldActive = m_iActive;
		bool bClosingActive = (tabIdx == m_iActive);

		CHorizontalLayoutUI::Remove(pTab);
		RebuildTabListFromChildren();
		SyncBoundTabLayoutRemove(tabIdx);

		int iNewActive = -1;
		if( !m_tabs.empty() ) {
			if( bClosingActive ) {
				// 优先激活左侧；若无左侧则激活原右侧（删除后仍落在 tabIdx）
				if( tabIdx > 0 )
					iNewActive = tabIdx - 1;
				else
					iNewActive = 0;
			}
			else if( iOldActive > tabIdx ) {
				iNewActive = iOldActive - 1;
			}
			else {
				iNewActive = iOldActive;
			}
			if( iNewActive >= GetTabCount() )
				iNewActive = GetTabCount() - 1;
		}

		if( iNewActive >= 0 ) {
			// 强制走完整激活流程（避免 index 碰巧相同而早退、未刷新样式/绑定页）
			m_iActive = -1;
			SetActiveTab(iNewActive, false);
		}
		else {
			m_iActive = -1;
			NeedUpdate();
		}

		UpdateScrollInfo();
		Invalidate();
		if( m_pManager != NULL )
			m_pManager->SendNotify(this, DUI_MSGTYPE_TABCLOSE, (WPARAM)tabIdx, 0);
	}

	void CTabBarUI::RemoveAllTabs()
	{
		RemoveUnlockedTabs();
	}

	void CTabBarUI::RemoveUnlockedTabs()
	{
		for( int i = GetTabCount() - 1; i >= 0; --i ) {
			if( m_tabs[i] != NULL && !m_tabs[i]->IsLocked() )
				RemoveTab(i);
		}
	}

	void CTabBarUI::RemoveOtherTabs(int iKeepIndex)
	{
		if( iKeepIndex < 0 || iKeepIndex >= GetTabCount() ) return;
		for( int i = GetTabCount() - 1; i >= 0; --i ) {
			if( i == iKeepIndex ) continue;
			if( m_tabs[i] != NULL && !m_tabs[i]->IsLocked() )
				RemoveTab(i);
		}
	}

	void CTabBarUI::RemoveTabsToLeft(int iIndex)
	{
		if( iIndex < 0 || iIndex >= GetTabCount() ) return;
		for( int i = iIndex - 1; i >= 0; --i ) {
			if( m_tabs[i] != NULL && !m_tabs[i]->IsLocked() )
				RemoveTab(i);
		}
	}

	void CTabBarUI::RemoveTabsToRight(int iIndex)
	{
		if( iIndex < 0 || iIndex >= GetTabCount() ) return;
		for( int i = GetTabCount() - 1; i > iIndex; --i ) {
			if( m_tabs[i] != NULL && !m_tabs[i]->IsLocked() )
				RemoveTab(i);
		}
	}

	void CTabBarUI::SetContextMenuXml(LPCTSTR pstrXml)
	{
		if( pstrXml == NULL || *pstrXml == _T('\0') ) {
			m_sContextMenuXml.Empty();
			m_bContextMenuEnabled = true;
			return;
		}
		if( _tcsicmp(pstrXml, _T("false")) == 0 || _tcscmp(pstrXml, _T("0")) == 0 ) {
			m_sContextMenuXml.Empty();
			m_bContextMenuEnabled = false;
			return;
		}
		if( _tcsicmp(pstrXml, _T("true")) == 0 || _tcscmp(pstrXml, _T("1")) == 0
			|| _tcsicmp(pstrXml, _T("builtin")) == 0 ) {
			m_sContextMenuXml.Empty();
			m_bContextMenuEnabled = true;
			return;
		}
		m_sContextMenuXml = pstrXml;
		m_bContextMenuEnabled = true;
	}

	void CTabBarUI::SetContextMenuEnabled(bool bEnable)
	{
		m_bContextMenuEnabled = bEnable;
	}

	CControlUI* CTabBarUI::FindMenuItemByName(CMenuUI* pMenu, LPCTSTR pstrName) const
	{
		if( pMenu == NULL || pstrName == NULL ) return NULL;
		for( int i = 0; i < pMenu->GetCount(); ++i ) {
			CControlUI* pItem = pMenu->GetItemAt(i);
			if( pItem != NULL && pItem->GetName() == pstrName )
				return pItem;
		}
		return NULL;
	}

	void CTabBarUI::BuildBuiltinContextMenu(CMenuUI* pMenu)
	{
		if( pMenu == NULL ) return;
		pMenu->RemoveAll();

		auto addSep = [&]() {
			CMenuElementUI* pLine = new CMenuElementUI;
			pLine->SetLineType();
			pMenu->Add(pLine);
		};

		auto addItem = [&](LPCTSTR pstrName, LPCTSTR pstrText, bool bSepBefore) {
			if( bSepBefore ) addSep();
			CMenuElementUI* pItem = new CMenuElementUI;
			pItem->SetName(pstrName);
			pItem->SetText(pstrText);
			pItem->SetFixedHeight(30);
			pMenu->Add(pItem);
		};

		addItem(_T("tabbar_close"), _T("关闭"), false);
		addItem(_T("tabbar_close_others"), _T("关闭其他"), true);
		addItem(_T("tabbar_close_left"), _T("关闭左侧"), true);
		addItem(_T("tabbar_close_right"), _T("关闭右侧"), true);
		addItem(_T("tabbar_lock"), _T("锁定"), true);
	}

	void CTabBarUI::PrepareContextMenuItems(CMenuUI* pMenu, int iTabIndex)
	{
		if( pMenu == NULL ) return;
		CTabButtonUI* pTab = GetTab(iTabIndex);
		if( pTab == NULL ) return;

		CDuiString sIdx;
		sIdx.Format(_T("%d"), iTabIndex);

		bool bLocked = pTab->IsLocked();
		bool bHasOther = false;
		bool bHasLeft = false;
		bool bHasRight = false;
		for( int i = 0; i < GetTabCount(); ++i ) {
			if( m_tabs[i] == NULL || m_tabs[i]->IsLocked() ) continue;
			if( i != iTabIndex ) bHasOther = true;
			if( i < iTabIndex ) bHasLeft = true;
			if( i > iTabIndex ) bHasRight = true;
		}

		CControlUI* pClose = FindMenuItemByName(pMenu, _T("tabbar_close"));
		if( pClose != NULL ) {
			pClose->SetUserData(sIdx);
			pClose->SetEnabled(!bLocked);
		}
		CControlUI* pOthers = FindMenuItemByName(pMenu, _T("tabbar_close_others"));
		if( pOthers != NULL ) {
			pOthers->SetUserData(sIdx);
			pOthers->SetEnabled(bHasOther);
		}
		CControlUI* pLeft = FindMenuItemByName(pMenu, _T("tabbar_close_left"));
		if( pLeft != NULL ) {
			pLeft->SetUserData(sIdx);
			pLeft->SetEnabled(bHasLeft);
		}
		CControlUI* pRight = FindMenuItemByName(pMenu, _T("tabbar_close_right"));
		if( pRight != NULL ) {
			pRight->SetUserData(sIdx);
			pRight->SetEnabled(bHasRight);
		}
		CControlUI* pLock = FindMenuItemByName(pMenu, _T("tabbar_lock"));
		if( pLock != NULL ) {
			pLock->SetUserData(sIdx);
			pLock->SetText(bLocked ? _T("解锁") : _T("锁定"));
			pLock->SetEnabled(true);
		}
	}

	void CTabBarUI::ShowTabContextMenu(int iTabIndex, POINT ptScreen)
	{
		if( !IsContextMenuEnabled() || m_pManager == NULL ) return;
		CTabButtonUI* pTab = GetTab(iTabIndex);
		if( pTab == NULL ) return;

		SetActiveTab(iTabIndex, true);

		// CMenuWnd 仍需一个最小 Window/Menu 壳；默认用内联串，不读磁盘文件
		static LPCTSTR s_pszBuiltinMenuShell =
			_T("<Window>")
			_T("<Default name=\"Menu\" shared=\"true\" value=\"bordersize=&quot;1&quot; bordercolor=&quot;0xFFD9D9D9&quot; borderround=&quot;4,4&quot; inset=&quot;4,4,4,4&quot; itemtextpadding=&quot;14,0,14,0&quot; bkcolor=&quot;0xFFFFFFFF&quot; itemtextcolor=&quot;0xFF333333&quot; itemhottextcolor=&quot;0xFF1677FF&quot; itemhotbkcolor=&quot;0xFFE6F4FF&quot; itemselectedtextcolor=&quot;0xFF1677FF&quot; itemselectedbkcolor=&quot;0xFFE6F4FF&quot; itemdisabledtextcolor=&quot;0xFFBFBFBF&quot;\" />")
			_T("<Default name=\"MenuElement\" shared=\"true\" value=\"height=&quot;30&quot; linepadding=&quot;12,0,12,0&quot;\" />")
			_T("<Menu/>")
			_T("</Window>");

		bool bCustom = !m_sContextMenuXml.IsEmpty();
		STRINGorID xml = bCustom ? STRINGorID(m_sContextMenuXml.GetData()) : STRINGorID(s_pszBuiltinMenuShell);
		CMenuWnd* pMenuWnd = CMenuWnd::CreateMenu(NULL, xml, ptScreen, m_pManager, NULL,
			eMenuAlignment_Left | eMenuAlignment_Top);
		if( pMenuWnd == NULL ) return;

		CMenuUI* pMenu = pMenuWnd->GetMenuUI();
		if( pMenu == NULL ) return;

		if( !bCustom )
			BuildBuiltinContextMenu(pMenu);
		PrepareContextMenuItems(pMenu, iTabIndex);

		pMenuWnd->ResizeMenu();
		m_iContextMenuTab = iTabIndex;
		m_bContextMenuPending = true;
	}

	bool CTabBarUI::HandleContextMenuClick(WPARAM wParam)
	{
		if( !m_bContextMenuPending ) return false;
		MenuCmd* pMenuCmd = (MenuCmd*)wParam;
		if( pMenuCmd == NULL ) return false;

		CDuiString sName = pMenuCmd->szName;
		bool bOurs = (sName.CompareNoCase(_T("tabbar_close")) == 0
			|| sName.CompareNoCase(_T("tabbar_close_others")) == 0
			|| sName.CompareNoCase(_T("tabbar_close_left")) == 0
			|| sName.CompareNoCase(_T("tabbar_close_right")) == 0
			|| sName.CompareNoCase(_T("tabbar_lock")) == 0);
		if( !bOurs ) return false;

		int iTab = _ttoi(pMenuCmd->szUserData);
		if( m_pManager != NULL )
			m_pManager->DeletePtr(pMenuCmd);
		else
			delete pMenuCmd;

		m_bContextMenuPending = false;
		m_iContextMenuTab = -1;

		if( iTab < 0 || iTab >= GetTabCount() ) return true;

		if( sName.CompareNoCase(_T("tabbar_close")) == 0 ) {
			RemoveTab(iTab);
		}
		else if( sName.CompareNoCase(_T("tabbar_close_others")) == 0 ) {
			RemoveOtherTabs(iTab);
		}
		else if( sName.CompareNoCase(_T("tabbar_close_left")) == 0 ) {
			RemoveTabsToLeft(iTab);
		}
		else if( sName.CompareNoCase(_T("tabbar_close_right")) == 0 ) {
			RemoveTabsToRight(iTab);
		}
		else if( sName.CompareNoCase(_T("tabbar_lock")) == 0 ) {
			CTabButtonUI* pTab = GetTab(iTab);
			if( pTab != NULL )
				pTab->SetLocked(!pTab->IsLocked());
		}
		return true;
	}

	int CTabBarUI::GetTabCount() const
	{
		return (int)m_tabs.size();
	}

	CTabButtonUI* CTabBarUI::GetTab(int iIndex) const
	{
		if( iIndex < 0 || iIndex >= (int)m_tabs.size() ) return NULL;
		return m_tabs[iIndex];
	}

	int CTabBarUI::GetTabIndex(CTabButtonUI* pTab) const
	{
		for( size_t i = 0; i < m_tabs.size(); ++i ) {
			if( m_tabs[i] == pTab ) return (int)i;
		}
		return -1;
	}

	int CTabBarUI::FindTabByTitle(LPCTSTR pstrTitle) const
	{
		if( pstrTitle == NULL ) return -1;
		for( size_t i = 0; i < m_tabs.size(); ++i ) {
			if( m_tabs[i] != NULL && m_tabs[i]->GetTabTitle() == pstrTitle )
				return (int)i;
		}
		return -1;
	}

	bool CTabBarUI::SetActiveTab(int iIndex, bool bCheckAllow)
	{
		if( iIndex < 0 || iIndex >= (int)m_tabs.size() ) return false;
		if( iIndex == m_iActive ) {
			RequestScrollToTab(iIndex);
			return true;
		}

		int iOld = m_iActive;
		if( bCheckAllow && !QueryAllowNotify(DUI_MSGTYPE_TABSELECTING, (WPARAM)iIndex, (LPARAM)iOld) )
			return false;

		for( size_t i = 0; i < m_tabs.size(); ++i )
			m_tabs[i]->SetActive(false);

		m_iActive = iIndex;
		m_tabs[iIndex]->SetActive(true);
		RequestScrollToTab(iIndex);
		SyncBoundTabLayout();

		if( m_pManager != NULL )
			m_pManager->SendNotify(this, DUI_MSGTYPE_TABSELECT, (WPARAM)m_iActive, (LPARAM)iOld);
		Invalidate();
		return true;
	}

	CTabButtonUI* CTabBarUI::GetActiveTabButton() const
	{
		return GetTab(m_iActive);
	}

	bool CTabBarUI::ActivateNextTab(bool bCheckAllow)
	{
		int nCount = GetTabCount();
		if( nCount <= 0 ) return false;
		int iNext = (m_iActive < 0) ? 0 : (m_iActive + 1) % nCount;
		return SetActiveTab(iNext, bCheckAllow);
	}

	bool CTabBarUI::ActivatePrevTab(bool bCheckAllow)
	{
		int nCount = GetTabCount();
		if( nCount <= 0 ) return false;
		int iPrev = (m_iActive < 0) ? 0 : (m_iActive - 1 + nCount) % nCount;
		return SetActiveTab(iPrev, bCheckAllow);
	}

	void CTabBarUI::SetTabWidth(int nWidth)
	{
		if( nWidth < 40 ) nWidth = 40;
		m_nTabWidth = nWidth;
		if( !m_bFlexibleWidth ) {
			for( size_t i = 0; i < m_tabs.size(); ++i ) {
				if( m_tabs[i] != NULL ) m_tabs[i]->SetButtonWidth(m_nTabWidth);
			}
		}
		NeedUpdate();
	}

	void CTabBarUI::SetFlexibleTabWidth(bool bFlexible)
	{
		if( m_bFlexibleWidth == bFlexible ) return;
		m_bFlexibleWidth = bFlexible;
		ApplyTabWidths();
		NeedUpdate();
	}

	void CTabBarUI::SetTabMinWidth(int nWidth)
	{
		if( nWidth < 0 ) nWidth = 0;
		if( m_nTabMinWidth == nWidth ) return;
		m_nTabMinWidth = nWidth;
		if( m_bFlexibleWidth ) {
			ApplyTabWidths();
			NeedUpdate();
		}
	}

	void CTabBarUI::SetTabMaxWidth(int nWidth)
	{
		if( nWidth < 0 ) nWidth = 0;
		if( m_nTabMaxWidth == nWidth ) return;
		m_nTabMaxWidth = nWidth;
		if( m_bFlexibleWidth ) {
			ApplyTabWidths();
			NeedUpdate();
		}
	}

	void CTabBarUI::SetScrollBtnWidth(int nWidth)
	{
		if( nWidth < 20 ) nWidth = 20;
		if( nWidth > 64 ) nWidth = 64;
		if( m_nScrollBtnWidth == nWidth ) return;
		m_nScrollBtnWidth = nWidth;
		if( m_pBtnLeft != NULL ) m_pBtnLeft->SetFixedWidth(m_nScrollBtnWidth);
		if( m_pBtnRight != NULL ) m_pBtnRight->SetFixedWidth(m_nScrollBtnWidth);
		NeedUpdate();
	}

	void CTabBarUI::MoveTab(int iFrom, int iTo)
	{
		if( iFrom == iTo ) return;
		if( iFrom < 0 || iTo < 0 ) return;
		if( iFrom >= (int)m_tabs.size() || iTo >= (int)m_tabs.size() ) return;

		iTo = ClampMoveTarget(iFrom, iTo);
		if( iFrom == iTo ) return;

		CTabButtonUI* pTab = m_tabs[iFrom];
		if( pTab == NULL ) return;
		CTabButtonUI* pActive = GetActiveTabButton();

		SetAutoDestroy(false);
		CHorizontalLayoutUI::Remove(pTab);
		RebuildTabListFromChildren();

		int insertAt = iTo;
		if( insertAt > GetTabCount() ) insertAt = GetTabCount();
		int childIdx = insertAt;
		int btnIdx = GetTabAppendChildIndex();
		if( childIdx > btnIdx ) childIdx = btnIdx;
		CHorizontalLayoutUI::AddAt(pTab, childIdx);
		SetAutoDestroy(true);
		RebuildTabListFromChildren();

		if( pActive != NULL )
			m_iActive = GetTabIndex(pActive);
		else
			m_iActive = -1;

		SyncBoundTabLayoutMove(iFrom, iTo);
		SyncBoundTabLayout();

		if( m_pManager != NULL )
			m_pManager->SendNotify(this, DUI_MSGTYPE_TABMOVE, (WPARAM)iFrom, (LPARAM)iTo);

		RequestScrollToTab(m_iActive);
		NeedUpdate();
		Invalidate();
	}

	int CTabBarUI::GetPinnedCount() const
	{
		int n = 0;
		for( size_t i = 0; i < m_tabs.size(); ++i ) {
			if( m_tabs[i] != NULL && m_tabs[i]->IsLocked() )
				++n;
		}
		return n;
	}

	int CTabBarUI::ClampMoveTarget(int iFrom, int iTo) const
	{
		if( m_bEnsuringPinned ) return iTo;
		CTabButtonUI* pFrom = GetTab(iFrom);
		if( pFrom == NULL ) return iTo;
		int nCount = GetTabCount();
		if( nCount <= 0 ) return iTo;
		if( iTo < 0 ) iTo = 0;
		if( iTo >= nCount ) iTo = nCount - 1;

		int nPinned = GetPinnedCount();
		if( pFrom->IsLocked() ) {
			int maxPin = nPinned - 1;
			if( maxPin < 0 ) maxPin = 0;
			if( iTo > maxPin ) iTo = maxPin;
		}
		else {
			if( iTo < nPinned ) iTo = nPinned;
		}
		return iTo;
	}

	void CTabBarUI::EnsurePinnedTabsLeft()
	{
		if( m_bEnsuringPinned || m_tabs.empty() ) return;
		m_bEnsuringPinned = true;

		std::vector<CTabButtonUI*> order;
		order.reserve(m_tabs.size());
		for( size_t i = 0; i < m_tabs.size(); ++i ) {
			if( m_tabs[i] != NULL && m_tabs[i]->IsLocked() )
				order.push_back(m_tabs[i]);
		}
		for( size_t i = 0; i < m_tabs.size(); ++i ) {
			if( m_tabs[i] != NULL && !m_tabs[i]->IsLocked() )
				order.push_back(m_tabs[i]);
		}

		for( int target = 0; target < (int)order.size(); ++target ) {
			int cur = GetTabIndex(order[target]);
			if( cur >= 0 && cur != target )
				MoveTab(cur, target);
		}

		m_bEnsuringPinned = false;
	}

	void CTabBarUI::OnTabLockChanged(CTabButtonUI* pTab)
	{
		if( pTab == NULL || m_bEnsuringPinned ) return;
		int idx = GetTabIndex(pTab);
		if( idx < 0 ) return;

		if( pTab->IsLocked() ) {
			int pinnedBefore = 0;
			for( int i = 0; i < GetTabCount(); ++i ) {
				if( m_tabs[i] == NULL || m_tabs[i] == pTab ) continue;
				if( m_tabs[i]->IsLocked() ) ++pinnedBefore;
			}
			if( idx != pinnedBefore )
				MoveTab(idx, pinnedBefore);
		}
		else {
			int pinnedCount = GetPinnedCount();
			if( idx != pinnedCount )
				MoveTab(idx, pinnedCount);
		}
		EnsurePinnedTabsLeft();
		NeedUpdate();
		Invalidate();
	}

	void CTabBarUI::BeginDragGhost(CTabButtonUI* pTab, POINT ptClient)
	{
		if( pTab == NULL ) return;
		RECT rc = pTab->GetPos();
		m_szDragGhost.cx = rc.right - rc.left;
		m_szDragGhost.cy = rc.bottom - rc.top;
		if( m_szDragGhost.cx < 40 ) m_szDragGhost.cx = 40;
		if( m_szDragGhost.cy < 20 ) m_szDragGhost.cy = 20;
		m_ptDragHotspot.x = ptClient.x - rc.left;
		m_ptDragHotspot.y = ptClient.y - rc.top;
		if( m_ptDragHotspot.x < 0 ) m_ptDragHotspot.x = 0;
		if( m_ptDragHotspot.y < 0 ) m_ptDragHotspot.y = 0;
		if( m_ptDragHotspot.x > m_szDragGhost.cx ) m_ptDragHotspot.x = m_szDragGhost.cx / 2;
		if( m_ptDragHotspot.y > m_szDragGhost.cy ) m_ptDragHotspot.y = m_szDragGhost.cy / 2;
		m_ptDragMouse = ptClient;
		m_iDragHoverIdx = m_nDragSrcIdx;
		m_bDragging = true;
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		Invalidate();
	}

	void CTabBarUI::EndDragGhost()
	{
		if( !m_bDragging && m_iDragHoverIdx < 0 ) {
			m_nDragSrcIdx = -1;
			return;
		}
		m_bDragging = false;
		m_iDragHoverIdx = -1;
		m_szDragGhost.cx = m_szDragGhost.cy = 0;
		Invalidate();
	}

	void CTabBarUI::UpdateDragGhost(POINT ptClient)
	{
		m_ptDragMouse = ptClient;
		CTabButtonUI* pHot = HitTestTab(ptClient);
		int iHover = (pHot != NULL) ? GetTabIndex(pHot) : -1;
		if( iHover >= 0 && m_nDragSrcIdx >= 0 ) {
			int iClamped = ClampMoveTarget(m_nDragSrcIdx, iHover);
			if( iClamped != iHover )
				iHover = iClamped;
		}
		m_iDragHoverIdx = iHover;
		Invalidate();
	}

	void CTabBarUI::PaintDragDropIndicator(IRenderContext& ctx)
	{
		if( !m_bDragging || m_iDragHoverIdx < 0 || m_nDragSrcIdx < 0 ) return;
		int iHover = ClampMoveTarget(m_nDragSrcIdx, m_iDragHoverIdx);
		if( iHover == m_nDragSrcIdx ) return;
		CTabButtonUI* pHover = GetTab(iHover);
		if( pHover == NULL || !IsTabFullyInViewport(pHover) ) return;

		RECT rc = pHover->GetPos();
		int mid = (rc.left + rc.right) / 2;
		int x = (m_ptDragMouse.x >= mid) ? rc.right - 1 : rc.left;
		RECT rcLine = { x, rc.top + 3, x, rc.bottom - 3 };
		ctx.DrawLine(rcLine, 2, GetAdjustColor(0xFF1677FF));
	}

	void CTabBarUI::PaintDragGhost(IRenderContext& ctx)
	{
		if( !m_bDragging || m_nDragSrcIdx < 0 ) return;
		CTabButtonUI* pTab = GetTab(m_nDragSrcIdx);
		if( pTab == NULL ) return;

		RECT rcGhost = { 0 };
		rcGhost.left = m_ptDragMouse.x - m_ptDragHotspot.x;
		rcGhost.top = m_ptDragMouse.y - m_ptDragHotspot.y;
		if( rcGhost.top < m_rcItem.top ) rcGhost.top = m_rcItem.top;
		if( rcGhost.top + m_szDragGhost.cy > m_rcItem.bottom )
			rcGhost.top = m_rcItem.bottom - m_szDragGhost.cy;
		rcGhost.right = rcGhost.left + m_szDragGhost.cx;
		rcGhost.bottom = rcGhost.top + m_szDragGhost.cy;

		DWORD dwBk = m_dwTabSelectedBkColor != 0 ? m_dwTabSelectedBkColor : 0xFFBAE0FF;
		dwBk = (dwBk & 0x00FFFFFF) | 0xC0000000;
		DWORD dwBorder = m_dwTabSelectedBorderColor != 0 ? m_dwTabSelectedBorderColor : 0xFF1677FF;
		dwBorder = (dwBorder & 0x00FFFFFF) | 0xE0000000;
		DWORD dwText = m_dwTabSelectedTextColor != 0 ? m_dwTabSelectedTextColor : 0xFF1677FF;
		dwText = (dwText & 0x00FFFFFF) | 0xE0000000;

		ctx.FillRoundRect(rcGhost, 8, 8, GetAdjustColor(dwBk));
		ctx.DrawRoundRect(rcGhost, 1, 8, 8, GetAdjustColor(dwBorder));

		RECT rcText = rcGhost;
		rcText.left += 12;
		rcText.right -= 12;
		CDuiString sTitle = pTab->GetTabTitle();
		ctx.DrawText(rcText, sTitle.GetData(), GetAdjustColor(dwText), -1,
			DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
	}

	void CTabBarUI::RebuildTabListFromChildren()
	{
		m_tabs.clear();
		m_tabHover.clear();
		for( int i = 0; i < GetCount(); ++i ) {
			CControlUI* p = GetItemAt(i);
			if( p == NULL ) continue;
			CTabButtonUI* pTab = static_cast<CTabButtonUI*>(p->GetInterface(DUI_CTR_TABBUTTON));
			if( pTab != NULL ) {
				m_tabs.push_back(pTab);
				m_tabHover.push_back(false);
			}
		}
	}

	bool CTabBarUI::Add(CControlUI* pControl)
	{
		if( pControl == NULL ) return false;
		CTabButtonUI* pTab = static_cast<CTabButtonUI*>(pControl->GetInterface(DUI_CTR_TABBUTTON));
		if( pTab != NULL ) {
			EnsureScrollButtons();
			return AddAt(pControl, GetTabCount());
		}
		return CHorizontalLayoutUI::Add(pControl);
	}

	bool CTabBarUI::AddAt(CControlUI* pControl, int iIndex)
	{
		if( pControl == NULL ) return false;

		CTabButtonUI* pTab = static_cast<CTabButtonUI*>(pControl->GetInterface(DUI_CTR_TABBUTTON));
		if( pTab != NULL ) {
			EnsureScrollButtons();
			if( iIndex < 0 ) iIndex = 0;
			if( iIndex > GetTabCount() ) iIndex = GetTabCount();
			// 子控件索引：滚动钮固定在末尾，标签插在按钮前
			int childIdx = iIndex;
			int btnIdx = GetTabAppendChildIndex();
			if( childIdx > btnIdx ) childIdx = btnIdx;

			bool ret = CHorizontalLayoutUI::AddAt(pControl, childIdx);
			if( !ret ) return false;
			RebuildTabListFromChildren();
			if( m_bFlexibleWidth )
				ApplyTabWidths();
			else if( pTab->GetFixedWidth() <= 0 )
				pTab->SetButtonWidth(m_nTabWidth > 0 ? m_nTabWidth : 150);
			pTab->UpdateStyle();
			if( m_iActive < 0 )
				SetActiveTab(0);
			UpdateScrollInfo();
			NeedUpdate();
			return true;
		}
		return CHorizontalLayoutUI::AddAt(pControl, iIndex);
	}

	bool CTabBarUI::Remove(CControlUI* pControl)
	{
		if( pControl == m_pBtnLeft || pControl == m_pBtnRight || pControl == m_pBtnAdd )
			return false;
		if( pControl == m_pIconLeft || pControl == m_pIconRight || pControl == m_pIconAdd )
			return false;

		CTabButtonUI* pTab = pControl
			? static_cast<CTabButtonUI*>(pControl->GetInterface(DUI_CTR_TABBUTTON))
			: NULL;
		if( pTab != NULL && pTab->IsLocked() )
			return false;

		int tabIdx = pTab ? GetTabIndex(pTab) : -1;
		bool ret = CHorizontalLayoutUI::Remove(pControl);
		if( !ret ) return false;

		RebuildTabListFromChildren();
		if( tabIdx >= 0 ) {
			int iOldActive = m_iActive;
			bool bClosingActive = (iOldActive == tabIdx);
			SyncBoundTabLayoutRemove(tabIdx);
			int iNewActive = -1;
			if( !m_tabs.empty() ) {
				if( bClosingActive ) {
					if( tabIdx > 0 )
						iNewActive = tabIdx - 1;
					else
						iNewActive = 0;
				}
				else if( iOldActive > tabIdx ) {
					iNewActive = iOldActive - 1;
				}
				else {
					iNewActive = iOldActive;
				}
				if( iNewActive >= GetTabCount() )
					iNewActive = GetTabCount() - 1;
			}
			if( iNewActive >= 0 ) {
				m_iActive = -1;
				SetActiveTab(iNewActive, false);
			}
			else {
				m_iActive = -1;
			}
			UpdateScrollInfo();
		}
		return true;
	}

	void CTabBarUI::RemoveAll()
	{
		m_tabs.clear();
		m_tabHover.clear();
		m_iActive = -1;
		m_nScrollOffset = 0;
		m_nMaxScrollOffset = 0;
		m_bOverflow = false;
		m_pBtnLeft = NULL;
		m_pBtnRight = NULL;
		m_pBtnAdd = NULL;
		DestroyChromeIcons();
		CHorizontalLayoutUI::RemoveAll();
	}

	void CTabBarUI::UpdateScrollInfo()
	{
		ApplyTabWidths();
		int totalWidth = EstimateTabsWidth();
		RECT rcInset = GetInset();
		int fullView = (m_rcItem.right - m_rcItem.left) - rcInset.left - rcInset.right;
		int addReserve = GetAddReserveWidth();
		m_bOverflow = (totalWidth + addReserve > fullView && fullView > 0);

		int viewWidth = fullView - addReserve;
		if( m_bOverflow )
			viewWidth -= m_nScrollBtnWidth * 2;

		if( m_bOverflow && viewWidth > 0 )
			m_nMaxScrollOffset = viewWidth - totalWidth;
		else
			m_nMaxScrollOffset = 0;

		if( m_nScrollOffset < m_nMaxScrollOffset )
			SetScrollOffset(m_nMaxScrollOffset);
		else if( m_nMaxScrollOffset == 0 && m_nScrollOffset != 0 )
			SetScrollOffset(0);
		else
			UpdateScrollButtonState();
	}

	void CTabBarUI::UpdateScrollButtonsPos()
	{
		if( m_pBtnLeft == NULL || m_pBtnRight == NULL ) return;

		RECT rcInset = GetInset();
		RECT rcClient = m_rcItem;
		rcClient.left += rcInset.left;
		rcClient.top += rcInset.top;
		rcClient.right -= rcInset.right;
		rcClient.bottom -= rcInset.bottom;

		int addReserve = GetAddReserveWidth();
		int rightEdge = rcClient.right - addReserve;

		if( !m_bOverflow ) {
			m_pBtnLeft->SetVisible(false);
			m_pBtnRight->SetVisible(false);
			if( m_pIconLeft != NULL ) m_pIconLeft->SetVisible(false);
			if( m_pIconRight != NULL ) m_pIconRight->SetVisible(false);
			m_nScrollHover = 0;
			return;
		}

		m_pBtnLeft->SetVisible(true);
		m_pBtnRight->SetVisible(true);

		RECT rcLeft = { rcClient.left, rcClient.top, rcClient.left + m_nScrollBtnWidth, rcClient.bottom };
		RECT rcRight = { rightEdge - m_nScrollBtnWidth, rcClient.top, rightEdge, rcClient.bottom };
		// 极端窄宽时避免右钮叠到左钮右侧形成「‹ ›」双控件假象
		if( rcRight.left < rcLeft.right )
			rcRight.left = rcLeft.right;

		PlaceChromeFloat(m_pBtnLeft, rcLeft);
		PlaceChromeFloat(m_pBtnRight, rcRight);

		int iconSize = m_nScrollBtnWidth - 8;
		if( iconSize < 18 ) iconSize = 18;
		if( iconSize > 28 ) iconSize = 28;
		if( m_pIconLeft != NULL ) {
			int ix = (rcLeft.left + rcLeft.right - iconSize) / 2;
			int iy = (rcLeft.top + rcLeft.bottom - iconSize) / 2;
			RECT rcIcon = { ix, iy, ix + iconSize, iy + iconSize };
			m_pIconLeft->SetVisible(true);
			m_pIconLeft->SetPos(rcIcon, false);
		}
		if( m_pIconRight != NULL ) {
			int ix = (rcRight.left + rcRight.right - iconSize) / 2;
			int iy = (rcRight.top + rcRight.bottom - iconSize) / 2;
			RECT rcIcon = { ix, iy, ix + iconSize, iy + iconSize };
			m_pIconRight->SetVisible(true);
			m_pIconRight->SetPos(rcIcon, false);
		}

		UpdateScrollButtonState();
	}

	void CTabBarUI::UpdateAddButtonPos()
	{
		if( !m_bShowAdd || m_pBtnAdd == NULL ) {
			if( m_pBtnAdd != NULL ) m_pBtnAdd->SetVisible(false);
			if( m_pIconAdd != NULL ) m_pIconAdd->SetVisible(false);
			return;
		}

		RECT rcInset = GetInset();
		RECT rcClient = m_rcItem;
		rcClient.left += rcInset.left;
		rcClient.top += rcInset.top;
		rcClient.right -= rcInset.right;
		rcClient.bottom -= rcInset.bottom;

		RECT rcAdd = { 0, rcClient.top, 0, rcClient.bottom };
		if( m_bOverflow ) {
			rcAdd.right = rcClient.right;
			rcAdd.left = rcAdd.right - m_nAddBtnWidth;
		}
		else {
			int x = rcClient.left;
			if( !m_tabs.empty() && m_tabs.back() != NULL ) {
				RECT rcLast = m_tabs.back()->GetPos();
				x = rcLast.right;
			}
			if( x + m_nAddBtnWidth > rcClient.right )
				x = rcClient.right - m_nAddBtnWidth;
			if( x < rcClient.left ) x = rcClient.left;
			rcAdd.left = x;
			rcAdd.right = x + m_nAddBtnWidth;
		}

		m_pBtnAdd->SetVisible(true);
		DWORD dwChromeBk = GetChromeBkColor();
		m_pBtnAdd->SetBkColor(m_bAddHover ? 0xFFECECEC : dwChromeBk);
		PlaceChromeFloat(m_pBtnAdd, rcAdd);

		int iconSize = m_nAddBtnWidth - 10;
		if( iconSize < 14 ) iconSize = 14;
		if( iconSize > 20 ) iconSize = 20;
		if( m_pIconAdd != NULL ) {
			int ix = (rcAdd.left + rcAdd.right - iconSize) / 2;
			int iy = (rcAdd.top + rcAdd.bottom - iconSize) / 2;
			RECT rcIcon = { ix, iy, ix + iconSize, iy + iconSize };
			m_pIconAdd->SetVisible(true);
			m_pIconAdd->SetTintColor(m_bAddHover ? 0xFF1677FF : 0xFF595959);
			m_pIconAdd->SetPos(rcIcon, false);
		}
	}

	void CTabBarUI::UpdateScrollButtonState()
	{
		if( m_pBtnLeft == NULL || m_pBtnRight == NULL ) return;
		if( !m_bOverflow ) return;
		bool bLeft = (m_nScrollOffset < 0);
		bool bRight = (m_nScrollOffset > m_nMaxScrollOffset);
		m_pBtnLeft->SetEnabled(bLeft);
		m_pBtnRight->SetEnabled(bRight);
		if( !bLeft && m_nScrollHover < 0 ) m_nScrollHover = 0;
		if( !bRight && m_nScrollHover > 0 ) m_nScrollHover = 0;
		if( m_pIconLeft != NULL ) {
			DWORD dwTint = 0xFFB0B0B8;
			if( bLeft )
				dwTint = (m_nScrollHover < 0) ? 0xFF1677FF : 0xFF333333;
			m_pIconLeft->SetTintColor(dwTint);
		}
		if( m_pIconRight != NULL ) {
			DWORD dwTint = 0xFFB0B0B8;
			if( bRight )
				dwTint = (m_nScrollHover > 0) ? 0xFF1677FF : 0xFF333333;
			m_pIconRight->SetTintColor(dwTint);
		}
	}

	void CTabBarUI::SetScrollOffset(int nOffset)
	{
		if( nOffset < m_nMaxScrollOffset ) nOffset = m_nMaxScrollOffset;
		if( nOffset > 0 ) nOffset = 0;
		if( nOffset == m_nScrollOffset ) {
			UpdateScrollButtonState();
			return;
		}
		int delta = nOffset - m_nScrollOffset;
		m_nScrollOffset = nOffset;
		if( delta != 0 && !m_bApplyingScroll ) {
			m_bApplyingScroll = true;
			OffsetUnpinnedTabs(delta);
			m_bApplyingScroll = false;
		}
		UpdateScrollButtonState();
		FlushScrollToLeadingEdge();
		StretchLastVisibleTab();
		Invalidate();
	}

	void CTabBarUI::ApplyScrollOffset()
	{
		if( m_bApplyingScroll || m_nScrollOffset == 0 ) return;
		m_bApplyingScroll = true;
		OffsetUnpinnedTabs(m_nScrollOffset);
		m_bApplyingScroll = false;
	}

	void CTabBarUI::RequestScrollToTab(int iIndex)
	{
		m_iPendingScrollTab = iIndex;
		ScrollToTab(iIndex);
	}

	void CTabBarUI::ScrollToTab(int iIndex)
	{
		if( iIndex < 0 || iIndex >= (int)m_tabs.size() ) return;

		CTabButtonUI* pTab = m_tabs[iIndex];
		if( pTab == NULL ) return;
		// 钉住标签不参与滚动，始终留在左侧
		if( iIndex < GetPinnedCount() )
			return;

		RECT r = pTab->GetPos();
		int tabW = r.right - r.left;
		if( tabW <= 0 ) {
			m_iPendingScrollTab = iIndex;
			return;
		}

		int contentLeft = r.left - m_nScrollOffset;
		int contentRight = r.right - m_nScrollOffset;

		RECT rcView = GetScrollViewportRect();
		int viewLeft = rcView.left;
		int viewRight = rcView.right;
		if( viewRight <= viewLeft ) {
			m_iPendingScrollTab = iIndex;
			return;
		}

		if( m_nMaxScrollOffset >= 0 )
			return;

		int nTarget = m_nScrollOffset;
		int nDir = 0;
		if( contentLeft + m_nScrollOffset < viewLeft ) {
			nTarget = viewLeft - contentLeft;
			nDir = -1;
		}
		else if( contentRight + m_nScrollOffset > viewRight ) {
			nTarget = viewRight - contentRight;
			nDir = 1;
		}
		else {
			return;
		}

		int nSnapped = CalcSnappedScrollOffset(nTarget, nDir);
		if( !IsTabFullyVisibleAtOffset(iIndex, nSnapped) )
			nSnapped = nTarget;
		SetScrollOffset(nSnapped);
	}

	void CTabBarUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		if( m_bApplyingScroll ) {
			CControlUI::SetPos(rc, bNeedInvalidate);
			return;
		}

		EnsureScrollButtons();
		EnsurePinnedTabsLeft();

		RECT rcUserInset = m_rcInset;
		int fullView = (rc.right - rc.left) - rcUserInset.left - rcUserInset.right;
		ApplyTabWidths(fullView);
		int totalWidth = EstimateTabsWidth();
		int addReserve = GetAddReserveWidth();
		bool bOverflowLayout = (totalWidth + addReserve > fullView && fullView > 0);

		RECT layoutInset = rcUserInset;
		if( bOverflowLayout ) {
			layoutInset.left += m_nScrollBtnWidth;
			layoutInset.right += m_nScrollBtnWidth + addReserve;
		}
		else if( addReserve > 0 ) {
			// 未溢出时给「+」留出尾部空间，避免与末标签重叠
			layoutInset.right += addReserve;
		}
		// 直接改 inset，避免 SetInset→NeedUpdate 递归
		m_rcInset = layoutInset;

		int savedOffset = m_nScrollOffset;
		m_nScrollOffset = 0;
		CHorizontalLayoutUI::SetPos(rc, bNeedInvalidate);

		m_rcInset = rcUserInset;

		totalWidth = 0;
		int pinnedWidth = 0;
		int unpinnedWidth = 0;
		int nPinned = GetPinnedCount();
		for( size_t i = 0; i < m_tabs.size(); ++i ) {
			if( m_tabs[i] == NULL ) continue;
			RECT tr = m_tabs[i]->GetPos();
			int tw = tr.right - tr.left;
			totalWidth += tw;
			if( (int)i < nPinned ) pinnedWidth += tw;
			else unpinnedWidth += tw;
		}
		// 溢出按「标签总宽 + 加号 vs 满宽可视区」判定
		m_bOverflow = (totalWidth + addReserve > fullView && fullView > 0);
		if( m_bOverflow != bOverflowLayout ) {
			// 布局所用溢出态与实测不一致时重排一次（防递归）
			if( !m_bRelayouting ) {
				m_bRelayouting = true;
				SetPos(rc, bNeedInvalidate);
				m_bRelayouting = false;
				return;
			}
		}
		int viewWidth = fullView - addReserve - (m_bOverflow ? m_nScrollBtnWidth * 2 : 0);
		int scrollViewW = viewWidth - pinnedWidth;
		if( scrollViewW < 0 ) scrollViewW = 0;
		// 仅未锁定标签参与滚动；钉住区宽度固定占左侧
		if( m_bOverflow && scrollViewW > 0 && unpinnedWidth > scrollViewW )
			m_nMaxScrollOffset = scrollViewW - unpinnedWidth;
		else
			m_nMaxScrollOffset = 0;

		int off = savedOffset;
		if( off < m_nMaxScrollOffset ) off = m_nMaxScrollOffset;
		if( off > 0 ) off = 0;
		if( m_bOverflow )
			off = CalcSnappedScrollOffset(off, 0);
		m_nScrollOffset = off;
		ApplyScrollOffset();
		FlushScrollToLeadingEdge();
		StretchLastVisibleTab();
		UpdateScrollButtonsPos();
		UpdateAddButtonPos();

		if( m_iPendingScrollTab >= 0 ) {
			int iScroll = m_iPendingScrollTab;
			m_iPendingScrollTab = -1;
			ScrollToTab(iScroll);
			// ScrollToTab → SetScrollOffset 内已 Stretch；此处再兜一次
			StretchLastVisibleTab();
			UpdateScrollButtonsPos();
			UpdateAddButtonPos();
		}
	}

	CTabButtonUI* CTabBarUI::HitTestTab(POINT pt) const
	{
		if( HitTestScrollButton(pt) != 0 ) return NULL;
		if( HitTestAddButton(pt) ) return NULL;
		for( size_t i = 0; i < m_tabs.size(); ++i ) {
			CTabButtonUI* pTab = m_tabs[i];
			if( pTab == NULL || !pTab->IsVisible() ) continue;
			if( !IsTabFullyInViewport(pTab) ) continue;
			RECT rc = pTab->GetPos();
			if( ::PtInRect(&rc, pt) ) return pTab;
		}
		return NULL;
	}

	int CTabBarUI::HitTestScrollButton(POINT pt) const
	{
		if( !m_bOverflow ) return 0;
		if( m_pBtnLeft != NULL && m_pBtnLeft->IsVisible() ) {
			RECT rc = m_pBtnLeft->GetPos();
			if( ::PtInRect(&rc, pt) ) return -1;
		}
		if( m_pBtnRight != NULL && m_pBtnRight->IsVisible() ) {
			RECT rc = m_pBtnRight->GetPos();
			if( ::PtInRect(&rc, pt) ) return 1;
		}
		return 0;
	}

	bool CTabBarUI::HitTestAddButton(POINT pt) const
	{
		if( !m_bShowAdd || m_pBtnAdd == NULL || !m_pBtnAdd->IsVisible() ) return false;
		RECT rc = m_pBtnAdd->GetPos();
		return ::PtInRect(&rc, pt) != FALSE;
	}

	bool CTabBarUI::IsPointInClose(CTabButtonUI* pTab, POINT pt) const
	{
		if( pTab == NULL || pTab->IsLocked() ) return false;
		RECT rc = pTab->GetCloseRect();
		return ::PtInRect(&rc, pt) != FALSE;
	}

	void CTabBarUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			CHorizontalLayoutUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_BUTTONDOWN ) {
			if( HitTestAddButton(event.ptMouse) ) {
				EndDragGhost();
				m_nDragSrcIdx = -1;
				return;
			}
			int btn = HitTestScrollButton(event.ptMouse);
			if( btn != 0 ) {
				if( (btn < 0 && m_pBtnLeft != NULL && m_pBtnLeft->IsEnabled()) ||
					(btn > 0 && m_pBtnRight != NULL && m_pBtnRight->IsEnabled()) ) {
					ScrollByStep(btn);
				}
				EndDragGhost();
				m_nDragSrcIdx = -1;
				return;
			}
			CTabButtonUI* pTab = HitTestTab(event.ptMouse);
			EndDragGhost();
			m_nDragSrcIdx = -1;
			if( pTab != NULL && !IsPointInClose(pTab, event.ptMouse) ) {
				m_nDragSrcIdx = GetTabIndex(pTab);
				m_ptDragDown = event.ptMouse;
				m_ptDragMouse = event.ptMouse;
			}
			SetFocus();
			return;
		}
		if( event.Type == UIEVENT_BUTTONUP ) {
			if( HitTestAddButton(event.ptMouse) ) {
				EndDragGhost();
				m_nDragSrcIdx = -1;
				NotifyAddTab();
				return;
			}
			if( m_bDragging && m_nDragSrcIdx >= 0 ) {
				int iFrom = m_nDragSrcIdx;
				CTabButtonUI* pTab = HitTestTab(event.ptMouse);
				int iTo = (pTab != NULL) ? GetTabIndex(pTab) : m_iDragHoverIdx;
				EndDragGhost();
				m_nDragSrcIdx = -1;
				if( iTo >= 0 && iTo != iFrom )
					MoveTab(iFrom, iTo);
				return;
			}
			CTabButtonUI* pTab = HitTestTab(event.ptMouse);
			if( pTab != NULL ) {
				int upIdx = GetTabIndex(pTab);
				// 关闭命中不依赖 drag 源（按下在 ✕ 上时不会设置 m_nDragSrcIdx）
				if( IsPointInClose(pTab, event.ptMouse) ) {
					EndDragGhost();
					m_nDragSrcIdx = -1;
					RemoveTab(upIdx);
					return;
				}
				if( m_nDragSrcIdx >= 0 && upIdx == m_nDragSrcIdx )
					SetActiveTab(upIdx);
			}
			EndDragGhost();
			m_nDragSrcIdx = -1;
			return;
		}
		if( event.Type == UIEVENT_MBUTTONDOWN ) {
			EndDragGhost();
			m_nDragSrcIdx = -1;
			return;
		}
		if( event.Type == UIEVENT_MBUTTONUP ) {
			CTabButtonUI* pTab = HitTestTab(event.ptMouse);
			if( pTab != NULL )
				RemoveTab(pTab);
			return;
		}
		if( event.Type == UIEVENT_DBLCLICK ) {
			EndDragGhost();
			m_nDragSrcIdx = -1;
			if( HitTestAddButton(event.ptMouse) || HitTestScrollButton(event.ptMouse) != 0 )
				return;
			CTabButtonUI* pTab = HitTestTab(event.ptMouse);
			if( pTab != NULL && !IsPointInClose(pTab, event.ptMouse) )
				RemoveTab(pTab);
			return;
		}
		if( event.Type == UIEVENT_CONTEXTMENU ) {
			CTabButtonUI* pTab = HitTestTab(event.ptMouse);
			if( pTab != NULL && IsContextMenuEnabled() ) {
				POINT pt = event.ptMouse;
				if( m_pManager != NULL )
					::ClientToScreen(m_pManager->GetPaintWindow(), &pt);
				ShowTabContextMenu(GetTabIndex(pTab), pt);
			}
			return;
		}
		if( event.Type == UIEVENT_KEYDOWN ) {
			if( (event.chKey == _T('W') || event.chKey == _T('w')) &&
				(::GetKeyState(VK_CONTROL) & 0x8000) != 0 ) {
				if( m_iActive >= 0 )
					RemoveTab(m_iActive);
				return;
			}
			if( event.chKey == VK_TAB ) {
				if( (::GetKeyState(VK_SHIFT) & 0x8000) != 0 )
					ActivatePrevTab(true);
				else
					ActivateNextTab(true);
				return;
			}
		}
		if( event.Type == UIEVENT_SCROLLWHEEL ) {
			if( m_nMaxScrollOffset < 0 ) {
				int nStep = GetScrollStep();
				if( LOWORD(event.wParam) == SB_LINEDOWN )
					SetScrollOffset(CalcSnappedScrollOffset(m_nScrollOffset - nStep, 1));
				else
					SetScrollOffset(CalcSnappedScrollOffset(m_nScrollOffset + nStep, -1));
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEMOVE ) {
			if( m_nDragSrcIdx >= 0 && (::GetKeyState(VK_LBUTTON) & 0x8000) != 0 ) {
				if( !m_bDragging ) {
					int dx = event.ptMouse.x - m_ptDragDown.x;
					int dy = event.ptMouse.y - m_ptDragDown.y;
					if( dx < 0 ) dx = -dx;
					if( dy < 0 ) dy = -dy;
					if( dx >= 5 || dy >= 5 ) {
						CTabButtonUI* pSrc = GetTab(m_nDragSrcIdx);
						if( pSrc != NULL )
							BeginDragGhost(pSrc, event.ptMouse);
					}
				}
				if( m_bDragging ) {
					UpdateDragGhost(event.ptMouse);
					return;
				}
			}
			else if( m_bDragging ) {
				EndDragGhost();
				m_nDragSrcIdx = -1;
			}

			bool bAddHot = HitTestAddButton(event.ptMouse);
			if( bAddHot != m_bAddHover ) {
				m_bAddHover = bAddHot;
				UpdateAddButtonPos();
				Invalidate();
			}
			int nScrollHot = HitTestScrollButton(event.ptMouse);
			if( nScrollHot < 0 && (m_pBtnLeft == NULL || !m_pBtnLeft->IsEnabled()) ) nScrollHot = 0;
			if( nScrollHot > 0 && (m_pBtnRight == NULL || !m_pBtnRight->IsEnabled()) ) nScrollHot = 0;
			if( nScrollHot != m_nScrollHover ) {
				m_nScrollHover = nScrollHot;
				UpdateScrollButtonState();
				Invalidate();
			}
			CTabButtonUI* pHot = HitTestTab(event.ptMouse);
			bool bNeed = false;
			for( size_t i = 0; i < m_tabs.size(); ++i ) {
				bool hover = (m_tabs[i] == pHot);
				if( i < m_tabHover.size() && hover != m_tabHover[i] ) {
					m_tabHover[i] = hover;
					m_tabs[i]->ApplyHoverStyle(hover);
					bNeed = true;
				}
				if( m_tabs[i] != NULL )
					m_tabs[i]->SetCloseHover(hover && IsPointInClose(m_tabs[i], event.ptMouse));
			}
			if( bNeed ) Invalidate();
			return;
		}
		if( event.Type == UIEVENT_MOUSELEAVE ) {
			if( m_bAddHover ) {
				m_bAddHover = false;
				UpdateAddButtonPos();
			}
			if( m_nScrollHover != 0 ) {
				m_nScrollHover = 0;
				UpdateScrollButtonState();
			}
			for( size_t i = 0; i < m_tabs.size(); ++i ) {
				if( i < m_tabHover.size() && m_tabHover[i] ) {
					m_tabHover[i] = false;
					m_tabs[i]->SetCloseHover(false);
					m_tabs[i]->ApplyHoverStyle(false);
				}
			}
			Invalidate();
			return;
		}

		CHorizontalLayoutUI::DoEvent(event);
	}

	void CTabBarUI::DrawChromeSeparator(IRenderContext& ctx, int x, int yTop, int yBottom)
	{
		if( !m_bShowTabSeparator || m_dwTabSeparatorColor == 0 ) return;
		if( yBottom <= yTop ) return;
		RECT rcLine = { x, yTop, x, yBottom };
		ctx.DrawLine(rcLine, 1, GetAdjustColor(m_dwTabSeparatorColor));
	}

	void CTabBarUI::PaintChromeSeparators(IRenderContext& ctx)
	{
		if( !m_bShowTabSeparator || m_dwTabSeparatorColor == 0 ) return;

		RECT rcInset = GetInset();
		int yTop = m_rcItem.top + rcInset.top;
		int yBottom = m_rcItem.bottom - rcInset.bottom;

		// ‹ | 标签…
		if( m_bOverflow && m_pBtnLeft != NULL && m_pBtnLeft->IsVisible() ) {
			RECT rc = m_pBtnLeft->GetPos();
			DrawChromeSeparator(ctx, rc.right - 1, yTop, yBottom);
		}
		// …标签 | ›
		if( m_bOverflow && m_pBtnRight != NULL && m_pBtnRight->IsVisible() ) {
			RECT rc = m_pBtnRight->GetPos();
			DrawChromeSeparator(ctx, rc.left, yTop, yBottom);
		}
		// › | +  或  末标签 | +
		if( m_bShowAdd && m_pBtnAdd != NULL && m_pBtnAdd->IsVisible() ) {
			RECT rc = m_pBtnAdd->GetPos();
			DrawChromeSeparator(ctx, rc.left, yTop, yBottom);
		}
	}

	bool CTabBarUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		RECT rcTemp = { 0 };
		if( !::IntersectRect(&rcTemp, &rcPaint, &m_rcItem) ) return true;

		CRenderClipScope clip(ctx, rcTemp);
		CControlUI::DoPaint(ctx, rcPaint, pStopControl);

		RECT rcView = GetTabViewportRect();
		int nPinned = GetPinnedCount();
		int pinnedW = GetPinnedTabsWidth();

		// 钉住区：始终画在左侧，不随滚动裁掉
		if( nPinned > 0 && pinnedW > 0 ) {
			RECT rcPin = rcView;
			rcPin.right = rcPin.left + pinnedW;
			if( rcPin.right > rcView.right ) rcPin.right = rcView.right;
			if( ::IntersectRect(&rcTemp, &rcPaint, &rcPin) ) {
				CRenderClipScope pinClip(ctx, rcTemp);
				for( int i = 0; i < nPinned; ++i ) {
					CTabButtonUI* pTab = m_tabs[i];
					if( pTab == NULL || !pTab->IsVisible() ) continue;
					if( pTab == pStopControl ) return false;
					if( !IsTabFullyInViewport(pTab) ) continue;
					RECT rcTab = pTab->GetPos();
					if( !::IntersectRect(&rcTemp, &rcPaint, &rcTab) ) continue;
					if( !pTab->Paint(ctx, rcPaint, pStopControl) ) return false;
					if( m_bDragging && i == m_nDragSrcIdx )
						ctx.DrawColor(rcTab, GetAdjustColor(0x99F5F5F5));
				}
			}
			// 钉住区与滚动区交界线
			if( rcPin.right < rcView.right )
				DrawChromeSeparator(ctx, rcPin.right - 1, rcView.top, rcView.bottom);
		}

		RECT rcScroll = GetScrollViewportRect();
		if( ::IntersectRect(&rcTemp, &rcPaint, &rcScroll) ) {
			CRenderClipScope tabClip(ctx, rcTemp);
			for( size_t i = (size_t)nPinned; i < m_tabs.size(); ++i ) {
				CTabButtonUI* pTab = m_tabs[i];
				if( pTab == NULL || !pTab->IsVisible() ) continue;
				if( pTab == pStopControl ) return false;
				if( !IsTabFullyInViewport(pTab) ) continue;
				RECT rcTab = pTab->GetPos();
				if( !::IntersectRect(&rcTemp, &rcPaint, &rcTab) ) continue;
				if( !pTab->Paint(ctx, rcPaint, pStopControl) ) return false;
				if( m_bDragging && (int)i == m_nDragSrcIdx )
					ctx.DrawColor(rcTab, GetAdjustColor(0x99F5F5F5));
			}
		}

		PaintChromeBackplates(ctx, rcPaint);
		PaintChromeSeparators(ctx);
		PaintDragDropIndicator(ctx);
		PaintDragGhost(ctx);
		return true;
	}

	void CTabBarUI::PaintChromeBackplates(IRenderContext& ctx, const RECT& rcPaint)
	{
		DWORD dwBk = GetChromeBkColor();
		if( dwBk != 0 ) {
			if( m_bOverflow && m_pBtnLeft != NULL && m_pBtnLeft->IsVisible() ) {
				DWORD dwLeft = (m_nScrollHover < 0 && m_pBtnLeft->IsEnabled()) ? 0xFFECECEC : dwBk;
				ctx.DrawColor(m_pBtnLeft->GetPos(), GetAdjustColor(dwLeft));
			}
			if( m_bOverflow && m_pBtnRight != NULL && m_pBtnRight->IsVisible() ) {
				DWORD dwRight = (m_nScrollHover > 0 && m_pBtnRight->IsEnabled()) ? 0xFFECECEC : dwBk;
				ctx.DrawColor(m_pBtnRight->GetPos(), GetAdjustColor(dwRight));
			}
			if( m_bShowAdd && m_pBtnAdd != NULL && m_pBtnAdd->IsVisible() ) {
				DWORD dwAdd = m_bAddHover ? 0xFFECECEC : dwBk;
				ctx.DrawColor(m_pBtnAdd->GetPos(), GetAdjustColor(dwAdd));
			}
		}

		// 图标不在子树中，只在这里绘制一次（避免与 Container 叠画成「<<」）
		ctx.SuspendClip();
		if( m_pIconLeft != NULL && m_pIconLeft->IsVisible() )
			m_pIconLeft->Paint(ctx, rcPaint, NULL);
		if( m_pIconRight != NULL && m_pIconRight->IsVisible() )
			m_pIconRight->Paint(ctx, rcPaint, NULL);
		if( m_pIconAdd != NULL && m_pIconAdd->IsVisible() )
			m_pIconAdd->Paint(ctx, rcPaint, NULL);
		ctx.ResumeClip();
	}

	void CTabBarUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		auto parseColor = [](LPCTSTR v) -> DWORD {
			if( v == NULL || *v == _T('\0') ) return 0;
			if( *v == _T('#') ) v = ::CharNext(v);
			LPTSTR p = NULL;
			return _tcstoul(v, &p, 16);
		};

		if( _tcsicmp(pstrName, _T("tab-width")) == 0 || _tcsicmp(pstrName, _T("tabwidth")) == 0 ) {
			SetTabWidth(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("flexible")) == 0 || _tcsicmp(pstrName, _T("flexiblewidth")) == 0
			|| _tcsicmp(pstrName, _T("tab-flexible")) == 0 ) {
			SetFlexibleTabWidth(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("tab-min-width")) == 0 || _tcsicmp(pstrName, _T("tabminwidth")) == 0 ) {
			SetTabMinWidth(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("tab-max-width")) == 0 || _tcsicmp(pstrName, _T("tabmaxwidth")) == 0 ) {
			SetTabMaxWidth(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("bindtablayoutname")) == 0 ) {
			BindTabLayoutName(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("scrollbtnwidth")) == 0 || _tcsicmp(pstrName, _T("scroll-btn-width")) == 0 ) {
			SetScrollBtnWidth(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("showadd")) == 0 || _tcsicmp(pstrName, _T("show-add")) == 0 ) {
			SetShowAdd(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("addbtnwidth")) == 0 || _tcsicmp(pstrName, _T("add-btn-width")) == 0 ) {
			SetAddBtnWidth(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("tabbkcolor")) == 0 || _tcsicmp(pstrName, _T("tab-bkcolor")) == 0 ) {
			SetTabBkColor(parseColor(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("tabhotbkcolor")) == 0 || _tcsicmp(pstrName, _T("tab-hotbkcolor")) == 0 ) {
			SetTabHotBkColor(parseColor(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("tabselectedbkcolor")) == 0 || _tcsicmp(pstrName, _T("tab-selectedbkcolor")) == 0
			|| _tcsicmp(pstrName, _T("activetabbkcolor")) == 0 ) {
			SetTabSelectedBkColor(parseColor(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("tabtextcolor")) == 0 || _tcsicmp(pstrName, _T("tab-textcolor")) == 0 ) {
			SetTabTextColor(parseColor(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("tabhottextcolor")) == 0 || _tcsicmp(pstrName, _T("tab-hottextcolor")) == 0 ) {
			SetTabHotTextColor(parseColor(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("tabselectedtextcolor")) == 0 || _tcsicmp(pstrName, _T("tab-selectedtextcolor")) == 0
			|| _tcsicmp(pstrName, _T("activetabtextcolor")) == 0 ) {
			SetTabSelectedTextColor(parseColor(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("tabbordercolor")) == 0 || _tcsicmp(pstrName, _T("tab-bordercolor")) == 0 ) {
			SetTabBorderColor(parseColor(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("tabselectedbordercolor")) == 0 || _tcsicmp(pstrName, _T("tab-selectedbordercolor")) == 0 ) {
			SetTabSelectedBorderColor(parseColor(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("tabbordersize")) == 0 || _tcsicmp(pstrName, _T("tab-bordersize")) == 0 ) {
			SetTabBorderSize(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("tabselectedbordersize")) == 0 || _tcsicmp(pstrName, _T("tab-selectedbordersize")) == 0 ) {
			SetTabSelectedBorderSize(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("showtabseparator")) == 0 || _tcsicmp(pstrName, _T("show-tab-separator")) == 0 ) {
			SetShowTabSeparator(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("tabseparatorcolor")) == 0 || _tcsicmp(pstrName, _T("tab-separatorcolor")) == 0 ) {
			SetTabSeparatorColor(parseColor(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("closetextcolor")) == 0 || _tcsicmp(pstrName, _T("close-textcolor")) == 0 ) {
			SetCloseTextColor(parseColor(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("closehotbkcolor")) == 0 || _tcsicmp(pstrName, _T("close-hotbkcolor")) == 0 ) {
			SetCloseHotBkColor(parseColor(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("closehottextcolor")) == 0 || _tcsicmp(pstrName, _T("close-hottextcolor")) == 0 ) {
			SetCloseHotTextColor(parseColor(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("contextmenu")) == 0 || _tcsicmp(pstrName, _T("context-menu")) == 0
			|| _tcsicmp(pstrName, _T("menuxml")) == 0 ) {
			SetContextMenuXml(pstrValue);
		}
		else {
			CHorizontalLayoutUI::SetAttribute(pstrName, pstrValue);
		}
	}

	void CTabBarUI::RefreshTabStyles()
	{
		for( size_t i = 0; i < m_tabs.size(); ++i ) {
			if( m_tabs[i] != NULL ) m_tabs[i]->UpdateStyle();
		}
		Invalidate();
	}

	void CTabBarUI::SetTabBkColor(DWORD dwColor)
	{
		m_dwTabBkColor = dwColor;
		RefreshTabStyles();
	}

	void CTabBarUI::SetTabHotBkColor(DWORD dwColor)
	{
		m_dwTabHotBkColor = dwColor;
		RefreshTabStyles();
	}

	void CTabBarUI::SetTabSelectedBkColor(DWORD dwColor)
	{
		m_dwTabSelectedBkColor = dwColor;
		RefreshTabStyles();
	}

	void CTabBarUI::SetTabTextColor(DWORD dwColor)
	{
		m_dwTabTextColor = dwColor;
		RefreshTabStyles();
	}

	void CTabBarUI::SetTabHotTextColor(DWORD dwColor)
	{
		m_dwTabHotTextColor = dwColor;
		RefreshTabStyles();
	}

	void CTabBarUI::SetTabSelectedTextColor(DWORD dwColor)
	{
		m_dwTabSelectedTextColor = dwColor;
		RefreshTabStyles();
	}

	void CTabBarUI::SetTabBorderColor(DWORD dwColor)
	{
		m_dwTabBorderColor = dwColor;
		RefreshTabStyles();
	}

	void CTabBarUI::SetTabSelectedBorderColor(DWORD dwColor)
	{
		m_dwTabSelectedBorderColor = dwColor;
		RefreshTabStyles();
	}

	void CTabBarUI::SetTabBorderSize(int nSize)
	{
		m_nTabBorderSize = nSize;
		RefreshTabStyles();
	}

	void CTabBarUI::SetTabSelectedBorderSize(int nSize)
	{
		m_nTabSelectedBorderSize = nSize;
		RefreshTabStyles();
	}

	void CTabBarUI::SetShowTabSeparator(bool bShow)
	{
		m_bShowTabSeparator = bShow;
		Invalidate();
	}

	void CTabBarUI::SetTabSeparatorColor(DWORD dwColor)
	{
		m_dwTabSeparatorColor = dwColor;
		Invalidate();
	}

	void CTabBarUI::SetCloseTextColor(DWORD dwColor)
	{
		m_dwCloseTextColor = dwColor;
		RefreshTabStyles();
	}

	void CTabBarUI::SetCloseHotBkColor(DWORD dwColor)
	{
		m_dwCloseHotBkColor = dwColor;
		RefreshTabStyles();
	}

	void CTabBarUI::SetCloseHotTextColor(DWORD dwColor)
	{
		m_dwCloseHotTextColor = dwColor;
		RefreshTabStyles();
	}
}
