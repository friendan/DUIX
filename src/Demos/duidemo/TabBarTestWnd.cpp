#include "stdafx.h"
#include "TabBarTestWnd.h"

DUI_BEGIN_MESSAGE_MAP(CTabBarTestWnd, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, CTabBarTestWnd::OnClick)
DUI_END_MESSAGE_MAP()

CTabBarTestWnd::CTabBarTestWnd()
	: m_pTabBar(NULL)
	, m_pPages(NULL)
	, m_pStatus(NULL)
	, m_nNextTabId(1)
{
}

CTabBarTestWnd::~CTabBarTestWnd()
{
}

void CTabBarTestWnd::Open(HWND hParent)
{
	CTabBarTestWnd* pWnd = new CTabBarTestWnd();
	pWnd->Create(hParent, _T("TabBar Test"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE, 0, 0, 900, 560);
	pWnd->CenterWindow();
	pWnd->ShowWindow(true);
}

void CTabBarTestWnd::OnFinalMessage(HWND hWnd)
{
	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
}

CDuiString CTabBarTestWnd::GetSkinFile()
{
	return _T("tabbartest.html");
}

LPCTSTR CTabBarTestWnd::GetWindowClassName() const
{
	return _T("TabBarTestWnd");
}

void CTabBarTestWnd::InitWindow()
{
	m_pTabBar = static_cast<CTabBarUI*>(m_pm.FindControl(_T("tabbar")));
	m_pPages = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("pages")));
	m_pStatus = static_cast<CLabelUI*>(m_pm.FindControl(_T("status")));
	if( m_pTabBar != NULL )
		m_nNextTabId = m_pTabBar->GetTabCount() + 1;
	ApplyWidthModeFromCheck();
	SetStatus(_T("就绪：锁定钉在左侧；双击关闭；右键菜单；拖拽排序"));
}

void CTabBarTestWnd::ApplyWidthModeFromCheck()
{
	if( m_pTabBar == NULL ) return;
	bool bFlex = IsCheckSelected(_T("chk_flexible"));
	m_pTabBar->SetFlexibleTabWidth(bFlex);
	if( !bFlex )
		m_pTabBar->SetTabWidth(140);
	else {
		m_pTabBar->SetTabMinWidth(100);
		m_pTabBar->SetTabMaxWidth(300);
	}
	SetStatus(bFlex ? _T("宽度模式：弹性（min=100, max=300）") : _T("宽度模式：固定 140"));
}

bool CTabBarTestWnd::IsCheckSelected(LPCTSTR pstrName) const
{
	CCheckBoxUI* pCheck = static_cast<CCheckBoxUI*>(m_pm.FindControl(pstrName));
	return pCheck != NULL && pCheck->IsSelected();
}

CLabelUI* CTabBarTestWnd::CreatePageLabel(LPCTSTR pstrText)
{
	CLabelUI* pLabel = new CLabelUI();
	pLabel->SetText(pstrText);
	pLabel->SetAttribute(_T("text-align"), _T("center"));
	pLabel->SetAttribute(_T("vertical-align"), _T("vcenter"));
	pLabel->SetAttribute(_T("color"), _T("#FF333333"));
	pLabel->SetAttribute(_T("font-size"), _T("13"));
	return pLabel;
}

void CTabBarTestWnd::SetStatus(LPCTSTR pstrText)
{
	if( m_pStatus != NULL )
		m_pStatus->SetText(pstrText ? pstrText : _T(""));
}

void CTabBarTestWnd::AddNewTab()
{
	if( m_pTabBar == NULL || m_pPages == NULL ) return;

	CDuiString sTitle;
	sTitle.Format(_T("新标签 %d"), m_nNextTabId++);
	CTabButtonUI* pTab = m_pTabBar->AddTab(sTitle);
	if( pTab == NULL ) return;
	pTab->SetAttribute(_T("tabler-outline"), _T("file-text"));

	CDuiString sPage;
	sPage.Format(_T("这是「%s」的页面内容"), sTitle.GetData());
	m_pPages->Add(CreatePageLabel(sPage));

	int iNew = m_pTabBar->GetTabCount() - 1;
	m_pTabBar->SetActiveTab(iNew);

	CDuiString sStatus;
	sStatus.Format(_T("已添加标签 [%d] %s，当前共 %d 个"), iNew, sTitle.GetData(), m_pTabBar->GetTabCount());
	SetStatus(sStatus);
}

