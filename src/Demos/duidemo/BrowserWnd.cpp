#include "stdafx.h"
#include "BrowserWnd.h"

DUI_BEGIN_MESSAGE_MAP(CBrowserWnd, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, CBrowserWnd::OnClick)
DUI_END_MESSAGE_MAP()

CBrowserWnd::CBrowserWnd()
	: m_pTabBar(NULL)
	, m_pPages(NULL)
	, m_nNextTabId(1)
{
}

CBrowserWnd::~CBrowserWnd()
{
}

void CBrowserWnd::Open(HWND hParent)
{
	CBrowserWnd* pWnd = new CBrowserWnd();
	pWnd->Create(hParent, _T("Browser Shell"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE, 0, 0, 1280, 800);
	pWnd->CenterWindow();
	pWnd->ShowWindow(true);
}

void CBrowserWnd::OnFinalMessage(HWND hWnd)
{
	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
}

CDuiString CBrowserWnd::GetSkinFile()
{
	return _T("browser.html");
}

LPCTSTR CBrowserWnd::GetWindowClassName() const
{
	return _T("BrowserWnd");
}

void CBrowserWnd::InitWindow()
{
	m_pTabBar = static_cast<CTabBarUI*>(m_pm.FindControl(_T("tabbar")));
	m_pPages = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T("pages")));
	if( m_pTabBar != NULL && m_pTabBar->GetTabCount() == 0 )
		AddNewTab(_T("新标签页"));
}

void CBrowserWnd::AddNewTab(LPCTSTR pstrTitle)
{
	if( m_pTabBar == NULL || m_pPages == NULL ) return;

	CDuiString sTitle;
	if( pstrTitle != NULL && *pstrTitle != _T('\0') )
		sTitle = pstrTitle;
	else
		sTitle.Format(_T("新标签 %d"), m_nNextTabId);
	++m_nNextTabId;

	CTabButtonUI* pTab = m_pTabBar->AddTab(sTitle);
	if( pTab == NULL ) return;
	pTab->SetAttribute(_T("tabler-outline"), _T("world"));

	CLabelUI* pLabel = new CLabelUI();
	pLabel->SetText(sTitle);
	pLabel->SetAttribute(_T("text-align"), _T("center"));
	pLabel->SetAttribute(_T("vertical-align"), _T("vcenter"));
	pLabel->SetAttribute(_T("color"), _T("#FFC8C8C8"));
	pLabel->SetAttribute(_T("font-size"), _T("14"));
	m_pPages->Add(pLabel);

	m_pTabBar->SetActiveTab(m_pTabBar->GetTabCount() - 1);
}

void CBrowserWnd::Notify(TNotifyUI& msg)
{
	if( msg.pSender == m_pTabBar && msg.sType == DUI_MSGTYPE_TABADD )
		AddNewTab();
	WindowImplBase::Notify(msg);
}

void CBrowserWnd::OnClick(TNotifyUI& msg)
{
	WindowImplBase::OnClick(msg);
}
