#pragma once

// 布局测试窗：加载 layouttest.htm，改皮肤即可验证窗口/布局
class CLayoutTestWnd : public WindowImplBase
{
public:
	CLayoutTestWnd();
	~CLayoutTestWnd();

	static void Open(HWND hParent);

	void OnFinalMessage(HWND hWnd) override;
	CDuiString GetSkinFile() override;
	LPCTSTR GetWindowClassName() const override;
	void InitWindow() override;
	void Notify(TNotifyUI& msg) override;

	DUI_DECLARE_MESSAGE_MAP()
	void OnClick(TNotifyUI& msg) override;
};
