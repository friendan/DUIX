#include "StdAfx.h"
#include "TrayIcon.h"
#include <shobjidl.h>

namespace DuiLib
{
	namespace
	{
		const size_t kTipMax = sizeof(((NOTIFYICONDATA*)0)->szTip) / sizeof(TCHAR);
		const size_t kInfoMax = sizeof(((NOTIFYICONDATA*)0)->szInfo) / sizeof(TCHAR);
		const size_t kInfoTitleMax = sizeof(((NOTIFYICONDATA*)0)->szInfoTitle) / sizeof(TCHAR);

		// 保存 Hide 前的 GWL_EXSTYLE（存 ex+1，避免 ex==0 时 SetProp(NULL) 删属性）
		static LPCTSTR kPropTaskbarExStyle = _T("DuiLib.TaskbarExStyle");
		static LPCTSTR kPropTaskbarHidden = _T("DuiLib.TaskbarHidden");

		static void TaskbarDeleteTab(HWND hWnd)
		{
			if( hWnd == NULL ) return;
			ITaskbarList* pList = NULL;
			if( FAILED(::CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER,
				IID_ITaskbarList, (void**)&pList)) || pList == NULL )
				return;
			if( SUCCEEDED(pList->HrInit()) )
				pList->DeleteTab(hWnd);
			pList->Release();
		}

