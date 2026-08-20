#include "StdAfx.h"
#include <algorithm>
namespace DuiLib
{
	namespace
	{
		LRESULT HitWindowResizeDeep(CControlUI* p, POINT pt)
		{
			if( p == NULL || !p->IsVisible() ) return HTCLIENT;
			if( !::PtInRect(&p->GetPos(), pt) ) return HTCLIENT;

			CContainerUI* pCont = static_cast<CContainerUI*>(p->GetInterface(DUI_CTR_CONTAINER));
			if( pCont != NULL ) {
				for( int i = pCont->GetCount() - 1; i >= 0; --i ) {
					LRESULT ht = HitWindowResizeDeep(pCont->GetItemAt(i), pt);
					if( ht != HTCLIENT ) return ht;
				}
			}
			return p->HitWindowResize(pt);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	//
	DUI_BEGIN_MESSAGE_MAP(WindowImplBase, CNotifyPump)
		DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK,WindowImplBase::OnClick)
	DUI_END_MESSAGE_MAP()

	WindowImplBase::WindowImplBase()
		: m_pDefaultTrayMenu(NULL)
		, m_bTrayAutoCreated(false)
		, m_bSyncOwnerMove(false)
		, m_bSyncOwnerSize(false)
		, m_bHaveOwnerOffset(false)
		, m_bSyncingOwner(false)
		, m_bForceClose(false)
	{
		m_ptOwnerOffset.x = m_ptOwnerOffset.y = 0;
		m_szOwnerDelta.cx = m_szOwnerDelta.cy = 0;
	}

	void WindowImplBase::SetSyncOwnerMove(bool sync)
	{
		m_bSyncOwnerMove = sync;
		if( !m_bSyncOwnerMove && !m_bSyncOwnerSize ) {
			m_bHaveOwnerOffset = false;
			m_ptOwnerOffset.x = m_ptOwnerOffset.y = 0;
			m_szOwnerDelta.cx = m_szOwnerDelta.cy = 0;
		}
		else if( m_hWnd != NULL && ::IsWindow(m_hWnd) )
			CaptureOwnerSyncOffset();
	}

	void WindowImplBase::SetSyncOwnerSize(bool sync)
	{
		m_bSyncOwnerSize = sync;
		if( !m_bSyncOwnerMove && !m_bSyncOwnerSize ) {
			m_bHaveOwnerOffset = false;
			m_ptOwnerOffset.x = m_ptOwnerOffset.y = 0;
			m_szOwnerDelta.cx = m_szOwnerDelta.cy = 0;
		}
		else if( m_hWnd != NULL && ::IsWindow(m_hWnd) )
			CaptureOwnerSyncOffset();
	}

	bool WindowImplBase::FitToShapeImage(bool clampWorkArea, bool bCenter, int workAreaPercent)
	{
		SIZE sz = { 0, 0 };
		if( !m_pm.CalcShapeWindowClientSize(sz, clampWorkArea, workAreaPercent) )
			return false;
		ResizeClient(sz.cx, sz.cy);
		if( bCenter ) CenterWindow();
		m_pm.Invalidate();
		return true;
	}

	HWND WindowImplBase::ResolveSyncOwner() const
	{
		if( m_hWnd == NULL || !::IsWindow(m_hWnd) ) return NULL;
		HWND hOwner = ::GetWindow(m_hWnd, GW_OWNER);
		if( hOwner == NULL || !::IsWindow(hOwner) ) return NULL;
		return hOwner;
	}

	void WindowImplBase::CaptureOwnerSyncOffset()
	{
		m_bHaveOwnerOffset = false;
		m_ptOwnerOffset.x = m_ptOwnerOffset.y = 0;
		m_szOwnerDelta.cx = m_szOwnerDelta.cy = 0;
		if( !m_bSyncOwnerMove && !m_bSyncOwnerSize ) return;
		HWND hOwner = ResolveSyncOwner();
		if( hOwner == NULL ) return;
		// GetWindowRect = 屏幕物理像素（副屏可为负坐标）；跨 DPI 屏用打开时像素差，随窗一起平移/缩放
		RECT rcSelf = { 0 }, rcOwner = { 0 };
		if( !::GetWindowRect(m_hWnd, &rcSelf) ) return;
		if( !::GetWindowRect(hOwner, &rcOwner) ) return;
		m_ptOwnerOffset.x = rcOwner.left - rcSelf.left;
		m_ptOwnerOffset.y = rcOwner.top - rcSelf.top;
		m_szOwnerDelta.cx = (rcOwner.right - rcOwner.left) - (rcSelf.right - rcSelf.left);
		m_szOwnerDelta.cy = (rcOwner.bottom - rcOwner.top) - (rcSelf.bottom - rcSelf.top);
		m_bHaveOwnerOffset = true;
	}

	static void QueryHwndTrackSize(HWND hWnd, SIZE& szMin, SIZE& szMax)
	{
		szMin.cx = ::GetSystemMetrics(SM_CXMINTRACK);
		szMin.cy = ::GetSystemMetrics(SM_CYMINTRACK);
		szMax.cx = ::GetSystemMetrics(SM_CXMAXTRACK);
		szMax.cy = ::GetSystemMetrics(SM_CYMAXTRACK);
		if( hWnd == NULL || !::IsWindow(hWnd) ) return;
		MINMAXINFO mmi = {};
		mmi.ptMinTrackSize.x = szMin.cx;
		mmi.ptMinTrackSize.y = szMin.cy;
		mmi.ptMaxTrackSize.x = szMax.cx;
		mmi.ptMaxTrackSize.y = szMax.cy;
		::SendMessage(hWnd, WM_GETMINMAXINFO, 0, (LPARAM)&mmi);
		szMin.cx = mmi.ptMinTrackSize.x;
		szMin.cy = mmi.ptMinTrackSize.y;
		szMax.cx = mmi.ptMaxTrackSize.x;
		szMax.cy = mmi.ptMaxTrackSize.y;
	}

