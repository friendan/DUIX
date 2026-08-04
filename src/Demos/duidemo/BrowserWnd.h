#pragma once

// 浏览器壳：只继承 WindowImplBase；导航在 Notify 里处理，不依赖消息映射 this
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
	void AddNewTab(LPCTSTR pstrTitle = NULL, LPCTSTR pstrUrl = NULL);
	CWebBrowserUI* GetActiveBrowser() const;
	void NavigateAddressBar();
	void HandleNavCommand(LPCTSTR pstrName);
	void OnBrowserTitleChanged(CWebBrowserUI* pWeb, LPCTSTR title);
	void OnBrowserNavigated(CWebBrowserUI* pWeb, LPCTSTR url, bool success);
	bool OnUrlBoxNotify(void* param);

	class HostEvents : public CWebBrowserHostEvents
	{
	public:
		explicit HostEvents(CBrowserWnd* pOwner) : m_pOwner(pOwner) {}
		virtual void OnDocumentTitleChanged(CWebBrowserUI* pWeb, LPCTSTR title);
		virtual void OnNavigationCompleted(CWebBrowserUI* pWeb, LPCTSTR url, bool success);
	private:
		CBrowserWnd* m_pOwner;
	};

private:
	CTabBarUI* m_pTabBar;
	CTabLayoutUI* m_pPages;
	CEditUI* m_pUrlBox;
	CWebBrowserUI* m_pActiveBrowser; // 不依赖 TabLayout 下标
	int m_nNextTabId;
	HostEvents m_hostEvents;
};