void CTabBarTestWnd::RemoveActiveTab()
{
	if( m_pTabBar == NULL ) return;
	int iActive = m_pTabBar->GetActiveTab();
	if( iActive < 0 ) {
		SetStatus(_T("没有选中的标签"));
		return;
	}
	CTabButtonUI* pTab = m_pTabBar->GetTab(iActive);
	if( pTab != NULL && pTab->IsLocked() ) {
		SetStatus(_T("当前标签已锁定，无法关闭"));
		return;
	}
	m_pTabBar->RemoveTab(iActive);
}

void CTabBarTestWnd::ToggleLockActive()
{
	if( m_pTabBar == NULL ) return;
	CTabButtonUI* pTab = m_pTabBar->GetActiveTabButton();
	if( pTab == NULL ) {
		SetStatus(_T("没有选中的标签"));
		return;
	}
	bool bLock = !pTab->IsLocked();
	pTab->SetLocked(bLock);
	CDuiString sStatus;
	sStatus.Format(_T("标签 [%d]「%s」%s"), m_pTabBar->GetActiveTab(),
		pTab->GetTabTitle().GetData(), bLock ? _T("已锁定") : _T("已解锁"));
	SetStatus(sStatus);
}

void CTabBarTestWnd::Notify(TNotifyUI& msg)
{
	if( msg.sType == DUI_MSGTYPE_SELECTCHANGED && msg.pSender != NULL
		&& msg.pSender->GetName() == _T("chk_flexible") ) {
		ApplyWidthModeFromCheck();
		return;
	}
	if( msg.pSender == m_pTabBar ) {
		if( msg.sType == DUI_MSGTYPE_TABCLOSING ) {
			if( IsCheckSelected(_T("chk_block_close")) ) {
				m_pTabBar->CancelNotify();
				CDuiString s;
				s.Format(_T("已拦截关闭索引 %d"), (int)msg.wParam);
				SetStatus(s);
			}
		}
		else if( msg.sType == DUI_MSGTYPE_TABSELECTING ) {
			if( IsCheckSelected(_T("chk_block_select")) ) {
				m_pTabBar->CancelNotify();
				CDuiString s;
				s.Format(_T("已拦截切换 %d→%d"), (int)msg.lParam, (int)msg.wParam);
				SetStatus(s);
			}
		}
		else if( msg.sType == DUI_MSGTYPE_TABSELECT ) {
			CDuiString s;
			s.Format(_T("tabselect: 新=%d 旧=%d，共 %d 标签"),
				(int)msg.wParam, (int)msg.lParam,
				m_pTabBar != NULL ? m_pTabBar->GetTabCount() : 0);
			SetStatus(s);
		}
		else if( msg.sType == DUI_MSGTYPE_TABCLOSE ) {
			CDuiString s;
			s.Format(_T("tabclose: 已关闭索引 %d（页已自动同步），剩余 %d 标签"),
				(int)msg.wParam, m_pTabBar != NULL ? m_pTabBar->GetTabCount() : 0);
			SetStatus(s);
		}
		else if( msg.sType == DUI_MSGTYPE_TABADD ) {
			AddNewTab();
			SetStatus(_T("tabadd: 已通过「+」添加标签"));
		}
		else if( msg.sType == DUI_MSGTYPE_TABMOVE ) {
			CDuiString s;
			s.Format(_T("tabmove: %d→%d（页已同步），当前选中 %d"),
				(int)msg.wParam, (int)msg.lParam,
				m_pTabBar != NULL ? m_pTabBar->GetActiveTab() : -1);
			SetStatus(s);
		}
	}
	WindowImplBase::Notify(msg);
}

void CTabBarTestWnd::OnClick(TNotifyUI& msg)
{
	CDuiString sName = msg.pSender->GetName();
	if( sName == _T("closebtn") ) {
		Close();
		return;
	}
	if( sName == _T("btn_close_active") ) {
		RemoveActiveTab();
		return;
	}
	if( sName == _T("btn_toggle_lock") ) {
		ToggleLockActive();
		return;
	}
	if( sName == _T("btn_remove_unlocked") ) {
		if( m_pTabBar == NULL || m_pPages == NULL ) return;
		m_pTabBar->RemoveUnlockedTabs();
		SetStatus(_T("已移除所有未锁定标签"));
		return;
	}
	WindowImplBase::OnClick(msg);
}
