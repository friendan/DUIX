#include "StdAfx.h"
#include "WebBrowserIeEngine.h"
#include "UIWebBrowser.h"
#include <atlconv.h>
#include <atlcomcli.h>
#include "../Utils/downloadmgr.h"
#include <mshtml.h>

namespace DuiLib
{
	////////////////////////////////////////////////////////////////////////
	//
	CWebBrowserIeHost::CWebBrowserIeHost(CWebBrowserUI* pFacade)
		: m_pWebBrowser2(NULL)
		, _pHtmlWnd2(NULL)
		, m_dwRef(0)
		, m_dwCookie(0)
		, m_pFacade(pFacade)
		, m_pWebBrowserEventHandler(NULL)
		, m_pHostEvents(NULL)
		, m_bCanGoBack(false)
		, m_bCanGoForward(false)
	{
		m_clsid = CLSID_WebBrowser;
	}

	bool CWebBrowserIeHost::DoCreateControl()
	{
		if (!CActiveXUI::DoCreateControl())
			return false;
		GetManager()->AddTranslateAccelerator(this);
		GetControl(IID_IWebBrowser2, (LPVOID*)&m_pWebBrowser2);
		RegisterEventHandler(TRUE);
		return true;
	}

	void CWebBrowserIeHost::ReleaseControl()
	{
		m_bCreated=false;
		GetManager()->RemoveTranslateAccelerator(this);
		RegisterEventHandler(FALSE);
	}

	CWebBrowserIeHost::~CWebBrowserIeHost()
	{
		ReleaseControl();
	}

	STDMETHODIMP CWebBrowserIeHost::GetTypeInfoCount( UINT *iTInfo )
	{
		*iTInfo = 0;
		return E_NOTIMPL;
	}

	STDMETHODIMP CWebBrowserIeHost::GetTypeInfo( UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo )
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP CWebBrowserIeHost::GetIDsOfNames( REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid,DISPID *rgDispId )
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP CWebBrowserIeHost::Invoke( DISPID dispIdMember, REFIID riid, LCID lcid,WORD wFlags, DISPPARAMS* pDispParams,VARIANT* pVarResult, EXCEPINFO* pExcepInfo,UINT* puArgErr )
	{
		if ((riid != IID_NULL))
			return E_INVALIDARG;

		switch(dispIdMember)
		{
		case DISPID_BEFORENAVIGATE2:
			BeforeNavigate2(
				pDispParams->rgvarg[6].pdispVal,
				pDispParams->rgvarg[5].pvarVal,
				pDispParams->rgvarg[4].pvarVal,
				pDispParams->rgvarg[3].pvarVal,
				pDispParams->rgvarg[2].pvarVal,
				pDispParams->rgvarg[1].pvarVal,
				pDispParams->rgvarg[0].pboolVal);
			break;
		case DISPID_COMMANDSTATECHANGE:
			CommandStateChange(
				pDispParams->rgvarg[1].lVal,
				pDispParams->rgvarg[0].boolVal);
			break;
		case DISPID_NAVIGATECOMPLETE2:
			NavigateComplete2(
				pDispParams->rgvarg[1].pdispVal,
				pDispParams->rgvarg[0].pvarVal);
			break;
		case DISPID_NAVIGATEERROR:
			NavigateError(
				pDispParams->rgvarg[4].pdispVal,
				pDispParams->rgvarg[3].pvarVal,
				pDispParams->rgvarg[2].pvarVal,
				pDispParams->rgvarg[1].pvarVal,
				pDispParams->rgvarg[0].pboolVal);
			break;
		case DISPID_STATUSTEXTCHANGE:
			break;
			//  	case DISPID_NEWWINDOW2:
			//  		break;
		case DISPID_NEWWINDOW3:
			NewWindow3(
				pDispParams->rgvarg[4].ppdispVal,
				pDispParams->rgvarg[3].pboolVal,
				pDispParams->rgvarg[2].uintVal,
				pDispParams->rgvarg[1].bstrVal,
				pDispParams->rgvarg[0].bstrVal);
			break;
		case DISPID_TITLECHANGE:
			{
				TitleChange(pDispParams->rgvarg[0].bstrVal);
				break;
			}
		case DISPID_DOCUMENTCOMPLETE:
			{
				DocumentComplete(
					pDispParams->rgvarg[1].pdispVal,
					pDispParams->rgvarg[0].pvarVal);

				break;
			}
		default:
			return DISP_E_MEMBERNOTFOUND;
		}
		return S_OK;
	}

