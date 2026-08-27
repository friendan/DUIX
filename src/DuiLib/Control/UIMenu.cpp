#include "StdAfx.h"

#include "UIMenu.h"
#include "UISvgBox.h"

namespace DuiLib {

	/////////////////////////////////////////////////////////////////////////////////////
	//
	IMPLEMENT_DUICONTROL(CMenuUI)

		CMenuUI::CMenuUI():
		m_pWindow(NULL)
		, m_bShowIconLine(true)
	{
		if (GetHeader() != NULL)
			GetHeader()->SetVisible(false);
		// 默认项内边距（Win32 RECT：left,top,right,bottom）= 图标槽宽 + 右侧留白
		// 二级菜单无 XML 时也有左边距；一级菜单可被 item-padding 覆盖
		RECT rcItemPad = { ITEM_DEFAULT_ICON_WIDTH + 6, 0, 14, 0 };
		SetItemTextPadding(rcItemPad);
	}

	CMenuUI::~CMenuUI()
	{

	}

	LPCTSTR CMenuUI::GetClass() const
	{
		return _T("MenuUI");
	}

	LPVOID CMenuUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, _T("Menu")) == 0 ) return static_cast<CMenuUI*>(this);
		return CListUI::GetInterface(pstrName);
	}

	UINT CMenuUI::GetListType()
	{
		return LT_MENU;
	}

	void CMenuUI::DoEvent(TEventUI& event)
	{
		return __super::DoEvent(event);
	}

	bool CMenuUI::Add(CControlUI* pControl)
	{
		CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(_T("MenuElement")));
		if (pMenuItem == NULL)
			return false;

		for (int i = 0; i < pMenuItem->GetCount(); ++i)
		{
			if (pMenuItem->GetItemAt(i)->GetInterface(_T("MenuElement")) != NULL)
			{
				(static_cast<CMenuElementUI*>(pMenuItem->GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetInternVisible(false);
			}
		}
		return CListUI::Add(pControl);
	}

	bool CMenuUI::AddAt(CControlUI* pControl, int iIndex)
	{
		CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(_T("MenuElement")));
		if (pMenuItem == NULL)
			return false;

		for (int i = 0; i < pMenuItem->GetCount(); ++i)
		{
			if (pMenuItem->GetItemAt(i)->GetInterface(_T("MenuElement")) != NULL)
			{
				(static_cast<CMenuElementUI*>(pMenuItem->GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetInternVisible(false);
			}
		}
		return CListUI::AddAt(pControl, iIndex);
	}

	int CMenuUI::GetItemIndex(CControlUI* pControl) const
	{
		CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(_T("MenuElement")));
		if (pMenuItem == NULL)
			return -1;

		return __super::GetItemIndex(pControl);
	}

	bool CMenuUI::SetItemIndex(CControlUI* pControl, int iIndex)
	{
		CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(_T("MenuElement")));
		if (pMenuItem == NULL)
			return false;

		return __super::SetItemIndex(pControl, iIndex);
	}

	bool CMenuUI::Remove(CControlUI* pControl)
	{
		CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(_T("MenuElement")));
		if (pMenuItem == NULL)
			return false;

		return __super::Remove(pControl);
	}

	SIZE CMenuUI::EstimateSize(SIZE szAvailable)
	{
		int cxFixed = 0;
		int cyFixed = 0;
		for( int it = 0; it < GetCount(); it++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(GetItemAt(it));
			if( !pControl->IsVisible() ) continue;
			SIZE sz = pControl->EstimateSize(szAvailable);
			cyFixed += sz.cy;
			if( cxFixed < sz.cx )
				cxFixed = sz.cx;
		}

		for (int it = 0; it < GetCount(); it++) {
			CControlUI* pControl = static_cast<CControlUI*>(GetItemAt(it));
			if (!pControl->IsVisible()) continue;

			pControl->SetFixedWidth(MulDiv(cxFixed, 100, GetManager()->GetDPIObj()->GetScale()));
		}

		return CDuiSize(cxFixed, cyFixed);
	}

	void CMenuUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("icon-line")) == 0
			|| _tcsicmp(pstrName, _T("show-icon-line")) == 0 ) {
			SetShowIconLine(_tcsicmp(pstrValue, _T("true")) == 0
				|| _tcscmp(pstrValue, _T("1")) == 0);
			return;
		}
		CListUI::SetAttribute(pstrName, pstrValue);
	}

	void CMenuUI::SetShowIconLine(bool bShow)
	{
		if( m_bShowIconLine == bShow ) return;
		m_bShowIconLine = bShow;
		Invalidate();
	}

	bool CMenuUI::IsShowIconLine() const
	{
		return m_bShowIconLine;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//

	CMenuWnd::CMenuWnd():
	isClosing(false),
		m_bCaptured(false),
		m_xml(_T("")),
		m_pOwner(NULL),
		m_pLayout()
	{
		m_dwAlignment = eMenuAlignment_Left | eMenuAlignment_Top;
	}

	CMenuWnd::~CMenuWnd()
	{
		
	}

	void CMenuWnd::Close(UINT nRet)
	{
		if (!::IsWindow(m_hWnd)) return;
		PostMessage(WM_CLOSE, (WPARAM)nRet, 0L);
		isClosing = true;
	}


	BOOL CMenuWnd::Receive(ContextMenuParam param)
	{
		switch (param.wParam)
		{
		case 1:
			Close();
			break;
		case 2:
			{
				HWND hParent = GetParent(m_hWnd);
				while (hParent != NULL)
				{
					if (hParent == param.hWnd)
					{
						Close();
						break;
					}
					hParent = GetParent(hParent);
				}
			}
			break;
		default:
			break;
		}

		return TRUE;
	}

	CMenuWnd* CMenuWnd::CreateMenu(CMenuElementUI* pOwner, STRINGorID xml, POINT point, CPaintManagerUI* pMainPaintManager, CStdStringPtrMap* pMenuCheckInfo /*= NULL*/, DWORD dwAlignment /*= eMenuAlignment_Left | eMenuAlignment_Top*/)
	{
		CMenuWnd* pMenu = new CMenuWnd;
		pMenu->Init(pOwner, xml, point, pMainPaintManager, pMenuCheckInfo, dwAlignment);
		return pMenu;
	}

	void CMenuWnd::DestroyMenu()
	{
		CStdStringPtrMap* mCheckInfos = CMenuWnd::GetGlobalContextMenuObserver().GetMenuCheckInfo();
		if (mCheckInfos != NULL)
		{
			for(int i = 0; i < mCheckInfos->GetSize(); i++) {
				MenuItemInfo* pItemInfo = (MenuItemInfo*)mCheckInfos->Find(mCheckInfos->GetAt(i));
				if(pItemInfo != NULL) {
					delete pItemInfo;
					pItemInfo = NULL;
				}
			}
			mCheckInfos->Resize(0);
		}
	}
	
	MenuItemInfo* CMenuWnd::SetMenuItemInfo(LPCTSTR pstrName, bool bChecked)
	{
		if(pstrName == NULL || lstrlen(pstrName) <= 0) return NULL;

		CStdStringPtrMap* mCheckInfos = CMenuWnd::GetGlobalContextMenuObserver().GetMenuCheckInfo();
		if (mCheckInfos != NULL)
		{
			MenuItemInfo* pItemInfo = (MenuItemInfo*)mCheckInfos->Find(pstrName);
			if(pItemInfo == NULL) {
				pItemInfo = new MenuItemInfo;
				lstrcpy(pItemInfo->szName, pstrName);
				pItemInfo->bChecked = bChecked;
				mCheckInfos->Insert(pstrName, pItemInfo);
			}
			else {
				pItemInfo->bChecked = bChecked;
			}

			return pItemInfo;
		}
		return NULL;
	}

	void CMenuWnd::Init(CMenuElementUI* pOwner, STRINGorID xml, POINT point,
		CPaintManagerUI* pMainPaintManager, CStdStringPtrMap* pMenuCheckInfo/* = NULL*/,
		DWORD dwAlignment/* = eMenuAlignment_Left | eMenuAlignment_Top*/)
	{

		m_BasedPoint = point;
		m_pOwner = pOwner;
		m_pLayout = NULL;
		m_xml = xml;
		m_dwAlignment = dwAlignment;

		// 如果是一级菜单的创建
		if (pOwner == NULL)
		{
			ASSERT(pMainPaintManager != NULL);
			CMenuWnd::GetGlobalContextMenuObserver().SetManger(pMainPaintManager);
			if (pMenuCheckInfo != NULL)
				CMenuWnd::GetGlobalContextMenuObserver().SetMenuCheckInfo(pMenuCheckInfo);
		}

		CMenuWnd::GetGlobalContextMenuObserver().AddReceiver(this);

		Create((m_pOwner == NULL) ? pMainPaintManager->GetPaintWindow() : m_pOwner->GetManager()->GetPaintWindow(), NULL, WS_POPUP , WS_EX_TOOLWINDOW | WS_EX_TOPMOST, CDuiRect());
		
		// HACK: Don't deselect the parent's caption
		HWND hWndParent = m_hWnd;
		while( ::GetParent(hWndParent) != NULL ) hWndParent = ::GetParent(hWndParent);

		::ShowWindow(m_hWnd, SW_SHOW);
		::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
	}

	LPCTSTR CMenuWnd::GetWindowClassName() const
	{
		return _T("DuiMenuWnd");
	}


	void CMenuWnd::Notify(TNotifyUI& msg)
	{
		if( CMenuWnd::GetGlobalContextMenuObserver().GetManager() != NULL) 
		{
			if( msg.sType == _T("click") || msg.sType == _T("valuechanged") ) 
			{
				CMenuWnd::GetGlobalContextMenuObserver().GetManager()->SendNotify(msg, false);
			}
		}

	}

	CControlUI* CMenuWnd::CreateControl( LPCTSTR pstrClassName )
	{
		if (_tcsicmp(pstrClassName, _T("Menu")) == 0)
		{
			return new CMenuUI();
		}
		else if (_tcsicmp(pstrClassName, _T("MenuElement")) == 0)
		{
			return new CMenuElementUI();
		}
		return NULL;
	}


	void CMenuWnd::OnFinalMessage(HWND hWnd)
	{
		RemoveObserver();
		if( m_pOwner != NULL ) {
			for( int i = 0; i < m_pOwner->GetCount(); i++ ) {
				if( static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)->GetInterface(_T("MenuElement"))) != NULL ) {
					(static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)))->SetOwner(m_pOwner->GetParent());
					(static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)))->SetVisible(false);
					(static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetInternVisible(false);
				}
			}
			m_pOwner->m_pWindow = NULL;
			m_pOwner->m_uButtonState &= ~ UISTATE_PUSHED;
			m_pOwner->Invalidate();

			// 内部创建的内部删除
			delete this;
		}
	}

	LRESULT CMenuWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bool bShowShadow = false;
		if( m_pOwner != NULL) {
			LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
			styleValue &= ~WS_CAPTION;
			::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
			RECT rcClient;
			::GetClientRect(*this, &rcClient);
			::SetWindowPos(*this, NULL, rcClient.left, rcClient.top, rcClient.right - rcClient.left, \
				rcClient.bottom - rcClient.top, SWP_FRAMECHANGED);

			m_pm.Init(m_hWnd);
			m_pm.GetDPIObj()->SetScale(m_pOwner->GetManager()->GetDPIObj()->GetDPI());
			// The trick is to add the items to the new container. Their owner gets
			// reassigned by this operation - which is why it is important to reassign
			// the items back to the righfull owner/manager when the window closes.
			m_pLayout = new CMenuUI();
			m_pm.SetForceUseSharedRes(true);
			m_pLayout->SetManager(&m_pm, NULL, true);
			LPCTSTR pDefaultAttributes = m_pOwner->GetManager()->GetDefaultAttributeList(_T("Menu"));
			if( pDefaultAttributes ) {
				m_pLayout->ApplyAttributeList(pDefaultAttributes);
			}
			// 继承一级菜单的 item-padding / padding，避免二级文字贴左边
			{
				CControlUI* pParentCtrl = m_pOwner->GetParent();
				CMenuUI* pParentMenu = pParentCtrl
					? static_cast<CMenuUI*>(pParentCtrl->GetInterface(_T("Menu")))
					: NULL;
				if( pParentMenu != NULL ) {
					RECT rcItemPad = pParentMenu->GetItemTextPadding();
					if( m_pOwner->GetManager() != NULL && m_pOwner->GetManager()->GetDPIObj() != NULL )
						m_pOwner->GetManager()->GetDPIObj()->ScaleBack(&rcItemPad);
					m_pLayout->SetItemTextPadding(rcItemPad);
					m_pLayout->SetPadding(pParentMenu->GetPadding());
				}
			}
			m_pLayout->GetList()->SetAutoDestroy(false);

			for( int i = 0; i < m_pOwner->GetCount(); i++ ) {
				if(m_pOwner->GetItemAt(i)->GetInterface(_T("MenuElement")) != NULL ){
					(static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)))->SetOwner(m_pLayout);
					m_pLayout->Add(static_cast<CControlUI*>(m_pOwner->GetItemAt(i)));
				}
			}

			CShadowUI *pShadow = m_pOwner->GetManager()->GetShadow();
			pShadow->CopyShadow(m_pm.GetShadow());
			bShowShadow = m_pm.GetShadow()->IsShowShadow();
			m_pm.GetShadow()->ShowShadow(false);
			m_pm.SetLayered(m_pOwner->GetManager()->IsLayered());
			m_pm.AttachDialog(m_pLayout);
			m_pm.AddNotifier(this);

			ResizeSubMenu();
		}
		else {
			m_pm.Init(m_hWnd);
			CPaintManagerUI* pMainPm = CMenuWnd::GetGlobalContextMenuObserver().GetManager();
			if( pMainPm != NULL && pMainPm->GetDPIObj() != NULL )
				m_pm.GetDPIObj()->SetScale(pMainPm->GetDPIObj()->GetDPI());
			CDialogBuilder builder;

			CControlUI* pRoot = builder.Create(m_xml, NULL, this, &m_pm);
			if( pRoot == NULL ) {
				bHandled = TRUE;
				return -1;
			}
			bShowShadow = m_pm.GetShadow()->IsShowShadow();
			m_pm.GetShadow()->ShowShadow(false);
			m_pm.AttachDialog(pRoot);
			m_pm.AddNotifier(this);

			ResizeMenu();
		}
		CMenuUI* pMenuUI = GetMenuUI();
		if( pMenuUI != NULL )
			pMenuUI->m_pWindow = this;
		m_pm.GetShadow()->ShowShadow(bShowShadow);
		m_pm.GetShadow()->Create(&m_pm);
		return 0;
	}

	CMenuUI* CMenuWnd::GetMenuUI()
	{
		CControlUI* pRoot = m_pm.GetRoot();
		if( pRoot == NULL ) return NULL;
		return static_cast<CMenuUI*>(pRoot->GetInterface(_T("Menu")));
	}

	void CMenuWnd::ResizeMenu()
	{
		CControlUI* pRoot = m_pm.GetRoot();
		if( pRoot == NULL ) return;

		CDuiPoint point = m_BasedPoint;
#if defined(WIN32) && !defined(UNDER_CE)
		MONITORINFO oMonitor = {};
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST), &oMonitor);
		CDuiRect rcWork = oMonitor.rcWork;
