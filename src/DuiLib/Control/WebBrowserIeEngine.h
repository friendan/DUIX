#ifndef __WEBBROWSERIEENGINE_H__
#define __WEBBROWSERIEENGINE_H__

#pragma once

#include <MsHTML.h>
#include "Utils/WebBrowserEventHandler.h"
#include "IWebBrowserEngine.h"
#include <ExDisp.h>

namespace DuiLib
{
	/// 内部 IE ActiveX 宿主（不注册为 XML 控件）
	class CWebBrowserIeHost
		: public CActiveXUI
		, public IDocHostUIHandler
		, public IServiceProvider
		, public IOleCommandTarget
		, public IDispatch
		, public ITranslateAccelerator
		, public IInternetSecurityManager
	{
	public:
		explicit CWebBrowserIeHost(CWebBrowserUI* pFacade);
		virtual ~CWebBrowserIeHost();

		void SetWebBrowserEventHandler(CWebBrowserEventHandler* pEventHandler);
		void SetHostEvents(CWebBrowserHostEvents* pEvents);
		void Navigate2(LPCTSTR lpszUrl);
		void Refresh();
		void Refresh2(int Level);
		void Stop();
		void GoBack();
		void GoForward();
		void NavigateUrl(LPCTSTR lpszUrl);
		bool CanGoBack() const { return m_bCanGoBack; }
		bool CanGoForward() const { return m_bCanGoForward; }
		bool GetUrl(CDuiString& out) const;
		void ExecuteScript(LPCTSTR script);
		virtual bool DoCreateControl();
		IWebBrowser2* GetWebBrowser2(void);
		IDispatch* GetHtmlWindow();
		static DISPID FindId(IDispatch *pObj, LPOLESTR pName);
		static HRESULT InvokeMethod(IDispatch *pObj, LPOLESTR pMehtod, VARIANT *pVarResult, VARIANT *ps, int cArgs);
		static HRESULT GetProperty(IDispatch *pObj, LPOLESTR pName, VARIANT *pValue);
		static HRESULT SetProperty(IDispatch *pObj, LPOLESTR pName, VARIANT *pValue);

	protected:
		IWebBrowser2* m_pWebBrowser2;
		IHTMLWindow2* _pHtmlWnd2;
		LONG m_dwRef;
		DWORD m_dwCookie;
		CWebBrowserUI* m_pFacade;
		CWebBrowserEventHandler* m_pWebBrowserEventHandler;
		CWebBrowserHostEvents* m_pHostEvents;
		bool m_bCanGoBack;
		bool m_bCanGoForward;
		virtual void ReleaseControl();
		HRESULT RegisterEventHandler(BOOL inAdvise);

		void BeforeNavigate2(IDispatch *pDisp, VARIANT *&url, VARIANT *&Flags, VARIANT *&TargetFrameName, VARIANT *&PostData, VARIANT *&Headers, VARIANT_BOOL *&Cancel);
		void NavigateError(IDispatch *pDisp, VARIANT * &url, VARIANT *&TargetFrameName, VARIANT *&StatusCode, VARIANT_BOOL *&Cancel);
		void NavigateComplete2(IDispatch *pDisp, VARIANT *&url);
		void ProgressChange(LONG nProgress, LONG nProgressMax);
		void NewWindow3(IDispatch **pDisp, VARIANT_BOOL *&Cancel, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl);
		void CommandStateChange(long Command, VARIANT_BOOL Enable);
		void TitleChange(BSTR bstrTitle);
		void DocumentComplete(IDispatch *pDisp, VARIANT *&url);

	public:
		virtual LPCTSTR GetClass() const;
		virtual LPVOID GetInterface(LPCTSTR pstrName);

		STDMETHOD_(ULONG, AddRef)();
		STDMETHOD_(ULONG, Release)();
		STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObject);

		virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount(__RPC__out UINT *pctinfo);
		virtual HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid, __RPC__deref_out_opt ITypeInfo **ppTInfo);
		virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames(__RPC__in REFIID riid, __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames, UINT cNames, LCID lcid, __RPC__out_ecount_full(cNames) DISPID *rgDispId);
		virtual HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);

		STDMETHOD(ShowContextMenu)(DWORD dwID, POINT* pptPosition, IUnknown* pCommandTarget, IDispatch* pDispatchObjectHit);
		STDMETHOD(GetHostInfo)(DOCHOSTUIINFO* pInfo);
		STDMETHOD(ShowUI)(DWORD dwID, IOleInPlaceActiveObject* pActiveObject, IOleCommandTarget* pCommandTarget, IOleInPlaceFrame* pFrame, IOleInPlaceUIWindow* pDoc);
		STDMETHOD(HideUI)();
		STDMETHOD(UpdateUI)();
		STDMETHOD(EnableModeless)(BOOL fEnable);
		STDMETHOD(OnDocWindowActivate)(BOOL fActivate);
		STDMETHOD(OnFrameWindowActivate)(BOOL fActivate);
		STDMETHOD(ResizeBorder)(LPCRECT prcBorder, IOleInPlaceUIWindow* pUIWindow, BOOL fFrameWindow);
		STDMETHOD(TranslateAccelerator)(LPMSG lpMsg, const GUID* pguidCmdGroup, DWORD nCmdID);
		STDMETHOD(GetOptionKeyPath)(LPOLESTR* pchKey, DWORD dwReserved);
		STDMETHOD(GetDropTarget)(IDropTarget* pDropTarget, IDropTarget** ppDropTarget);
		STDMETHOD(GetExternal)(IDispatch** ppDispatch);
		STDMETHOD(TranslateUrl)(DWORD dwTranslate, OLECHAR* pchURLIn, OLECHAR** ppchURLOut);
		STDMETHOD(FilterDataObject)(IDataObject* pDO, IDataObject** ppDORet);

		STDMETHOD(QueryService)(REFGUID guidService, REFIID riid, void** ppvObject);

		virtual HRESULT STDMETHODCALLTYPE QueryStatus(__RPC__in_opt const GUID *pguidCmdGroup, ULONG cCmds, __RPC__inout_ecount_full(cCmds) OLECMD prgCmds[], __RPC__inout_opt OLECMDTEXT *pCmdText);
		virtual HRESULT STDMETHODCALLTYPE Exec(__RPC__in_opt const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, __RPC__in_opt VARIANT *pvaIn, __RPC__inout_opt VARIANT *pvaOut);

		STDMETHOD(Download)(IMoniker *pmk, IBindCtx *pbc, DWORD dwBindVerb, LONG grfBINDF, BINDINFO *pBindInfo, LPCOLESTR pszHeaders, LPCOLESTR pszRedir, UINT uiCP);

		virtual HRESULT STDMETHODCALLTYPE SetSecuritySite(__RPC__in_opt IInternetSecurityMgrSite *pSite) { return S_OK; }
		virtual HRESULT STDMETHODCALLTYPE GetSecuritySite(__RPC__deref_out_opt IInternetSecurityMgrSite **ppSite) { return S_OK; }
		virtual HRESULT STDMETHODCALLTYPE MapUrlToZone(__RPC__in LPCWSTR pwszUrl, __RPC__out DWORD *pdwZone, DWORD dwFlags) { return S_OK; }
		virtual HRESULT STDMETHODCALLTYPE GetSecurityId(__RPC__in LPCWSTR pwszUrl, __RPC__out_ecount_full(*pcbSecurityId) BYTE *pbSecurityId, __RPC__inout DWORD *pcbSecurityId, DWORD_PTR dwReserved) { return S_OK; }
		virtual HRESULT STDMETHODCALLTYPE ProcessUrlAction(__RPC__in LPCWSTR pwszUrl, DWORD dwAction, __RPC__out_ecount_full(cbPolicy) BYTE *pPolicy, DWORD cbPolicy, __RPC__in_opt BYTE *pContext, DWORD cbContext, DWORD dwFlags, DWORD dwReserved) { return S_OK; }
		virtual HRESULT STDMETHODCALLTYPE QueryCustomPolicy(__RPC__in LPCWSTR pwszUrl, __RPC__in REFGUID guidKey, __RPC__deref_out_ecount_full_opt(*pcbPolicy) BYTE **ppPolicy, __RPC__out DWORD *pcbPolicy, __RPC__in BYTE *pContext, DWORD cbContext, DWORD dwReserved) { return S_OK; }
		virtual HRESULT STDMETHODCALLTYPE SetZoneMapping(DWORD dwZone, __RPC__in LPCWSTR lpszPattern, DWORD dwFlags) { return S_OK; }
		virtual HRESULT STDMETHODCALLTYPE GetZoneMappings(DWORD dwZone, __RPC__deref_out_opt IEnumString **ppenumString, DWORD dwFlags) { return S_OK; }

		virtual LRESULT TranslateAccelerator(MSG *pMsg);
	};

	class CWebBrowserIeEngine : public IWebBrowserEngine
	{
	public:
		CWebBrowserIeEngine();
		virtual ~CWebBrowserIeEngine();

		virtual LPCTSTR GetName() const { return _T("ie"); }
		virtual bool Create(CControlUI* pOwner, HWND hParent, const RECT& rc);
		virtual void Destroy();
		virtual void SetPos(const RECT& rc);
		virtual void SetVisible(bool bVisible);
		virtual void Navigate(LPCTSTR url);
		virtual void GoBack();
		virtual void GoForward();
		virtual void Refresh();
		virtual void Stop();
		virtual bool CanGoBack() const;
		virtual bool CanGoForward() const;
		virtual bool GetUrl(CDuiString& out) const;
		virtual void ExecuteScript(LPCTSTR script);
		virtual HWND GetHostWindow() const;
		virtual void* GetNative();
		virtual void SetHostEvents(CWebBrowserHostEvents* pEvents);

		void SetIeEventHandler(CWebBrowserEventHandler* pHandler);
		CWebBrowserIeHost* GetHost() const { return m_pHost; }

	private:
		CWebBrowserIeHost* m_pHost;
		CWebBrowserHostEvents* m_pHostEvents;
	};
}

#endif