	void WindowImplBase::SyncOwnerGeometry(bool bPos, bool bSize)
	{
		if( !m_bHaveOwnerOffset || m_bSyncingOwner ) return;
		if( !bPos && !bSize ) return;
		HWND hOwner = ResolveSyncOwner();
		if( hOwner == NULL ) return;
		if( ::IsZoomed(hOwner) || ::IsIconic(hOwner) ) return;
		if( ::IsZoomed(m_hWnd) || ::IsIconic(m_hWnd) ) return;

		RECT rcSelf = { 0 };
		if( !::GetWindowRect(m_hWnd, &rcSelf) ) return;
		const int selfW = rcSelf.right - rcSelf.left;
		const int selfH = rcSelf.bottom - rcSelf.top;
		// 屏幕坐标：主屏/副屏（含负坐标）统一用物理像素偏移
		const int x = rcSelf.left + m_ptOwnerOffset.x;
		const int y = rcSelf.top + m_ptOwnerOffset.y;
		int w = selfW + m_szOwnerDelta.cx;
		int h = selfH + m_szOwnerDelta.cy;
		if( w < 1 ) w = 1;
		if( h < 1 ) h = 1;

		// SetWindowPos 不走跟踪缩放钳制，必须主动遵守 Owner 的 min/max-size
		if( bSize ) {
			SIZE szMin = { 0 }, szMax = { 0 };
			QueryHwndTrackSize(hOwner, szMin, szMax);
			if( szMin.cx > 0 && w < szMin.cx ) w = szMin.cx;
			if( szMin.cy > 0 && h < szMin.cy ) h = szMin.cy;
			if( szMax.cx > 0 && w > szMax.cx ) w = szMax.cx;
			if( szMax.cy > 0 && h > szMax.cy ) h = szMax.cy;
		}

		RECT rcOwner = { 0 };
		if( !::GetWindowRect(hOwner, &rcOwner) ) return;
		const int ownerW = rcOwner.right - rcOwner.left;
		const int ownerH = rcOwner.bottom - rcOwner.top;
		const bool bNeedPos = bPos && (rcOwner.left != x || rcOwner.top != y);
		const bool bNeedSize = bSize && (ownerW != w || ownerH != h);
		if( !bNeedPos && !bNeedSize ) return;

		UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;
		if( !bNeedPos ) flags |= SWP_NOMOVE;
		if( !bNeedSize ) flags |= SWP_NOSIZE;

		m_bSyncingOwner = true;
		::SetWindowPos(hOwner, NULL, x, y, w, h, flags);
		m_bSyncingOwner = false;
	}

	void WindowImplBase::SyncOwnerPosition()
	{
		if( !m_bSyncOwnerMove ) return;
		SyncOwnerGeometry(true, false);
	}

	void WindowImplBase::SyncOwnerSize()
	{
		if( !m_bSyncOwnerSize ) return;
		SyncOwnerGeometry(false, true);
	}

	void WindowImplBase::SyncOwnerShowState()
	{
		if( (!m_bSyncOwnerMove && !m_bSyncOwnerSize) || m_bSyncingOwner ) return;
		HWND hOwner = ResolveSyncOwner();
		if( hOwner == NULL ) return;

		const bool selfIconic = ::IsIconic(m_hWnd) != FALSE;
		const bool ownerIconic = ::IsIconic(hOwner) != FALSE;

		if( selfIconic ) {
			// 本窗最小化 → Owner 一起收起，否则禁用的主窗仍露在桌面上
			if( !ownerIconic ) {
				m_bSyncingOwner = true;
				::ShowWindow(hOwner, SW_MINIMIZE);
				m_bSyncingOwner = false;
			}
			return;
		}

		if( ownerIconic ) {
			m_bSyncingOwner = true;
			::ShowWindow(hOwner, SW_RESTORE);
			m_bSyncingOwner = false;
		}

		// 从最小化还原后重新对齐；最大化几何仍由 SyncOwnerGeometry 内 IsZoomed 跳过
		if( !::IsZoomed(m_hWnd) && !::IsIconic(m_hWnd) )
			SyncOwnerGeometry(m_bSyncOwnerMove, m_bSyncOwnerSize);
	}

	UINT WindowImplBase::ShowModal()
	{
		CaptureOwnerSyncOffset();
		UINT nRet = CWindowWnd::ShowModal();
		m_bHaveOwnerOffset = false;
		return nRet;
	}

	void WindowImplBase::ShowModalFake()
	{
		CaptureOwnerSyncOffset();
		CWindowWnd::ShowModalFake();
	}

	// maxbtn/restorebtn 由 WinImplBase 按最大化态互斥显隐；若挂在 TitleBar 且 show-max=false，不得强制显示。
	static CTitleBarUI* FindOwnerTitleBar(CControlUI* pControl)
	{
		for( CControlUI* p = pControl; p != NULL; p = p->GetParent() ) {
			CTitleBarUI* pBar = static_cast<CTitleBarUI*>(p->GetInterface(DUI_CTR_TITLEBAR));
			if( pBar != NULL ) return pBar;
		}
		return NULL;
	}

	static CTitleBarUI* FindMinimizeToTrayTitleBar(CPaintManagerUI& pm)
	{
		CControlUI* pNamed = pm.FindControl(_T("titlebar"));
		if( pNamed != NULL ) {
			CTitleBarUI* pBar = static_cast<CTitleBarUI*>(pNamed->GetInterface(DUI_CTR_TITLEBAR));
			if( pBar != NULL && pBar->IsMinimizeToTray() ) return pBar;
		}
		CStdPtrArray* pList = pm.FindSubControlsByClass(pm.GetRoot(), _T("TitleBarUI"));
		if( pList == NULL ) return NULL;
		for( int i = 0; i < pList->GetSize(); ++i ) {
			CControlUI* p = static_cast<CControlUI*>(pList->GetAt(i));
			CTitleBarUI* pBar = p ? static_cast<CTitleBarUI*>(p->GetInterface(DUI_CTR_TITLEBAR)) : NULL;
			if( pBar != NULL && pBar->IsMinimizeToTray() ) return pBar;
		}
		return NULL;
	}

