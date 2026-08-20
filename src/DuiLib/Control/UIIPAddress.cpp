#include "StdAfx.h"
#pragma comment( lib, "ws2_32.lib" )

DWORD GetLocalIpAddress()   
{   
	WORD wVersionRequested = MAKEWORD(2, 2);   
	WSADATA wsaData;   
	if (WSAStartup(wVersionRequested, &wsaData) != 0)   
		return 0;   
	char local[255] = {0};   
	gethostname(local, sizeof(local));   
	hostent* ph = gethostbyname(local);   
	if (ph == NULL)   
		return 0;   
	in_addr addr;   
	memcpy(&addr, ph->h_addr_list[0], sizeof(in_addr));   
	DWORD dwIP = MAKEIPADDRESS(addr.S_un.S_un_b.s_b1, addr.S_un.S_un_b.s_b2, addr.S_un.S_un_b.s_b3, addr.S_un.S_un_b.s_b4);
	return dwIP;
}

namespace DuiLib
{
	//CDateTimeUI::m_nDTUpdateFlag
#define IP_NONE   0
#define IP_UPDATE 1
#define IP_DELETE 2
#define IP_KEEP   3

	class CIPAddressWnd : public CWindowWnd
	{
	public:
		CIPAddressWnd();

		void Init(CIPAddressUI* pOwner);
		RECT CalPos();
		void RefreshColors();
		void CloseAndDetach();

		LPCTSTR GetWindowClassName() const;
		LPCTSTR GetSuperClassName() const;
		void OnFinalMessage(HWND hWnd);

		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnCtlColor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	protected:
		static void DisableVisualTheme(HWND hWnd);
		static BOOL CALLBACK EnumDisableVisualTheme(HWND hWnd, LPARAM lParam);

		CIPAddressUI* m_pOwner;
		HBRUSH m_hBkBrush;
		DWORD m_dwBrushColor;
		bool m_bInit;
	};

	CIPAddressWnd::CIPAddressWnd() : m_pOwner(NULL), m_hBkBrush(NULL), m_dwBrushColor(0), m_bInit(false)
	{
	}

	void CIPAddressWnd::DisableVisualTheme(HWND hWnd)
	{
		if( hWnd == NULL ) return;
		typedef HRESULT (WINAPI *PFNSetWindowTheme)(HWND, LPCWSTR, LPCWSTR);
		HMODULE hMod = ::GetModuleHandle(_T("uxtheme.dll"));
		if( hMod == NULL ) hMod = ::LoadLibrary(_T("uxtheme.dll"));
		if( hMod == NULL ) return;
		PFNSetWindowTheme pfn = (PFNSetWindowTheme)::GetProcAddress(hMod, "SetWindowTheme");
		if( pfn != NULL ) pfn(hWnd, L"", L"");
	}

	BOOL CALLBACK CIPAddressWnd::EnumDisableVisualTheme(HWND hWnd, LPARAM /*lParam*/)
	{
		DisableVisualTheme(hWnd);
		return TRUE;
	}

	void CIPAddressWnd::Init(CIPAddressUI* pOwner)
	{
		m_pOwner = pOwner;
		m_pOwner->m_nIPUpdateFlag = IP_NONE;

		if (m_hWnd == NULL)
		{
			INITCOMMONCONTROLSEX   CommCtrl;
			CommCtrl.dwSize=sizeof(CommCtrl);
			CommCtrl.dwICC=ICC_INTERNET_CLASSES;//指定Class
			if(InitCommonControlsEx(&CommCtrl))
			{
				RECT rcPos = CalPos();
				UINT uStyle = WS_CHILD | WS_TABSTOP | WS_GROUP;
				Create(m_pOwner->GetManager()->GetPaintWindow(), NULL, uStyle, 0, rcPos);
				// 关闭视觉样式，否则 SysIPAddress32 子 Edit 常忽略 CTLCOLOR 仍画系统白底
				DisableVisualTheme(m_hWnd);
				::EnumChildWindows(m_hWnd, EnumDisableVisualTheme, 0);
			}
			SetWindowFont(m_hWnd, m_pOwner->GetManager()->GetFontInfo(m_pOwner->GetFont())->hFont, TRUE);
		}

		if (m_pOwner->GetText().IsEmpty())
			m_pOwner->m_dwIP = GetLocalIpAddress();
		::SendMessage(m_hWnd, IPM_SETADDRESS, 0, m_pOwner->m_dwIP);
		RefreshColors();
		::ShowWindow(m_hWnd, SW_SHOW);
		::SetFocus(m_hWnd);

		m_bInit = true;    
	}