	STDMETHODIMP CWebBrowserIeHost::QueryInterface( REFIID riid, LPVOID *ppvObject )
	{
		*ppvObject = NULL;

		if( riid == IID_IDocHostUIHandler)
			*ppvObject = static_cast<IDocHostUIHandler*>(this);
		else if( riid == IID_IDispatch)
			*ppvObject = static_cast<IDispatch*>(this);
		else if( riid == IID_IServiceProvider)
			*ppvObject = static_cast<IServiceProvider*>(this);
		else if(riid == IID_IInternetSecurityManager ) {
			*ppvObject = static_cast<IInternetSecurityManager*>(this);
		}
		else if (riid == IID_IOleCommandTarget)
			*ppvObject = static_cast<IOleCommandTarget*>(this);

		if( *ppvObject != NULL )
			AddRef();
		return *ppvObject == NULL ? E_NOINTERFACE : S_OK;
	}

	STDMETHODIMP_(ULONG) CWebBrowserIeHost::AddRef()
	{
		InterlockedIncrement(&m_dwRef); 
		return m_dwRef;
	}

	STDMETHODIMP_(ULONG) CWebBrowserIeHost::Release()
	{
		ULONG ulRefCount = InterlockedDecrement(&m_dwRef);
		return ulRefCount; 
	}

	void CWebBrowserIeHost::Navigate2( LPCTSTR lpszUrl )
	{
		if (lpszUrl == NULL)
			return;

		if (m_pWebBrowser2)
		{
			CDuiVariant url;
			url.vt=VT_BSTR;
			url.bstrVal=T2BSTR(lpszUrl);
			m_pWebBrowser2->Navigate2(&url, NULL, NULL, NULL, NULL);
		}
	}

	void CWebBrowserIeHost::Refresh()
	{
		if (m_pWebBrowser2)
		{
			m_pWebBrowser2->Refresh();
		}
	}

	void CWebBrowserIeHost::Stop()
	{
		if( m_pWebBrowser2 )
			m_pWebBrowser2->Stop();
	}

	bool CWebBrowserIeHost::GetUrl(CDuiString& out) const
	{
		out.Empty();
		if( m_pWebBrowser2 == NULL ) return false;
		BSTR bstr = NULL;
		if( FAILED(m_pWebBrowser2->get_LocationURL(&bstr)) || bstr == NULL ) return false;
#ifdef _UNICODE
		out = bstr;
#else
		out = CDuiString(bstr);
#endif
		SysFreeString(bstr);
		return !out.IsEmpty();
	}

	void CWebBrowserIeHost::ExecuteScript(LPCTSTR script)
	{
		if( script == NULL || *script == _T('\0') ) return;
		IDispatch* pWin = GetHtmlWindow();
		if( pWin == NULL ) return;
		CComQIPtr<IHTMLWindow2> spWin(pWin);
		if( !spWin ) return;
		CComBSTR bstrCode(script);
		CComBSTR bstrLang(L"javascript");
		CComVariant vRet;
		HRESULT hr = spWin->execScript(bstrCode, bstrLang, &vRet);
		if( m_pHostEvents && m_pFacade ) {
			CDuiString s;
			if( SUCCEEDED(hr) && vRet.vt == VT_BSTR && vRet.bstrVal )
#ifdef _UNICODE
				s = vRet.bstrVal;
#else
				s = CDuiString(vRet.bstrVal);
#endif
			m_pHostEvents->OnExecuteScriptResult(m_pFacade, s.IsEmpty() ? NULL : s.GetData(), SUCCEEDED(hr));
		}
	}

	void CWebBrowserIeHost::GoBack()
	{
		if (m_pWebBrowser2)
		{
			m_pWebBrowser2->GoBack();
		}
	}
	void CWebBrowserIeHost::GoForward()
	{
		if (m_pWebBrowser2)
		{
			m_pWebBrowser2->GoForward();
		}
	}
	/// DWebBrowserEvents2
	void CWebBrowserIeHost::BeforeNavigate2( IDispatch *pDisp,VARIANT *&url,VARIANT *&Flags,VARIANT *&TargetFrameName,VARIANT *&PostData,VARIANT *&Headers,VARIANT_BOOL *&Cancel )
	{
		if (m_pWebBrowserEventHandler)
		{
			m_pWebBrowserEventHandler->BeforeNavigate2(m_pFacade, pDisp,url,Flags,TargetFrameName,PostData,Headers,Cancel);
		}
	}