#else
		CDuiRect rcWork;
		GetWindowRect(m_pOwner->GetManager()->GetPaintWindow(), &rcWork);
#endif
		SIZE szAvailable = { rcWork.right - rcWork.left, rcWork.bottom - rcWork.top };
		szAvailable = pRoot->EstimateSize(szAvailable);
		m_pm.SetInitSize(szAvailable.cx, szAvailable.cy);

		//必须是Menu标签作为xml的根节点
		CMenuUI *pMenuRoot = static_cast<CMenuUI*>(pRoot->GetInterface(_T("Menu")));
		if( pMenuRoot == NULL ) return;

		SIZE szInit = m_pm.GetInitSize();
		CDuiRect rc;
		rc.left = point.x;
		rc.top = point.y;
		rc.right = rc.left + szInit.cx;
		rc.bottom = rc.top + szInit.cy;

		int nWidth = rc.GetWidth();
		int nHeight = rc.GetHeight();

		if (m_dwAlignment & eMenuAlignment_Right)
		{
			rc.right = point.x;
			rc.left = rc.right - nWidth;
		}

		if (m_dwAlignment & eMenuAlignment_Bottom)
		{
			rc.bottom = point.y;
			rc.top = rc.bottom - nHeight;
		}

		// 钳制到显示器工作区（托盘在顶/底/侧栏时避免菜单被挡住）
		if( rc.bottom > rcWork.bottom ) {
			rc.bottom = point.y;
			rc.top = rc.bottom - nHeight;
		}
		if( rc.top < rcWork.top ) {
			rc.top = rcWork.top;
			rc.bottom = rc.top + nHeight;
			if( rc.bottom > rcWork.bottom )
				rc.bottom = rcWork.bottom;
		}
		if( rc.right > rcWork.right ) {
			rc.right = point.x;
			rc.left = rc.right - nWidth;
		}
		if( rc.left < rcWork.left ) {
			rc.left = rcWork.left;
			rc.right = rc.left + nWidth;
			if( rc.right > rcWork.right )
				rc.right = rcWork.right;
		}

		SetForegroundWindow(m_hWnd);
		MoveWindow(m_hWnd, rc.left, rc.top, rc.GetWidth(), rc.GetHeight(), FALSE);
		SetWindowPos(m_hWnd, HWND_TOPMOST, rc.left, rc.top, rc.GetWidth(), rc.GetHeight() + pMenuRoot->GetPadding().bottom + pMenuRoot->GetPadding().top, SWP_SHOWWINDOW);

		CThemeManager* pTm = CThemeManager::GetInstance();
		if( pTm != NULL ) pTm->ApplyMenuChrome(pMenuRoot);
	}

	void CMenuWnd::ResizeSubMenu()
	{
		// Position the popup window in absolute space
		RECT rcOwner = m_pOwner->GetPos();
		RECT rc = rcOwner;

		int cxFixed = 0;
		int cyFixed = 0;

#if defined(WIN32) && !defined(UNDER_CE)
		MONITORINFO oMonitor = {}; 
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
		CDuiRect rcWork = oMonitor.rcWork;
#else
		CDuiRect rcWork;
		GetWindowRect(m_pOwner->GetManager()->GetPaintWindow(), &rcWork);
#endif
		SIZE szAvailable = { rcWork.right - rcWork.left, rcWork.bottom - rcWork.top };

		for( int it = 0; it < m_pOwner->GetCount(); it++ ) {
			if(m_pOwner->GetItemAt(it)->GetInterface(_T("MenuElement")) != NULL ){
				CControlUI* pControl = static_cast<CControlUI*>(m_pOwner->GetItemAt(it));
				SIZE sz = pControl->EstimateSize(szAvailable);
				cyFixed += sz.cy;
				if( cxFixed < sz.cx ) cxFixed = sz.cx;
			}
		}

		RECT rcWindow;
		GetWindowRect(m_pOwner->GetManager()->GetPaintWindow(), &rcWindow);

		rc.top = rcOwner.top;
		rc.bottom = rc.top + cyFixed;
		::MapWindowRect(m_pOwner->GetManager()->GetPaintWindow(), HWND_DESKTOP, &rc);
		rc.left = rcWindow.right;
		rc.right = rc.left + cxFixed;
		rc.right += 2;

		bool bReachBottom = false;
		bool bReachRight = false;

		RECT rcPreWindow = {0};
		MenuObserverImpl::Iterator iterator(CMenuWnd::GetGlobalContextMenuObserver());
		MenuMenuReceiverImplBase* pReceiver = iterator.next();
		while( pReceiver != NULL ) {
			CMenuWnd* pContextMenu = dynamic_cast<CMenuWnd*>(pReceiver);
			if( pContextMenu != NULL ) {
				GetWindowRect(pContextMenu->GetHWND(), &rcPreWindow);

				bReachRight = rcPreWindow.left >= rcWindow.right;
				bReachBottom = rcPreWindow.top >= rcWindow.bottom;
				if( pContextMenu->GetHWND() == m_pOwner->GetManager()->GetPaintWindow() ||  bReachBottom || bReachRight )
					break;
			}
			pReceiver = iterator.next();
		}

		if (bReachBottom)
		{
			rc.bottom = rcWindow.top;
			rc.top = rc.bottom - cyFixed;
		}

		if (bReachRight)
		{
			rc.right = rcWindow.left;
			rc.left = rc.right - cxFixed;
		}

		if( rc.bottom > rcWork.bottom )
		{
			rc.bottom = rc.top;
			rc.top = rc.bottom - cyFixed;
		}

		if (rc.right > rcWork.right)
		{
			rc.right = rcWindow.left;
			rc.left = rc.right - cxFixed;
		}

		if( rc.top < rcWork.top )
		{
			rc.top = rcOwner.top;
			rc.bottom = rc.top + cyFixed;
		}

		if (rc.left < rcWork.left)
		{
			rc.left = rcWindow.right;
			rc.right = rc.left + cxFixed;
		}

		MoveWindow(m_hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top + m_pLayout->GetPadding().top + m_pLayout->GetPadding().bottom, FALSE);

		CThemeManager* pTm = CThemeManager::GetInstance();
		if( pTm != NULL && m_pLayout != NULL ) pTm->ApplyMenuChrome(m_pLayout);
	}

	void CMenuWnd::setDPI(int DPI) {
		m_pm.SetDPI(DPI);
	}


	LRESULT CMenuWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		HWND hFocusWnd = (HWND)wParam;

		BOOL bInMenuWindowList = FALSE;
		ContextMenuParam param;
		param.hWnd = GetHWND();

		MenuObserverImpl::Iterator iterator(CMenuWnd::GetGlobalContextMenuObserver());
		MenuMenuReceiverImplBase* pReceiver = iterator.next();
		while( pReceiver != NULL ) {
			CMenuWnd* pContextMenu = dynamic_cast<CMenuWnd*>(pReceiver);
			if( pContextMenu != NULL && pContextMenu->GetHWND() ==  hFocusWnd ) {
				bInMenuWindowList = TRUE;
				break;
			}
			pReceiver = iterator.next();
		}

		if( !bInMenuWindowList ) {
			param.wParam = 1;
			CMenuWnd::GetGlobalContextMenuObserver().RBroadcast(param);
			return 0;
		}
		return 0;
	}
	LRESULT CMenuWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		SIZE szBorderRadius = m_pm.GetBorderRadius();
		if( !::IsIconic(*this) ) {
			CDuiRect rcWnd;
			::GetWindowRect(*this, &rcWnd);
			rcWnd.Offset(-rcWnd.left, -rcWnd.top);
			rcWnd.right++; rcWnd.bottom++;
			SIZE szEllipse = CssRadiusToEllipse(szBorderRadius);
			HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szEllipse.cx, szEllipse.cy);
			::SetWindowRgn(*this, hRgn, TRUE);
			::DeleteObject(hRgn);
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT CMenuWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		switch( uMsg )
		{
		case WM_CREATE:       
			lRes = OnCreate(uMsg, wParam, lParam, bHandled); 
			break;
		case WM_KILLFOCUS:       
			lRes = OnKillFocus(uMsg, wParam, lParam, bHandled); 
			break;
		case WM_KEYDOWN:
			if( wParam == VK_ESCAPE || wParam == VK_LEFT)
				Close();
			break;
		case WM_SIZE:
			lRes = OnSize(uMsg, wParam, lParam, bHandled);
			break;
		case WM_CLOSE:
			if( m_pOwner != NULL )
			{
				m_pOwner->SetManager(m_pOwner->GetManager(), m_pOwner->GetParent(), false);
				m_pOwner->SetPos(m_pOwner->GetPos());
				m_pOwner->SetFocus();
			}
			break;
		case WM_CONTEXTMENU:
		case WM_RBUTTONUP:
			if(m_bCaptured) {
				m_bCaptured = false;
				ReleaseCapture();
				if( m_pOwner != NULL )
				{
					m_pOwner->SetManager(m_pOwner->GetManager(), m_pOwner->GetParent(), false);
					m_pOwner->SetPos(m_pOwner->GetPos());
					m_pOwner->SetFocus();
				}
			}
			break;
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
			m_bCaptured = true;
			SetCapture(m_hWnd);
			return 0L;
		default:
			bHandled = FALSE;
			break;
		}

		if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	IMPLEMENT_DUICONTROL(CMenuElementUI)

	CMenuElementUI::CMenuElementUI():
	m_pWindow(NULL),
		m_bDrawLine(false),
		m_dwLineColor((DWORD)DEFAULT_LINE_COLOR),
		m_pIcon(NULL),
		m_dwIconTint(0),
		m_bIconTint(false),
		m_bIconTintAuto(false),
		m_hRasterTint(NULL),
		m_dwRasterTintColor(0),
		m_nRasterTintW(0),
		m_nRasterTintH(0),
		m_bCheckItem(false),
		m_bShowExplandIcon(false)
	{
		m_cxyFixed.cy = ITEM_DEFAULT_HEIGHT;
		m_cxyFixed.cx = ITEM_DEFAULT_WIDTH;
		m_szIconSize.cy = ITEM_DEFAULT_ICON_SIZE;
		m_szIconSize.cx = ITEM_DEFAULT_ICON_SIZE;

		m_rcLinePadding.top = m_rcLinePadding.bottom = 0;
		m_rcLinePadding.left = DEFAULT_LINE_LEFT_INSET;
		m_rcLinePadding.right = DEFAULT_LINE_RIGHT_INSET;
		// 可点菜单项默认手型；分隔线 SetLineType 会禁用，不出手型；皮肤可用 cursor 覆盖
		SetCursor(DUI_HAND);
	}

	CMenuElementUI::~CMenuElementUI()
	{
		ClearRasterTintCache();
		if( m_pIcon != NULL ) {
			delete m_pIcon;
			m_pIcon = NULL;
		}
	}

	LPCTSTR CMenuElementUI::GetClass() const
	{
		return _T("MenuElementUI");
	}

	LPVOID CMenuElementUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, _T("MenuElement")) == 0 ) return static_cast<CMenuElementUI*>(this);    
		return CListContainerElementUI::GetInterface(pstrName);
	}

	void CMenuElementUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
	{
		CListContainerElementUI::SetManager(pManager, pParent, bInit);
		if( m_pIcon != NULL )
			m_pIcon->SetManager(pManager, this, bInit);
	}

	bool CMenuElementUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		RECT rcTemp = { 0 };
		if( !::IntersectRect(&rcTemp, &rcPaint, &m_rcItem) ) return true;

		if(m_bDrawLine)
		{
			RECT rcLinePadding = GetLinePadding();
			if( GetManager() != NULL )
				rcLinePadding = GetManager()->GetDPIObj()->Scale(rcLinePadding);
			// 线宽跟随当前项实际宽度（菜单 EstimateSize 后各项已统一为最大宽）
			RECT rcLine = {
				m_rcItem.left + rcLinePadding.left,
				m_rcItem.top + (m_rcItem.bottom - m_rcItem.top) / 2,
				m_rcItem.right - rcLinePadding.right,
				m_rcItem.top + (m_rcItem.bottom - m_rcItem.top) / 2
			};
			if( rcLine.right > rcLine.left )
				ctx.DrawLine(rcLine, 1, GetAdjustColor(m_dwLineColor));
			DrawIconGutterLine(ctx, m_rcItem);
		}
		else
		{
			CRenderClipScope clip(ctx, rcTemp);
			CMenuElementUI::DrawItemBk(ctx, m_rcItem);
			DrawItemText(ctx, m_rcItem);
			DrawItemIcon(ctx, m_rcItem);
			DrawItemExpland(ctx, m_rcItem);
			DrawIconGutterLine(ctx, m_rcItem);

			if( m_items.GetSize() > 0 ) {
				RECT rc = m_rcItem;

				RECT rcPadding = GetPadding();
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
						if( !pControl->IsVisible() ) continue;
						if( pControl->GetInterface(_T("MenuElement")) != NULL ) continue;
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
						if( !pControl->IsVisible() ) continue;
						if( pControl->GetInterface(_T("MenuElement")) != NULL ) continue;
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
		}

		if( m_pVerticalScrollBar != NULL ) {
			if( m_pVerticalScrollBar == pStopControl ) return false;
			if (m_pVerticalScrollBar->IsVisible()) {
				if( ::IntersectRect(&rcTemp, &rcPaint, &m_pVerticalScrollBar->GetPos()) ) {
					if( !m_pVerticalScrollBar->Paint(ctx, rcPaint, pStopControl) ) return false;
				}
			}
		}

		if( m_pHorizontalScrollBar != NULL ) {
			if( m_pHorizontalScrollBar == pStopControl ) return false;
			if (m_pHorizontalScrollBar->IsVisible()) {
				if( ::IntersectRect(&rcTemp, &rcPaint, &m_pHorizontalScrollBar->GetPos()) ) {
					if( !m_pHorizontalScrollBar->Paint(ctx, rcPaint, pStopControl) ) return false;
				}
			}
		}
		return true;
	}

	void CMenuElementUI::DrawIconGutterLine(IRenderContext& ctx, const RECT& rcItem)
	{
		// 项挂在 ListBody 下，须向上找 Menu（勿只用 GetParent）
		CMenuUI* pMenu = NULL;
		for( CControlUI* p = GetParent(); p != NULL; p = p->GetParent() ) {
			pMenu = static_cast<CMenuUI*>(p->GetInterface(_T("Menu")));
			if( pMenu != NULL ) break;
		}
		if( pMenu == NULL || !pMenu->IsShowIconLine() ) return;
		if( m_pOwner == NULL ) return;

		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		if( pInfo == NULL ) return;

		RECT rcTextPadding = pInfo->rcTextPadding;
		if( GetManager() != NULL )
			rcTextPadding = GetManager()->GetDPIObj()->Scale(rcTextPadding);
		// 无图标槽时不画（左 padding 过窄）
		if( rcTextPadding.left < 12 ) return;

		// 默认跟主题 color-border（与横分隔线 / ApplyMenuChrome 一致），热切主题后仍可读
		DWORD clr = 0;
		CThemeManager* pTm = CThemeManager::GetInstance();
		if( pTm != NULL && pTm->IsEnabled() )
			clr = pTm->GetColor(_T("color-border"), 0);
		if( clr == 0 && pInfo->dwLineColor != 0 )
			clr = pInfo->dwLineColor;
		if( clr == 0 && m_dwLineColor != 0 )
			clr = m_dwLineColor;
		if( clr == 0 )
			clr = (DWORD)DEFAULT_LINE_COLOR;

		const int x = rcItem.left + rcTextPadding.left - 1;
		RECT rcLine = { x, rcItem.top, x + 1, rcItem.bottom };
		ctx.DrawColor(rcLine, GetAdjustColor(clr));
	}

	void CMenuElementUI::DrawItemIcon(IRenderContext& ctx, const RECT& rcItem)
	{
		if( m_bCheckItem && !GetChecked() ) return;
		const bool bSvg = (m_pIcon != NULL && m_pIcon->IsVisible());
		if( !bSvg && m_strIcon.IsEmpty() ) return;

		SIZE cxyFixed = GetFixedSize();
		SIZE szIconSize = GetIconSize();
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		RECT rcTextPadding = GetManager()->GetDPIObj()->Scale(pInfo->rcTextPadding);
		RECT rcDest =
		{
			(rcTextPadding.left - szIconSize.cx) / 2,
			(cxyFixed.cy - szIconSize.cy) / 2,
			(rcTextPadding.left - szIconSize.cx) / 2 + szIconSize.cx,
			(cxyFixed.cy - szIconSize.cy) / 2 + szIconSize.cy
		};

		RECT rcAbs = {
			m_rcItem.left + rcDest.left,
			m_rcItem.top + rcDest.top,
			m_rcItem.left + rcDest.right,
			m_rcItem.top + rcDest.bottom
		};

		if( bSvg ) {
			m_pIcon->SetEnabled(IsEnabled());
			m_pIcon->SetColor(ResolveMenuIconColor(), false);
			m_pIcon->SetHoverColor(0, false);
			m_pIcon->SetActiveColor(0, false);
			m_pIcon->SetDisabledColor(0, false);
			m_pIcon->SetPos(rcAbs, false);
			m_pIcon->PaintIcon(ctx, m_rcPaint);
			return;
		}

		if( ShouldTintRasterIcon() ) {
			PaintRasterIcon(ctx, rcAbs);
			return;
		}

		// dest 以逻辑坐标写入，由 TDrawInfo::Parse 再 Scale（与 DrawItemExpland 一致）
		GetManager()->GetDPIObj()->ScaleBack(&rcDest);
		CDuiString pStrImage;
		pStrImage.Format(_T("dest='%d,%d,%d,%d'"), rcDest.left, rcDest.top, rcDest.right, rcDest.bottom);
		DrawImage(ctx, m_strIcon.GetData(), pStrImage.GetData());
	}

	DWORD CMenuElementUI::ResolveMenuIconColor() const
	{
		if( m_bIconTint && m_dwIconTint != 0 )
			return m_dwIconTint;
		if( m_pOwner == NULL ) return 0x333333FF;
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		if( pInfo == NULL ) return 0x333333FF;
		DWORD clr = pInfo->dwColor;
		if( (m_uButtonState & UISTATE_HOT) != 0 && pInfo->dwHoverColor != 0 )
			clr = pInfo->dwHoverColor;
		if( IsSelected() && pInfo->dwSelectedColor != 0 )
			clr = pInfo->dwSelectedColor;
		if( !IsEnabled() && pInfo->dwDisabledColor != 0 )
			clr = pInfo->dwDisabledColor;
		return clr != 0 ? clr : 0x333333FF;
	}

	void CMenuElementUI::DrawItemExpland(IRenderContext& ctx, const RECT& rcItem)
	{
		if (m_bShowExplandIcon) {
			CDuiString strExplandIcon;
			strExplandIcon = GetManager()->GetDefaultAttributeList(_T("ExpandIcon"));
			if (strExplandIcon.IsEmpty()) {
				return;
			}
			SIZE cxyFixed = GetManager()->GetDPIObj()->Scale(m_cxyFixed);
			int padding = GetManager()->GetDPIObj()->Scale(ITEM_DEFAULT_EXPLAND_ICON_WIDTH) / 3;
			const TDrawInfo* pDrawInfo = GetManager()->GetDrawInfo(strExplandIcon.GetData(), NULL);
			const TImageInfo *pImageInfo = GetManager()->GetImageEx(pDrawInfo->sImageName.GetData(), NULL, 0, false, pDrawInfo->bGdiplus);
			if (!pImageInfo) {
				return;
			}
			RECT rcDest =
			{
				cxyFixed.cx - pImageInfo->nX - padding,
				(cxyFixed.cy - pImageInfo->nY) / 2,
				cxyFixed.cx - pImageInfo->nX - padding + pImageInfo->nX,
				(cxyFixed.cy - pImageInfo->nY) / 2 + pImageInfo->nY
			};
			GetManager()->GetDPIObj()->ScaleBack(&rcDest);
			CDuiString pStrImage;
			pStrImage.Format(_T("dest='%d,%d,%d,%d'"), rcDest.left, rcDest.top, rcDest.right, rcDest.bottom);
			DrawImage(ctx, strExplandIcon.GetData(), pStrImage.GetData());
		}
	}

	void CMenuElementUI::DrawItemText(IRenderContext& ctx, const RECT& rcItem)
	{
		CDuiString sText = GetText();
		if( sText.IsEmpty() ) return;

		if( m_pOwner == NULL ) return;
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		DWORD iTextColor = pInfo->dwColor;
		if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			iTextColor = pInfo->dwHoverColor;
		}
		if( IsSelected() ) {
			iTextColor = pInfo->dwSelectedColor;
		}
		if( !IsEnabled() ) {
			iTextColor = pInfo->dwDisabledColor;
		}
		int nLinks = 0;
		RECT rcText = rcItem;
		RECT rcTextPadding = GetManager()->GetDPIObj()->Scale(pInfo->rcTextPadding);
		rcText.left += rcTextPadding.left;
		rcText.right -= rcTextPadding.right;
		rcText.top += rcTextPadding.top;
		rcText.bottom -= rcTextPadding.bottom;

		if( pInfo->bShowHtml )
			ctx.DrawHtmlText(rcText, sText.GetData(), GetAdjustColor(iTextColor), \
			NULL, NULL, nLinks, pInfo->nFont, DT_SINGLELINE | pInfo->uTextStyle);
		else
			ctx.DrawText(rcText, sText.GetData(), GetAdjustColor(iTextColor), \
			pInfo->nFont, DT_SINGLELINE | pInfo->uTextStyle);
	}


	SIZE CMenuElementUI::EstimateSize(SIZE szAvailable)
	{
		SIZE cxyFixed = GetManager()->GetDPIObj()->Scale(m_cxyFixed);
		SIZE cXY = {0};

		// 分隔线无文字：勿用 szAvailable（常为工作区全宽）作量测初值，否则会撑满屏幕
		if( m_bDrawLine ) {
			cXY.cx = cxyFixed.cx;
			cXY.cy = cxyFixed.cy;
			return cXY;
		}

		for( int it = 0; it < GetCount(); it++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(GetItemAt(it));
			if( !pControl->IsVisible() ) continue;
			SIZE sz = pControl->EstimateSize(szAvailable);
			cXY.cy += sz.cy;
			if( cXY.cx < sz.cx ) cXY.cx = sz.cx;
		}
		if(cXY.cy == 0) {
			CDuiString sText = GetText();
			if( sText.IsEmpty() ) {
				cXY.cx = cxyFixed.cx;
				cXY.cy = cxyFixed.cy;
			}
			else if( m_pOwner != NULL ) {
				TListInfoUI* pInfo = m_pOwner->GetListInfo();
				DWORD iTextColor = pInfo->dwColor;
				// 用足够大的上限量真实文字宽，不把工作区宽度当初值
				RECT rcText = { 0, 0, 9999, 9999 };
				RECT rcTextPadding = GetManager()->GetDPIObj()->Scale(pInfo->rcTextPadding);
				rcText.left += rcTextPadding.left;
				rcText.right -= rcTextPadding.right;
				UINT uStyle = DT_CALCRECT | DT_SINGLELINE | DT_LEFT | DT_TOP;
				if( pInfo->bShowHtml ) {
					RenderMeasureHtmlText(m_pManager, rcText, sText.GetData(), iTextColor, pInfo->nFont, uStyle);
				}
				else {
					RenderMeasureText(m_pManager, rcText, sText.GetData(), iTextColor, pInfo->nFont, uStyle);
				}
				cXY.cx = rcText.right - rcText.left + rcTextPadding.left + rcTextPadding.right;
				cXY.cy = rcText.bottom - rcText.top + rcTextPadding.top + rcTextPadding.bottom;
			}
		}

		if( cxyFixed.cy != 0 ) cXY.cy = cxyFixed.cy;
		if( cXY.cx < cxyFixed.cx )
			cXY.cx = cxyFixed.cx;

		return cXY;
	}

	void CMenuElementUI::DoEvent(TEventUI& event)
	{
		if( event.Type == UIEVENT_MOUSEENTER )
		{
			CListContainerElementUI::DoEvent(event);
			if( m_pWindow ) return;
			bool hasSubMenu = false;
			for( int i = 0; i < GetCount(); ++i )
			{
				if( GetItemAt(i)->GetInterface(_T("MenuElement")) != NULL )
				{
					(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetVisible(true);
					(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetInternVisible(true);

					hasSubMenu = true;
				}
			}
			if( hasSubMenu )
			{
				m_pOwner->SelectItem(GetIndex(), true);
				CreateMenuWnd();
			}
			else
			{
				ContextMenuParam param;
				param.hWnd = m_pManager->GetPaintWindow();
				param.wParam = 2;
				CMenuWnd::GetGlobalContextMenuObserver().RBroadcast(param);
				m_pOwner->SelectItem(GetIndex(), true);
			}
			return;
		}


		if (event.Type == UIEVENT_MOUSELEAVE) {

			bool hasSubMenu = false;
			for (int i = 0; i < GetCount(); ++i)
			{
				if (GetItemAt(i)->GetInterface(_T("MenuElement")) != NULL)
				{
					
					hasSubMenu = true;
				}
			}

			if (!hasSubMenu) {
				m_pOwner->SelectItem(-1, true);
			}
		}

		if( event.Type == UIEVENT_BUTTONUP )
		{
			if( IsEnabled() ){
				CListContainerElementUI::DoEvent(event);

				if( m_pWindow ) return;

				bool hasSubMenu = false;
				for( int i = 0; i < GetCount(); ++i ) {
					if( GetItemAt(i)->GetInterface(_T("MenuElement")) != NULL ) {
						(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetVisible(true);
						(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetInternVisible(true);

						hasSubMenu = true;
					}
				}
				if( hasSubMenu )
				{
					CreateMenuWnd();
				}
				else
				{
					SetChecked(!GetChecked());

					bool isClosing = false;
					CMenuUI* menuUI=static_cast<CMenuUI*>(GetManager()->GetRoot());
					isClosing = (menuUI->m_pWindow->isClosing);
					if (IsWindow(GetManager()->GetPaintWindow()) && !isClosing) {
						if (CMenuWnd::GetGlobalContextMenuObserver().GetManager() != NULL) {
							MenuCmd* pMenuCmd = new MenuCmd();
							lstrcpy(pMenuCmd->szName, GetName().GetData());
							lstrcpy(pMenuCmd->szUserData, GetUserData().GetData());
							lstrcpy(pMenuCmd->szText, GetText().GetData());
							pMenuCmd->bChecked = GetChecked();
							if (!PostMessage(CMenuWnd::GetGlobalContextMenuObserver().GetManager()->GetPaintWindow(), WM_MENUCLICK, (WPARAM)pMenuCmd, (LPARAM)this)) {
								delete pMenuCmd;
								pMenuCmd = NULL;
							}
						}
					}
					ContextMenuParam param;
					param.hWnd = m_pManager->GetPaintWindow();
					param.wParam = 1;
					CMenuWnd::GetGlobalContextMenuObserver().RBroadcast(param);
				}
			}

			return;
		}

		if ( event.Type == UIEVENT_KEYDOWN && event.chKey == VK_RIGHT )
		{
			if( m_pWindow ) return;
			bool hasSubMenu = false;
			for( int i = 0; i < GetCount(); ++i ) {
				if( GetItemAt(i)->GetInterface(_T("MenuElement")) != NULL ) {
					(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetVisible(true);
					(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetInternVisible(true);
					hasSubMenu = true;
				}
			}
			if( hasSubMenu ) {
				m_pOwner->SelectItem(GetIndex(), true);
				CreateMenuWnd();
			}
			else
			{
				ContextMenuParam param;
				param.hWnd = m_pManager->GetPaintWindow();
				param.wParam = 2;
				CMenuWnd::GetGlobalContextMenuObserver().RBroadcast(param);
				m_pOwner->SelectItem(GetIndex(), true);
			}

			return;
		}

		CListContainerElementUI::DoEvent(event);
	}

	CMenuWnd* CMenuElementUI::GetMenuWnd()
	{
		return m_pWindow;
	}

	void CMenuElementUI::CreateMenuWnd()
	{
		if( m_pWindow ) return;

		m_pWindow = new CMenuWnd();
		ASSERT(m_pWindow);

		ContextMenuParam param;
		param.hWnd = m_pManager->GetPaintWindow();
		param.wParam = 2;
		CMenuWnd::GetGlobalContextMenuObserver().RBroadcast(param);

		m_pWindow->Init(static_cast<CMenuElementUI*>(this), _T(""), CDuiPoint(), NULL);
	}

	void CMenuElementUI::SetLineType()
	{
		m_bDrawLine = true;
		if (m_cxyFixed.cy == 0 || m_cxyFixed.cy == ITEM_DEFAULT_HEIGHT)
			SetFixedHeight(DEFAULT_LINE_HEIGHT);

		// 分隔线默认左右等距内边距，随菜单项铺满宽度（不再按图标列缩进）
		RECT rcPad = { 8, 0, 8, 0 };
		m_rcLinePadding = rcPad;

		SetMouseChildEnabled(false);
		SetMouseEnabled(false);
		SetEnabled(false);
	}

	bool CMenuElementUI::GetLineType() const
	{
		return m_bDrawLine;
	}

	void CMenuElementUI::SetLineColor(DWORD color)
	{
		m_dwLineColor = color;
	}

	DWORD CMenuElementUI::GetLineColor() const
	{
		return m_dwLineColor;
	}
	void CMenuElementUI::SetLinePadding(RECT rcMargin)
	{
		m_rcLinePadding = rcMargin;
	}

	RECT CMenuElementUI::GetLinePadding() const
	{
		RECT rcLinePadding = m_rcLinePadding;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&rcLinePadding);
		return rcLinePadding;
	}

	void CMenuElementUI::SetIcon(LPCTSTR strIcon)
	{
		ClearSvgIcon();
		ClearRasterTintCache();
		m_strIcon = strIcon ? strIcon : _T("");
		Invalidate();
	}

	void CMenuElementUI::EnsureSvgIcon()
	{
		if( m_pIcon != NULL ) return;
		m_pIcon = new CSvgBoxUI;
		m_pIcon->SetMouseEnabled(false);
		m_pIcon->SetVisible(false);
		if( m_pManager != NULL )
			m_pIcon->SetManager(m_pManager, this, false);
	}

	void CMenuElementUI::ClearSvgIcon()
	{
		if( m_pIcon != NULL ) {
			m_pIcon->SetVisible(false);
			m_pIcon->LoadFromUtf8Data("");
		}
	}

	bool CMenuElementUI::IsIconLibAttr(LPCTSTR pstrName) const
	{
		return _tcsicmp(pstrName, _T("bsicon")) == 0
			|| _tcsicmp(pstrName, _T("iconpark")) == 0
			|| _tcsicmp(pstrName, _T("lucide")) == 0
			|| _tcsicmp(pstrName, _T("tabler-outline")) == 0
			|| _tcsicmp(pstrName, _T("tabler-filled")) == 0
			|| _tcsicmp(pstrName, _T("remixicon")) == 0
			|| _tcsicmp(pstrName, _T("twicon")) == 0;
	}

	void CMenuElementUI::SetIconLib(LPCTSTR pstrLib, LPCTSTR pstrName)
	{
		if( pstrLib == NULL || *pstrLib == _T('\0')
			|| pstrName == NULL || *pstrName == _T('\0')
			|| !IsIconLibAttr(pstrLib) ) {
			ClearSvgIcon();
			return;
		}
		m_strIcon.Empty();
		EnsureSvgIcon();
		m_pIcon->SetAttribute(pstrLib, pstrName);
		m_pIcon->SetVisible(true);
		Invalidate();
	}

	void CMenuElementUI::SetIconTint(DWORD dwColor)
	{
		m_bIconTint = (dwColor != 0);
		m_dwIconTint = dwColor;
		if( m_bIconTint ) m_bIconTintAuto = false;
		ClearRasterTintCache();
		Invalidate();
	}

	void CMenuElementUI::SetIconTintAuto(bool bAuto)
	{
		const bool bClearExplicit = bAuto && m_bIconTint;
		if( m_bIconTintAuto == bAuto && !bClearExplicit ) return;
		m_bIconTintAuto = bAuto;
		if( bAuto ) {
			m_bIconTint = false;
			m_dwIconTint = 0;
		}
		ClearRasterTintCache();
		Invalidate();
	}

	void CMenuElementUI::SetIconSize(LONG cx, LONG cy)
	{
		m_szIconSize.cx = cx;
		m_szIconSize.cy = cy;
		ClearRasterTintCache();
		Invalidate();
	}

	void CMenuElementUI::ClearRasterTintCache()
	{
		if( m_hRasterTint != NULL ) {
			IRenderDevice* pDev = GetRenderDevice();
			if( pDev != NULL ) pDev->InvalidateBitmapGpu(m_hRasterTint);
			::DeleteObject(m_hRasterTint);
			m_hRasterTint = NULL;
		}
		m_dwRasterTintColor = 0;
		m_nRasterTintW = 0;
		m_nRasterTintH = 0;
	}

	bool CMenuElementUI::ShouldTintRasterIcon() const
	{
		if( m_strIcon.IsEmpty() ) return false;
		if( m_pIcon != NULL && m_pIcon->IsVisible() ) return false;
		return m_bIconTintAuto || m_bIconTint;
	}

	bool CMenuElementUI::EnsureRasterTintCache(DWORD dwColor)
	{
		if( m_pManager == NULL || m_strIcon.IsEmpty() || dwColor == 0 )
			return false;

		SIZE sz = GetIconSize();
		const int nSize = (sz.cx > 0) ? sz.cx : ITEM_DEFAULT_ICON_SIZE;
		if( nSize <= 0 ) return false;

		if( m_hRasterTint != NULL && m_dwRasterTintColor == dwColor
			&& m_nRasterTintW == nSize && m_nRasterTintH == nSize )
			return true;

		ClearRasterTintCache();

		CDuiString sName = m_strIcon;
		const int nFile = sName.Find(_T("file='"));
		if( nFile >= 0 ) {
			sName = sName.Mid(nFile + 6);
			const int nEnd = sName.Find(_T('\''));
			if( nEnd >= 0 ) sName = sName.Left(nEnd);
		}
		else {
			CDuiString sPath;
			if( ParseCssUrlImage(m_strIcon.GetData(), sPath) )
				sName = sPath;
		}

		const TImageInfo* pSrc = m_pManager->GetImageEx(sName.GetData());
		if( pSrc == NULL || pSrc->hBitmap == NULL || pSrc->nX <= 0 || pSrc->nY <= 0 )
			return false;

		BITMAP bm = { 0 };
		if( !::GetObject(pSrc->hBitmap, sizeof(bm), &bm) || bm.bmWidth <= 0 || bm.bmHeight <= 0 )
			return false;

		LPBYTE pSrcBits = NULL;
		BYTE* pTempBits = NULL;
		if( bm.bmBits != NULL ) {
			pSrcBits = (LPBYTE)bm.bmBits;
		}
		else if( pSrc->pBits != NULL ) {
			pSrcBits = pSrc->pBits;
		}
		else {
			pTempBits = new BYTE[pSrc->nX * pSrc->nY * 4];
			BITMAPINFO bmi = {};
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = pSrc->nX;
			bmi.bmiHeader.biHeight = -pSrc->nY;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;
			HDC hScreen = ::GetDC(NULL);
			int nCopied = ::GetDIBits(hScreen, pSrc->hBitmap, 0, pSrc->nY, pTempBits, &bmi, DIB_RGB_COLORS);
			::ReleaseDC(NULL, hScreen);
			if( nCopied == 0 ) {
				delete[] pTempBits;
				return false;
			}
			pSrcBits = pTempBits;
		}

		BITMAPINFO bmiOut = {};
		bmiOut.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmiOut.bmiHeader.biWidth = nSize;
		bmiOut.bmiHeader.biHeight = -nSize;
		bmiOut.bmiHeader.biPlanes = 1;
		bmiOut.bmiHeader.biBitCount = 32;
		bmiOut.bmiHeader.biCompression = BI_RGB;
		LPBYTE pDest = NULL;
		HBITMAP hTint = ::CreateDIBSection(NULL, &bmiOut, DIB_RGB_COLORS, (void**)&pDest, NULL, 0);
		if( hTint == NULL || pDest == NULL ) {
			delete[] pTempBits;
			return false;
		}

		const BYTE tR = DuiColorR(dwColor);
		const BYTE tG = DuiColorG(dwColor);
		const BYTE tB = DuiColorB(dwColor);
		const int srcW = pSrc->nX;
		const int srcH = pSrc->nY;

		for( int y = 0; y < nSize; ++y ) {
			const int sy = y * srcH / nSize;
			for( int x = 0; x < nSize; ++x ) {
				const int sx = x * srcW / nSize;
				const BYTE* pS = pSrcBits + (sy * srcW + sx) * 4;
				BYTE* pD = pDest + (y * nSize + x) * 4;
				BYTE a = pS[3];
				if( !pSrc->bAlpha ) {
					const int lum = (pS[2] * 30 + pS[1] * 59 + pS[0] * 11) / 100;
					a = (BYTE)(255 - lum);
				}
				pD[0] = (BYTE)((DWORD)tB * a / 255);
				pD[1] = (BYTE)((DWORD)tG * a / 255);
				pD[2] = (BYTE)((DWORD)tR * a / 255);
				pD[3] = a;
			}
		}

		delete[] pTempBits;
		m_hRasterTint = hTint;
		m_dwRasterTintColor = dwColor;
		m_nRasterTintW = nSize;
		m_nRasterTintH = nSize;
		return true;
	}

	void CMenuElementUI::PaintRasterIcon(IRenderContext& ctx, const RECT& rcIcon)
	{
		const DWORD paint = ResolveMenuIconColor();
		if( EnsureRasterTintCache(paint) && m_hRasterTint != NULL ) {
			RECT rcBmp = { 0, 0, m_nRasterTintW, m_nRasterTintH };
			RECT rcCorners = { 0, 0, 0, 0 };
			ctx.DrawImage(m_hRasterTint, rcIcon, m_rcPaint, rcBmp, rcCorners, true, ScaleImageFade());
			return;
		}
		RECT rcDest = {
			rcIcon.left - m_rcItem.left,
			rcIcon.top - m_rcItem.top,
			rcIcon.right - m_rcItem.left,
			rcIcon.bottom - m_rcItem.top
		};
		GetManager()->GetDPIObj()->ScaleBack(&rcDest);
		CDuiString pStrImage;
		pStrImage.Format(_T("dest='%d,%d,%d,%d'"), rcDest.left, rcDest.top, rcDest.right, rcDest.bottom);
		DrawImage(ctx, m_strIcon.GetData(), pStrImage.GetData());
	}

	SIZE CMenuElementUI::GetIconSize()
	{
		SIZE szIconSize = m_szIconSize;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&szIconSize);
		return szIconSize;
	}


	void CMenuElementUI::SetChecked(bool bCheck/* = true*/)
	{
		SetItemInfo(GetName().GetData(), bCheck);
	}

	bool CMenuElementUI::GetChecked() const
	{
		CDuiString sName = GetName();
		LPCTSTR pstrName = sName.GetData();
		if(pstrName == NULL || lstrlen(pstrName) <= 0) return false;

		CStdStringPtrMap* mCheckInfos = CMenuWnd::GetGlobalContextMenuObserver().GetMenuCheckInfo();
		if (mCheckInfos != NULL)
		{
			MenuItemInfo* pItemInfo = (MenuItemInfo*)mCheckInfos->Find(pstrName);
			if(pItemInfo != NULL) {
				return pItemInfo->bChecked;
			}
		}
		return false;

	}

	void CMenuElementUI::SetCheckItem(bool bCheckItem/* = false*/)
	{
		m_bCheckItem = bCheckItem;
	}

	bool CMenuElementUI::GetCheckItem() const
	{
		return m_bCheckItem;
	}

	void CMenuElementUI::SetShowExplandIcon(bool bShow)
	{
		m_bShowExplandIcon = bShow;
	}

	void CMenuElementUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("icon")) == 0 || _tcsicmp(pstrName, _T("icon-src")) == 0 ){
			SetIcon(pstrValue);
		}
		else if( IsIconLibAttr(pstrName) ) {
			if( pstrValue == NULL || *pstrValue == _T('\0') )
				ClearSvgIcon();
			else
				SetIconLib(pstrName, pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("icon-tint")) == 0
			|| _tcsicmp(pstrName, _T("icon-color")) == 0 ) {
			if( pstrValue == NULL || *pstrValue == _T('\0')
				|| _tcsicmp(pstrValue, _T("none")) == 0
				|| _tcsicmp(pstrValue, _T("false")) == 0
				|| _tcsicmp(pstrValue, _T("original")) == 0 ) {
				SetIconTintAuto(false);
				SetIconTint(0);
			}
			else if( _tcsicmp(pstrValue, _T("auto")) == 0
				|| _tcsicmp(pstrValue, _T("true")) == 0 ) {
				SetIconTintAuto(true);
			}
			else {
				DWORD clr = 0;
				if( ParseColorString(pstrValue, clr) ) SetIconTint(clr);
			}
		}
		else if( _tcsicmp(pstrName, _T("icon-size")) == 0 ) {
			LPTSTR pstr = NULL;
			LONG cx = _tcstol(pstrValue, &pstr, 10);
			LONG cy = cx;
			if( pstr != NULL && (*pstr == _T(',') || *pstr == _T(' ') || *pstr == _T('x') || *pstr == _T('X')) ) {
				++pstr;
				while( *pstr == _T(' ') ) ++pstr;
				cy = _tcstol(pstr, &pstr, 10);
			}
			if( cx <= 0 ) cx = ITEM_DEFAULT_ICON_SIZE;
			if( cy <= 0 ) cy = cx;
			SetIconSize(cx, cy);
		}
		else if( _tcsicmp(pstrName, _T("check-item")) == 0 ) {		
			SetCheckItem(_tcsicmp(pstrValue, _T("true")) == 0 ? true : false);		
		}
		else if( _tcsicmp(pstrName, _T("checked")) == 0 ) {		
			CStdStringPtrMap* mCheckInfos = CMenuWnd::GetGlobalContextMenuObserver().GetMenuCheckInfo();
			if (mCheckInfos != NULL)
			{
				bool bFind = false;
				for(int i = 0; i < mCheckInfos->GetSize(); i++) {
					MenuItemInfo* itemInfo = (MenuItemInfo*)mCheckInfos->GetAt(i);
					if(lstrcmpi(itemInfo->szName, GetName().GetData()) == 0) {
						bFind = true;
						break;
					}
				}
				if(!bFind) SetChecked(_tcsicmp(pstrValue, _T("true")) == 0 ? true : false);
			}
		}	
		else if( _tcsicmp(pstrName, _T("line-type")) == 0){
			if (_tcsicmp(pstrValue, _T("true")) == 0)
				SetLineType();
		}
		else if( _tcsicmp(pstrName, _T("expand")) == 0 ) {
			SetShowExplandIcon(_tcsicmp(pstrValue, _T("true")) == 0 ? true : false);
		}
		else if( _tcsicmp(pstrName, _T("line-color")) == 0){
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) )
				SetLineColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("line-padding")) == 0 ) {
			RECT rcPadding = { 0 };
			if( ParseCssBoxToRect(pstrValue, rcPadding) )
				SetLinePadding(rcPadding);
		}
		else if	( _tcsicmp(pstrName, _T("height")) == 0){
			SetFixedHeight(_ttoi(pstrValue));
		}
		else
			CListContainerElementUI::SetAttribute(pstrName, pstrValue);
	}


	MenuItemInfo* CMenuElementUI::GetItemInfo(LPCTSTR pstrName)
	{
		if(pstrName == NULL || lstrlen(pstrName) <= 0) return NULL;

		CStdStringPtrMap* mCheckInfos = CMenuWnd::GetGlobalContextMenuObserver().GetMenuCheckInfo();
		if (mCheckInfos != NULL)
		{
			MenuItemInfo* pItemInfo = (MenuItemInfo*)mCheckInfos->Find(pstrName);
			if(pItemInfo != NULL) {
				return pItemInfo;
			}
		}

		return NULL;
	}

	MenuItemInfo* CMenuElementUI::SetItemInfo(LPCTSTR pstrName, bool bChecked)
	{
		if(pstrName == NULL || lstrlen(pstrName) <= 0) return NULL;

		CStdStringPtrMap* mCheckInfos = CMenuWnd::GetGlobalContextMenuObserver().GetMenuCheckInfo();
		if (mCheckInfos != NULL)
		{
			MenuItemInfo* pItemInfo = (MenuItemInfo*)mCheckInfos->Find(pstrName);
			if(pItemInfo == NULL) {
				pItemInfo = new MenuItemInfo;
				lstrcpy(pItemInfo->szName, pstrName);
				pItemInfo->bChecked = bChecked;
				mCheckInfos->Insert(pstrName, pItemInfo);
			}
			else {
				pItemInfo->bChecked = bChecked;
			}

			return pItemInfo;
		}
		return NULL;
	}
} // namespace DuiLib
