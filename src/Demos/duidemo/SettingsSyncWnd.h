#pragma once

/// 铺满 Owner 的设置窗：测 WindowImplBase SyncOwnerMove + SyncOwnerSize
class CSettingsSyncWnd : public WindowImplBase
{
public:
	CSettingsSyncWnd();
	~CSettingsSyncWnd();

	static void Open(HWND hOwner);

	void OnFinalMessage(HWND hWnd) override;
	CDuiString GetSkinFile() override;
	LPCTSTR GetWindowClassName() const override;
	void InitWindow() override;
	void Notify(TNotifyUI& msg) override;

	DUI_DECLARE_MESSAGE_MAP()
	void OnClick(TNotifyUI& msg) override;

	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& /*bHandled*/) override
	{
		if( uMsg == WM_KEYDOWN && wParam == VK_ESCAPE ) {
			Close(0);
			return TRUE;
		}
		return FALSE;
	}
};
