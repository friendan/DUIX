#pragma once

// 浏览器壳测试窗：加载 browser.html（TabBar + 地址栏 + 侧栏）
class CBrowserWnd : public WindowImplBase
{
public:
	CBrowserWnd();
	~CBrowserWnd();

	static void Open(HWND hParent);

	virtual void OnFinalMessage(HWND hWnd);
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName() const;
	virtual void InitWindow();
	virtual void Notify(TNotifyUI& msg);

	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);

private:
	void AddNewTab(LPCTSTR pstrTitle = NULL);

private:
	CTabBarUI* m_pTabBar;
	CTabLayoutUI* m_pPages;
	int m_nNextTabId;
};