	void CWebBrowserIeHost::NavigateError( IDispatch *pDisp,VARIANT * &url,VARIANT *&TargetFrameName,VARIANT *&StatusCode,VARIANT_BOOL *&Cancel )
	{
		if (m_pWebBrowserEventHandler)
		{
			m_pWebBrowserEventHandler->NavigateError(m_pFacade, pDisp,url,TargetFrameName,StatusCode,Cancel);
		}
		if( m_pHostEvents && m_pFacade ) {
			CDuiString sUrl;
			int nCode = 0;
			if( url && url->vt == VT_BSTR && url->bstrVal )
#ifdef _UNICODE
				sUrl = url->bstrVal;
#else
				sUrl = CDuiString(url->bstrVal);
#endif
			if( StatusCode && StatusCode->vt == VT_I4 )
				nCode = StatusCode->lVal;
			else if( StatusCode && StatusCode->vt == VT_I2 )
				nCode = StatusCode->iVal;
			m_pHostEvents->OnLoadError(m_pFacade, sUrl.GetData(), nCode, _T("NavigateError"));
		}
	}

	void CWebBrowserIeHost::NavigateComplete2( IDispatch *pDisp,VARIANT *&url )
	{
		CComPtr<IDispatch> spDoc;   
		m_pWebBrowser2->get_Document(&spDoc);   

		if (spDoc)
		{   
			CComQIPtr<ICustomDoc, &IID_ICustomDoc> spCustomDoc(spDoc);   
			if (spCustomDoc)   
				spCustomDoc->SetUIHandler(this);   
		}

		if (m_pWebBrowserEventHandler)
		{
			m_pWebBrowserEventHandler->NavigateComplete2(m_pFacade, pDisp,url);
		}
	}

	void CWebBrowserIeHost::ProgressChange( LONG nProgress, LONG nProgressMax )
	{
		if (m_pWebBrowserEventHandler)
		{
			m_pWebBrowserEventHandler->ProgressChange(m_pFacade, nProgress,nProgressMax);
		}
	}

	void CWebBrowserIeHost::NewWindow3( IDispatch **pDisp, VARIANT_BOOL *&Cancel, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl )
	{
		if (m_pWebBrowserEventHandler)
		{
			m_pWebBrowserEventHandler->NewWindow3(m_pFacade, pDisp,Cancel,dwFlags,bstrUrlContext,bstrUrl);
		}
		if( m_pHostEvents && m_pFacade ) {
			bool handled = false;
#ifdef _UNICODE
			m_pHostEvents->OnNewWindowRequested(m_pFacade, bstrUrl ? bstrUrl : _T(""), &handled);
#else
			CDuiString sUrl;
			if( bstrUrl ) sUrl = CDuiString(bstrUrl);
			m_pHostEvents->OnNewWindowRequested(m_pFacade, sUrl, &handled);
#endif
			if( handled && Cancel != NULL )
				*Cancel = VARIANT_TRUE;
		}
		(void)pDisp;
		(void)dwFlags;
		(void)bstrUrlContext;
	}
	void CWebBrowserIeHost::CommandStateChange(long Command,VARIANT_BOOL Enable)
	{
		if (m_pWebBrowserEventHandler)
		{
			m_pWebBrowserEventHandler->CommandStateChange(m_pFacade, Command,Enable);
		}
		bool bNav = false;
		if( Command == CSC_NAVIGATEBACK ) {
			m_bCanGoBack = (Enable != VARIANT_FALSE);
			bNav = true;
		}
		else if( Command == CSC_NAVIGATEFORWARD ) {
			m_bCanGoForward = (Enable != VARIANT_FALSE);
			bNav = true;
		}
		if( bNav && m_pHostEvents && m_pFacade )
			m_pHostEvents->OnHistoryChanged(m_pFacade);
	}