	static CTitleBarUI* FindCloseToTrayTitleBar(CPaintManagerUI& pm)
	{
		CControlUI* pNamed = pm.FindControl(_T("titlebar"));
		if( pNamed != NULL ) {
			CTitleBarUI* pBar = static_cast<CTitleBarUI*>(pNamed->GetInterface(DUI_CTR_TITLEBAR));
			if( pBar != NULL && pBar->IsCloseToTray() ) return pBar;
		}
		CStdPtrArray* pList = pm.FindSubControlsByClass(pm.GetRoot(), _T("TitleBarUI"));
		if( pList == NULL ) return NULL;
		for( int i = 0; i < pList->GetSize(); ++i ) {
			CControlUI* p = static_cast<CControlUI*>(pList->GetAt(i));
			CTitleBarUI* pBar = p ? static_cast<CTitleBarUI*>(p->GetInterface(DUI_CTR_TITLEBAR)) : NULL;
			if( pBar != NULL && pBar->IsCloseToTray() ) return pBar;
		}
		return NULL;
	}

	static void SyncMaxRestoreButtons(CPaintManagerUI& pm, bool bMaximized)
	{
		CControlUI* pMax = static_cast<CControlUI*>(pm.FindControl(_T("maxbtn")));
		CControlUI* pRestore = static_cast<CControlUI*>(pm.FindControl(_T("restorebtn")));
		if( pMax == NULL && pRestore == NULL ) return;

		CTitleBarUI* pBar = FindOwnerTitleBar(pMax != NULL ? pMax : pRestore);
		if( pBar != NULL && !pBar->IsShowMax() ) {
			if( pMax != NULL ) pMax->SetVisible(false);
			if( pRestore != NULL ) pRestore->SetVisible(false);
			return;
		}

		if( bMaximized ) {
			if( pMax != NULL ) pMax->SetVisible(false);
			if( pRestore != NULL ) pRestore->SetVisible(true);
		}
		else {
			if( pMax != NULL ) pMax->SetVisible(true);
			if( pRestore != NULL ) pRestore->SetVisible(false);
		}
	}

	void WindowImplBase::OnFinalMessage( HWND hWnd )
	{
		m_bHaveOwnerOffset = false;
		if( m_pDefaultTrayMenu != NULL ) {
			delete m_pDefaultTrayMenu;
			m_pDefaultTrayMenu = NULL;
		}
		if( m_bTrayAutoCreated )
			m_trayIcon.DeleteTrayIcon();
		m_pm.RemovePreMessageFilter(this);
		m_pm.RemoveNotifier(this);
		m_pm.ReapObjects(m_pm.GetRoot());
	}

	LRESULT WindowImplBase::ResponseDefaultKeyEvent(WPARAM wParam)
	{
		if (wParam == VK_RETURN)
		{
			return FALSE;
		}
		else if (wParam == VK_ESCAPE)
		{
			return TRUE;
		}

		return FALSE;
	}

	UINT WindowImplBase::GetClassStyle() const
	{
		return CS_DBLCLKS;
	}

	CControlUI* WindowImplBase::CreateControl(LPCTSTR pstrClass)
	{
		return NULL;
	}

	LPCTSTR WindowImplBase::QueryControlText(LPCTSTR lpstrId, LPCTSTR lpstrType)
	{
		return NULL;
	}

	LRESULT WindowImplBase::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& /*bHandled*/)
	{
        if (uMsg == WM_KEYDOWN)
        {
            switch (wParam)
            {
            case VK_RETURN:
            case VK_ESCAPE:
                return ResponseDefaultKeyEvent(wParam);
            default:
                break;
            }
        }
		return FALSE;
	}

	void WindowImplBase::ForceClose(UINT nRet)
	{
		m_bForceClose = true;
		Close(nRet);
	}

	CDuiString WindowImplBase::GetAutoTrayTooltip() const
	{
		TCHAR szTitle[256] = { 0 };
		if( m_hWnd != NULL )
			::GetWindowText(m_hWnd, szTitle, 255);
		if( szTitle[0] != _T('\0') )
			return szTitle;
		CControlUI* pNamed = const_cast<CPaintManagerUI&>(m_pm).FindControl(_T("titlebar"));
		if( pNamed != NULL ) {
			CTitleBarUI* pBar = static_cast<CTitleBarUI*>(pNamed->GetInterface(DUI_CTR_TITLEBAR));
			if( pBar != NULL ) {
				CDuiString s = pBar->GetTitle();
				if( !s.IsEmpty() ) return s;
			}
		}
		return _T("Application");
	}

	void WindowImplBase::EnsureAutoTray()
	{
		if( FindMinimizeToTrayTitleBar(m_pm) == NULL && FindCloseToTrayTitleBar(m_pm) == NULL )
			return;
		if( m_trayIcon.IsEnabled() )
			return;
		CDuiString tip = GetAutoTrayTooltip();
		if( m_trayIcon.Create(m_hWnd, 1, tip.GetData()) ) {
			m_bTrayAutoCreated = true;
			m_trayIcon.SetNotifyVersion(NOTIFYICON_VERSION_4);
		}
	}

	void WindowImplBase::ShowDefaultTrayMenu(POINT pt)
	{
		if( m_pDefaultTrayMenu != NULL ) {
			delete m_pDefaultTrayMenu;
			m_pDefaultTrayMenu = NULL;
		}
		static const TCHAR kXml[] =
			_T("<Menu border-width=\"1\" border-radius=\"4,4\" padding=\"4,4,4,4\" item-padding=\"0,14,0,32\">")
			_T("<MenuElement name=\"tray_show\" text=\"显示主窗口\"/>")
			_T("<MenuElement name=\"tray_exit\" text=\"退出\"/>")
			_T("</Menu>");
		m_pDefaultTrayMenu = new CMenuWnd();
		m_pDefaultTrayMenu->Init(NULL, kXml, pt, &m_pm);
		m_pDefaultTrayMenu->ResizeMenu();
	}

