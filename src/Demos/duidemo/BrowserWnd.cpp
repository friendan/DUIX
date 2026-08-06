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
	, m_nNextDelayTimerId(0xB1800001)
	, m_hostEvents(this)
{
}

CBrowserWnd::~CBrowserWnd()
{
	CancelDelayedNavigates();
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
	CancelDelayedNavigates();
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
	UpdateNavButtons();
}

bool CBrowserWnd::IsBrowserAlive(CWebBrowserUI* pWeb) const
{
	if( pWeb == NULL || m_pPages == NULL ) return false;
	for( int i = 0; i < m_pPages->GetCount(); ++i ) {
		if( m_pPages->GetItemAt(i) == pWeb ) return true;
	}
	return false;
}

CWebBrowserUI* CBrowserWnd::GetActiveBrowser()
{
	if( m_pActiveBrowser != NULL && !IsBrowserAlive(m_pActiveBrowser) )
		m_pActiveBrowser = NULL;
	if( m_pActiveBrowser != NULL )
		return m_pActiveBrowser;
	if( m_pPages == NULL || m_pPages->GetCount() <= 0 ) return NULL;
	int idx = m_pPages->GetCurSel();
	if( idx < 0 && m_pTabBar != NULL )
		idx = m_pTabBar->GetActiveTab();
	if( idx < 0 )
		idx = 0;
	CControlUI* p = m_pPages->GetItemAt(idx);
	if( p == NULL ) return NULL;
	m_pActiveBrowser = static_cast<CWebBrowserUI*>(p->GetInterface(DUI_CTR_WEBBROWSER));
	return m_pActiveBrowser;
}

void CBrowserWnd::OnTabsChanged()
{
	if( m_pTabBar != NULL && m_pTabBar->GetTabCount() > 0 ) {
		int idx = m_pTabBar->GetActiveTab();
		if( idx < 0 ) idx = 0;
		if( m_pPages != NULL ) {
			CControlUI* p = m_pPages->GetItemAt(idx);
			m_pActiveBrowser = p
				? static_cast<CWebBrowserUI*>(p->GetInterface(DUI_CTR_WEBBROWSER))
				: NULL;
			if( idx >= 0 ) m_pPages->SelectItem(idx);
		}
	}
	else {
		m_pActiveBrowser = NULL;
	}
	if( m_pUrlBox ) {
		if( m_pActiveBrowser )
			m_pUrlBox->SetText(m_pActiveBrowser->GetLocationUrl());
		else
			m_pUrlBox->SetText(_T(""));
	}
	UpdateNavButtons();
}

void CBrowserWnd::ApplyPlaceholderTabIcon(CTabButtonUI* pTab)
{
	if( pTab == NULL ) return;
	pTab->SetIconSize(16);
	pTab->SetTabIconLib(_T("tabler-filled"), _T("world"));
}

void CBrowserWnd::ApplyLoadingTabIcon(CTabButtonUI* pTab)
{
	if( pTab == NULL ) return;
	pTab->SetIconSize(16);
	pTab->SetTabLoading(true);
}

void CBrowserWnd::CancelDelayedNavigates()
{
	HWND hWnd = GetHWND();
	for( size_t i = 0; i < m_aDelayedNav.size(); ++i ) {
		if( hWnd != NULL )
			::KillTimer(hWnd, m_aDelayedNav[i].nTimerId);
	}
	m_aDelayedNav.clear();
}

void CBrowserWnd::ScheduleDelayedHomeNavigate(CWebBrowserUI* pBrowser)
{
	if( pBrowser == NULL || m_pTabBar == NULL ) return;
	const int nDelay = m_pTabBar->GetTabLoadingTestDelay();
	if( nDelay <= 0 ) {
		pBrowser->NavigateHomePage();
		return;
	}
	HWND hWnd = GetHWND();
	if( hWnd == NULL ) {
		pBrowser->NavigateHomePage();
		return;
	}
	DelayedNav item;
	item.nTimerId = m_nNextDelayTimerId++;
	item.pBrowser = pBrowser;
	m_aDelayedNav.push_back(item);
	::SetTimer(hWnd, item.nTimerId, (UINT)nDelay, NULL);
}