	void CWebBrowserIeHost::TitleChange(BSTR bstrTitle)
	{
		if (m_pWebBrowserEventHandler)
		{
			m_pWebBrowserEventHandler->TitleChange(m_pFacade, bstrTitle);
		}
		if( m_pHostEvents && m_pFacade ) {
#ifdef _UNICODE
			m_pHostEvents->OnDocumentTitleChanged(m_pFacade, bstrTitle ? bstrTitle : _T(""));
#else
			CDuiString s;
			if( bstrTitle ) s = CDuiString(bstrTitle);
			m_pHostEvents->OnDocumentTitleChanged(m_pFacade, s);
#endif
		}
	}

	void CWebBrowserIeHost::DocumentComplete(IDispatch *pDisp, VARIANT *&url)
	{
		if (m_pWebBrowserEventHandler)
		{
			m_pWebBrowserEventHandler->DocumentComplete(m_pFacade, pDisp, url);
		}
	}

	// IDownloadManager
	STDMETHODIMP CWebBrowserIeHost::Download( /* [in] */ IMoniker *pmk, /* [in] */ IBindCtx *pbc, /* [in] */ DWORD dwBindVerb, /* [in] */ LONG grfBINDF, /* [in] */ BINDINFO *pBindInfo, /* [in] */ LPCOLESTR pszHeaders, /* [in] */ LPCOLESTR pszRedir, /* [in] */ UINT uiCP )
	{
		if (m_pWebBrowserEventHandler)
		{
			return m_pWebBrowserEventHandler->Download(m_pFacade, pmk,pbc,dwBindVerb,grfBINDF,pBindInfo,pszHeaders,pszRedir,uiCP);
		}
		return S_OK;
	}

	// IDocHostUIHandler
	STDMETHODIMP CWebBrowserIeHost::ShowContextMenu( DWORD dwID, POINT* pptPosition, IUnknown* pCommandTarget, IDispatch* pDispatchObjectHit )
	{
		if (m_pWebBrowserEventHandler)
		{
			return m_pWebBrowserEventHandler->ShowContextMenu(m_pFacade, dwID,pptPosition,pCommandTarget,pDispatchObjectHit);
		}
		return S_FALSE;
	}

	STDMETHODIMP CWebBrowserIeHost::GetHostInfo( DOCHOSTUIINFO* pInfo )
	{
		if (pInfo != NULL) {
			pInfo->dwFlags |= DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIFLAG_NO3DOUTERBORDER;
		}
		if (m_pWebBrowserEventHandler) {
			return m_pWebBrowserEventHandler->GetHostInfo(m_pFacade, pInfo);
		}
		return S_OK;
	}

	STDMETHODIMP CWebBrowserIeHost::ShowUI( DWORD dwID, IOleInPlaceActiveObject* pActiveObject, IOleCommandTarget* pCommandTarget, IOleInPlaceFrame* pFrame, IOleInPlaceUIWindow* pDoc )
	{
		if (m_pWebBrowserEventHandler)
		{
			return m_pWebBrowserEventHandler->ShowUI(m_pFacade, dwID,pActiveObject,pCommandTarget,pFrame,pDoc);
		}
		return S_OK;
	}

	STDMETHODIMP CWebBrowserIeHost::HideUI()
	{
		if (m_pWebBrowserEventHandler)
		{
			return m_pWebBrowserEventHandler->HideUI(m_pFacade);
		}
		return S_OK;
	}

	STDMETHODIMP CWebBrowserIeHost::UpdateUI()
	{
		if (m_pWebBrowserEventHandler)
		{
			return m_pWebBrowserEventHandler->UpdateUI(m_pFacade);
		}
		return S_OK;
	}

	STDMETHODIMP CWebBrowserIeHost::EnableModeless( BOOL fEnable )
	{
		if (m_pWebBrowserEventHandler)
		{
			return m_pWebBrowserEventHandler->EnableModeless(m_pFacade, fEnable);
		}
		return E_NOTIMPL;
	}

	STDMETHODIMP CWebBrowserIeHost::OnDocWindowActivate( BOOL fActivate )
	{
		if (m_pWebBrowserEventHandler)
		{
			return m_pWebBrowserEventHandler->OnDocWindowActivate(m_pFacade, fActivate);
		}
		return E_NOTIMPL;
	}

	STDMETHODIMP CWebBrowserIeHost::OnFrameWindowActivate( BOOL fActivate )
	{
		if (m_pWebBrowserEventHandler)
		{
			return m_pWebBrowserEventHandler->OnFrameWindowActivate(m_pFacade, fActivate);
		}
		return E_NOTIMPL;
	}