		static void TaskbarAddTab(HWND hWnd)
		{
			if( hWnd == NULL ) return;
			ITaskbarList* pList = NULL;
			if( FAILED(::CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER,
				IID_ITaskbarList, (void**)&pList)) || pList == NULL )
				return;
			if( SUCCEEDED(pList->HrInit()) )
				pList->AddTab(hWnd);
			pList->Release();
		}

		void CopyBounded(TCHAR* dest, size_t cchDest, LPCTSTR src)
		{
			if( dest == NULL || cchDest == 0 ) return;
			dest[0] = _T('\0');
			if( src == NULL ) return;
#if defined(_MSC_VER) && _MSC_VER >= 1400
			_tcsncpy_s(dest, cchDest, src, _TRUNCATE);
#else
			_tcsncpy(dest, src, cchDest - 1);
			dest[cchDest - 1] = _T('\0');
#endif
		}
	}

	CTrayIcon::CTrayIcon()
	{
		ResetData();
	}

	CTrayIcon::~CTrayIcon()
	{
		DeleteTrayIcon();
	}

	void CTrayIcon::ResetData()
	{
		memset(&m_trayData, 0, sizeof(m_trayData));
		m_bEnabled = false;
		m_bVisible = false;
		m_bOwnIcon = false;
		m_hWnd = NULL;
		m_uMessage = UIMSG_TRAYICON;
		m_uNotifyVersion = 0;
		m_hIcon = NULL;
	}

	void CTrayIcon::ReleaseOwnedIcon()
	{
		if( m_bOwnIcon && m_hIcon != NULL ) {
			::DestroyIcon(m_hIcon);
			m_hIcon = NULL;
			m_bOwnIcon = false;
		}
	}

	void CTrayIcon::CopyTip(LPCTSTR pTip)
	{
		CopyBounded(m_trayData.szTip, kTipMax, pTip);
	}

	void CTrayIcon::CopyInfoTitle(LPCTSTR pTitle)
	{
		CopyBounded(m_trayData.szInfoTitle, kInfoTitleMax, pTitle);
	}

	void CTrayIcon::CopyInfoText(LPCTSTR pText)
	{
		CopyBounded(m_trayData.szInfo, kInfoMax, pText);
	}

	HICON CTrayIcon::LoadIconRes(UINT uRes, HINSTANCE hInst) const
	{
		if( uRes == 0 ) return NULL;
		if( hInst == NULL ) hInst = CPaintManagerUI::GetInstance();
		HICON h = (HICON)::LoadImage(hInst, MAKEINTRESOURCE(uRes), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
		if( h != NULL ) return h;
		// LoadIcon 返回共享图标，CopyIcon 后可安全 Destroy
		HICON shared = ::LoadIcon(hInst, MAKEINTRESOURCE(uRes));
		if( shared == NULL ) return NULL;
		return ::CopyIcon(shared);
	}

	HICON CTrayIcon::LoadIconFile(LPCTSTR pFile) const
	{
		if( pFile == NULL || *pFile == _T('\0') ) return NULL;
		return (HICON)::LoadImage(NULL, pFile, IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
	}

	HICON CTrayIcon::LoadApplicationIcon(HWND hWnd, bool bSmall)
	{
		HICON hShared = NULL;
		if( hWnd != NULL && ::IsWindow(hWnd) ) {
			hShared = (HICON)::SendMessage(hWnd, WM_GETICON, bSmall ? ICON_SMALL : ICON_BIG, 0);
			if( hShared == NULL && bSmall )
				hShared = (HICON)::SendMessage(hWnd, WM_GETICON, ICON_BIG, 0);
			if( hShared == NULL ) {
#ifdef GCLP_HICONSM
				hShared = (HICON)(ULONG_PTR)::GetClassLongPtr(hWnd, bSmall ? GCLP_HICONSM : GCLP_HICON);
				if( hShared == NULL && bSmall )
					hShared = (HICON)(ULONG_PTR)::GetClassLongPtr(hWnd, GCLP_HICON);
#else
				hShared = (HICON)(ULONG_PTR)::GetClassLong(hWnd, bSmall ? GCL_HICONSM : GCL_HICON);
				if( hShared == NULL && bSmall )
					hShared = (HICON)(ULONG_PTR)::GetClassLong(hWnd, GCL_HICON);
#endif
			}
			if( hShared != NULL ) {
				HICON hCopy = ::CopyIcon(hShared);
				if( hCopy != NULL ) return hCopy;
			}
		}

		TCHAR szPath[MAX_PATH] = { 0 };
		HINSTANCE hMod = CPaintManagerUI::GetInstance();
		if( hMod != NULL )
			::GetModuleFileName(hMod, szPath, MAX_PATH);
		if( szPath[0] == _T('\0') )
			::GetModuleFileName(NULL, szPath, MAX_PATH);
		if( szPath[0] == _T('\0') ) return NULL;

		HICON hLarge = NULL;
		HICON hSmallIcon = NULL;
		UINT n = ::ExtractIconEx(szPath, 0, &hLarge, &hSmallIcon, 1);
		if( n == 0 ) {
			// 某些模块 ExtractIconEx 对 index 0 失败时再试 PrivateExtractIcons 风格：ExtractIcon
			HICON h = ::ExtractIcon(hMod != NULL ? hMod : ::GetModuleHandle(NULL), szPath, 0);
			if( h == NULL || h == (HICON)1 ) return NULL;
			return h; // ExtractIcon 返回的可 DestroyIcon
		}
		if( bSmall ) {
			if( hSmallIcon != NULL ) {
				if( hLarge != NULL ) ::DestroyIcon(hLarge);
				return hSmallIcon;
			}
			return hLarge;
		}
		if( hLarge != NULL ) {
			if( hSmallIcon != NULL ) ::DestroyIcon(hSmallIcon);
			return hLarge;
		}
		return hSmallIcon;
	}

	bool CTrayIcon::ApplyAdd()
	{
		if( m_hIcon == NULL && m_hWnd != NULL ) {
			m_hIcon = LoadApplicationIcon(m_hWnd, true);
			m_bOwnIcon = (m_hIcon != NULL);
		}
		if( m_hIcon == NULL )
			return false;
		m_trayData.cbSize = sizeof(NOTIFYICONDATA);
		m_trayData.hWnd = m_hWnd;
		m_trayData.hIcon = m_hIcon;
		m_trayData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		m_trayData.uCallbackMessage = m_uMessage;
		if( !Shell_NotifyIcon(NIM_ADD, &m_trayData) )
			return false;
		m_bEnabled = true;
		m_bVisible = true;
		if( m_uNotifyVersion != 0 )
			SetNotifyVersion(m_uNotifyVersion);
		return true;
	}

	void CTrayIcon::CreateTrayIcon(HWND hRecvWnd, UINT uIconIDResource, LPCTSTR pToolTipText, UINT uMessage)
	{
		if( uIconIDResource == 0 )
			Create(hRecvWnd, 1, pToolTipText, uMessage);
		else
			Create(hRecvWnd, uIconIDResource, uIconIDResource, pToolTipText, uMessage, NULL);
	}

	bool CTrayIcon::Create(HWND hRecvWnd, UINT uId, LPCTSTR pToolTipText, UINT uMessage)
	{
		HICON hIcon = LoadApplicationIcon(hRecvWnd, true);
		if( hIcon == NULL ) return false;
		return Create(hRecvWnd, uId, hIcon, pToolTipText, uMessage, true);
	}

	bool CTrayIcon::Create(HWND hRecvWnd, UINT uId, HICON hIcon, LPCTSTR pToolTipText, UINT uMessage, bool bOwnIcon)
	{
		if( hRecvWnd == NULL || !::IsWindow(hRecvWnd) )
			return false;
		bool bOwn = bOwnIcon;
		if( hIcon == NULL ) {
			hIcon = LoadApplicationIcon(hRecvWnd, true);
			if( hIcon == NULL ) return false;
			bOwn = true;
		}
		DeleteTrayIcon();
		m_hWnd = hRecvWnd;
		if( uMessage != 0 ) m_uMessage = uMessage;
		else m_uMessage = UIMSG_TRAYICON;
		m_hIcon = hIcon;
		m_bOwnIcon = bOwn;
		m_trayData.uID = uId;
		CopyTip(pToolTipText);
		return ApplyAdd();
	}

	bool CTrayIcon::Create(HWND hRecvWnd, UINT uId, UINT uIconResource, LPCTSTR pToolTipText, UINT uMessage, HINSTANCE hInst)
	{
		HICON hIcon = NULL;
		if( uIconResource != 0 )
			hIcon = LoadIconRes(uIconResource, hInst);
		if( hIcon == NULL )
			hIcon = LoadApplicationIcon(hRecvWnd, true);
		if( hIcon == NULL ) return false;
		return Create(hRecvWnd, uId, hIcon, pToolTipText, uMessage, true);
	}

	bool CTrayIcon::CreateFromFile(HWND hRecvWnd, UINT uId, LPCTSTR pIconFile, LPCTSTR pToolTipText, UINT uMessage)
	{
		HICON hIcon = LoadIconFile(pIconFile);
		if( hIcon == NULL )
			hIcon = LoadApplicationIcon(hRecvWnd, true);
		if( hIcon == NULL ) return false;
		return Create(hRecvWnd, uId, hIcon, pToolTipText, uMessage, true);
	}

	void CTrayIcon::DeleteTrayIcon()
	{
		if( m_bEnabled || m_trayData.hWnd != NULL )
			Shell_NotifyIcon(NIM_DELETE, &m_trayData);
		ReleaseOwnedIcon();
		ResetData();
	}

	bool CTrayIcon::Recreate()
	{
		if( m_hWnd == NULL || m_hIcon == NULL ) return false;
		Shell_NotifyIcon(NIM_DELETE, &m_trayData);
		m_bEnabled = false;
		m_bVisible = false;
		return ApplyAdd();
	}

	bool CTrayIcon::SetTooltipText(LPCTSTR pToolTipText)
	{
		CopyTip(pToolTipText);
		if( !m_bEnabled ) return false;
		m_trayData.uFlags = NIF_TIP;
		return Shell_NotifyIcon(NIM_MODIFY, &m_trayData) == TRUE;
	}

	bool CTrayIcon::SetTooltipText(UINT uStringResource, HINSTANCE hInst)
	{
		TCHAR buf[256] = { 0 };
		if( hInst == NULL ) hInst = CPaintManagerUI::GetInstance();
		::LoadString(hInst, uStringResource, buf, 256);
		return SetTooltipText(buf);
	}

	CDuiString CTrayIcon::GetTooltipText() const
	{
		return m_trayData.szTip;
	}

	bool CTrayIcon::SetIcon(HICON hIcon, bool bOwnIcon)
	{
		if( hIcon == NULL ) return false;
		if( hIcon != m_hIcon )
			ReleaseOwnedIcon();
		m_hIcon = hIcon;
		m_bOwnIcon = bOwnIcon;
		m_trayData.hIcon = hIcon;
		if( !m_bEnabled ) return false;
		m_trayData.uFlags = NIF_ICON;
		if( Shell_NotifyIcon(NIM_MODIFY, &m_trayData) != TRUE )
			return false;
		m_bVisible = true;
		return true;
	}

	bool CTrayIcon::SetIcon(LPCTSTR pIconFile)
	{
		HICON hIcon = LoadIconFile(pIconFile);
		if( hIcon == NULL ) return false;
		return SetIcon(hIcon, true);
	}

	bool CTrayIcon::SetIcon(UINT uIconResource, HINSTANCE hInst)
	{
		HICON hIcon = LoadIconRes(uIconResource, hInst);
		if( hIcon == NULL ) return false;
		return SetIcon(hIcon, true);
	}

	bool CTrayIcon::SetIconFromApplication(HWND hWnd)
	{
		if( hWnd == NULL ) hWnd = m_hWnd;
		HICON hIcon = LoadApplicationIcon(hWnd, true);
		if( hIcon == NULL ) return false;
		return SetIcon(hIcon, true);
	}

	HICON CTrayIcon::GetIcon() const
	{
		return m_hIcon;
	}

	bool CTrayIcon::Show()
	{
		if( !m_bEnabled ) {
			if( m_hWnd != NULL && m_hIcon != NULL )
				return ApplyAdd();
			return false;
		}
		if( m_bVisible ) return true;
		m_trayData.uFlags = NIF_STATE;
		m_trayData.dwState = 0;
		m_trayData.dwStateMask = NIS_HIDDEN;
		if( Shell_NotifyIcon(NIM_MODIFY, &m_trayData) ) {
			m_bVisible = true;
			return true;
		}
		return Recreate();
	}

	bool CTrayIcon::Hide()
	{
		if( !m_bEnabled ) return false;
		if( !m_bVisible ) return true;
		m_trayData.uFlags = NIF_STATE;
		m_trayData.dwState = NIS_HIDDEN;
		m_trayData.dwStateMask = NIS_HIDDEN;
		if( Shell_NotifyIcon(NIM_MODIFY, &m_trayData) ) {
			m_bVisible = false;
			return true;
		}
		Shell_NotifyIcon(NIM_DELETE, &m_trayData);
		m_bEnabled = false;
		m_bVisible = false;
		return true;
	}

	void CTrayIcon::SetShowIcon()
	{
		Show();
	}

	void CTrayIcon::SetHideIcon()
	{
		Hide();
	}

	void CTrayIcon::RemoveIcon()
	{
		DeleteTrayIcon();
	}

	bool CTrayIcon::ShowBalloon(LPCTSTR pTitle, LPCTSTR pText, DWORD dwInfoFlags, UINT uTimeoutMs, HICON hBalloonIcon)
	{
		if( !m_bEnabled || !m_bVisible ) return false;
		CopyInfoTitle(pTitle);
		CopyInfoText(pText);
		m_trayData.uFlags = NIF_INFO;
		m_trayData.dwInfoFlags = dwInfoFlags;
		m_trayData.uTimeout = uTimeoutMs;
		if( hBalloonIcon != NULL ) {
			m_trayData.hBalloonIcon = hBalloonIcon;
			m_trayData.dwInfoFlags |= NIIF_USER;
#ifdef NIIF_LARGE_ICON
			m_trayData.dwInfoFlags |= NIIF_LARGE_ICON;
#endif
		}
		return Shell_NotifyIcon(NIM_MODIFY, &m_trayData) == TRUE;
	}

	bool CTrayIcon::HideBalloon()
	{
		if( !m_bEnabled ) return false;
		m_trayData.szInfo[0] = _T('\0');
		m_trayData.uFlags = NIF_INFO;
		return Shell_NotifyIcon(NIM_MODIFY, &m_trayData) == TRUE;
	}

	bool CTrayIcon::SetNotifyVersion(UINT uVersion)
	{
		if( !m_bEnabled ) {
			m_uNotifyVersion = uVersion;
			return false;
		}
		m_trayData.uVersion = uVersion;
		if( !Shell_NotifyIcon(NIM_SETVERSION, &m_trayData) )
			return false;
		m_uNotifyVersion = uVersion;
		return true;
	}

	UINT CTrayIcon::DecodeNotifyMsg(WPARAM wParam, LPARAM lParam, UINT uNotifyVersion)
	{
		if( uNotifyVersion >= NOTIFYICON_VERSION_4 )
			return (UINT)LOWORD(lParam);
		return (UINT)lParam;
	}

	UINT CTrayIcon::DecodeNotifyIconId(WPARAM wParam, LPARAM lParam, UINT uNotifyVersion)
	{
		if( uNotifyVersion >= NOTIFYICON_VERSION_4 )
			return (UINT)HIWORD(lParam);
		return (UINT)wParam;
	}

	POINT CTrayIcon::DecodeNotifyPos(WPARAM wParam, LPARAM /*lParam*/, UINT uNotifyVersion)
	{
		POINT pt = { 0, 0 };
		if( uNotifyVersion >= NOTIFYICON_VERSION_4 ) {
			pt.x = GET_X_LPARAM(wParam);
			pt.y = GET_Y_LPARAM(wParam);
		}
		else {
			::GetCursorPos(&pt);
		}
		return pt;
	}

	UINT CTrayIcon::GetTaskbarCreatedMsg()
	{
		static UINT s_msg = ::RegisterWindowMessage(_T("TaskbarCreated"));
		return s_msg;
	}

	bool CTrayIcon::IsWindowHiddenFromTaskbar(HWND hWnd)
	{
		if( hWnd == NULL || !::IsWindow(hWnd) ) return false;
		return ::GetProp(hWnd, kPropTaskbarHidden) != NULL;
	}

	bool CTrayIcon::RestoreWindowToTaskbarIfNeeded(HWND hWnd)
	{
		if( hWnd == NULL || !::IsWindow(hWnd) ) return false;
		if( ::GetProp(hWnd, kPropTaskbarHidden) == NULL ) return false;

		HANDLE hSaved = ::GetProp(hWnd, kPropTaskbarExStyle);
		if( hSaved != NULL ) {
			LONG_PTR ex = (LONG_PTR)(ULONG_PTR)hSaved - 1;
			::SetWindowLongPtr(hWnd, GWL_EXSTYLE, ex);
		}
		::RemoveProp(hWnd, kPropTaskbarExStyle);
		::RemoveProp(hWnd, kPropTaskbarHidden);

		LONG_PTR ex = ::GetWindowLongPtr(hWnd, GWL_EXSTYLE);
		HWND hOwner = ::GetWindow(hWnd, GW_OWNER);
		if( hOwner == NULL || (ex & WS_EX_APPWINDOW) != 0 )
			TaskbarAddTab(hWnd);
		return true;
	}

	void CTrayIcon::HideWindowFromTaskbar(HWND hWnd)
	{
		if( hWnd == NULL || !::IsWindow(hWnd) ) return;

		if( ::IsIconic(hWnd) )
			::ShowWindow(hWnd, SW_RESTORE);

		if( ::GetProp(hWnd, kPropTaskbarHidden) == NULL ) {
			LONG_PTR ex = ::GetWindowLongPtr(hWnd, GWL_EXSTYLE);
			::SetProp(hWnd, kPropTaskbarExStyle, (HANDLE)(ULONG_PTR)(ex + 1));
			::SetProp(hWnd, kPropTaskbarHidden, (HANDLE)1);
		}

		// 先藏窗，再改样式 / DeleteTab（可见时改 EXSTYLE 在部分系统上任务栏不刷新）
		::ShowWindow(hWnd, SW_HIDE);

		LONG_PTR ex = ::GetWindowLongPtr(hWnd, GWL_EXSTYLE);
		ex |= WS_EX_TOOLWINDOW;
		ex &= ~WS_EX_APPWINDOW;
		::SetWindowLongPtr(hWnd, GWL_EXSTYLE, ex);

		TaskbarDeleteTab(hWnd);
	}

	void CTrayIcon::ShowWindowOnTaskbar(HWND hWnd, bool bActivate)
	{
		if( hWnd == NULL || !::IsWindow(hWnd) ) return;
		RestoreWindowToTaskbarIfNeeded(hWnd);
		::ShowWindow(hWnd, SW_SHOW);
		if( bActivate )
			::SetForegroundWindow(hWnd);
	}
}
