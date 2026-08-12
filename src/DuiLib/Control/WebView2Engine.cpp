#include "StdAfx.h"

#ifdef DUILIB_HAS_WEBVIEW2

#include "WebView2Engine.h"
#include "UIWebBrowser.h"
#include <wrl.h>
#include <wrl/event.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <dcomp.h>
#include <vector>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "dcomp.lib")

using namespace Microsoft::WRL;

namespace DuiLib
{
	namespace
	{
		const TCHAR kCompHostClass[] = _T("DuiLib_WebView2CompHost");
		bool g_bCompHostClassReg = false;

		COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS MouseKeysFromWParam(WPARAM wParam)
		{
			int keys = COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_NONE;
			if( wParam & MK_LBUTTON ) keys |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_LEFT_BUTTON;
			if( wParam & MK_RBUTTON ) keys |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_RIGHT_BUTTON;
			if( wParam & MK_MBUTTON ) keys |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_MIDDLE_BUTTON;
			if( wParam & MK_SHIFT ) keys |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_SHIFT;
			if( wParam & MK_CONTROL ) keys |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_CONTROL;
			if( wParam & MK_XBUTTON1 ) keys |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_X_BUTTON1;
			if( wParam & MK_XBUTTON2 ) keys |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_X_BUTTON2;
			return (COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS)keys;
		}

		bool ReadIStreamToBuffer(IStream* pStream, std::vector<BYTE>& out)
		{
			out.clear();
			if( pStream == NULL ) return false;
			STATSTG st = {};
			if( SUCCEEDED(pStream->Stat(&st, STATFLAG_NONAME)) && st.cbSize.QuadPart > 0
				&& st.cbSize.QuadPart < 8 * 1024 * 1024 ) {
				ULONG n = (ULONG)st.cbSize.QuadPart;
				out.resize(n);
				ULONG read = 0;
				LARGE_INTEGER zero = {};
				pStream->Seek(zero, STREAM_SEEK_SET, NULL);
				if( FAILED(pStream->Read(out.data(), n, &read)) || read == 0 ) {
					out.clear();
					return false;
				}
				if( read < n ) out.resize(read);
				return true;
			}
			BYTE chunk[4096];
			for( ;; ) {
				ULONG read = 0;
				HRESULT hr = pStream->Read(chunk, sizeof(chunk), &read);
				if( read > 0 ) out.insert(out.end(), chunk, chunk + read);
				if( FAILED(hr) || read == 0 ) break;
				if( out.size() > 8 * 1024 * 1024 ) {
					out.clear();
					return false;
				}
			}
			return !out.empty();
		}
	}

	void CWebView2Engine::RegisterCompHostClass()
	{
		if( g_bCompHostClassReg ) return;
		WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wc.lpfnWndProc = CompHostWndProc;
		wc.hInstance = (HINSTANCE)::GetModuleHandle(NULL);
		wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
		wc.lpszClassName = kCompHostClass;
		::RegisterClassEx(&wc);
		g_bCompHostClassReg = true;
	}

	LRESULT CALLBACK CWebView2Engine::CompHostWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		CWebView2Engine* self = reinterpret_cast<CWebView2Engine*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if( self != NULL )
			return self->HandleCompHostMessage(hWnd, uMsg, wParam, lParam);
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	CWebView2Engine::CWebView2Engine()
		: m_pOwner(NULL)
		, m_pFacade(NULL)
		, m_pHostEvents(NULL)
		, m_hParent(NULL)
		, m_hCompHost(NULL)
		, m_bVisible(true)
		, m_bReady(false)
		, m_bCompositionActive(false)
		, m_bOwnDComp(false)
		, m_bTrackingMouse(false)
		, m_sHostMode(_T("window"))
		, m_sEffectiveHost(_T("window"))
		, m_pController(NULL)
		, m_pCompController(NULL)
		, m_pWebView(NULL)
		, m_pDComp(NULL)
		, m_pDCompTarget(NULL)
		, m_pDCompVisual(NULL)
	{
		ZeroMemory(&m_rc, sizeof(m_rc));
		ZeroMemory(&m_tokNavStarting, sizeof(m_tokNavStarting));
		ZeroMemory(&m_tokNavCompleted, sizeof(m_tokNavCompleted));
		ZeroMemory(&m_tokTitleChanged, sizeof(m_tokTitleChanged));
		ZeroMemory(&m_tokNewWindow, sizeof(m_tokNewWindow));
		ZeroMemory(&m_tokCursorChanged, sizeof(m_tokCursorChanged));
		ZeroMemory(&m_tokFaviconChanged, sizeof(m_tokFaviconChanged));
		ZeroMemory(&m_tokHistoryChanged, sizeof(m_tokHistoryChanged));
		ZeroMemory(&m_tokDownloadStarting, sizeof(m_tokDownloadStarting));
	}