void CBrowserWnd::OnDelayedHomeNavigate(UINT_PTR nTimerId)
{
	HWND hWnd = GetHWND();
	if( hWnd != NULL )
		::KillTimer(hWnd, nTimerId);
	CWebBrowserUI* pBrowser = NULL;
	for( size_t i = 0; i < m_aDelayedNav.size(); ++i ) {
		if( m_aDelayedNav[i].nTimerId == nTimerId ) {
			pBrowser = m_aDelayedNav[i].pBrowser;
			m_aDelayedNav.erase(m_aDelayedNav.begin() + (int)i);
			break;
		}
	}
	if( pBrowser == NULL || !IsBrowserAlive(pBrowser) ) return;
	// 延迟结束：仍显示 Loading，开始真正导航
	CTabButtonUI* pTab = FindTabForBrowser(pBrowser);
	if( pTab != NULL )
		ApplyLoadingTabIcon(pTab);
	pBrowser->NavigateHomePage();
}

LRESULT CBrowserWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if( uMsg == WM_TIMER ) {
		const UINT_PTR nId = (UINT_PTR)wParam;
		for( size_t i = 0; i < m_aDelayedNav.size(); ++i ) {
			if( m_aDelayedNav[i].nTimerId == nId ) {
				OnDelayedHomeNavigate(nId);
				return 0;
			}
		}
	}
	return WindowImplBase::HandleMessage(uMsg, wParam, lParam);
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

	m_pPages->Add(pBrowser);

	CTabButtonUI* pTab = m_pTabBar->AddTab(sTitle);
	if( pTab == NULL ) return;
	ApplyLoadingTabIcon(pTab);

	int iNew = m_pTabBar->GetTabCount() - 1;
	if( m_pTabBar->GetActiveTab() != iNew )
		m_pTabBar->SetActiveTab(iNew);
	m_pPages->SelectItem(iNew);
	m_pActiveBrowser = pBrowser;
	if( m_pUrlBox ) m_pUrlBox->SetText(pBrowser->GetHomePage());
	UpdateNavButtons();

	// 测试延迟：先转圈，到期后再 NavigateHomePage
	ScheduleDelayedHomeNavigate(pBrowser);
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

bool CBrowserWnd::HandleThemeCommand(LPCTSTR pstrName)
{
	if( pstrName == NULL || *pstrName == _T('\0') ) return false;
	if( _tcscmp(pstrName, _T("btn_theme_default")) != 0
		&& _tcscmp(pstrName, _T("btn_theme_azure")) != 0
		&& _tcscmp(pstrName, _T("btn_theme_emerald")) != 0
		&& _tcscmp(pstrName, _T("btn_theme_graphite")) != 0
		&& _tcscmp(pstrName, _T("btn_theme_dark")) != 0 )
		return false;

	CThemeManager* tm = CThemeManager::GetInstance();
	if( tm == NULL ) return true;
	if( !tm->IsEnabled() ) tm->SetEnabled(true);

	LPCTSTR themeId = _T("default");
	if( _tcscmp(pstrName, _T("btn_theme_azure")) == 0 ) themeId = _T("azure");
	else if( _tcscmp(pstrName, _T("btn_theme_emerald")) == 0 ) themeId = _T("emerald");
	else if( _tcscmp(pstrName, _T("btn_theme_graphite")) == 0 ) themeId = _T("graphite");
	else if( _tcscmp(pstrName, _T("btn_theme_dark")) == 0 ) themeId = _T("dark");
	tm->ApplyTheme(themeId);
	return true;
}

