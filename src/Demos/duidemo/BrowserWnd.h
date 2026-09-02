#pragma once

#include <vector>

// 浏览器壳：只继承 WindowImplBase；导航在 Notify 里处理，不依赖消息映射 this
class CBrowserWnd : public WindowImplBase
{
public:
	CBrowserWnd();
	~CBrowserWnd();

	static void Open(HWND hParent);

	void OnFinalMessage(HWND hWnd) override;
	CDuiString GetSkinFile() override;
	LPCTSTR GetWindowClassName() const override;
	void InitWindow() override;
	void Notify(TNotifyUI& msg) override;
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	DUI_DECLARE_MESSAGE_MAP()
	void OnClick(TNotifyUI& msg) override;

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
		void OnDocumentTitleChanged(CWebBrowserUI* pWeb, LPCTSTR title) override;
		void OnNavigationStarting(CWebBrowserUI* pWeb, LPCTSTR url, bool* pCancel) override;
		void OnNavigationCompleted(CWebBrowserUI* pWeb, LPCTSTR url, bool success) override;
		void OnFaviconChanged(CWebBrowserUI* pWeb, const BYTE* pData, DWORD dwSize) override;
		void OnHistoryChanged(CWebBrowserUI* pWeb) override;
		void OnNewWindowRequested(CWebBrowserUI* pWeb, LPCTSTR url, bool* pHandled) override;
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