	CWebView2Engine::~CWebView2Engine()
	{
		Destroy();
	}

	void CWebView2Engine::SetHostEvents(CWebBrowserHostEvents* pEvents)
	{
		m_pHostEvents = pEvents;
	}

	void CWebView2Engine::SetUserDataFolder(LPCTSTR path)
	{
		m_sUserDataFolder = path ? path : _T("");
	}

	void CWebView2Engine::SetHostMode(LPCTSTR mode)
	{
		if( mode == NULL || *mode == _T('\0') ) {
			m_sHostMode = _T("window");
			return;
		}
		if( _tcsicmp(mode, _T("composition")) == 0 || _tcsicmp(mode, _T("compose")) == 0
			|| _tcsicmp(mode, _T("visual")) == 0 )
			m_sHostMode = _T("composition");
		else
			m_sHostMode = _T("window");
	}

	LPCTSTR CWebView2Engine::GetHostMode() const
	{
		return m_sEffectiveHost.IsEmpty() ? m_sHostMode.GetData() : m_sEffectiveHost.GetData();
	}

	bool CWebView2Engine::WantComposition() const
	{
		return _tcsicmp(m_sHostMode.GetData(), _T("composition")) == 0;
	}

	bool CWebView2Engine::EnsureCompHostWindow()
	{
		if( m_hCompHost != NULL ) return true;
		if( m_hParent == NULL ) return false;
		RegisterCompHostClass();

		int w = m_rc.right - m_rc.left;
		int h = m_rc.bottom - m_rc.top;
		if( w < 1 ) w = 1;
		if( h < 1 ) h = 1;

		m_hCompHost = ::CreateWindowEx(
			0, kCompHostClass, _T(""),
			WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | (m_bVisible ? WS_VISIBLE : 0),
			m_rc.left, m_rc.top, w, h,
			m_hParent, NULL, (HINSTANCE)::GetModuleHandle(NULL), NULL);
		if( m_hCompHost == NULL ) return false;
		::SetWindowLongPtr(m_hCompHost, GWLP_USERDATA, (LONG_PTR)this);
		return true;
	}

	bool CWebView2Engine::EnsureCompVisual()
	{
		if( m_pDCompVisual != NULL ) return true;
		if( m_hCompHost == NULL ) return false;

		HRESULT hr = DCompositionCreateDevice2(NULL, __uuidof(IDCompositionDevice), (void**)&m_pDComp);
		if( FAILED(hr) || m_pDComp == NULL ) {
			m_pDComp = NULL;
			return false;
		}
		m_bOwnDComp = true;

		hr = m_pDComp->CreateTargetForHwnd(m_hCompHost, TRUE, &m_pDCompTarget);
		if( FAILED(hr) || m_pDCompTarget == NULL ) {
			DestroyCompHost();
			return false;
		}
		hr = m_pDComp->CreateVisual(&m_pDCompVisual);
		if( FAILED(hr) || m_pDCompVisual == NULL ) {
			DestroyCompHost();
			return false;
		}
		hr = m_pDCompTarget->SetRoot(m_pDCompVisual);
		if( FAILED(hr) ) {
			DestroyCompHost();
			return false;
		}
		hr = m_pDComp->Commit();
		if( FAILED(hr) ) {
			DestroyCompHost();
			return false;
		}
		return true;
	}

	void CWebView2Engine::DestroyCompHost()
	{
		if( m_pCompController ) {
			if( m_tokCursorChanged.value )
				m_pCompController->remove_CursorChanged(m_tokCursorChanged);
			m_pCompController->put_RootVisualTarget(NULL);
			m_pCompController->Release();
			m_pCompController = NULL;
			ZeroMemory(&m_tokCursorChanged, sizeof(m_tokCursorChanged));
		}
		if( m_pDCompTarget ) {
			m_pDCompTarget->SetRoot(NULL);
			m_pDCompTarget->Release();
			m_pDCompTarget = NULL;
		}
		if( m_pDCompVisual ) {
			m_pDCompVisual->Release();
			m_pDCompVisual = NULL;
		}
		if( m_pDComp && m_bOwnDComp ) {
			m_pDComp->Release();
			m_pDComp = NULL;
		}
		m_bOwnDComp = false;
		if( m_hCompHost ) {
			::SetWindowLongPtr(m_hCompHost, GWLP_USERDATA, 0);
			::DestroyWindow(m_hCompHost);
			m_hCompHost = NULL;
		}
		m_bCompositionActive = false;
		m_bTrackingMouse = false;
	}

