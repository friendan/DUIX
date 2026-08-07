#pragma once

#include <vector>

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
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);

private:
	void AddNewTab(LPCTSTR pstrTitle = NULL, LPCTSTR pstrUrl = NULL);
	void ApplyPlaceholderTabIcon(CTabButtonUI* pTab);
	void ApplyLoadingTabIcon(CTabButtonUI* pTab);
	void ScheduleDelayedHomeNavigate(CWebBrowserUI* pBrowser);
	void CancelDelayedNavigates();
	void OnDelayedHomeNavigate(UINT_PTR nTimerId);
	/// 校验缓存；关标签后可能已销毁，勿直接用裸指针
	CWebBrowserUI* GetActiveBrowser();
	void NavigateAddressBar();
	CDuiString ReadAddressBarText() const;
	CDuiString ResolveNavigateInput(LPCTSTR pstrInput) const;
	static bool LooksLikeUrl(LPCTSTR pstrInput);
	static CDuiString UrlEncodeUtf8(LPCTSTR pstr);
	static CDuiString BuildSearchUrl(LPCTSTR pstrQuery);
	void HandleNavCommand(LPCTSTR pstrName);
	bool HandleThemeCommand(LPCTSTR pstrName);
	void ShowBrowserMenu();
	void HandleMenuCommand(LPCTSTR pstrName);
	void SetToolbarLoading(bool bLoading);
	void SyncToolbarLoadingFromActiveTab();
	void OnBrowserTitleChanged(CWebBrowserUI* pWeb, LPCTSTR title);
	void OnBrowserNavigated(CWebBrowserUI* pWeb, LPCTSTR url, bool success);
	void OnBrowserNavigationStarting(CWebBrowserUI* pWeb, LPCTSTR url);
	void OnBrowserFaviconChanged(CWebBrowserUI* pWeb, const BYTE* pData, DWORD dwSize);
	void OnBrowserHistoryChanged(CWebBrowserUI* pWeb);
	void OnBrowserNewWindow(CWebBrowserUI* pWeb, LPCTSTR url, bool* pHandled);
	void UpdateNavButtons();
	void OnTabsChanged();
	CTabButtonUI* FindTabForBrowser(CWebBrowserUI* pWeb) const;
	bool OnUrlBoxNotify(void* param);
	bool IsBrowserAlive(CWebBrowserUI* pWeb) const;
	/// 深色标签栏上过暗/几乎透明的过渡 favicon 不可用（保持占位图）
	static bool IsFaviconReadableOnDarkTab(const BYTE* pData, DWORD dwSize);

	class HostEvents : public CWebBrowserHostEvents
	{
	public:
		explicit HostEvents(CBrowserWnd* pOwner) : m_pOwner(pOwner) {}
		virtual void OnDocumentTitleChanged(CWebBrowserUI* pWeb, LPCTSTR title);
		virtual void OnNavigationStarting(CWebBrowserUI* pWeb, LPCTSTR url, bool* pCancel);
		virtual void OnNavigationCompleted(CWebBrowserUI* pWeb, LPCTSTR url, bool success);
		virtual void OnFaviconChanged(CWebBrowserUI* pWeb, const BYTE* pData, DWORD dwSize);
		virtual void OnHistoryChanged(CWebBrowserUI* pWeb);
		virtual void OnNewWindowRequested(CWebBrowserUI* pWeb, LPCTSTR url, bool* pHandled);
	private:
		CBrowserWnd* m_pOwner;
	};

	struct DelayedNav
	{
		UINT_PTR nTimerId;
		CWebBrowserUI* pBrowser;
	};

private:
	CTabBarUI* m_pTabBar;
	CTabLayoutUI* m_pPages;
	CEditUI* m_pUrlBox;
	CWebBrowserUI* m_pActiveBrowser; // 不依赖 TabLayout 下标
	CMenuWnd* m_pMenu;
	int m_nNextTabId;
	UINT_PTR m_nNextDelayTimerId;
	bool m_bToolbarLoading;
	std::vector<DelayedNav> m_aDelayedNav;
	HostEvents m_hostEvents;
};
