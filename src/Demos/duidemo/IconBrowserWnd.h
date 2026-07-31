#pragma once

#include <vector>

namespace DuiLib { struct IconEntry; }

// 图标浏览器：分组 + 搜索 + 可视区虚拟列表（只创建视口内控件）
class CIconBrowserWnd : public WindowImplBase
{
public:
	CIconBrowserWnd(const DuiLib::IconEntry* pEntries, int nCount, LPCTSTR pstrAttr, LPCTSTR pstrTitle);
	~CIconBrowserWnd();

	static void Open(HWND hParent, const DuiLib::IconEntry* pEntries, int nCount, LPCTSTR pstrAttr, LPCTSTR pstrTitle);

	virtual void OnFinalMessage(HWND hWnd);
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName() const;
	virtual void InitWindow();
	virtual void Notify(TNotifyUI& msg);
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);

private:
	enum { ROW_HEADER_H = 32, ROW_ICON_H = 68, ICONS_PER_ROW = 8, VIEW_BUFFER_PX = 200 };

	struct VirtRow
	{
		bool bHeader;
		CDuiString sHeader;
		std::vector<const DuiLib::IconEntry*> cells;
		int nHeight;
	};

	void RebuildData();
	void RefreshViewport(bool bForce = false);
	void UpdateTitle(LPCTSTR pstrExtra = NULL);
	bool CopyTextToClipboard(LPCTSTR pstrText);
	int GetViewportHeight() const;
	static CDuiString GetPrefix(LPCTSTR pstrName);
	static bool MatchFilter(LPCTSTR pstrName, LPCTSTR pstrFilter);
	CControlUI* CreateIconCell(const DuiLib::IconEntry* pEntry);
	CControlUI* CreateHeaderRow(LPCTSTR pstrText);
	CControlUI* CreateIconRow(const std::vector<const DuiLib::IconEntry*>& cells);
	CControlUI* CreateSpacer(int nHeight);

private:
	const DuiLib::IconEntry* m_pEntries;
	int m_nCount;
	CDuiString m_sAttr;
	CDuiString m_sTitle;
	CDuiString m_sFilter;

	CVerticalLayoutUI* m_pIconList;
	CLabelUI* m_pTitleLabel;
	CEditUI* m_pSearchEdit;

	std::vector<VirtRow> m_rows;
	std::vector<int> m_rowTops; // 每行顶部 y
	int m_nTotalHeight;
	int m_nMatched;

	int m_nVisFirst;
	int m_nVisLast;
	bool m_bRefreshing;
};