	STDMETHODIMP CWebBrowserIeHost::ResizeBorder( LPCRECT prcBorder, IOleInPlaceUIWindow* pUIWindow, BOOL fFrameWindow )
	{
		if (m_pWebBrowserEventHandler)
		{
			return m_pWebBrowserEventHandler->ResizeBorder(m_pFacade, prcBorder,pUIWindow,fFrameWindow);
		}
		return E_NOTIMPL;
	}

	STDMETHODIMP CWebBrowserIeHost::TranslateAccelerator( LPMSG lpMsg, const GUID* pguidCmdGroup, DWORD nCmdID )
	{
		if (m_pWebBrowserEventHandler)
		{
			return m_pWebBrowserEventHandler->TranslateAccelerator(m_pFacade, lpMsg,pguidCmdGroup,nCmdID);
		}
		return S_FALSE;
	}

	LRESULT CWebBrowserIeHost::TranslateAccelerator( MSG *pMsg )
	{
		if(pMsg->message < WM_KEYFIRST || pMsg->message > WM_KEYLAST)
			return S_FALSE;

		if( m_pWebBrowser2 == NULL )
			return E_NOTIMPL;

		// 当前Web窗口不是焦点,不处理加速键
		BOOL bIsChild = FALSE;
		HWND hTempWnd = NULL;
		HWND hWndFocus = ::GetFocus();

		hTempWnd = hWndFocus;
		while(hTempWnd != NULL)
		{
			if(hTempWnd == m_hwndHost)
			{
				bIsChild = TRUE;
				break;
			}
			hTempWnd = ::GetParent(hTempWnd);
		}
		if(!bIsChild)
			return S_FALSE;

		IOleInPlaceActiveObject *pObj;
		if (FAILED(m_pWebBrowser2->QueryInterface(IID_IOleInPlaceActiveObject, (LPVOID *)&pObj)))
			return S_FALSE;

		HRESULT hResult = pObj->TranslateAccelerator(pMsg);
		pObj->Release();
		return hResult;
	}

	STDMETHODIMP CWebBrowserIeHost::GetOptionKeyPath( LPOLESTR* pchKey, DWORD dwReserved )
	{
		if (m_pWebBrowserEventHandler)
		{
			return m_pWebBrowserEventHandler->GetOptionKeyPath(m_pFacade, pchKey,dwReserved);
		}
		return E_NOTIMPL;
	}

	STDMETHODIMP CWebBrowserIeHost::GetDropTarget( IDropTarget* pDropTarget, IDropTarget** ppDropTarget )
	{
		if (m_pWebBrowserEventHandler)
		{
			return m_pWebBrowserEventHandler->GetDropTarget(m_pFacade, pDropTarget,ppDropTarget);
		}
		return S_FALSE;	// 使用系统拖拽
	}

	STDMETHODIMP CWebBrowserIeHost::GetExternal( IDispatch** ppDispatch )
	{
		if (m_pWebBrowserEventHandler)
		{
			return m_pWebBrowserEventHandler->GetExternal(m_pFacade, ppDispatch);
		}
		return S_FALSE;
	}

	STDMETHODIMP CWebBrowserIeHost::TranslateUrl( DWORD dwTranslate, OLECHAR* pchURLIn, OLECHAR** ppchURLOut )
	{
		if (m_pWebBrowserEventHandler)
		{
			return m_pWebBrowserEventHandler->TranslateUrl(m_pFacade, dwTranslate,pchURLIn,ppchURLOut);
		}
		else
		{
			*ppchURLOut = 0;
			return E_NOTIMPL;
		}
	}

	STDMETHODIMP CWebBrowserIeHost::FilterDataObject( IDataObject* pDO, IDataObject** ppDORet )
	{
		if (m_pWebBrowserEventHandler)
		{
			return m_pWebBrowserEventHandler->FilterDataObject(m_pFacade, pDO,ppDORet);
		}
		else
		{
			*ppDORet = 0;
			return E_NOTIMPL;
		}
	}

	void CWebBrowserIeHost::SetWebBrowserEventHandler( CWebBrowserEventHandler* pEventHandler )
	{
		if ( pEventHandler!=NULL && m_pWebBrowserEventHandler!=pEventHandler)
			m_pWebBrowserEventHandler=pEventHandler;
	}