	bool WindowImplBase::ProcessDefaultTrayMenuCommand(LPCTSTR pstrName)
	{
		if( pstrName == NULL ) return false;
		if( _tcsicmp(pstrName, _T("tray_show")) == 0 ) {
			CTrayIcon::ShowWindowOnTaskbar(m_hWnd, true);
			return true;
		}
		if( _tcsicmp(pstrName, _T("tray_exit")) == 0 ) {
			ForceClose(0);
			return true;
		}
		return false;
	}

	bool WindowImplBase::ProcessAutoTrayMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult)
	{
		lResult = 0;
		if( !m_bTrayAutoCreated )
			return false;

		if( uMsg == CTrayIcon::GetTaskbarCreatedMsg() ) {
			m_trayIcon.Recreate();
			return true;
		}
		if( uMsg == UIMSG_TRAYICON ) {
			UINT ev = CTrayIcon::DecodeNotifyMsg(wParam, lParam, m_trayIcon.GetNotifyVersion());
			POINT pt = CTrayIcon::DecodeNotifyPos(wParam, lParam, m_trayIcon.GetNotifyVersion());
			if( ev == WM_LBUTTONUP ) {
				if( !::IsWindowVisible(m_hWnd) || CTrayIcon::IsWindowHiddenFromTaskbar(m_hWnd) )
					CTrayIcon::ShowWindowOnTaskbar(m_hWnd, true);
				else
					CTrayIcon::HideWindowFromTaskbar(m_hWnd);
			}
			else if( ev == WM_RBUTTONUP ) {
				ShowDefaultTrayMenu(pt);
			}
			return true;
		}
		if( uMsg == WM_MENUCLICK ) {
			MenuCmd* pMenuCmd = (MenuCmd*)wParam;
			if( pMenuCmd == NULL ) return false;
			CDuiString sName = pMenuCmd->szName;
			const bool bOurs = ProcessDefaultTrayMenuCommand(sName.GetData());
			if( bOurs )
				m_pm.DeletePtr(pMenuCmd);
			return bOurs;
		}
		return false;
	}

	LRESULT WindowImplBase::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if( !m_bForceClose && FindCloseToTrayTitleBar(m_pm) != NULL ) {
			CTrayIcon::HideWindowFromTaskbar(m_hWnd);
			bHandled = TRUE;
			return 0;
		}
		m_bForceClose = false;
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

