#include "StdAfx.h"
#include "UIWebBrowser.h"
#include "WebBrowserIeEngine.h"
#include <ExDisp.h>
#include <atlconv.h>
#include <atlcomcli.h>
#include <MsHTML.h>

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CWebBrowserUI)
	IMPLEMENT_DUICONTROL(CWebView2UI)

	CWebBrowserUI::CWebBrowserUI()
		: m_pEngine(NULL)
		, m_sHostMode(_T("window"))
		, m_bAutoNavi(false)
		, m_bFallback(true)
		, m_bForceEngine(false)
		, m_pHostEvents(NULL)
		, m_pWebBrowserEventHandler(NULL)
	{
		SetMouseEnabled(false);
	}

	CWebBrowserUI::~CWebBrowserUI()
	{
		DestroyEngine();
	}

	LPCTSTR CWebBrowserUI::GetClass() const
	{
		return _T("WebBrowserUI");
	}

	LPVOID CWebBrowserUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_WEBBROWSER) == 0 ) return static_cast<CWebBrowserUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	CDuiString CWebBrowserUI::ResolveDefaultEngine() const
	{
#ifdef DUILIB_HAS_WEBVIEW2
		if( CWebBrowserEngineFactory::Instance().IsRegistered(_T("webview2")) )
			return _T("webview2");
#endif
		return _T("ie");
	}

	void CWebBrowserUI::SetEngine(LPCTSTR name)
	{
		m_sWantedEngine = name ? name : _T("");
		m_bForceEngine = !m_sWantedEngine.IsEmpty();
		DestroyEngine();
		if( m_pManager ) EnsureEngine();
	}

	LPCTSTR CWebBrowserUI::GetEngineName() const
	{
		return m_sEngineName;
	}

	void CWebBrowserUI::SetEngineFallback(bool bFallback)
	{
		m_bFallback = bFallback;
	}

	bool CWebBrowserUI::IsEngineFallback() const
	{
		return m_bFallback;
	}

	void CWebBrowserUI::SetHomePage(LPCTSTR lpszUrl)
	{
		m_sHomePage = lpszUrl ? lpszUrl : _T("");
	}

	LPCTSTR CWebBrowserUI::GetHomePage() const
	{
		return m_sHomePage;
	}

	void CWebBrowserUI::SetAutoNavigation(bool bAuto)
	{
		m_bAutoNavi = bAuto;
	}

	bool CWebBrowserUI::IsAutoNavigation() const
	{
		return m_bAutoNavi;
	}

	void CWebBrowserUI::SetUserDataFolder(LPCTSTR path)
	{
		m_sUserDataFolder = path ? path : _T("");
		if( m_pEngine ) m_pEngine->SetUserDataFolder(m_sUserDataFolder);
	}

	void CWebBrowserUI::SetHostMode(LPCTSTR mode)
	{
		CDuiString s = _T("window");
		if( mode && *mode ) {
			if( _tcsicmp(mode, _T("composition")) == 0 || _tcsicmp(mode, _T("compose")) == 0
				|| _tcsicmp(mode, _T("visual")) == 0 )
				s = _T("composition");
			else if( _tcsicmp(mode, _T("window")) == 0 || _tcsicmp(mode, _T("hwnd")) == 0 )
				s = _T("window");
		}
		if( m_sHostMode == s ) return;
		m_sHostMode = s;
		DestroyEngine();
		if( m_pManager ) EnsureEngine();
	}

	LPCTSTR CWebBrowserUI::GetHostMode() const
	{
		return m_sHostMode.IsEmpty() ? _T("window") : (LPCTSTR)m_sHostMode;
	}

	void CWebBrowserUI::SetHostEvents(CWebBrowserHostEvents* pEvents)
	{
		m_pHostEvents = pEvents;
		if( m_pEngine ) m_pEngine->SetHostEvents(pEvents);
	}

	void CWebBrowserUI::SetWebBrowserEventHandler(CWebBrowserEventHandler* pEventHandler)
	{
		m_pWebBrowserEventHandler = pEventHandler;
		if( m_pEngine && _tcsicmp(m_sEngineName, _T("ie")) == 0 ) {
			CWebBrowserIeEngine* pIe = static_cast<CWebBrowserIeEngine*>(m_pEngine);
			pIe->SetIeEventHandler(pEventHandler);
		}
	}

	void CWebBrowserUI::DestroyEngine()
	{
		if( m_pEngine ) {
			m_pEngine->Destroy();
			delete m_pEngine;
			m_pEngine = NULL;
		}
		m_sEngineName.Empty();
	}

	void CWebBrowserUI::EnsureEngine()
	{
		if( m_pEngine != NULL ) return;
		if( m_pManager == NULL ) return;
		HWND hParent = m_pManager->GetPaintWindow();
		if( hParent == NULL ) return;

		CWebBrowserEngineFactory::Instance().EnsureBuiltinEngines();
		CDuiString want = m_bForceEngine ? m_sWantedEngine : ResolveDefaultEngine();
		if( want.IsEmpty() ) want = ResolveDefaultEngine();

		IWebBrowserEngine* pEng = CWebBrowserEngineFactory::Instance().Create(want);
		if( pEng == NULL && m_bFallback && _tcsicmp(want, _T("ie")) != 0 )
			pEng = CWebBrowserEngineFactory::Instance().Create(_T("ie"));
		if( pEng == NULL ) return;

		if( !m_sUserDataFolder.IsEmpty() )
			pEng->SetUserDataFolder(m_sUserDataFolder);
		pEng->SetHostMode(GetHostMode());
		pEng->SetHostEvents(m_pHostEvents);

		RECT rc = m_rcItem;
		if( !pEng->Create(this, hParent, rc) ) {
			delete pEng;
			pEng = NULL;
			if( m_bFallback && _tcsicmp(want, _T("ie")) != 0 ) {
				pEng = CWebBrowserEngineFactory::Instance().Create(_T("ie"));
				if( pEng ) {
					pEng->SetHostEvents(m_pHostEvents);
					if( !pEng->Create(this, hParent, rc) ) {
						delete pEng;
						pEng = NULL;
					}
				}
			}
		}
		if( pEng == NULL ) return;

		m_pEngine = pEng;
		m_sEngineName = pEng->GetName();
		if( _tcsicmp(m_sEngineName, _T("ie")) == 0 && m_pWebBrowserEventHandler ) {
			static_cast<CWebBrowserIeEngine*>(m_pEngine)->SetIeEventHandler(m_pWebBrowserEventHandler);
		}

		m_pEngine->SetVisible(IsVisible());
		m_pEngine->SetPos(m_rcItem);

		if( !m_sPendingUrl.IsEmpty() ) {
			m_pEngine->Navigate(m_sPendingUrl);
			m_sPendingUrl.Empty();
		}
		else if( m_bAutoNavi && !m_sHomePage.IsEmpty() ) {
			m_pEngine->Navigate(m_sHomePage);
		}
	}

	void CWebBrowserUI::Init()
	{
		CControlUI::Init();
		EnsureEngine();
	}

	void CWebBrowserUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
	{
		CControlUI::SetManager(pManager, pParent, bInit);
		if( pManager && bInit ) EnsureEngine();
	}

	void CWebBrowserUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		EnsureEngine();
		if( m_pEngine ) m_pEngine->SetPos(m_rcItem);
	}

	void CWebBrowserUI::SetVisible(bool bVisible)
	{
		CControlUI::SetVisible(bVisible);
		if( m_pEngine ) m_pEngine->SetVisible(IsVisible());
	}

	void CWebBrowserUI::SetInternVisible(bool bVisible)
	{
		CControlUI::SetInternVisible(bVisible);
		if( m_pEngine ) m_pEngine->SetVisible(IsVisible());
	}

	void CWebBrowserUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("home-page")) == 0 || _tcsicmp(pstrName, _T("homepage")) == 0 ) {
			SetHomePage(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("auto-navi")) == 0 || _tcsicmp(pstrName, _T("autonavi")) == 0 ) {
			SetAutoNavigation(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("engine")) == 0 ) {
			SetEngine(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("engine-fallback")) == 0 || _tcsicmp(pstrName, _T("fallback")) == 0 ) {
			SetEngineFallback(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("ie")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("user-data-folder")) == 0 || _tcsicmp(pstrName, _T("userdata")) == 0 ) {
			SetUserDataFolder(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("host")) == 0 || _tcsicmp(pstrName, _T("host-mode")) == 0 ) {
			SetHostMode(pstrValue);
		}
		else {
			CControlUI::SetAttribute(pstrName, pstrValue);
		}
	}

	void CWebBrowserUI::Navigate2(LPCTSTR lpszUrl)
	{
		NavigateUrl(lpszUrl);
	}

	void CWebBrowserUI::NavigateUrl(LPCTSTR lpszUrl)
	{
		if( lpszUrl == NULL || *lpszUrl == _T('\0') ) return;
		EnsureEngine();
		if( m_pEngine ) m_pEngine->Navigate(lpszUrl);
		else m_sPendingUrl = lpszUrl;
	}

	void CWebBrowserUI::NavigateHomePage()
	{
		if( !m_sHomePage.IsEmpty() )
			NavigateUrl(m_sHomePage);
	}

	void CWebBrowserUI::Refresh()
	{
		EnsureEngine();
		if( m_pEngine ) m_pEngine->Refresh();
	}

	void CWebBrowserUI::Refresh2(int Level)
	{
		EnsureEngine();
		if( m_pEngine && _tcsicmp(m_sEngineName, _T("ie")) == 0 ) {
			CWebBrowserIeEngine* pIe = static_cast<CWebBrowserIeEngine*>(m_pEngine);
			if( pIe->GetHost() ) pIe->GetHost()->Refresh2(Level);
		}
		else if( m_pEngine ) {
			m_pEngine->Refresh();
		}
	}

	void CWebBrowserUI::GoBack()
	{
		EnsureEngine();
		if( m_pEngine ) m_pEngine->GoBack();
	}

	void CWebBrowserUI::GoForward()
	{
		EnsureEngine();
		if( m_pEngine ) m_pEngine->GoForward();
	}

	void* CWebBrowserUI::GetNative()
	{
		EnsureEngine();
		return m_pEngine ? m_pEngine->GetNative() : NULL;
	}

	IWebBrowser2* CWebBrowserUI::GetWebBrowser2()
	{
		if( m_pEngine == NULL || _tcsicmp(m_sEngineName, _T("ie")) != 0 ) return NULL;
		return reinterpret_cast<IWebBrowser2*>(m_pEngine->GetNative());
	}

	IDispatch* CWebBrowserUI::GetHtmlWindow()
	{
		if( m_pEngine == NULL || _tcsicmp(m_sEngineName, _T("ie")) != 0 ) return NULL;
		CWebBrowserIeEngine* pIe = static_cast<CWebBrowserIeEngine*>(m_pEngine);
		return pIe->GetHost() ? pIe->GetHost()->GetHtmlWindow() : NULL;
	}

	DISPID CWebBrowserUI::FindId(IDispatch *pObj, LPOLESTR pName)
	{
		return CWebBrowserIeHost::FindId(pObj, pName);
	}

	HRESULT CWebBrowserUI::InvokeMethod(IDispatch *pObj, LPOLESTR pMehtod, VARIANT *pVarResult, VARIANT *ps, int cArgs)
	{
		return CWebBrowserIeHost::InvokeMethod(pObj, pMehtod, pVarResult, ps, cArgs);
	}

	HRESULT CWebBrowserUI::GetProperty(IDispatch *pObj, LPOLESTR pName, VARIANT *pValue)
	{
		return CWebBrowserIeHost::GetProperty(pObj, pName, pValue);
	}

	HRESULT CWebBrowserUI::SetProperty(IDispatch *pObj, LPOLESTR pName, VARIANT *pValue)
	{
		return CWebBrowserIeHost::SetProperty(pObj, pName, pValue);
	}

	////////////////////////////////////////////////////////////////////////
	CWebView2UI::CWebView2UI()
	{
		SetEngine(_T("webview2"));
		SetEngineFallback(true);
	}

	LPCTSTR CWebView2UI::GetClass() const
	{
		return _T("WebView2UI");
	}

	LPVOID CWebView2UI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, _T("WebView2")) == 0 ) return static_cast<CWebView2UI*>(this);
		return CWebBrowserUI::GetInterface(pstrName);
	}
}