	void CWebBrowserIeHost::SetHostEvents(CWebBrowserHostEvents* pEvents)
	{
		m_pHostEvents = pEvents;
	}

	void CWebBrowserIeHost::Refresh2( int Level )
	{
		if( m_pWebBrowser2 == NULL ) return;
		CDuiVariant vLevel;
		vLevel.vt=VT_I4;
		vLevel.intVal=Level;
		m_pWebBrowser2->Refresh2(&vLevel);
	}

	void CWebBrowserIeHost::NavigateUrl( LPCTSTR lpszUrl )
	{
		if (m_pWebBrowser2 && lpszUrl)
		{
			m_pWebBrowser2->Navigate((BSTR)SysAllocString(T2BSTR(lpszUrl)),NULL,NULL,NULL,NULL);
		}
	}

	LPCTSTR CWebBrowserIeHost::GetClass() const
	{
		return _T("WebBrowserIeHost");
	}

	LPVOID CWebBrowserIeHost::GetInterface( LPCTSTR pstrName )
	{
		return CActiveXUI::GetInterface(pstrName);
	}

	STDMETHODIMP CWebBrowserIeHost::QueryService( REFGUID guidService, REFIID riid, void** ppvObject )
	{
		HRESULT hr = E_NOINTERFACE;
		*ppvObject = NULL;

		if ( guidService == SID_SDownloadManager && riid == IID_IDownloadManager)
		{
			*ppvObject = this;
			return S_OK;
		}
		if(guidService == SID_SInternetSecurityManager && riid == IID_IInternetSecurityManager) {
			*ppvObject = this;
			return S_OK;
		}
		return hr;
	}

	HRESULT CWebBrowserIeHost::RegisterEventHandler( BOOL inAdvise )
	{
		CComPtr<IWebBrowser2> pWebBrowser;
		CComPtr<IConnectionPointContainer>  pCPC;
		CComPtr<IConnectionPoint> pCP;
		HRESULT hr = GetControl(IID_IWebBrowser2, (void**)&pWebBrowser);
		if (FAILED(hr))
			return hr;
		hr=pWebBrowser->QueryInterface(IID_IConnectionPointContainer,(void **)&pCPC);
		if (FAILED(hr))
			return hr;
		hr=pCPC->FindConnectionPoint(DIID_DWebBrowserEvents2,&pCP);
		if (FAILED(hr))
			return hr;

		if (inAdvise)
		{
			hr = pCP->Advise((IDispatch*)this, &m_dwCookie);
		}
		else
		{
			hr = pCP->Unadvise(m_dwCookie);
		}
		return hr; 
	}

	DISPID CWebBrowserIeHost::FindId( IDispatch *pObj, LPOLESTR pName )
	{
		DISPID id = 0;
		if(FAILED(pObj->GetIDsOfNames(IID_NULL,&pName,1,LOCALE_SYSTEM_DEFAULT,&id))) id = -1;
		return id;
	}