#if defined(WIN32) && !defined(UNDER_CE)
	LRESULT WindowImplBase::OnNcActivate(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if( ::IsIconic(*this) ) bHandled = FALSE;
		return (wParam == 0) ? TRUE : FALSE;
	}

	LRESULT WindowImplBase::OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT WindowImplBase::OnNcPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 0;
	}


	BOOL WindowImplBase::IsInStaticControl(CControlUI *pControl)
	{
		BOOL bRet = FALSE;
		if (!pControl)
		{
			return bRet;
		}

		CDuiString strClassName;
		std::vector<CDuiString> vctStaticName;

		strClassName = pControl->GetClass();
		strClassName.MakeLower();
		vctStaticName.push_back(_T("controlui"));
		vctStaticName.push_back(_T("textui"));
		vctStaticName.push_back(_T("labelui"));
		vctStaticName.push_back(_T("containerui"));
		vctStaticName.push_back(_T("horizontallayoutui"));
		vctStaticName.push_back(_T("verticallayoutui"));
		vctStaticName.push_back(_T("tablayoutui"));
		vctStaticName.push_back(_T("childlayoutui"));
		vctStaticName.push_back(_T("dialoglayoutui"));
		vctStaticName.push_back(_T("progresscontainerui"));
		std::vector<CDuiString>::iterator it = std::find(vctStaticName.begin(), vctStaticName.end(), strClassName);
		if (vctStaticName.end() != it)
		{
			CControlUI* pParent = pControl->GetParent();
			while (pParent)
			{
				strClassName = pParent->GetClass();
				strClassName.MakeLower();
				it = std::find(vctStaticName.begin(), vctStaticName.end(), strClassName);
				if (vctStaticName.end() == it)
				{
					return bRet;
				}

				pParent = pParent->GetParent();
			}

			bRet = TRUE;
		}

		return bRet;
	}

	LRESULT WindowImplBase::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
		::ScreenToClient(*this, &pt);

		RECT rcClient;
		::GetClientRect(*this, &rcClient);

		// 注意：不要对 maxbtn/restorebtn 返回 HTMAXBUTTON。
		// 返回 HTMAXBUTTON 后鼠标消息变成 WM_NCMOUSE*，客户区 TrackMouseEvent
		// 会在进入该区域时触发 WM_MOUSELEAVE 并用 (-1,-1) 清掉 hover，
		// 导致最大化按钮 image-hover 不生效（最小化/关闭仍为 HTCLIENT 则正常）。
		// Win11 Snap Layouts 若需要，应另做非客户区 hover 同步，而不是牺牲热态绘制。

		if (!::IsZoomed(*this))
		{
			RECT rcSizeBox = m_pm.GetSizeBox();
			if (pt.y < rcClient.top + rcSizeBox.top)
			{
				if (pt.x < rcClient.left + rcSizeBox.left) return HTTOPLEFT;
				if (pt.x > rcClient.right - rcSizeBox.right) return HTTOPRIGHT;
				return HTTOP;
			}
			else if (pt.y > rcClient.bottom - rcSizeBox.bottom)
			{
				if (pt.x < rcClient.left + rcSizeBox.left) return HTBOTTOMLEFT;
				if (pt.x > rcClient.right - rcSizeBox.right) return HTBOTTOMRIGHT;
				return HTBOTTOM;
			}

			if (pt.x < rcClient.left + rcSizeBox.left) return HTLEFT;
			if (pt.x > rcClient.right - rcSizeBox.right) return HTRIGHT;
		}

		// 控件边缘缩放宿主（TabLayout 等可只开 N 条边；与窗口 size-box 互补）
		// FindControl 会跳过 mouse=false（如 window 模式 WebBrowser），可能拿不到 TabLayout；
		// 先沿命中链，再从 root 深搜含该点且开了 window-resize 的控件。
		{
			CControlUI* pHitCtrl = m_pm.FindControl(pt);
			for (CControlUI* pWalk = pHitCtrl; pWalk != NULL; pWalk = pWalk->GetParent()) {
				LRESULT ht = pWalk->HitWindowResize(pt);
				if (ht != HTCLIENT) return ht;
			}
			CControlUI* pRoot = m_pm.GetRoot();
			if (pRoot != NULL) {
				LRESULT ht = HitWindowResizeDeep(pRoot, pt);
				if (ht != HTCLIENT) return ht;
			}
		}

		// SidePanel fill-host：面板边缘缩放宿主（size-box 为 0 或未盖满客户区时补一刀）
		{
			CControlUI* pHitCtrl = m_pm.FindControl(pt);
			for (CControlUI* pWalk = pHitCtrl; pWalk != NULL; pWalk = pWalk->GetParent()) {
				CSidePanelUI* pSp = static_cast<CSidePanelUI*>(pWalk->GetInterface(DUI_CTR_SIDEPANEL));
				if (pSp == NULL) continue;
				LRESULT ht = pSp->HitHostResize(pt);
				if (ht != HTCLIENT) return ht;
				break;
			}
		}

		// action 属性驱动的拖拽：不受 caption rect 限制；IsCaptionDragHit 区分空白/交互区
		{
			CControlUI* pHitCtrl = m_pm.FindControl(pt);
			if (pHitCtrl != NULL) {
				if (pHitCtrl->IsCaptionDragHit(pt))
					return HTCAPTION;

				UIAction leafAct = pHitCtrl->GetAction();
				// 自身有 title 但点在交互区（如 TabBar 标签/+）：保持 HTCLIENT，不向上/窗口级拖拽
				// PreferClientHit：SETCURSOR / cursor / 已配热态视觉；新控件用基类 *-hover 即可，不必再改此处
				if (leafAct == UIACTION_NONE && !pHitCtrl->PreferClientHit()) {
					CControlUI* pWalk = pHitCtrl->GetParent();
					while (pWalk != NULL) {
						if (pWalk->IsCaptionDragHit(pt))
							return HTCAPTION;
						UIAction parentAct = pWalk->GetAction();
						// 父级是 title 但该点不可拖（交互区）→ 停止向上，留给客户区点击
						if (parentAct == UIACTION_TITLE || parentAct == UIACTION_MOVEWINDOW)
							break;
						if (parentAct != UIACTION_NONE) break;
						pWalk = pWalk->GetParent();
					}
					UIAction winAct = m_pm.GetWindowAction();
					if (winAct == UIACTION_TITLE || winAct == UIACTION_MOVEWINDOW)
						return HTCAPTION;
				}
			}
			else {
				UIAction winAct = m_pm.GetWindowAction();
				if (winAct == UIACTION_TITLE || winAct == UIACTION_MOVEWINDOW)
					return HTCAPTION;
			}
		}

		RECT rcCaption = m_pm.GetCaptionRect();
		if (0 > rcCaption.bottom)
		{
			rcCaption.bottom = rcClient.bottom;
		}

		if (pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right
			&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom)
		{
			if (IsInStaticControl(m_pm.FindControl(pt)))
			{
				return HTCAPTION;
			}
		}

		return HTCLIENT;
	}

	LRESULT WindowImplBase::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		MONITORINFO mi = {};
		mi.cbSize = sizeof(mi);
		// 用窗口所在屏（多显示器）；勿用 PRIMARY，否则副屏最大化会套主屏工作区
		::GetMonitorInfo(::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST), &mi);
		const RECT& rcWork = mi.rcWork;
		const RECT& rcMon = mi.rcMonitor;
		const int cxWork = rcWork.right - rcWork.left;
		const int cyWork = rcWork.bottom - rcWork.top;
		// ptMaxPosition 相对当前显示器左上角；保留任务栏占用（含顶部任务栏）
		// 旧逻辑对非主屏 OffsetRect 到 (0,0) 会抹掉 rcWork.top 偏移，标题栏被挡

		LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
		lpMMI->ptMaxPosition.x = rcWork.left - rcMon.left;
		lpMMI->ptMaxPosition.y = rcWork.top - rcMon.top;
		lpMMI->ptMaxSize.x = cxWork;
		lpMMI->ptMaxSize.y = cyWork;
		lpMMI->ptMaxTrackSize.x = m_pm.GetMaxSize().cx == 0 ? cxWork : m_pm.GetMaxSize().cx;
		lpMMI->ptMaxTrackSize.y = m_pm.GetMaxSize().cy == 0 ? cyWork : m_pm.GetMaxSize().cy;
		lpMMI->ptMinTrackSize.x = m_pm.GetMinSize().cx;
		lpMMI->ptMinTrackSize.y = m_pm.GetMinSize().cy;

		// SyncOwnerSize：本窗可缩范围不得小于 Owner 的 min-size（铺满时差为 0 → 与主窗同限）
		if( m_bSyncOwnerSize && m_bHaveOwnerOffset ) {
			HWND hOwner = ResolveSyncOwner();
			if( hOwner != NULL ) {
				SIZE oMin = { 0 }, oMax = { 0 };
				QueryHwndTrackSize(hOwner, oMin, oMax);
				const int selfMinW = oMin.cx - m_szOwnerDelta.cx;
				const int selfMinH = oMin.cy - m_szOwnerDelta.cy;
				if( selfMinW > lpMMI->ptMinTrackSize.x )
					lpMMI->ptMinTrackSize.x = selfMinW > 1 ? selfMinW : 1;
				if( selfMinH > lpMMI->ptMinTrackSize.y )
					lpMMI->ptMinTrackSize.y = selfMinH > 1 ? selfMinH : 1;
				if( oMax.cx > 0 ) {
					const int selfMaxW = oMax.cx - m_szOwnerDelta.cx;
					if( selfMaxW > 0 && lpMMI->ptMaxTrackSize.x > selfMaxW )
						lpMMI->ptMaxTrackSize.x = selfMaxW;
				}
				if( oMax.cy > 0 ) {
					const int selfMaxH = oMax.cy - m_szOwnerDelta.cy;
					if( selfMaxH > 0 && lpMMI->ptMaxTrackSize.y > selfMaxH )
						lpMMI->ptMaxTrackSize.y = selfMaxH;
				}
			}
		}

		bHandled = TRUE;
		return 0;
	}

	LRESULT WindowImplBase::OnMouseWheel(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnMouseHover(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}
#endif

	LRESULT WindowImplBase::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		SIZE szBorderRadius = m_pm.GetBorderRadius();
#if defined(WIN32) && !defined(UNDER_CE)
		if( !::IsIconic(*this) ) {
			if( !m_pm.ApplyWindowShapeRgn(*this) ) {
				CDuiRect rcWnd;
				::GetWindowRect(*this, &rcWnd);
				rcWnd.Offset(-rcWnd.left, -rcWnd.top);
				rcWnd.right++; rcWnd.bottom++;
				SIZE szEllipse = CssRadiusToEllipse(szBorderRadius);
				HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szEllipse.cx, szEllipse.cy);
				::SetWindowRgn(*this, hRgn, TRUE);
				::DeleteObject(hRgn);
			}

			if (m_pm.IsValid() && wParam == SIZE_RESTORED) {
				SyncMaxRestoreButtons(m_pm, false);
			}
		}
		// SyncOwner*：最小化/还原与 Owner 联动（几何同步仍跳过 iconic/zoomed 矩形）
		if( wParam == SIZE_MINIMIZED || wParam == SIZE_RESTORED )
			SyncOwnerShowState();
