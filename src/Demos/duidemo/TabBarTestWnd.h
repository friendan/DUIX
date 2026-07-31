#pragma once

// TabBar 专项测试窗：切换 / 关闭 / 锁定 / 溢出滚动 / 拖拽插入排序 / 拦截 / 动态增删
class CTabBarTestWnd : public WindowImplBase
{
public:
	CTabBarTestWnd();
	~CTabBarTestWnd();

	static void Open(HWND hParent);

	virtual void OnFinalMessage(HWND hWnd);
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName() const;
	virtual void InitWindow();
	virtual void Notify(TNotifyUI& msg);

	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);

private:
	void SetStatus(LPCTSTR pstrText);
	void AddNewTab();
	void RemoveActiveTab();
	void ToggleLockActive();
	void ApplyWidthModeFromCheck();
	CLabelUI* CreatePageLabel(LPCTSTR pstrText);
	bool IsCheckSelected(LPCTSTR pstrName) const;

private:
	CTabBarUI* m_pTabBar;
	CTabLayoutUI* m_pPages;
	CLabelUI* m_pStatus;
	int m_nNextTabId;
};
