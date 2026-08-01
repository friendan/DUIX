#pragma once

// 布局测试窗：加载 layouttest.htm，改皮肤即可验证窗口/布局
class CLayoutTestWnd : public WindowImplBase
{
public:
	CLayoutTestWnd();
	~CLayoutTestWnd();

	static void Open(HWND hParent);

	virtual void OnFinalMessage(HWND hWnd);
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName() const;
	virtual void InitWindow();
	virtual void Notify(TNotifyUI& msg);

	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);
};
