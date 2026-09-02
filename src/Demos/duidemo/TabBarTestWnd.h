#pragma once

// TabBar 专项测试窗：切换 / 关闭 / 锁定 / 溢出滚动 / 拖拽插入排序 / 拦截 / 动态增删
class CTabBarTestWnd : public WindowImplBase
{
public:
	CTabBarTestWnd();
	~CTabBarTestWnd();

	static void Open(HWND hParent);

	void OnFinalMessage(HWND hWnd) override;
	CDuiString GetSkinFile() override;
	LPCTSTR GetWindowClassName() const override;
	void InitWindow() override;
	void Notify(TNotifyUI& msg) override;

	DUI_DECLARE_MESSAGE_MAP()
	void OnClick(TNotifyUI& msg) override;

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
