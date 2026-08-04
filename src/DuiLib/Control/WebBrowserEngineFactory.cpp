#include "StdAfx.h"
#include "IWebBrowserEngine.h"
#include "WebBrowserIeEngine.h"
#include "WebBrowserCefEngine.h"
#ifdef DUILIB_HAS_WEBVIEW2
#include "WebView2Engine.h"
#endif

namespace DuiLib
{
	static IWebBrowserEngine* CreateIeEngine() { return new CWebBrowserIeEngine(); }
	static IWebBrowserEngine* CreateCefEngine() { return new CWebBrowserCefEngine(); }
#ifdef DUILIB_HAS_WEBVIEW2
	static IWebBrowserEngine* CreateWebView2Engine() { return new CWebView2Engine(); }
#endif

	CWebBrowserEngineFactory& CWebBrowserEngineFactory::Instance()
	{
		static CWebBrowserEngineFactory s;
		return s;
	}

	CWebBrowserEngineFactory::CWebBrowserEngineFactory()
		: m_bBuiltins(false)
	{
	}

	void CWebBrowserEngineFactory::EnsureBuiltinEngines()
	{
		if( m_bBuiltins ) return;
		m_bBuiltins = true;
		Register(_T("ie"), CreateIeEngine);
		Register(_T("cef"), CreateCefEngine);
#ifdef DUILIB_HAS_WEBVIEW2
		Register(_T("webview2"), CreateWebView2Engine);
#endif
	}

	void CWebBrowserEngineFactory::Register(LPCTSTR name, WebBrowserEngineCreator fn)
	{
		if( name == NULL || *name == _T('\0') || fn == NULL ) return;
		m_map[CDuiString(name)] = fn;
	}

	IWebBrowserEngine* CWebBrowserEngineFactory::Create(LPCTSTR name) const
	{
		const_cast<CWebBrowserEngineFactory*>(this)->EnsureBuiltinEngines();
		if( name == NULL || *name == _T('\0') ) return NULL;
		std::map<CDuiString, WebBrowserEngineCreator>::const_iterator found = m_map.find(CDuiString(name));
		if( found == m_map.end() || found->second == NULL ) return NULL;
		return found->second();
	}

	bool CWebBrowserEngineFactory::IsRegistered(LPCTSTR name) const
	{
		const_cast<CWebBrowserEngineFactory*>(this)->EnsureBuiltinEngines();
		if( name == NULL || *name == _T('\0') ) return false;
		return m_map.find(CDuiString(name)) != m_map.end();
	}
}