	HRESULT CWebView2Engine::StartWindowController(ICoreWebView2Environment* env)
	{
		CWebView2Engine* self = this;
		m_sEffectiveHost = _T("window");
		m_bCompositionActive = false;
		return env->CreateCoreWebView2Controller(
			m_hParent,
			Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
				[self](HRESULT result2, ICoreWebView2Controller* controller) -> HRESULT {
					if( FAILED(result2) || controller == NULL ) return result2;
					self->m_pController = controller;
					self->m_pController->AddRef();
					self->m_pController->get_CoreWebView2(&self->m_pWebView);
					self->OnControllerReady(false);
					return S_OK;
				}).Get());
	}

	HRESULT CWebView2Engine::StartCompositionController(ICoreWebView2Environment* env)
	{
		ComPtr<ICoreWebView2Environment3> env3;
		HRESULT hr = env->QueryInterface(IID_PPV_ARGS(&env3));
		if( FAILED(hr) || !env3 )
			return StartWindowController(env);

		if( !EnsureCompHostWindow() || !EnsureCompVisual() )
			return StartWindowController(env);

		CWebView2Engine* self = this;
		ComPtr<ICoreWebView2Environment> envKeep(env);
		return env3->CreateCoreWebView2CompositionController(
			m_hParent,
			Callback<ICoreWebView2CreateCoreWebView2CompositionControllerCompletedHandler>(
				[self, envKeep](HRESULT result2, ICoreWebView2CompositionController* compController) -> HRESULT {
					if( FAILED(result2) || compController == NULL ) {
						self->DestroyCompHost();
						if( envKeep )
							return self->StartWindowController(envKeep.Get());
						return result2;
					}
					self->m_pCompController = compController;
					self->m_pCompController->AddRef();
					self->m_pCompController->put_RootVisualTarget(self->m_pDCompVisual);
					if( self->m_pDComp ) self->m_pDComp->Commit();

					ComPtr<ICoreWebView2Controller> controller;
					if( SUCCEEDED(compController->QueryInterface(IID_PPV_ARGS(&controller))) && controller ) {
						self->m_pController = controller.Detach();
						self->m_pController->get_CoreWebView2(&self->m_pWebView);
					}

					CWebView2Engine* s2 = self;
					self->m_pCompController->add_CursorChanged(
						Callback<ICoreWebView2CursorChangedEventHandler>(
							[s2](ICoreWebView2CompositionController* sender, IUnknown*) -> HRESULT {
								if( sender == NULL || s2->m_hCompHost == NULL ) return S_OK;
								HCURSOR cur = NULL;
								sender->get_Cursor(&cur);
								if( cur ) ::SetCursor(cur);
								return S_OK;
							}).Get(), &self->m_tokCursorChanged);

					self->m_sEffectiveHost = _T("composition");
					self->m_bCompositionActive = true;
					self->OnControllerReady(true);
					return S_OK;
				}).Get());
	}

	void CWebView2Engine::OnControllerReady(bool /*composition*/)
	{
		m_bReady = true;
		ApplyBounds();
		if( m_pController )
			m_pController->put_IsVisible(m_bVisible ? TRUE : FALSE);
		if( m_hCompHost )
			::ShowWindow(m_hCompHost, m_bVisible ? SW_SHOW : SW_HIDE);
		AttachHandlers();
		FlushPendingNavigate();
		if( m_pFacade ) m_pFacade->ScheduleNativeResizeHook(true);
	}

	bool CWebView2Engine::Create(CControlUI* pOwner, HWND hParent, const RECT& rc)
	{
		if( pOwner == NULL || hParent == NULL ) return false;

		LPWSTR ver = NULL;
		HRESULT hrAvail = GetAvailableCoreWebView2BrowserVersionString(NULL, &ver);
		if( FAILED(hrAvail) || ver == NULL ) {
			if( ver ) CoTaskMemFree(ver);
			return false;
		}
		CoTaskMemFree(ver);

		Destroy();
		m_pOwner = pOwner;
		m_pFacade = static_cast<CWebBrowserUI*>(pOwner->GetInterface(DUI_CTR_WEBBROWSER));
		m_hParent = hParent;
		m_rc = rc;
		m_bReady = false;
		m_sEffectiveHost = m_sHostMode.IsEmpty() ? _T("window") : m_sHostMode;

		LPCWSTR userData = NULL;
		CDuiString folder = m_sUserDataFolder;
		if( folder.IsEmpty() ) {
			TCHAR buf[MAX_PATH] = { 0 };
			if( SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, buf)) ) {
				folder = buf;
				folder += _T("\\DuiLib\\WebView2");
				::CreateDirectory(folder.GetData(), NULL);
			}
		}
