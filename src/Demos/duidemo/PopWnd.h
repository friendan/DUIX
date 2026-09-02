#pragma once

//////////////////////////////////////////////////////////////////////////
///

class CPopWnd : public WindowImplBase
{
public:
	CPopWnd(void);
	~CPopWnd(void);

public:
	void OnFinalMessage( HWND ) override;
	CDuiString GetSkinFile() override;
	LPCTSTR GetWindowClassName( void ) const override;
	void Notify( TNotifyUI &msg ) override;
	void InitWindow() override;

	DUI_DECLARE_MESSAGE_MAP()
	void OnClick(TNotifyUI& msg) override;
	virtual void OnSelectChanged( TNotifyUI &msg );
	virtual void OnItemSelect( TNotifyUI &msg );

	LRESULT OnSysCommand( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled ) override;
	LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& /*bHandled*/) override
	{
		if (uMsg == WM_KEYDOWN)
		{
			switch (wParam)
			{
			case VK_RETURN:
			case VK_ESCAPE:
				return ResponseDefaultKeyEvent(wParam);
			default:
				break;
			}
		}
		return FALSE;
	}
private:
	CButtonUI* m_pCloseBtn;
	CButtonUI* m_pMaxBtn;
	CButtonUI* m_pRestoreBtn;
	CButtonUI* m_pMinBtn;
	CButtonUI* m_pMenuBtn;
};
