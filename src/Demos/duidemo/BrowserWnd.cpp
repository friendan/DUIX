#include "stdafx.h"
#include "BrowserWnd.h"

DUI_BEGIN_MESSAGE_MAP(CBrowserWnd, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, CBrowserWnd::OnClick)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_RETURN, CBrowserWnd::OnClick)
DUI_END_MESSAGE_MAP()

CBrowserWnd::CBrowserWnd()
	: m_pTabBar(NULL)
	, m_pPages(NULL)
	, m_pUrlBox(NULL)
	, m_pActiveBrowser(NULL)
	, m_nNextTabId(1)
	, m_hostEvents(this)
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
	{
		CControlUI* p = m_pm.FindControl(_T("tabbar"));
		m_pTabBar = p ? static_cast<CTabBarUI*>(p->GetInterface(DUI_CTR_TABBAR)) : NULL;
	}
	{
		CControlUI* p = m_pm.FindControl(_T("pages"));
		m_pPages = p ? static_cast<CTabLayoutUI*>(p->GetInterface(DUI_CTR_TABLAYOUT)) : NULL;
	}
	{
		CControlUI* p = m_pm.FindControl(_T("urlBox"));
		m_pUrlBox = p ? static_cast<CEditUI*>(p->GetInterface(DUI_CTR_EDIT)) : NULL;
	}
	if( m_pUrlBox ) {
		m_pUrlBox->SetMaxChar(2048);
		m_pUrlBox->OnNotify += MakeDelegate(this, &CBrowserWnd::OnUrlBoxNotify);
	}
	if( m_pTabBar != NULL && m_pTabBar->GetTabCount() == 0 )
		AddNewTab(_T("新标签页"), _T("https://project.feishu.cn"));
}

CWebBrowserUI* CBrowserWnd::GetActiveBrowser() const
{
	if( m_pActiveBrowser != NULL )
		return m_pActiveBrowser;
	if( m_pPages == NULL ) return NULL;
	int idx = m_pPages->GetCurSel();
	if( idx < 0 && m_pTabBar != NULL )
		idx = m_pTabBar->GetActiveTab();
	if( idx < 0 && m_pPages->GetCount() > 0 )
		idx = 0;
	CControlUI* p = m_pPages->GetItemAt(idx);
	if( p == NULL ) return NULL;
	return static_cast<CWebBrowserUI*>(p->GetInterface(DUI_CTR_WEBBROWSER));
}

void CBrowserWnd::AddNewTab(LPCTSTR pstrTitle, LPCTSTR pstrUrl)
{
	if( m_pTabBar == NULL || m_pPages == NULL ) return;

	CDuiString sTitle;
	if( pstrTitle != NULL && *pstrTitle != _T('\0') )
		sTitle = pstrTitle;
	else
		sTitle.Format(_T("新标签 %d"), m_nNextTabId);
	++m_nNextTabId;

	CWebBrowserUI* pBrowser = new CWebBrowserUI();
	pBrowser->SetEngine(_T("webview2"));
	pBrowser->SetEngineFallback(true);
	// window 宿主更稳
	pBrowser->SetHostMode(_T("window"));
	pBrowser->SetAutoNavigation(false);
	pBrowser->SetHostEvents(&m_hostEvents);
	if( pstrUrl != NULL && *pstrUrl != _T('\0') )
		pBrowser->SetHomePage(pstrUrl);
	else
		pBrowser->SetHomePage(_T("https://project.feishu.cn"));
	pBrowser->SetAutoNavigation(true);

	m_pPages->Add(pBrowser);

	CTabButtonUI* pTab = m_pTabBar->AddTab(sTitle);
	if( pTab == NULL ) return;
	pTab->SetAttribute(_T("tabler-outline"), _T("world"));

	int iNew = m_pTabBar->GetTabCount() - 1;
	if( m_pTabBar->GetActiveTab() != iNew )
		m_pTabBar->SetActiveTab(iNew);
	m_pPages->SelectItem(iNew);
	m_pActiveBrowser = pBrowser;
	if( m_pUrlBox ) m_pUrlBox->SetText(pBrowser->GetHomePage());
}

void CBrowserWnd::NavigateAddressBar()
{
	CWebBrowserUI* pBrowser = GetActiveBrowser();
	if( pBrowser == NULL || m_pUrlBox == NULL ) return;

	if( m_pPages != NULL )
		m_pPages->SelectItem(pBrowser);

	CDuiString url;
	HWND hEdit = m_pUrlBox->GetHWND();
	if( hEdit != NULL ) {
		int cchLen = ::GetWindowTextLength(hEdit) + 1;
		LPTSTR pstr = static_cast<LPTSTR>(_alloca(cchLen * sizeof(TCHAR)));
		if( pstr ) {
			::GetWindowText(hEdit, pstr, cchLen);
			url = pstr;
		}
	}
	if( url.IsEmpty() )
		url = m_pUrlBox->GetText();
	url.Trim();
	if( url.IsEmpty() ) return;

	if( url.Find(_T("://")) < 0
		&& url.Find(_T("about:")) != 0
		&& url.Find(_T("data:")) != 0
		&& url.Find(_T("file:")) != 0 )
	{
		CDuiString full;
		full.Format(_T("https://%s"), (LPCTSTR)url);
		url = full;
	}

	pBrowser->NavigateUrl(url);
	if( m_pUrlBox->GetText() != url )
		m_pUrlBox->SetText(url);
}