	RECT CIPAddressWnd::CalPos()
	{
		CDuiRect rcPos = m_pOwner->GetPos();
		return rcPos;
	}

	void CIPAddressWnd::RefreshColors()
	{
		if( m_hBkBrush != NULL ) {
			::DeleteObject(m_hBkBrush);
			m_hBkBrush = NULL;
		}
		m_dwBrushColor = 0;
		if( m_hWnd != NULL ) {
			::InvalidateRect(m_hWnd, NULL, TRUE);
			HWND hChild = ::GetWindow(m_hWnd, GW_CHILD);
			while( hChild != NULL ) {
				::InvalidateRect(hChild, NULL, TRUE);
				hChild = ::GetWindow(hChild, GW_HWNDNEXT);
			}
		}
	}

	LPCTSTR CIPAddressWnd::GetWindowClassName() const
	{
		return _T("IPAddressWnd");
	}

	LPCTSTR CIPAddressWnd::GetSuperClassName() const
	{
		return WC_IPADDRESS;
	}

	void CIPAddressWnd::CloseAndDetach()
	{
		m_bInit = false;
		if( m_pOwner != NULL && m_pOwner->m_pWindow == this )
			m_pOwner->m_pWindow = NULL;
		m_pOwner = NULL;
		if( ::IsWindow(m_hWnd) )
			::DestroyWindow(m_hWnd);
		else
			delete this;
	}

	void CIPAddressWnd::OnFinalMessage(HWND /*hWnd*/)
	{
		if( m_hBkBrush != NULL ) {
			::DeleteObject(m_hBkBrush);
			m_hBkBrush = NULL;
		}
		if( m_pOwner != NULL && m_pOwner->m_pWindow == this )
			m_pOwner->m_pWindow = NULL;
		m_pOwner = NULL;
		delete this;
	}