	HRESULT CWebBrowserIeHost::InvokeMethod( IDispatch *pObj, LPOLESTR pMehtod, VARIANT *pVarResult, VARIANT *ps, int cArgs )
	{
		DISPID dispid = FindId(pObj, pMehtod);
		if(dispid == -1) return E_FAIL;

		DISPPARAMS dispparams;
		dispparams.cArgs = cArgs;
		dispparams.rgvarg = ps;
		dispparams.cNamedArgs = 0;
		dispparams.rgdispidNamedArgs = NULL;

		return pObj->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &dispparams, pVarResult, NULL, NULL);
	}

	HRESULT CWebBrowserIeHost::GetProperty( IDispatch *pObj, LPOLESTR pName, VARIANT *pValue )
	{
		DISPID dispid = FindId(pObj, pName);
		if(dispid == -1) return E_FAIL;

		DISPPARAMS ps;
		ps.cArgs = 0;
		ps.rgvarg = NULL;
		ps.cNamedArgs = 0;
		ps.rgdispidNamedArgs = NULL;

		return pObj->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &ps, pValue, NULL, NULL);
	}

	HRESULT CWebBrowserIeHost::SetProperty( IDispatch *pObj, LPOLESTR pName, VARIANT *pValue )
	{
		DISPID dispid = FindId(pObj, pName);
		if(dispid == -1) return E_FAIL;

		DISPPARAMS ps;
		ps.cArgs = 1;
		ps.rgvarg = pValue;
		ps.cNamedArgs = 0;
		ps.rgdispidNamedArgs = NULL;

		return pObj->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYPUT, &ps, NULL, NULL, NULL);
	}

	IDispatch* CWebBrowserIeHost::GetHtmlWindow()
	{
		IDispatch* pDp =  NULL;
		HRESULT hr = E_FAIL;
		if (m_pWebBrowser2)
			hr=m_pWebBrowser2->get_Document(&pDp);

		if (FAILED(hr))
			return NULL;

		CComQIPtr<IHTMLDocument2> pHtmlDoc2 = pDp;

		if (pHtmlDoc2 == NULL)
			return NULL;

		hr=pHtmlDoc2->get_parentWindow(&_pHtmlWnd2);

		if (FAILED(hr))
			return NULL;

		IDispatch *pHtmlWindown = NULL;
		hr=_pHtmlWnd2->QueryInterface(IID_IDispatch, (void**)&pHtmlWindown);
		if (FAILED(hr))
			return NULL;

		return pHtmlWindown;
	}

	IWebBrowser2* CWebBrowserIeHost::GetWebBrowser2( void )
	{
		return m_pWebBrowser2;
	}

	HRESULT STDMETHODCALLTYPE CWebBrowserIeHost::QueryStatus( __RPC__in_opt const GUID *pguidCmdGroup, ULONG cCmds, __RPC__inout_ecount_full(cCmds ) OLECMD prgCmds[ ], __RPC__inout_opt OLECMDTEXT *pCmdText )
	{
		HRESULT hr = OLECMDERR_E_NOTSUPPORTED;
		return hr;
	}

	HRESULT STDMETHODCALLTYPE CWebBrowserIeHost::Exec( __RPC__in_opt const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, __RPC__in_opt VARIANT *pvaIn, __RPC__inout_opt VARIANT *pvaOut )
	{
		HRESULT hr = S_OK;

		if (pguidCmdGroup && IsEqualGUID(*pguidCmdGroup, CGID_DocHostCommandHandler))
		{

			switch (nCmdID) 
			{

			case OLECMDID_SHOWSCRIPTERROR:
				{
					IHTMLDocument2*             pDoc = NULL;
					IHTMLWindow2*               pWindow = NULL;
					IHTMLEventObj*              pEventObj = NULL;
					BSTR                        rgwszNames[5] = 
					{ 
						SysAllocString(L"errorLine"),
						SysAllocString(L"errorCharacter"),
						SysAllocString(L"errorCode"),
						SysAllocString(L"errorMessage"),
						SysAllocString(L"errorUrl")
					};
					DISPID                      rgDispIDs[5];
					VARIANT                     rgvaEventInfo[5];
					DISPPARAMS                  params;
					BOOL                        fContinueRunningScripts = true;
					int                         i;

					params.cArgs = 0;
					params.cNamedArgs = 0;

					// Get the document that is currently being viewed.
					hr = pvaIn->punkVal->QueryInterface(IID_IHTMLDocument2, (void **) &pDoc);    
					// Get document.parentWindow.
					hr = pDoc->get_parentWindow(&pWindow);
					pDoc->Release();
					// Get the window.event object.
					hr = pWindow->get_event(&pEventObj);
					// Get the error info from the window.event object.
					for (i = 0; i < 5; i++) 
					{  
						// Get the property's dispID.
						hr = pEventObj->GetIDsOfNames(IID_NULL, &rgwszNames[i], 1, 
							LOCALE_SYSTEM_DEFAULT, &rgDispIDs[i]);
						// Get the value of the property.
						hr = pEventObj->Invoke(rgDispIDs[i], IID_NULL,
							LOCALE_SYSTEM_DEFAULT,
							DISPATCH_PROPERTYGET, &params, &rgvaEventInfo[i],
							NULL, NULL);
						SysFreeString(rgwszNames[i]);
					}

					// At this point, you would normally alert the user with 
					// the information about the error, which is now contained
					// in rgvaEventInfo[]. Or, you could just exit silently.

					(*pvaOut).vt = VT_BOOL;
					if (fContinueRunningScripts)
					{
						// Continue running scripts on the page.
						(*pvaOut).boolVal = VARIANT_TRUE;
					}
					else
					{
						// Stop running scripts on the page.
						(*pvaOut).boolVal = VARIANT_FALSE;   
					} 
					break;
				}
			default:
				hr = OLECMDERR_E_NOTSUPPORTED;
				break;
			}
		}
		else
		{
			hr = OLECMDERR_E_UNKNOWNGROUP;
		}
		return (hr);
	}

	////////////////////////////////////////////////////////////////////////
	// CWebBrowserIeEngine
	CWebBrowserIeEngine::CWebBrowserIeEngine()
		: m_pHost(NULL)
		, m_pHostEvents(NULL)
	{
	}

	CWebBrowserIeEngine::~CWebBrowserIeEngine()
	{
		Destroy();
	}

	void CWebBrowserIeEngine::Destroy()
	{
		if( m_pHost != NULL ) {
			delete m_pHost;
			m_pHost = NULL;
		}
	}

	bool CWebBrowserIeEngine::Create(CControlUI* pOwner, HWND /*hParent*/, const RECT& rc)
	{
		if( pOwner == NULL || pOwner->GetManager() == NULL ) return false;
		Destroy();
		CWebBrowserUI* pFacade = static_cast<CWebBrowserUI*>(pOwner->GetInterface(DUI_CTR_WEBBROWSER));
		if( pFacade == NULL ) return false;
		m_pHost = new CWebBrowserIeHost(pFacade);
		m_pHost->SetHostEvents(m_pHostEvents);
		m_pHost->SetManager(pOwner->GetManager(), pOwner, false);
		m_pHost->SetDelayCreate(false);
		RECT rcPos = rc;
		m_pHost->SetPos(rcPos, false);
		return m_pHost->GetWebBrowser2() != NULL;
	}

	void CWebBrowserIeEngine::SetPos(const RECT& rc)
	{
		if( m_pHost ) m_pHost->SetPos((RECT&)rc, false);
	}

	void CWebBrowserIeEngine::SetVisible(bool bVisible)
	{
		if( m_pHost ) {
			m_pHost->SetVisible(bVisible);
			m_pHost->SetInternVisible(bVisible);
		}
	}

	void CWebBrowserIeEngine::Navigate(LPCTSTR url)
	{
		if( m_pHost ) m_pHost->Navigate2(url);
	}

	void CWebBrowserIeEngine::GoBack()
	{
		if( m_pHost ) m_pHost->GoBack();
	}

	void CWebBrowserIeEngine::GoForward()
	{
		if( m_pHost ) m_pHost->GoForward();
	}

	bool CWebBrowserIeEngine::CanGoBack() const
	{
		return m_pHost != NULL && m_pHost->CanGoBack();
	}

	bool CWebBrowserIeEngine::CanGoForward() const
	{
		return m_pHost != NULL && m_pHost->CanGoForward();
	}

	void CWebBrowserIeEngine::Refresh()
	{
		if( m_pHost ) m_pHost->Refresh();
	}

	void CWebBrowserIeEngine::Stop()
	{
		if( m_pHost ) m_pHost->Stop();
	}

	bool CWebBrowserIeEngine::GetUrl(CDuiString& out) const
	{
		if( m_pHost == NULL ) { out.Empty(); return false; }
		return m_pHost->GetUrl(out);
	}

	void CWebBrowserIeEngine::ExecuteScript(LPCTSTR script)
	{
		if( m_pHost ) m_pHost->ExecuteScript(script);
	}

	HWND CWebBrowserIeEngine::GetHostWindow() const
	{
		return m_pHost ? m_pHost->GetHostWindow() : NULL;
	}

	void* CWebBrowserIeEngine::GetNative()
	{
		return m_pHost ? (void*)m_pHost->GetWebBrowser2() : NULL;
	}

	void CWebBrowserIeEngine::SetHostEvents(CWebBrowserHostEvents* pEvents)
	{
		m_pHostEvents = pEvents;
		if( m_pHost ) m_pHost->SetHostEvents(pEvents);
	}

	void CWebBrowserIeEngine::SetIeEventHandler(CWebBrowserEventHandler* pHandler)
	{
		if( m_pHost ) m_pHost->SetWebBrowserEventHandler(pHandler);
	}
}