#endif
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (wParam == SC_CLOSE)
		{
			if( FindCloseToTrayTitleBar(m_pm) != NULL ) {
				CTrayIcon::HideWindowFromTaskbar(m_hWnd);
				bHandled = TRUE;
				return 0;
			}
			bHandled = TRUE;
			SendMessage(WM_CLOSE);
			return 0;
		}
		// min-to-tray：禁止 DefWindowProc 走 SC_MINIMIZE（否则会以 iconic 留在任务栏）
		if( (wParam & 0xFFF0) == SC_MINIMIZE ) {
			if( FindMinimizeToTrayTitleBar(m_pm) != NULL ) {
				CTrayIcon::HideWindowFromTaskbar(m_hWnd);
				bHandled = TRUE;
				return 0;
			}
		}
#if defined(WIN32) && !defined(UNDER_CE)
		BOOL bZoomed = ::IsZoomed(*this);
		LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
		if( ::IsZoomed(*this) != bZoomed ) {
			SyncMaxRestoreButtons(m_pm, ::IsZoomed(*this) != FALSE);
		}
#else
		LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
#endif
		return lRes;
	}

	LRESULT WindowImplBase::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// 调整窗口样式
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;
		::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

		// 关联UI管理器
		m_pm.Init(m_hWnd, GetManagerName());
		// 注册PreMessage回调
		m_pm.AddPreMessageFilter(this);

		// 创建主窗口
		CControlUI* pRoot=NULL;
		CDialogBuilder builder;
		CDuiString sSkinType = GetSkinType();
		if (!sSkinType.IsEmpty()) {
			STRINGorID xml(_ttoi(GetSkinFile().GetData()));
			pRoot = builder.Create(xml, sSkinType.GetData(), this, &m_pm);
		}
		else {
			pRoot = builder.Create(GetSkinFile().GetData(), NULL, this, &m_pm);
		}

		if (pRoot == NULL) {
			CDuiString sError = _T("加载资源文件失败：");
			sError += GetSkinFile();
			MessageBox(NULL, sError.GetData(), _T("Duilib") ,MB_OK|MB_ICONERROR);
			ExitProcess(1);
			return 0;
		}
		m_pm.AttachDialog(pRoot);
		// 添加Notify事件接口
		m_pm.AddNotifier(this);
		// 皮肤 size 属性：创建后按 InitSize 调整客户区（未设置则保持 Create 尺寸）
		{
			SIZE szInit = m_pm.GetInitSize();
			if( szInit.cx > 0 && szInit.cy > 0 )
				ResizeClient(szInit.cx, szInit.cy);
		}
		// 窗口初始化完毕
		InitWindow();
		// min-to-tray / close-to-tray：若应用未自行 Create 托盘则自动创建 + 默认菜单
		EnsureAutoTray();
		return 0;
	}

	LRESULT WindowImplBase::OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnKillFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnWindowPosChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		const WINDOWPOS* pwp = reinterpret_cast<const WINDOWPOS*>(lParam);
		// 正在 SetWindowPos(Owner) 时忽略，避免重入
		if( pwp != NULL && !m_bSyncingOwner ) {
			const bool bMoved = (pwp->flags & SWP_NOMOVE) == 0;
			const bool bSized = (pwp->flags & SWP_NOSIZE) == 0;
			if( bSized && m_bSyncOwnerSize ) {
				// 含跨屏 DPI 引起的尺寸变化：同步 Owner；Move 开启时位置一并跟
				SyncOwnerGeometry(bMoved && m_bSyncOwnerMove, true);
			}
			else if( bMoved && m_bSyncOwnerMove ) {
				if( bSized )
					CaptureOwnerSyncOffset();
				else
					SyncOwnerPosition();
			}
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnDisplayChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// 显示器插拔 / 布局变化后重抓偏移（屏幕坐标可能整体平移）
		if( m_bSyncOwnerMove || m_bSyncOwnerSize )
			CaptureOwnerSyncOffset();
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnDPIChanged(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// 与 MainWnd / HiDPI 一致：系统建议矩形为准，勿再按比例二次缩放窗口
		m_pm.SetDPI(LOWORD(wParam), false);
		RECT* const prcNewWindow = reinterpret_cast<RECT*>(lParam);
		if( prcNewWindow != NULL ) {
			m_bSyncingOwner = true;
			::SetWindowPos(m_hWnd, NULL,
				prcNewWindow->left, prcNewWindow->top,
				prcNewWindow->right - prcNewWindow->left,
				prcNewWindow->bottom - prcNewWindow->top,
				SWP_NOZORDER | SWP_NOACTIVATE);
			m_bSyncingOwner = false;
		}
		if( m_pm.GetRoot() != NULL )
			m_pm.GetRoot()->NeedUpdate();

		// 跨屏 DPI：本窗已落在新物理矩形上，按原偏移把 Owner 一起带过去，再重抓差
		if( m_bHaveOwnerOffset )
			SyncOwnerGeometry(m_bSyncOwnerMove, m_bSyncOwnerSize);
		if( m_bSyncOwnerMove || m_bSyncOwnerSize )
			CaptureOwnerSyncOffset();

		bHandled = TRUE;
		return 0;
	}

	LRESULT WindowImplBase::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		// 鼠标消息最外层入口日志：确认点击是否真到了本窗（含客户区(左/右)与非客户区 NC）。
		// 开销：类型判断(常量) + 开关原子读，都极快；只有这些消息且开了日志才做坐标/命中计算。
		// 注意：NC 消息 lParam 是屏幕坐标、wParam 是命中测试 HT 值；客户区消息 lParam 是客户坐标。
		if( (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_RBUTTONDOWN
			 || uMsg == WM_RBUTTONUP || uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONUP
			 || uMsg == WM_CONTEXTMENU
			 || uMsg == WM_NCLBUTTONDOWN || uMsg == WM_NCLBUTTONUP || uMsg == WM_NCRBUTTONDOWN
			 || uMsg == WM_NCRBUTTONUP || uMsg == WM_NCMBUTTONDOWN || uMsg == WM_NCMBUTTONUP)
			&& CDuiLog::IsEnabled() ) {
			POINT spta = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			POINT cpta = spta;
			::ScreenToClient(m_hWnd, &cpta);
			RECT rcct;
			::GetClientRect(m_hWnd, &rcct);
			HWND hfp = ::WindowFromPoint(spta);
			BOOL bNC = (uMsg >= WM_NCLBUTTONDOWN && uMsg <= WM_NCMBUTTONUP);
			DUILOG(_T("[HandleMessage] uMsg=0x%X%hs screen=(%d,%d) client=(%d,%d) win=(%d,%d,%d,%d) hit=0x%p"),
				uMsg, bNC ? "(NC) " : "", spta.x, spta.y, cpta.x, cpta.y,
				rcct.left, rcct.top, rcct.right, rcct.bottom, (void*)hfp);
		}
		switch (uMsg)
		{
		case WM_CREATE:			lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
		case WM_CLOSE:			lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
		case WM_DESTROY:		lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
#if defined(WIN32) && !defined(UNDER_CE)
		case WM_NCACTIVATE:		lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
		case WM_NCCALCSIZE:		lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
		case WM_NCPAINT:		lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
		case WM_NCHITTEST:		lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
		case WM_NCLBUTTONDOWN:
		case WM_NCLBUTTONUP:
		case WM_NCLBUTTONDBLCLK: {
			if (wParam == HTMAXBUTTON) {
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				::ScreenToClient(m_hWnd, &pt);
				LPARAM param = MAKELPARAM(pt.x, pt.y);
				if (uMsg == WM_NCLBUTTONDOWN) {
					::SendMessage(m_hWnd, WM_LBUTTONDOWN, wParam, param);
				}
				else if (uMsg == WM_NCLBUTTONUP) {
					::SendMessage(m_hWnd, WM_LBUTTONUP, wParam, param);
				}
				return 0;
			}
			bHandled = FALSE;
			break;
		}
		// 非客户区右键（action:title 把空白区判成 HTCAPTION 时，系统发给 WM_NCRBUTTON*）。
		// 处理：左键仍走 DefWindowProc 拖拽；右键转回客户区消息，让 DUIX 正常走 WM_CONTEXTMENU 弹菜单。
		case WM_NCRBUTTONDOWN:
		case WM_NCRBUTTONUP:
		case WM_NCRBUTTONDBLCLK: {
			POINT spt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };   // NC 消息坐标是屏幕坐标
			POINT cpt = spt;
			::ScreenToClient(m_hWnd, &cpt);
			// 转客户区坐标，交给客户区右键流程（与 HTMAXBUTTON 的转发同模式）
			if( uMsg == WM_NCRBUTTONDOWN )
				::SendMessage(m_hWnd, WM_RBUTTONDOWN, wParam, MAKELPARAM(cpt.x, cpt.y));
			else if( uMsg == WM_NCRBUTTONUP )
				::SendMessage(m_hWnd, WM_RBUTTONUP, wParam, MAKELPARAM(cpt.x, cpt.y));
			else if( uMsg == WM_NCRBUTTONDBLCLK )
				::SendMessage(m_hWnd, WM_RBUTTONDBLCLK, wParam, MAKELPARAM(cpt.x, cpt.y));
			// 抬起后补发 WM_CONTEXTMENU（系统对非客户区不会自动生成），DUIX 据此弹菜单
			if( uMsg == WM_NCRBUTTONUP )
				::SendMessage(m_hWnd, WM_CONTEXTMENU, wParam, MAKELPARAM(spt.x, spt.y));
			return 0;
		}
		case WM_NCMOUSEHOVER:
		case WM_NCMOUSELEAVE:
		case WM_NCMOUSEMOVE: {
			if (wParam == HTMAXBUTTON) {
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				::ScreenToClient(m_hWnd, &pt);
				LPARAM param = MAKELPARAM(pt.x, pt.y);
				LRESULT lr = 0;
				if (uMsg == WM_NCMOUSELEAVE) {
					m_pm.MessageHandler(WM_MOUSELEAVE, 0, 0, lr);
				}
				else if (uMsg == WM_NCMOUSEMOVE) {
					TRACKMOUSEEVENT tme = { 0 };
					tme.cbSize = sizeof(tme);
					tme.dwFlags = TME_LEAVE | TME_NONCLIENT;
					tme.hwndTrack = m_hWnd;
					::TrackMouseEvent(&tme);
					// wParam 必须是按键标志，不能传 HTMAXBUTTON
					m_pm.MessageHandler(WM_MOUSEMOVE, 0, param, lr);
				}
				return 0;
			}
			bHandled = FALSE;
			break;
		}
		case WM_GETMINMAXINFO:	lRes = OnGetMinMaxInfo(uMsg, wParam, lParam, bHandled); break;
		case WM_MOUSEWHEEL:		lRes = OnMouseWheel(uMsg, wParam, lParam, bHandled); break;
#endif
		case WM_SIZE:			lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
		case WM_WINDOWPOSCHANGED: lRes = OnWindowPosChanged(uMsg, wParam, lParam, bHandled); break;
		case WM_DISPLAYCHANGE:	lRes = OnDisplayChange(uMsg, wParam, lParam, bHandled); break;
#if defined(WIN32) && !defined(UNDER_CE)
		case WM_DPICHANGED:		lRes = OnDPIChanged(uMsg, wParam, lParam, bHandled); break;
#endif
		case WM_CHAR:		lRes = OnChar(uMsg, wParam, lParam, bHandled); break;
		case WM_SYSCOMMAND:		lRes = OnSysCommand(uMsg, wParam, lParam, bHandled); break;
		case WM_KEYDOWN:		lRes = OnKeyDown(uMsg, wParam, lParam, bHandled); break;
		case WM_KILLFOCUS:		lRes = OnKillFocus(uMsg, wParam, lParam, bHandled); break;
		case WM_SETFOCUS:		lRes = OnSetFocus(uMsg, wParam, lParam, bHandled); break;
		case WM_LBUTTONUP:		lRes = OnLButtonUp(uMsg, wParam, lParam, bHandled); break;
		case WM_LBUTTONDOWN:	lRes = OnLButtonDown(uMsg, wParam, lParam, bHandled); break;
		case WM_MOUSEMOVE:		lRes = OnMouseMove(uMsg, wParam, lParam, bHandled); break;
		case WM_MOUSEHOVER:	lRes = OnMouseHover(uMsg, wParam, lParam, bHandled); break;
		default:				bHandled = FALSE; break;
		}
		if (bHandled) return lRes;

		if( ProcessAutoTrayMessage(uMsg, wParam, lParam, lRes) )
			return lRes;

		lRes = HandleCustomMessage(uMsg, wParam, lParam, bHandled);
		if (bHandled) return lRes;

		if (m_pm.MessageHandler(uMsg, wParam, lParam, lRes))
			return lRes;
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}

	LRESULT WindowImplBase::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LONG WindowImplBase::GetStyle()
	{
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;

		return styleValue;
	}

	void WindowImplBase::OnClick(TNotifyUI& msg)
	{
		CControlUI* pSender = msg.pSender;
		if (pSender != NULL) {
			switch (pSender->GetAction())
			{
			case UIACTION_CLOSE:
				if( FindCloseToTrayTitleBar(m_pm) != NULL )
					CTrayIcon::HideWindowFromTaskbar(m_hWnd);
				else
					Close();
				return;
			case UIACTION_MIN:
				if( FindMinimizeToTrayTitleBar(m_pm) != NULL )
					CTrayIcon::HideWindowFromTaskbar(m_hWnd);
				else
					SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
				return;
			case UIACTION_MAX:
				SendMessage(WM_SYSCOMMAND, ::IsZoomed(m_hWnd) ? SC_RESTORE : SC_MAXIMIZE, 0);
				return;
			case UIACTION_COPY:
				{
					CDuiString text = pSender->GetCustomAttribute(_T("copy-text"));
					if (text.IsEmpty()) text = pSender->GetText();
					if (!text.IsEmpty() && ::OpenClipboard(m_hWnd)) {
						::EmptyClipboard();
						size_t nBytes = (text.GetLength() + 1) * sizeof(TCHAR);
						HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, nBytes);
						if (hMem) {
							void* p = ::GlobalLock(hMem);
							if (p) {
								memcpy(p, text.GetData(), nBytes);
								::GlobalUnlock(hMem);
#ifdef _UNICODE
								::SetClipboardData(CF_UNICODETEXT, hMem);
#else
								::SetClipboardData(CF_TEXT, hMem);
#endif
							}
						}
						::CloseClipboard();
					}
				}
				return;
			default:
				break;
			}
		}

		CDuiString sCtrlName = msg.pSender->GetName();
		// TitleBar 系统按钮由 CTitleBarUI 处理；to-tray 时在此再兜底 Hide
		if( sCtrlName == _T("closebtn") || sCtrlName == _T("minbtn")
			|| sCtrlName == _T("maxbtn") || sCtrlName == _T("restorebtn") ) {
			CTitleBarUI* pBar = FindOwnerTitleBar(msg.pSender);
			if( pBar == NULL ) {
				CControlUI* pNamed = m_pm.FindControl(_T("titlebar"));
				if( pNamed != NULL )
					pBar = static_cast<CTitleBarUI*>(pNamed->GetInterface(DUI_CTR_TITLEBAR));
			}
			if( pBar != NULL ) {
				if( sCtrlName == _T("minbtn") && pBar->IsMinimizeToTray() )
					CTrayIcon::HideWindowFromTaskbar(m_hWnd);
				else if( sCtrlName == _T("closebtn") && pBar->IsCloseToTray() )
					CTrayIcon::HideWindowFromTaskbar(m_hWnd);
				return;
			}
		}
		if( sCtrlName == _T("closebtn") ) {
			if( FindCloseToTrayTitleBar(m_pm) != NULL )
				CTrayIcon::HideWindowFromTaskbar(m_hWnd);
			else
				Close();
			return; 
		}
		else if( sCtrlName == _T("minbtn")) {
			if( FindMinimizeToTrayTitleBar(m_pm) != NULL )
				CTrayIcon::HideWindowFromTaskbar(m_hWnd);
			else
				SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
			return;
		}
		else if( sCtrlName == _T("maxbtn")) { 
			SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); 
			return; 
		}
		else if( sCtrlName == _T("restorebtn")) { 
			SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); 
			return; 
		}
		return;
	}

	void WindowImplBase::Notify(TNotifyUI& msg)
	{
		return CNotifyPump::NotifyPump(msg);
	}
}