void CBrowserWnd::HandleNavCommand(LPCTSTR pstrName)
{
	if( pstrName == NULL || *pstrName == _T('\0') ) return;
	CWebBrowserUI* pBrowser = GetActiveBrowser();
	if( _tcscmp(pstrName, _T("goBtn")) == 0 ) {
		NavigateAddressBar();
		return;
	}
	if( pBrowser == NULL ) return;
	if( _tcscmp(pstrName, _T("backBtn")) == 0 )
		pBrowser->GoBack();
	else if( _tcscmp(pstrName, _T("forwardBtn")) == 0 )
		pBrowser->GoForward();
	else if( _tcscmp(pstrName, _T("refreshBtn")) == 0 )
		pBrowser->Refresh();
	else if( _tcscmp(pstrName, _T("homeBtn")) == 0 || _tcscmp(pstrName, _T("sideToolHome")) == 0 )
		pBrowser->NavigateHomePage();
}

bool CBrowserWnd::OnUrlBoxNotify(void* param)
{
	TNotifyUI* pMsg = reinterpret_cast<TNotifyUI*>(param);
	if( pMsg == NULL ) return false;
	if( pMsg->sType == DUI_MSGTYPE_RETURN ) {
		NavigateAddressBar();
		return true;
	}
	return false;
}

void CBrowserWnd::HostEvents::OnDocumentTitleChanged(CWebBrowserUI* pWeb, LPCTSTR title)
{
	if( m_pOwner ) m_pOwner->OnBrowserTitleChanged(pWeb, title);
}

void CBrowserWnd::HostEvents::OnNavigationCompleted(CWebBrowserUI* pWeb, LPCTSTR url, bool success)
{
	if( m_pOwner ) m_pOwner->OnBrowserNavigated(pWeb, url, success);
}

void CBrowserWnd::OnBrowserTitleChanged(CWebBrowserUI* pWeb, LPCTSTR title)
{
	if( pWeb == NULL || m_pTabBar == NULL || m_pPages == NULL ) return;
	int idx = -1;
	for( int i = 0; i < m_pPages->GetCount(); ++i ) {
		if( m_pPages->GetItemAt(i) == pWeb ) { idx = i; break; }
	}
	if( idx < 0 ) return;
	CTabButtonUI* pTab = m_pTabBar->GetTab(idx);
	if( pTab && title ) pTab->SetText(title);
}

void CBrowserWnd::OnBrowserNavigated(CWebBrowserUI* pWeb, LPCTSTR url, bool /*success*/)
{
	if( pWeb == NULL || m_pUrlBox == NULL ) return;
	if( GetActiveBrowser() != pWeb ) return;
	if( url ) m_pUrlBox->SetText(url);
}

void CBrowserWnd::Notify(TNotifyUI& msg)
{
	if( msg.pSender == m_pTabBar ) {
		if( msg.sType == DUI_MSGTYPE_TABADD )
			AddNewTab();
		else if( msg.sType == DUI_MSGTYPE_TABSELECT ) {
			int idx = (int)msg.wParam;
			if( m_pPages && idx >= 0 ) {
				CControlUI* p = m_pPages->GetItemAt(idx);
				m_pActiveBrowser = p
					? static_cast<CWebBrowserUI*>(p->GetInterface(DUI_CTR_WEBBROWSER))
					: NULL;
				m_pPages->SelectItem(idx);
			}
		}
	}

	if( msg.sType == DUI_MSGTYPE_RETURN && msg.pSender != NULL ) {
		if( msg.pSender == m_pUrlBox || msg.pSender->GetName() == _T("urlBox") )
			NavigateAddressBar();
	}
	else if( msg.sType == DUI_MSGTYPE_CLICK && msg.pSender != NULL ) {
		HandleNavCommand(msg.pSender->GetName());
	}

	WindowImplBase::Notify(msg);
}

void CBrowserWnd::OnClick(TNotifyUI& msg)
{
	if( msg.pSender == NULL ) {
		WindowImplBase::OnClick(msg);
		return;
	}
	if( msg.sType == DUI_MSGTYPE_RETURN ) {
		if( msg.pSender == m_pUrlBox || msg.pSender->GetName() == _T("urlBox") )
			NavigateAddressBar();
		return;
	}
	CDuiString name = msg.pSender->GetName();
	HandleNavCommand(name);
	if( name == _T("goBtn") || name == _T("backBtn") || name == _T("forwardBtn")
		|| name == _T("refreshBtn") || name == _T("homeBtn") || name == _T("sideToolHome") )
		return;
	WindowImplBase::OnClick(msg);
}