void CBrowserWnd::HandleNavCommand(LPCTSTR pstrName)
{
	if( pstrName == NULL || *pstrName == _T('\0') ) return;
	CWebBrowserUI* pBrowser = GetActiveBrowser();
	if( _tcscmp(pstrName, _T("goBtn")) == 0 ) {
		if( pBrowser == NULL ) {
			// 无标签时转到：按地址栏开新页
			CDuiString url;
			if( m_pUrlBox ) {
				url = m_pUrlBox->GetText();
				url.Trim();
			}
			if( url.IsEmpty() )
				AddNewTab(_T("新标签页"), _T("https://project.feishu.cn"));
			else
				AddNewTab(NULL, url);
			return;
		}
		NavigateAddressBar();
		return;
	}
	if( _tcscmp(pstrName, _T("homeBtn")) == 0 || _tcscmp(pstrName, _T("sideToolHome")) == 0 ) {
		if( pBrowser == NULL )
			AddNewTab(_T("主页"), _T("https://project.feishu.cn"));
		else
			pBrowser->NavigateHomePage();
		return;
	}
	if( pBrowser == NULL ) return;
	if( _tcscmp(pstrName, _T("backBtn")) == 0 )
		pBrowser->GoBack();
	else if( _tcscmp(pstrName, _T("forwardBtn")) == 0 )
		pBrowser->GoForward();
	else if( _tcscmp(pstrName, _T("refreshBtn")) == 0 )
		pBrowser->Refresh();
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

void CBrowserWnd::HostEvents::OnNavigationStarting(CWebBrowserUI* pWeb, LPCTSTR url, bool* /*pCancel*/)
{
	if( m_pOwner ) m_pOwner->OnBrowserNavigationStarting(pWeb, url);
}

void CBrowserWnd::HostEvents::OnNavigationCompleted(CWebBrowserUI* pWeb, LPCTSTR url, bool success)
{
	if( m_pOwner ) m_pOwner->OnBrowserNavigated(pWeb, url, success);
}

void CBrowserWnd::HostEvents::OnFaviconChanged(CWebBrowserUI* pWeb, const BYTE* pData, DWORD dwSize)
{
	if( m_pOwner ) m_pOwner->OnBrowserFaviconChanged(pWeb, pData, dwSize);
}

void CBrowserWnd::HostEvents::OnHistoryChanged(CWebBrowserUI* pWeb)
{
	if( m_pOwner ) m_pOwner->OnBrowserHistoryChanged(pWeb);
}

void CBrowserWnd::HostEvents::OnNewWindowRequested(CWebBrowserUI* pWeb, LPCTSTR url, bool* pHandled)
{
	if( m_pOwner ) m_pOwner->OnBrowserNewWindow(pWeb, url, pHandled);
}

CTabButtonUI* CBrowserWnd::FindTabForBrowser(CWebBrowserUI* pWeb) const
{
	if( pWeb == NULL || m_pTabBar == NULL || m_pPages == NULL ) return NULL;
	int idx = -1;
	for( int i = 0; i < m_pPages->GetCount(); ++i ) {
		if( m_pPages->GetItemAt(i) == pWeb ) { idx = i; break; }
	}
	if( idx < 0 ) return NULL;
	return m_pTabBar->GetTab(idx);
}

void CBrowserWnd::OnBrowserTitleChanged(CWebBrowserUI* pWeb, LPCTSTR title)
{
	CTabButtonUI* pTab = FindTabForBrowser(pWeb);
	if( pTab && title ) pTab->SetTabTitle(title);
}

void CBrowserWnd::OnBrowserNavigated(CWebBrowserUI* pWeb, LPCTSTR url, bool /*success*/)
{
	if( pWeb == NULL ) return;
	if( url && *url )
		pWeb->SetLocationUrl(url);
	CTabButtonUI* pTab = FindTabForBrowser(pWeb);
	// 导航结束仍无可读 favicon：关转圈，回占位 globe
	if( pTab != NULL && pTab->IsTabLoading() )
		ApplyPlaceholderTabIcon(pTab);
	if( m_pUrlBox == NULL ) return;
	if( GetActiveBrowser() != pWeb ) return;
	m_pUrlBox->SetText(pWeb->GetLocationUrl());
}

void CBrowserWnd::OnBrowserNavigationStarting(CWebBrowserUI* pWeb, LPCTSTR /*url*/)
{
	CTabButtonUI* pTab = FindTabForBrowser(pWeb);
	if( pTab != NULL )
		ApplyLoadingTabIcon(pTab);
}

void CBrowserWnd::OnBrowserFaviconChanged(CWebBrowserUI* pWeb, const BYTE* pData, DWORD dwSize)
{
	CTabButtonUI* pTab = FindTabForBrowser(pWeb);
	if( pTab == NULL ) return;
	if( pData != NULL && dwSize > 0 ) {
		// WebView2 导航中途常先推一个偏黑默认图标，深色标签上看不清；跳过等真正站点图标
		if( !IsFaviconReadableOnDarkTab(pData, dwSize) )
			return;
		pTab->SetIconSize(16);
		if( !pTab->SetTabIcon(pData, dwSize) ) {
			if( pTab->IsTabLoading() )
				return;
			ApplyPlaceholderTabIcon(pTab);
		}
	}
	else {
		// 加载中清空：保持转圈；加载结束后清空：占位
		if( pTab->IsTabLoading() )
			return;
		ApplyPlaceholderTabIcon(pTab);
	}
}

bool CBrowserWnd::IsFaviconReadableOnDarkTab(const BYTE* pData, DWORD dwSize)
{
	if( pData == NULL || dwSize == 0 ) return false;
	TImageInfo* pInfo = CRenderEngine::LoadImageFromMemory(pData, dwSize, 0);
	if( pInfo == NULL || pInfo->hBitmap == NULL || pInfo->nX <= 0 || pInfo->nY <= 0 ) {
		if( pInfo != NULL ) CRenderEngine::FreeImage(pInfo);
		return false;
	}

	BITMAP bm = { 0 };
	if( ::GetObject(pInfo->hBitmap, sizeof(bm), &bm) == 0 || bm.bmBits == NULL ) {
		CRenderEngine::FreeImage(pInfo);
		return false;
	}

	const int nPixels = pInfo->nX * pInfo->nY;
	const BYTE* pBits = static_cast<const BYTE*>(bm.bmBits);
	int nOpaque = 0;
	int nLumSum = 0;
	int nMaxLum = 0;
	for( int i = 0; i < nPixels; ++i ) {
		const BYTE* px = pBits + i * 4;
		const int a = px[3];
		if( a < 40 ) continue;
		// 预乘 BGRA：还原近似亮度
		int b = px[0], g = px[1], r = px[2];
		if( a < 255 ) {
			r = (r * 255) / a;
			g = (g * 255) / a;
			b = (b * 255) / a;
		}
		const int lum = (r * 30 + g * 59 + b * 11) / 100;
		nLumSum += lum;
		if( lum > nMaxLum ) nMaxLum = lum;
		++nOpaque;
	}
	CRenderEngine::FreeImage(pInfo);

	if( nOpaque < 4 ) return false;
	const int nAvg = nLumSum / nOpaque;
	// 深色标签底约 #3C3C46：平均/峰值都偏黑的过渡图（Edge 默认文档图标）丢弃
	if( nAvg < 55 && nMaxLum < 100 ) return false;
	return true;
}

void CBrowserWnd::OnBrowserHistoryChanged(CWebBrowserUI* pWeb)
{
	if( pWeb != NULL && GetActiveBrowser() != pWeb ) return;
	UpdateNavButtons();
}

void CBrowserWnd::OnBrowserNewWindow(CWebBrowserUI* /*pWeb*/, LPCTSTR url, bool* pHandled)
{
	if( pHandled == NULL ) return;
	if( url == NULL || *url == _T('\0') ) {
		*pHandled = false;
		return;
	}
	// 拦截弹窗 / target=_blank，改为本壳新标签
	AddNewTab(NULL, url);
	*pHandled = true;
}

void CBrowserWnd::UpdateNavButtons()
{
	CWebBrowserUI* pBrowser = GetActiveBrowser();
	const bool bBack = (pBrowser != NULL && pBrowser->CanGoBack());
	const bool bForward = (pBrowser != NULL && pBrowser->CanGoForward());
	CControlUI* pBack = m_pm.FindControl(_T("backBtn"));
	CControlUI* pForward = m_pm.FindControl(_T("forwardBtn"));
	if( pBack ) pBack->SetEnabled(bBack);
	if( pForward ) pForward->SetEnabled(bForward);
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
				if( m_pUrlBox && m_pActiveBrowser )
					m_pUrlBox->SetText(m_pActiveBrowser->GetLocationUrl());
				UpdateNavButtons();
			}
		}
		else if( msg.sType == DUI_MSGTYPE_TABCLOSE ) {
			// 关光时不会再发 tabselect；必须清掉已销毁的活动页指针
			OnTabsChanged();
		}
	}

	if( msg.sType == DUI_MSGTYPE_RETURN && msg.pSender != NULL ) {
		if( msg.pSender == m_pUrlBox || msg.pSender->GetName() == _T("urlBox") )
			NavigateAddressBar();
	}
	else if( msg.sType == DUI_MSGTYPE_CLICK && msg.pSender != NULL ) {
		LPCTSTR name = msg.pSender->GetName();
		if( !HandleThemeCommand(name) )
			HandleNavCommand(name);
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
	// 导航 / 主题按钮已在 Notify 里处理，避免 NotifyPump→OnClick 再执行一遍
	CDuiString name = msg.pSender->GetName();
	if( name == _T("goBtn") || name == _T("backBtn") || name == _T("forwardBtn")
		|| name == _T("refreshBtn") || name == _T("homeBtn") || name == _T("sideToolHome")
		|| name == _T("btn_theme_default") || name == _T("btn_theme_azure")
		|| name == _T("btn_theme_emerald") || name == _T("btn_theme_graphite")
		|| name == _T("btn_theme_dark") )
		return;
	WindowImplBase::OnClick(msg);
}