#ifdef _UNICODE
		userData = folder.IsEmpty() ? NULL : folder.GetData();
#else
		userData = NULL;
#endif

		CWebView2Engine* self = this;
		const bool wantComp = WantComposition();
		HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
			NULL, userData, NULL,
			Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
				[self, wantComp](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
					if( FAILED(result) || env == NULL || self->m_hParent == NULL )
						return result;
					if( wantComp )
						return self->StartCompositionController(env);
					return self->StartWindowController(env);
				}).Get());

		return SUCCEEDED(hr);
	}

	void CWebView2Engine::AttachHandlers()
	{
		if( m_pWebView == NULL ) return;

		CWebView2Engine* self = this;
		m_pWebView->add_NavigationStarting(
			Callback<ICoreWebView2NavigationStartingEventHandler>(
				[self](ICoreWebView2* /*sender*/, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
					if( self->m_pHostEvents && self->m_pFacade && args ) {
						LPWSTR uri = NULL;
						args->get_Uri(&uri);
						bool cancel = false;
						self->m_pHostEvents->OnNavigationStarting(self->m_pFacade, uri ? uri : L"", &cancel);
						if( uri ) CoTaskMemFree(uri);
						if( cancel ) args->put_Cancel(TRUE);
					}
					return S_OK;
				}).Get(), &m_tokNavStarting);

		m_pWebView->add_NavigationCompleted(
			Callback<ICoreWebView2NavigationCompletedEventHandler>(
				[self](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
					if( self->m_pHostEvents && self->m_pFacade ) {
						BOOL ok = FALSE;
						if( args ) args->get_IsSuccess(&ok);
						LPWSTR uri = NULL;
						if( sender ) sender->get_Source(&uri);
						const wchar_t* pUri = uri ? uri : L"";
						self->m_pHostEvents->OnNavigationCompleted(self->m_pFacade, pUri, ok ? true : false);
						if( !ok && args ) {
							COREWEBVIEW2_WEB_ERROR_STATUS st = COREWEBVIEW2_WEB_ERROR_STATUS_UNKNOWN;
							args->get_WebErrorStatus(&st);
							CDuiString sErr;
							sErr.Format(_T("WebErrorStatus=%d"), (int)st);
							self->m_pHostEvents->OnLoadError(self->m_pFacade, pUri, (int)st, sErr.GetData());
						}
						if( uri ) CoTaskMemFree(uri);
						self->m_pHostEvents->OnHistoryChanged(self->m_pFacade);
					}
					// 同站/回主页时 FaviconChanged 可能不触发，导航完成再拉一次
					self->RequestFavicon();
					return S_OK;
				}).Get(), &m_tokNavCompleted);

		m_pWebView->add_DocumentTitleChanged(
			Callback<ICoreWebView2DocumentTitleChangedEventHandler>(
				[self](ICoreWebView2* sender, IUnknown* /*args*/) -> HRESULT {
					if( self->m_pHostEvents && self->m_pFacade && sender ) {
						LPWSTR title = NULL;
						sender->get_DocumentTitle(&title);
						self->m_pHostEvents->OnDocumentTitleChanged(self->m_pFacade, title ? title : L"");
						if( title ) CoTaskMemFree(title);
					}
					return S_OK;
				}).Get(), &m_tokTitleChanged);

		m_pWebView->add_NewWindowRequested(
			Callback<ICoreWebView2NewWindowRequestedEventHandler>(
				[self](ICoreWebView2* /*sender*/, ICoreWebView2NewWindowRequestedEventArgs* args) -> HRESULT {
					if( self->m_pHostEvents && self->m_pFacade && args ) {
						LPWSTR uri = NULL;
						args->get_Uri(&uri);
						bool handled = false;
						self->m_pHostEvents->OnNewWindowRequested(self->m_pFacade, uri ? uri : L"", &handled);
						if( uri ) CoTaskMemFree(uri);
						if( handled ) args->put_Handled(TRUE);
					}
					return S_OK;
				}).Get(), &m_tokNewWindow);

		m_pWebView->add_HistoryChanged(
			Callback<ICoreWebView2HistoryChangedEventHandler>(
				[self](ICoreWebView2* /*sender*/, IUnknown* /*args*/) -> HRESULT {
					if( self->m_pHostEvents && self->m_pFacade )
						self->m_pHostEvents->OnHistoryChanged(self->m_pFacade);
					return S_OK;
				}).Get(), &m_tokHistoryChanged);

		ComPtr<ICoreWebView2_15> wv15;
		if( SUCCEEDED(m_pWebView->QueryInterface(IID_PPV_ARGS(&wv15))) && wv15 ) {
			wv15->add_FaviconChanged(
				Callback<ICoreWebView2FaviconChangedEventHandler>(
					[self](ICoreWebView2* /*sender*/, IUnknown* /*args*/) -> HRESULT {
						self->RequestFavicon();
						return S_OK;
					}).Get(), &m_tokFaviconChanged);
			RequestFavicon();
		}

		ComPtr<ICoreWebView2_4> wv4;
		if( SUCCEEDED(m_pWebView->QueryInterface(IID_PPV_ARGS(&wv4))) && wv4 ) {
			wv4->add_DownloadStarting(
				Callback<ICoreWebView2DownloadStartingEventHandler>(
					[self](ICoreWebView2* /*sender*/, ICoreWebView2DownloadStartingEventArgs* args) -> HRESULT {
						if( !self->m_pHostEvents || !self->m_pFacade || !args ) return S_OK;
						LPWSTR uri = NULL;
						LPWSTR path = NULL;
						ComPtr<ICoreWebView2DownloadOperation> op;
						if( SUCCEEDED(args->get_DownloadOperation(&op)) && op ) {
							op->get_Uri(&uri);
							op->get_ResultFilePath(&path);
						}
						if( path == NULL )
							args->get_ResultFilePath(&path);
						bool cancel = false;
						self->m_pHostEvents->OnDownloadStarting(
							self->m_pFacade,
							uri ? uri : L"",
							path ? path : L"",
							&cancel);
						if( cancel ) args->put_Cancel(TRUE);
						if( uri ) CoTaskMemFree(uri);
						if( path ) CoTaskMemFree(path);
						return S_OK;
					}).Get(), &m_tokDownloadStarting);
		}
	}

	void CWebView2Engine::RequestFavicon()
	{
		if( m_pWebView == NULL || m_pHostEvents == NULL || m_pFacade == NULL ) return;
		ComPtr<ICoreWebView2_15> wv15;
		if( FAILED(m_pWebView->QueryInterface(IID_PPV_ARGS(&wv15))) || !wv15 ) return;

		CWebView2Engine* self = this;
		wv15->GetFavicon(COREWEBVIEW2_FAVICON_IMAGE_FORMAT_PNG,
			Callback<ICoreWebView2GetFaviconCompletedHandler>(
				[self](HRESULT errorCode, IStream* stream) -> HRESULT {
					if( self->m_pHostEvents == NULL || self->m_pFacade == NULL ) return S_OK;
					if( FAILED(errorCode) || stream == NULL ) {
						self->m_pHostEvents->OnFaviconChanged(self->m_pFacade, NULL, 0);
						return S_OK;
					}
					std::vector<BYTE> buf;
					if( !ReadIStreamToBuffer(stream, buf) || buf.empty() ) {
						self->m_pHostEvents->OnFaviconChanged(self->m_pFacade, NULL, 0);
						return S_OK;
					}
					self->m_pHostEvents->OnFaviconChanged(self->m_pFacade, buf.data(), (DWORD)buf.size());
					return S_OK;
				}).Get());
	}

	void CWebView2Engine::Destroy()
	{
		if( m_pWebView ) {
			if( m_tokNavStarting.value ) m_pWebView->remove_NavigationStarting(m_tokNavStarting);
			if( m_tokNavCompleted.value ) m_pWebView->remove_NavigationCompleted(m_tokNavCompleted);
			if( m_tokTitleChanged.value ) m_pWebView->remove_DocumentTitleChanged(m_tokTitleChanged);
			if( m_tokNewWindow.value ) m_pWebView->remove_NewWindowRequested(m_tokNewWindow);
			if( m_tokHistoryChanged.value ) m_pWebView->remove_HistoryChanged(m_tokHistoryChanged);
			if( m_tokDownloadStarting.value ) {
				ComPtr<ICoreWebView2_4> wv4;
				if( SUCCEEDED(m_pWebView->QueryInterface(IID_PPV_ARGS(&wv4))) && wv4 )
					wv4->remove_DownloadStarting(m_tokDownloadStarting);
			}
			if( m_tokFaviconChanged.value ) {
				ComPtr<ICoreWebView2_15> wv15;
				if( SUCCEEDED(m_pWebView->QueryInterface(IID_PPV_ARGS(&wv15))) && wv15 )
					wv15->remove_FaviconChanged(m_tokFaviconChanged);
			}
			m_pWebView->Release();
			m_pWebView = NULL;
		}
		// composition：先摘 Visual，再 Close Controller
		if( m_pCompController ) {
			if( m_tokCursorChanged.value )
				m_pCompController->remove_CursorChanged(m_tokCursorChanged);
			m_pCompController->put_RootVisualTarget(NULL);
			ZeroMemory(&m_tokCursorChanged, sizeof(m_tokCursorChanged));
		}
		if( m_pController ) {
			m_pController->Close();
			m_pController->Release();
			m_pController = NULL;
		}
		if( m_pCompController ) {
			m_pCompController->Release();
			m_pCompController = NULL;
		}
		if( m_pDCompTarget ) {
			m_pDCompTarget->SetRoot(NULL);
			m_pDCompTarget->Release();
			m_pDCompTarget = NULL;
		}
		if( m_pDCompVisual ) {
			m_pDCompVisual->Release();
			m_pDCompVisual = NULL;
		}
		if( m_pDComp && m_bOwnDComp ) {
			m_pDComp->Release();
		}
		m_pDComp = NULL;
		m_bOwnDComp = false;
		if( m_hCompHost ) {
			::SetWindowLongPtr(m_hCompHost, GWLP_USERDATA, 0);
			::DestroyWindow(m_hCompHost);
			m_hCompHost = NULL;
		}
		m_bCompositionActive = false;
		m_bTrackingMouse = false;
		m_bReady = false;
		m_hParent = NULL;
		m_pOwner = NULL;
		m_pFacade = NULL;
		ZeroMemory(&m_tokNavStarting, sizeof(m_tokNavStarting));
		ZeroMemory(&m_tokNavCompleted, sizeof(m_tokNavCompleted));
		ZeroMemory(&m_tokTitleChanged, sizeof(m_tokTitleChanged));
		ZeroMemory(&m_tokNewWindow, sizeof(m_tokNewWindow));
		ZeroMemory(&m_tokFaviconChanged, sizeof(m_tokFaviconChanged));
		ZeroMemory(&m_tokHistoryChanged, sizeof(m_tokHistoryChanged));
		ZeroMemory(&m_tokDownloadStarting, sizeof(m_tokDownloadStarting));
	}

	void CWebView2Engine::ApplyBounds()
	{
		if( m_bCompositionActive && m_hCompHost != NULL ) {
			int w = m_rc.right - m_rc.left;
			int h = m_rc.bottom - m_rc.top;
			if( w < 1 ) w = 1;
			if( h < 1 ) h = 1;
			::SetWindowPos(m_hCompHost, NULL, m_rc.left, m_rc.top, w, h,
				SWP_NOZORDER | SWP_NOACTIVATE);
			if( m_pController ) {
				RECT local = { 0, 0, w, h };
				m_pController->put_Bounds(local);
			}
			if( m_pDComp ) m_pDComp->Commit();
			return;
		}
		if( m_pController == NULL ) return;
		RECT rc = m_rc;
		m_pController->put_Bounds(rc);
	}

	void CWebView2Engine::SetPos(const RECT& rc)
	{
		m_rc = rc;
		ApplyBounds();
	}

	void CWebView2Engine::SetVisible(bool bVisible)
	{
		m_bVisible = bVisible;
		if( m_pController ) m_pController->put_IsVisible(bVisible ? TRUE : FALSE);
		if( m_hCompHost ) ::ShowWindow(m_hCompHost, bVisible ? SW_SHOW : SW_HIDE);
	}

	void CWebView2Engine::FlushPendingNavigate()
	{
		if( !m_bReady || m_pWebView == NULL || m_sPendingUrl.IsEmpty() ) return;
		CDuiString url = m_sPendingUrl;
		m_sPendingUrl.Empty();
		Navigate(url.GetData());
	}

	void CWebView2Engine::Navigate(LPCTSTR url)
	{
		if( url == NULL || *url == _T('\0') ) return;
		if( !m_bReady || m_pWebView == NULL ) {
			m_sPendingUrl = url;
			return;
		}
#ifdef _UNICODE
		m_pWebView->Navigate(url);
#else
		CA2W w(url);
		m_pWebView->Navigate(w);
#endif
	}

	void CWebView2Engine::GoBack()
	{
		if( m_pWebView ) {
			BOOL can = FALSE;
			m_pWebView->get_CanGoBack(&can);
			if( can ) m_pWebView->GoBack();
		}
	}

	void CWebView2Engine::GoForward()
	{
		if( m_pWebView ) {
			BOOL can = FALSE;
			m_pWebView->get_CanGoForward(&can);
			if( can ) m_pWebView->GoForward();
		}
	}

	bool CWebView2Engine::CanGoBack() const
	{
		if( m_pWebView == NULL ) return false;
		BOOL can = FALSE;
		m_pWebView->get_CanGoBack(&can);
		return can ? true : false;
	}

	bool CWebView2Engine::CanGoForward() const
	{
		if( m_pWebView == NULL ) return false;
		BOOL can = FALSE;
		m_pWebView->get_CanGoForward(&can);
		return can ? true : false;
	}

	void CWebView2Engine::Refresh()
	{
		if( m_pWebView ) m_pWebView->Reload();
	}

	void CWebView2Engine::Stop()
	{
		if( m_pWebView ) m_pWebView->Stop();
	}

	void CWebView2Engine::OpenDevToolsWindow()
	{
		if( m_pWebView ) m_pWebView->OpenDevToolsWindow();
	}

	bool CWebView2Engine::GetUrl(CDuiString& out) const
	{
		out.Empty();
		if( m_pWebView == NULL ) return false;
		LPWSTR uri = NULL;
		if( FAILED(m_pWebView->get_Source(&uri)) || uri == NULL ) return false;
		out = uri;
		CoTaskMemFree(uri);
		m_sCachedUrl = out;
		return !out.IsEmpty();
	}

	void CWebView2Engine::ExecuteScript(LPCTSTR script)
	{
		if( m_pWebView == NULL || script == NULL || *script == _T('\0') ) return;
		CWebView2Engine* self = this;
#ifdef _UNICODE
		LPCWSTR js = script;
#else
		CA2W w(script);
		LPCWSTR js = w;
#endif
		m_pWebView->ExecuteScript(js,
			Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
				[self](HRESULT errorCode, LPCWSTR resultObjectAsJson) -> HRESULT {
					if( self->m_pHostEvents && self->m_pFacade ) {
						bool ok = SUCCEEDED(errorCode);
#ifdef _UNICODE
						self->m_pHostEvents->OnExecuteScriptResult(
							self->m_pFacade, resultObjectAsJson, ok);
#else
						CDuiString s;
						if( resultObjectAsJson ) s = CDuiString(resultObjectAsJson);
						self->m_pHostEvents->OnExecuteScriptResult(
							self->m_pFacade, s.IsEmpty() ? NULL : s.GetData(), ok);
#endif
					}
					return S_OK;
				}).Get());
	}

	HWND CWebView2Engine::GetHostWindow() const
	{
		if( m_hCompHost ) return m_hCompHost;
		return m_hParent;
	}

	void* CWebView2Engine::GetNative()
	{
		return (void*)m_pWebView;
	}

	void CWebView2Engine::ForwardMouse(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if( m_pCompController == NULL ) return;

		COREWEBVIEW2_MOUSE_EVENT_KIND kind;
		UINT mouseData = 0;
		switch( uMsg ) {
		case WM_MOUSEMOVE: kind = COREWEBVIEW2_MOUSE_EVENT_KIND_MOVE; break;
		case WM_LBUTTONDOWN: kind = COREWEBVIEW2_MOUSE_EVENT_KIND_LEFT_BUTTON_DOWN; break;
		case WM_LBUTTONUP: kind = COREWEBVIEW2_MOUSE_EVENT_KIND_LEFT_BUTTON_UP; break;
		case WM_LBUTTONDBLCLK: kind = COREWEBVIEW2_MOUSE_EVENT_KIND_LEFT_BUTTON_DOUBLE_CLICK; break;
		case WM_RBUTTONDOWN: kind = COREWEBVIEW2_MOUSE_EVENT_KIND_RIGHT_BUTTON_DOWN; break;
		case WM_RBUTTONUP: kind = COREWEBVIEW2_MOUSE_EVENT_KIND_RIGHT_BUTTON_UP; break;
		case WM_RBUTTONDBLCLK: kind = COREWEBVIEW2_MOUSE_EVENT_KIND_RIGHT_BUTTON_DOUBLE_CLICK; break;
		case WM_MBUTTONDOWN: kind = COREWEBVIEW2_MOUSE_EVENT_KIND_MIDDLE_BUTTON_DOWN; break;
		case WM_MBUTTONUP: kind = COREWEBVIEW2_MOUSE_EVENT_KIND_MIDDLE_BUTTON_UP; break;
		case WM_MBUTTONDBLCLK: kind = COREWEBVIEW2_MOUSE_EVENT_KIND_MIDDLE_BUTTON_DOUBLE_CLICK; break;
		case WM_MOUSEWHEEL:
			kind = COREWEBVIEW2_MOUSE_EVENT_KIND_WHEEL;
			mouseData = GET_WHEEL_DELTA_WPARAM(wParam);
			break;
		case WM_MOUSEHWHEEL:
			kind = COREWEBVIEW2_MOUSE_EVENT_KIND_HORIZONTAL_WHEEL;
			mouseData = GET_WHEEL_DELTA_WPARAM(wParam);
			break;
		case WM_XBUTTONDOWN: kind = COREWEBVIEW2_MOUSE_EVENT_KIND_X_BUTTON_DOWN; break;
		case WM_XBUTTONUP: kind = COREWEBVIEW2_MOUSE_EVENT_KIND_X_BUTTON_UP; break;
		case WM_MOUSELEAVE: kind = COREWEBVIEW2_MOUSE_EVENT_KIND_LEAVE; break;
		default: return;
		}

		POINT pt = { 0, 0 };
		WPARAM keySrc = wParam;
		if( uMsg == WM_MOUSEWHEEL || uMsg == WM_MOUSEHWHEEL ) {
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
			::ScreenToClient(m_hCompHost, &pt);
			keySrc = GET_KEYSTATE_WPARAM(wParam);
		}
		else if( uMsg != WM_MOUSELEAVE ) {
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
		}

		m_pCompController->SendMouseInput(kind, MouseKeysFromWParam(keySrc), mouseData, pt);
	}

	LRESULT CWebView2Engine::HandleCompHostMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch( uMsg ) {
		case WM_NCHITTEST:
			if( m_pFacade != NULL ) {
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				if( m_pFacade->HitNativeHostResize(pt) != HTCLIENT )
					return HTTRANSPARENT;
			}
			break;
		case WM_SETCURSOR:
			if( m_pCompController ) {
				HCURSOR cur = NULL;
				m_pCompController->get_Cursor(&cur);
				if( cur ) {
					::SetCursor(cur);
					return TRUE;
				}
			}
			break;
		case WM_MOUSEMOVE:
			if( !m_bTrackingMouse ) {
				TRACKMOUSEEVENT tme = { sizeof(tme) };
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hWnd;
				::TrackMouseEvent(&tme);
				m_bTrackingMouse = true;
			}
			ForwardMouse(uMsg, wParam, lParam);
			return 0;
		case WM_MOUSELEAVE:
			m_bTrackingMouse = false;
			ForwardMouse(uMsg, wParam, lParam);
			return 0;
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDBLCLK:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL:
			if( uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN || uMsg == WM_MBUTTONDOWN )
				::SetFocus(hWnd);
			ForwardMouse(uMsg, wParam, lParam);
			return 0;
		case WM_ERASEBKGND:
			return 1;
		case WM_PAINT: {
			PAINTSTRUCT ps;
			::BeginPaint(hWnd, &ps);
			::EndPaint(hWnd, &ps);
			return 0;
		}
		default:
			break;
		}
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

#endif // DUILIB_HAS_WEBVIEW2