	LRESULT CIPAddressWnd::OnCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = TRUE;
		if( m_pOwner == NULL ) return (LRESULT)::GetStockObject(NULL_BRUSH);
		HDC hDC = (HDC)wParam;
		DWORD dwText = m_pOwner->GetNativeColor();
		DWORD clrColor = m_pOwner->GetNativeBackgroundColor();
		::SetBkMode(hDC, TRANSPARENT);
		::SetTextColor(hDC, DuiColorToCOLORREF(dwText));
		::SetBkColor(hDC, DuiColorToCOLORREF(clrColor));
		if( m_hBkBrush == NULL || m_dwBrushColor != clrColor ) {
			if( m_hBkBrush != NULL ) ::DeleteObject(m_hBkBrush);
			m_hBkBrush = ::CreateSolidBrush(DuiColorToCOLORREF(clrColor));
			m_dwBrushColor = clrColor;
		}
		return (LRESULT)m_hBkBrush;
	}

	LRESULT CIPAddressWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		if( uMsg == WM_KILLFOCUS )
		{
			bHandled = TRUE;
			return 0;
			lRes = OnKillFocus(uMsg, wParam, lParam, bHandled);
		}
		else if (uMsg == WM_KEYUP && (wParam == VK_DELETE || wParam == VK_BACK))
		{
			LRESULT lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
			if( m_pOwner != NULL ) {
				m_pOwner->m_nIPUpdateFlag = IP_DELETE;
				m_pOwner->UpdateText();
			}
			PostMessage(WM_CLOSE);
			return lRes;
		}
		else if (uMsg == WM_KEYUP && wParam == VK_ESCAPE)
		{
			LRESULT lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
			if( m_pOwner != NULL )
				m_pOwner->m_nIPUpdateFlag = IP_KEEP;
			PostMessage(WM_CLOSE);
			return lRes;
		}
		else if( uMsg == OCM_COMMAND ) {
			if( GET_WM_COMMAND_CMD(wParam, lParam) == EN_KILLFOCUS )
			{
				lRes = OnKillFocus(uMsg, wParam, lParam, bHandled);
			}
		}
		else if( uMsg == WM_CTLCOLOREDIT || uMsg == WM_CTLCOLORSTATIC
			|| uMsg == OCM__BASE + WM_CTLCOLOREDIT || uMsg == OCM__BASE + WM_CTLCOLORSTATIC )
		{
			lRes = OnCtlColor(uMsg, wParam, lParam, bHandled);
		}
		else if( uMsg == WM_ERASEBKGND )
		{
			if( m_pOwner == NULL ) return 1;
			RECT rc = { 0 };
			::GetClientRect(m_hWnd, &rc);
			DWORD clrColor = m_pOwner->GetNativeBackgroundColor();
			if( m_hBkBrush == NULL || m_dwBrushColor != clrColor ) {
				if( m_hBkBrush != NULL ) ::DeleteObject(m_hBkBrush);
				m_hBkBrush = ::CreateSolidBrush(DuiColorToCOLORREF(clrColor));
				m_dwBrushColor = clrColor;
			}
			::FillRect((HDC)wParam, &rc, m_hBkBrush);
			return 1;
		}
		else bHandled = FALSE;
		if( !bHandled ) return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
		return lRes;
	}

	LRESULT CIPAddressWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		HWND hWndFocus = GetFocus();
		while (hWndFocus)
		{
			if (GetFocus() == m_hWnd)
			{
				bHandled = TRUE;
				return 0;
			}
			hWndFocus = GetParent(hWndFocus);
		}

		LRESULT lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
		if( m_pOwner != NULL && m_pOwner->m_nIPUpdateFlag == IP_NONE )
		{
			::SendMessage(m_hWnd, IPM_GETADDRESS, 0, (LPARAM)&m_pOwner->m_dwIP);
			m_pOwner->m_nIPUpdateFlag = IP_UPDATE;
			m_pOwner->UpdateText();
		}
		::ShowWindow(m_hWnd, SW_HIDE);
		return lRes;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	IMPLEMENT_DUICONTROL(CIPAddressUI)

		CIPAddressUI::CIPAddressUI()
	{
		m_dwIP = GetLocalIpAddress();
		m_bReadOnly = false;
		m_pWindow = NULL;
		m_nIPUpdateFlag=IP_UPDATE;
		m_dwNativeBkColor = 0xFFFFFFFF;
		m_dwNativeTextColor = 0x000000E0;
		m_bNativeBkColorCustom = false;
		m_bNativeTextColorSet = false;
		UpdateText();
		m_nIPUpdateFlag = IP_NONE;
	}

	CIPAddressUI::~CIPAddressUI()
	{
		if( m_pManager != NULL && m_pManager->GetFocus() == this )
			m_pManager->ReapObjects(this);
		if( m_pWindow != NULL ) {
			CIPAddressWnd* pWnd = m_pWindow;
			m_pWindow = NULL;
			pWnd->CloseAndDetach();
		}
	}

	LPCTSTR CIPAddressUI::GetClass() const
	{
		return _T("IPAddressUI");
	}

	LPVOID CIPAddressUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, DUI_CTR_IPADDRESS) == 0 ) return static_cast<CIPAddressUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	DWORD CIPAddressUI::GetIP()
	{
		return m_dwIP;
	}

	void CIPAddressUI::SetIP(DWORD dwIP)
	{
		m_dwIP = dwIP;
		UpdateText();
	}

	void CIPAddressUI::SetReadOnly(bool bReadOnly)
	{
		m_bReadOnly = bReadOnly;
		Invalidate();
	}

	bool CIPAddressUI::IsReadOnly() const
	{
		return m_bReadOnly;
	}

	void CIPAddressUI::SetNativeBackgroundColor(DWORD dwBackgroundColor)
	{
		m_dwNativeBkColor = dwBackgroundColor;
		m_bNativeBkColorCustom = true;
		SyncNativeShellColors();
	}

	DWORD CIPAddressUI::GetNativeBackgroundColor() const
	{
		if( m_bNativeBkColorCustom ) return m_dwNativeBkColor;
		return GetBackgroundColor();
	}

	void CIPAddressUI::SetNativeColor(DWORD dwColor)
	{
		m_dwNativeTextColor = dwColor;
		m_bNativeTextColorSet = true;
		SyncNativeShellColors();
	}

	DWORD CIPAddressUI::GetNativeColor() const
	{
		if( m_bNativeTextColorSet ) return m_dwNativeTextColor;
		return GetColor();
	}

	void CIPAddressUI::SyncNativeShellColors()
	{
		if( m_pWindow != NULL ) m_pWindow->RefreshColors();
	}

	void CIPAddressUI::UpdateText()
	{
		if (m_nIPUpdateFlag == IP_DELETE)
			SetText(_T(""));
		else if (m_nIPUpdateFlag == IP_UPDATE)
		{
			TCHAR szIP[MAX_PATH] = {0};
			in_addr addr;
			addr.S_un.S_addr = m_dwIP;
			_stprintf(szIP, _T("%d.%d.%d.%d"), addr.S_un.S_un_b.s_b4, addr.S_un.S_un_b.s_b3, addr.S_un.S_un_b.s_b2, addr.S_un.S_un_b.s_b1);
			SetText(szIP);
		}
	}

	void CIPAddressUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( !BubbleEvent(event) ) CLabelUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETCURSOR && IsEnabled() )
		{
			::SetCursor(::LoadCursor(NULL, IDC_IBEAM));
			return;
		}
		if( event.Type == UIEVENT_WINDOWSIZE )
		{
			if( m_pWindow != NULL ) m_pManager->SetFocusNeeded(this);
		}
		if( event.Type == UIEVENT_SCROLLWHEEL )
		{
			if( m_pWindow != NULL ) return;
		}
		if( event.Type == UIEVENT_SETFOCUS && IsEnabled() ) 
		{
			if( m_pWindow ) 
			{
				return;
			}
			if( m_pParent == NULL || m_pManager == NULL || m_pManager->GetFocus() != this ) {
				Invalidate();
				return;
			}
			m_pWindow = new CIPAddressWnd();
			ASSERT(m_pWindow);
			m_pWindow->Init(this);
			m_pWindow->ShowWindow();
		}
		if( event.Type == UIEVENT_KILLFOCUS && IsEnabled() ) 
		{
			Invalidate();
		}
		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK || event.Type == UIEVENT_RBUTTONDOWN) 
		{
			if( IsEnabled() ) {
				GetManager()->ReleaseCapture();
				if( IsFocused() && m_pWindow == NULL )
				{
					m_pWindow = new CIPAddressWnd();
					ASSERT(m_pWindow);
				}
				if( m_pWindow != NULL )
				{
					m_pWindow->Init(this);
					m_pWindow->ShowWindow();
				}
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEMOVE ) 
		{
			return;
		}
		if( event.Type == UIEVENT_BUTTONUP ) 
		{
			return;
		}
		if( event.Type == UIEVENT_CONTEXTMENU )
		{
			return;
		}
		if( event.Type == UIEVENT_MOUSEENTER )
		{
			return;
		}
		if( event.Type == UIEVENT_MOUSELEAVE )
		{
			return;
		}

		CLabelUI::DoEvent(event);
	}

	void CIPAddressUI::SetAttribute( LPCTSTR pstrName, LPCTSTR pstrValue )
	{
		if( _tcsicmp(pstrName, _T("native-background-color")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetNativeBackgroundColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("native-color")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) ) SetNativeColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("readonly")) == 0 ) {
			SetReadOnly(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else CLabelUI::SetAttribute(pstrName, pstrValue);
	}
}
