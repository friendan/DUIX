#pragma once

/// 铺满 Owner 的设置窗：测 WindowImplBase SyncOwnerMove + SyncOwnerSize
class CSettingsSyncWnd : public WindowImplBase
{
public:
	CSettingsSyncWnd();
	~CSettingsSyncWnd();

	static void Open(HWND hOwner);

	virtual void OnFinalMessage(HWND hWnd);
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName() const;
	virtual void InitWindow();
	virtual void Notify(TNotifyUI& msg);

	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);

	virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& /*bHandled*/)
	{
		if( uMsg == WM_KEYDOWN && wParam == VK_ESCAPE ) {
			Close(0);
			return TRUE;
		}
		return FALSE;
	}
};
