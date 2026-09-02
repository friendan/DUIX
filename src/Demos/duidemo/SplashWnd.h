#pragma once

//////////////////////////////////////////////////////////////////////////
///

class CSplashWnd : public WindowImplBase
{
public:
	static int MessageBox(HWND hParent)
	{
		CSplashWnd* pWnd = new CSplashWnd();
		pWnd->Create(hParent, _T("msgwnd"), WS_POPUP | WS_CLIPCHILDREN, WS_EX_TOOLWINDOW);
		pWnd->CenterWindow();
		return pWnd->ShowModal();
	}

public:
	CSplashWnd(void);
	~CSplashWnd(void);

public:
	void OnFinalMessage( HWND ) override;
	CDuiString GetSkinFile() override;
	LPCTSTR GetWindowClassName( void ) const override;
	void InitWindow() override;

	DUI_DECLARE_MESSAGE_MAP()
	void OnClick(TNotifyUI& msg) override;

	LRESULT OnSysCommand( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled ) override;
	LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
};
