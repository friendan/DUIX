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

	void OnFinalMessage(HWND hWnd) override;
	CDuiString GetSkinFile() override;
	LPCTSTR GetWindowClassName() const override;
	void InitWindow() override;
	void Notify(TNotifyUI& msg) override;
	LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

	DUI_DECLARE_MESSAGE_MAP()
	void OnClick(TNotifyUI& msg) override;

private:
	enum { ROW_HEADER_H = 32, ROW_ICON_H = 68, CELL_W = 120, VIEW_BUFFER_PX = 200 };

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
	void GetPageInfo(int& nCurPage, int& nTotalPages) const;
	int GetPageScrollStep() const;
	bool CopyTextToClipboard(LPCTSTR pstrText);
	int GetViewportHeight() const;
	int GetViewportWidth() const;
	int CalcIconsPerRow() const;
	bool SyncIconsPerRow(); // 列数变化时重建数据，返回是否变化
	// WM_SIZE 早于 PaintManager 布局；最大化/还原前先按客户区 SetPos 根节点
	void EnsureRootLaidOut();
	static CDuiString GetPrefix(LPCTSTR pstrName);
	static bool MatchFilter(LPCTSTR pstrName, LPCTSTR pstrFilter);
	CControlUI* CreateIconCell(const DuiLib::IconEntry* pEntry);
	CControlUI* CreateHeaderRow(LPCTSTR pstrText);
	CControlUI* CreateIconRow(const std::vector<const DuiLib::IconEntry*>& cells);
	CControlUI* CreateSpacer(int nHeight);

	void ShowIconContextMenu(CControlUI* pCell);
	void OpenExportForCell(CControlUI* pCell);
	void ApplyWallpaperForCell(CControlUI* pCell);
	bool ScrollIconListByKey(WPARAM vk);
	static CControlUI* FindIconCell(CControlUI* pFrom);

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
	int m_nIconsPerRow;

	int m_nVisFirst;
	int m_nVisLast;
	bool m_bRefreshing;
	int m_nTitlePage; // 标题里上次显示的页码，用于滚动时按需刷新
	CDuiString m_sTitleExtra; // 匹配数等附加文案

	CMenuWnd* m_pMenu;
	CControlUI* m_pCtxCell; // 右键菜单对应的图标格
};
